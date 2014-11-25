#include "natty.h"

#include <utility>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>

#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/app/webrtc/jsep.h"
#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "webrtc/base/common.h"
#include "webrtc/p2p/base/sessiondescription.h"
#include "talk/app/webrtc/test/fakeconstraints.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/portallocatorfactory.h"
#include "talk/session/media/mediasession.h"
#include "webrtc/p2p/base/constants.h"
#include "talk/examples/peerconnection/client/defaults.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/app/webrtc/datachannelinterface.h"
#include "webrtc/base/json.h"
#include "webrtc/base/logging.h"
#include "talk/media/devices/devicemanager.h"
#include <string.h>

using namespace std;
using namespace webrtc;

#define DEBUG(fmt, ...) printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__);

void Natty::InputStream::read(Natty* natty) {
  while (getline(std::cin, input)) 
    natty->ReadMessage(input);  
}

Natty::Natty(rtc::Thread* thread) : thread_(thread) {
  
}

Natty::~Natty() {
  if (outfile) {
    outfile.close();
  }
  ASSERT(peer_connection_.get() == NULL);
}

bool Natty::connection_active() const {
  return peer_connection_.get() != NULL;
}

std::string GetEnvVarOrDefault(const char* env_var_name,
    const char* default_value) {
  std::string value;
  const char* env_var = getenv(env_var_name);
  if (env_var)
    value = env_var;

  if (value.empty())
    value = default_value;

  return value;
}

std::string GetPeerConnectionString() {
  return GetEnvVarOrDefault("WEBRTC_CONNECT", "stun:stun.l.google.com:19302");
}

void Natty::AddStunServers(IceServers *servers) {
  if (!strchr(stun_servers_.c_str(), ',')) {
    /* only one STUN server */
    IceServer server;
    server.uri = stun_servers_;
    servers->push_back(server);
  } else {
    std::istringstream iss(stun_servers_);
    std::string serv;
    while(getline(iss, serv, ',')) {
      IceServer server;
      server.uri = serv;
      servers->push_back(server);
    }
  }
}

bool Natty::InitializePeerConnection() {
  IceServers servers;
  FakeConstraints constraints;
  webrtc::InternalDataChannelInit dci;
  const string& dc_name = "datachannel";
  // "enable" DTLS-SRTP initially just to get the data
  // channel initialized
  // channel.cc ShouldSetupDtlsSrtp() returns false now
  // so this will never actually happen
  constraints.AddOptional(
      webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, false);
  constraints.AddOptional(
      webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, false);
  constraints.AddOptional(
      webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, true);

  ASSERT(peer_connection_factory_.get() == NULL);
  ASSERT(peer_connection_.get() == NULL);

  peer_connection_factory_  = CreatePeerConnectionFactory();

  if (!peer_connection_factory_.get()) {
    LOG(INFO) << "Failed to initialize peer connection factory";
    Shutdown();
    return false;
  }
  LOG(INFO) << "Created peer connection factory";

  AddStunServers(&servers);

  /* Creating a peer connection object is when
   * we start to generate ICE candidates
   */
  peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers, &constraints, NULL, NULL, this);
  dci.reliable = false;
  data_channel_ = peer_connection_->CreateDataChannel(dc_name, &dci);

  if (!peer_connection_.get()) {
    LOG(INFO) << "Create peer connection failed";
    Shutdown();
  }

  LOG(INFO) << "Created peer connection";
  return peer_connection_.get() != NULL;
}

void Natty::Shutdown() {
  LOG(INFO) << "Deleting peer connection";
  if (data_channel_) { 
    data_channel_->Close();
  }
  if (peer_connection_) {
    peer_connection_->Close();
    peer_connection_ = NULL;
  }
  peer_connection_factory_ = NULL;
  rtc::CleanupSSL();
  outfile.close();
}

//
// PeerConnectionObserver implementation.
//

void Natty::OnError() {
  /* should we shut down here? */
  LOG(INFO) << "Peer connection error encountered";
  LOG(INFO) << __FUNCTION__;
}

void Natty::OnAddStream(MediaStreamInterface* stream) {
  LOG(INFO) << "Successfully added stream";
}

/* the answerer removes his media stream before disconnecting
 * this is triggered when that happens and means we
 * were able to communicate 
 */
void Natty::OnRemoveStream(MediaStreamInterface* stream) {
  LOG(INFO) << "Successfully removed stream";
}

void Natty::OnSignalingChange(PeerConnectionInterface::SignalingState new_state) {
  LOG(INFO) << "Signaling state change";
}

void Natty::OnStateChange(
    PeerConnectionObserver::StateType state_changed) {
  LOG(INFO) << "Peer Connection state change " << state_changed;
}

/* Once an ice candidate have been found PeerConnection will call the
 * observer function OnIceCandidate. The candidate is serialized
 * to be sent to the remote peer.
 */
void Natty::OnIceCandidate(const IceCandidateInterface* candidate) {
  Json::FastWriter writer;
  Json::Value jmessage;

  jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
  jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
  std::string sdp;
  if (!candidate->ToString(&sdp)) {
    LOG(LS_ERROR) << "Failed to serialize candidate";
    return;
  }

  jmessage[kCandidateSdpName] = sdp;
  outfile << writer.write(jmessage);
  outfile.flush();
}

void Natty::OnIceConnectionChange(IceConnState new_state) {
  LOG(INFO) << "New connection state " << new_state;
}

void Natty::OnRenegotiationNeeded() {
  LOG(INFO) << "Renegotiation needed";
}

void Natty::ProcessIceCandidateMsg(const std::string& message, 
                        Json::Value& jmessage) {
  std::string sdp_mid;
  int sdp_mlineindex = 0;
  std::string sdp;
  if (!GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid) ||
      !GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName,
                            &sdp_mlineindex) ||
      !GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp)) {
    LOG(INFO) << "Can't parse received message";
    return;
  }
  rtc::scoped_ptr<IceCandidateInterface> candidate(
      CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp));
  LOG(INFO) << "Remote candidate information";

  if (!candidate.get()) {
    LOG(WARNING) << "Can't parse received candidate message.";
    return;
  }
  if (!peer_connection_->AddIceCandidate(candidate.get())) {
    LOG(WARNING) << "Failed to apply the received candidate";
    return;
  }
  LOG(INFO) << candidate.get()->candidate().ToString();
  LOG(INFO) << " Received candidate :" << message;
  return;

}

/* New natty JSON arrived on stdin
 * if type is defined, we have an SDP message
 * otherwise, it's a remote ICE candidate 
 */
void Natty::ReadMessage(const std::string& message) {
  Json::Reader reader;
  Json::Value jmessage;
  std::string type;

  if (!reader.parse(message, jmessage)) {
    LOG(INFO) << "Received an unknown message.";
    return;
  }
  GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);

  if (!type.empty()) {
    std::string sdp;
    if (!GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName, &sdp)) {
      LOG(INFO) << "Can't parse received session description message.";
      return;
    }
    session_description = CreateSessionDescription(type, sdp);
    if (!session_description) {
      LOG(INFO) << "Can't parse SDP message";
      return;
    }
    LOG(INFO) << "Received session description " << message << " sending answer back";

    peer_connection_->SetRemoteDescription(
        NattySessionObserver::Create(), session_description);
    if (session_description->type() ==
        webrtc::SessionDescriptionInterface::kOffer) {
      peer_connection_->CreateAnswer(this, NULL);
      LOG(INFO) << "signaling state " << peer_connection_->signaling_state();
    }
    return;
  }
  else {
    /* ICE candidate message processing */
    ProcessIceCandidateMsg(message, jmessage);  
  }
};

/* Used when ICE has checked all candidate pairs
 * and failed to find a connection for at least one
 */
void Natty::OnFailure(const std::string& msg) {
  Json::FastWriter writer;
  Json::Value jmessage;
  jmessage["type"] = "error";
  jmessage["message"] = msg;
  outfile << writer.write(jmessage);
  outfile.flush();
  /* close peer connection session */
  Shutdown();
}

void Natty::OnDataChannel(DataChannelInterface* data_channel) {
  LOG(INFO) << "New data channel created " << data_channel;
}

void Natty::OnIceComplete() {
  LOG(INFO) << "ICE finished gathering candidates!";
}

// PeerConnectionObserver implementation.
void Natty::OnServerConnectionFailure() {

}

void Natty::Init(bool offer, const char* out, const char *stuns) {

  if (!strlen(stuns)) {
    LOG(LERROR) << "Can't specify invalid/empty STUN server";
    std::exit(EXIT_FAILURE);
  }

  LOG(INFO) << "STUN server is " << stuns;

  /* OpenDumpFile checks if stdout should be redirected to an outfile */
  OpenDumpFile(out);

  InitializePeerConnection();

  stun_servers_ = stuns;

  if (offer) {
    peer_connection_->CreateOffer(this, NULL);
  }

}

/* read just opens an input stream for standard input */
void Natty::ProcessInput() {
  Natty::InputStream is;
  is.read(this);

}

/* need to update this to accept stdout when the stdout
 * option is blank 
 *
 */
void Natty::OpenDumpFile(const std::string& filename) {
  if (!filename.empty()) {
    outfile.open(filename.c_str());
  }
  else {
    outfile.copyfmt(std::cout);
    outfile.clear(std::cout.rdstate());
    outfile.basic_ios<char>::rdbuf(std::cout.rdbuf());
  }
}

/* Jsep CreateOffer and CreateAnswer on success callback
 * 
 * the generated blob of SDP data contains session information
 * and configuration
 * this is called after createOffer and createAnswer
 */
void Natty::OnSuccess(SessionDescriptionInterface* desc) {
  LOG(INFO) << "Setting local description";
  peer_connection_->SetLocalDescription(
      NattySessionObserver::Create(), desc);
  Json::FastWriter writer;
  Json::Value jmessage;
  jmessage[kSessionDescriptionTypeName] = desc->type();

  std::string sdp;
  desc->ToString(&sdp);
  jmessage[kSessionDescriptionSdpName] = sdp;
  outfile << writer.write(jmessage); 
  outfile.flush();
}

