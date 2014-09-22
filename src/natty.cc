/**
 * Copyright (C) 2014 Lantern
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "natty.h"

#include <utility>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>

#include "talk/app/webrtc/videosourceinterface.h"
#include "webrtc/base/common.h"
#include "talk/p2p/base/sessiondescription.h"
#include "talk/app/webrtc/test/fakeconstraints.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/portallocatorfactory.h"
#include "talk/session/media/mediasession.h"
#include "talk/p2p/base/constants.h"
#include "talk/app/webrtc/datachannelinterface.h"
#include "webrtc/base/json.h"
#include "webrtc/base/logging.h"
#include "talk/media/devices/devicemanager.h"

using namespace std;

#define DEBUG(fmt, ...) printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__);

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

typedef webrtc::PeerConnectionInterface::IceServers IceServers;
typedef webrtc::PeerConnectionInterface::IceServer IceServer;;

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

void Natty::InputStream::read(Natty* natty) {
  //std::ifstream filein("cand-input.txt");
  while (getline(std::cin, input)) {
    /* need to remove new lines or the SDP won't be valid */
    if (input == "exit") {
      natty->Shutdown();
      break;
    }

    if (input.empty()) {
      /* terminate input on empty line */
      //std::cout << "\n";
      continue;
    }
    natty->ReadMessage(input);  
  }

}

string Natty::InputStream::build() const {
  string str = ss.str();
  str.erase(
      std::remove(str.begin(), str.end(), '\n'), str.end()
      ); 
  return str;
}

class NattySessionObserver
: public webrtc::SetSessionDescriptionObserver {
  public:
    static NattySessionObserver* Create() {
      return
        new rtc::RefCountedObject<NattySessionObserver>();
    }
    virtual void OnSuccess() {
      LOG(INFO) << __FUNCTION__;
    }
    virtual void OnFailure(const std::string& error) {
      LOG(INFO) << __FUNCTION__ << " " << error;
    }

  protected:
    NattySessionObserver() {}
    ~NattySessionObserver() {}
};

bool NattySocket::Wait(int cms, bool process_io) {
  return rtc::PhysicalSocketServer::Wait(-1,
        process_io);
}

MessageClient::~MessageClient() {
  delete socket_;
}

void MessageClient::OnMessage(rtc::Message *pmsg) {
  NattyMessage* msg = static_cast<NattyMessage*>(pmsg->pdata);
  
  delete msg;
}


Natty::Natty(PeerConnectionClient* client,
    rtc::Thread* thread
    )
: peer_id_(-1),
  highestPrioritySeen(0),
  thread_(thread) {
  }

Natty::~Natty() {
  if (outfile != NULL) {
    outfile.close();
  }
  ASSERT(peer_connection_.get() == NULL);
}

bool Natty::connection_active() const {
  return peer_connection_.get() != NULL;
}

PeerConnectionClient* Natty::GetClient() {
  return client_;
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

void Natty::AddStreams() {
  const char kStreamLabel[] = "stream_label";
  const uint16 kDefaultServerPort = 8888;
  const char kAudioLabel[] = "audio_label";
  if (active_streams_.find(kStreamLabel) != active_streams_.end())
    return;  // Already added.

  rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
      peer_connection_factory_->CreateAudioTrack(
          kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL)));
  stream =
      peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);

  stream->AddTrack(audio_track);
  if (!peer_connection_->AddStream(stream, NULL)) {
    LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
  }
  typedef std::pair<std::string,
                    rtc::scoped_refptr<webrtc::MediaStreamInterface> >
      MediaStreamPair;
  active_streams_.insert(MediaStreamPair(stream->label(), stream));
}


bool Natty::InitializePeerConnection() {

  IceServers servers;
  IceServer server;
  webrtc::FakeConstraints constraints;
  constraints.SetAllowRtpDataChannels();

  ASSERT(peer_connection_factory_.get() == NULL);
  ASSERT(peer_connection_.get() == NULL);

  peer_connection_factory_  = webrtc::CreatePeerConnectionFactory();

  if (!peer_connection_factory_.get()) {
    LOG(INFO) << "Failed to initialize peer connection factory";
    Shutdown();
    return false;
  }
  LOG(INFO) << "Created peer connection factory";

  server.uri = GetPeerConnectionString();
  servers.push_back(server);

  peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers, &constraints, allocator_factory_.get(), NULL, this);

  if (!peer_connection_.get()) {
    LOG(INFO) << "Create peer connection failed";
    Shutdown();
  }
  AddStreams();
  allocator = peer_connection_->GetAllocator();
  ASSERT(allocator != NULL);

  LOG(INFO) << "Created peer connection";

  return peer_connection_.get() != NULL;
}

void Natty::Shutdown() {
  LOG(INFO) << "Deleting peer connection";
  peer_connection_ = NULL;
  peer_connection_factory_ = NULL;
  peer_id_ = -1;
  //active_streams_.clear();
  rtc::CleanupSSL();
  thread_->Stop();
}

//
// PeerConnectionObserver implementation.
//

void Natty::OnError() {
  LOG(INFO) << __FUNCTION__;
}

void Natty::OnAddStream(webrtc::MediaStreamInterface* stream) {
  LOG(INFO) << "Successfully added stream";
}

/* the answerer removes his media stream before disconnecting
 * this is triggered when that happens and means we
 * were able to communicate 
 */
void Natty::OnRemoveStream(webrtc::MediaStreamInterface* stream) {
  PickFinalCandidate();
}

void Natty::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
  if (new_state == webrtc::PeerConnectionInterface::SignalingState::kClosed || 
      new_state == webrtc::PeerConnectionInterface::SignalingState::kHaveRemotePrAnswer
      ) {
    PickFinalCandidate();
  }
}

void Natty::OnStateChange(
    webrtc::PeerConnectionObserver::StateType state_changed) {
}

void Natty::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
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

void Natty::OnRenegotiationNeeded() {
  LOG(INFO) << "Renegotiation needed";
}

void Natty::ReadMessage(const std::string& message) {
  Json::Reader reader;
  Json::Value jmessage;
  std::string type;
  std::string json_object;

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
    session_description = webrtc::CreateSessionDescription(type, sdp);
    if (!session_description) {
      LOG(INFO) << "Can't parse SDP message";
      return;
    }
    LOG(INFO) << "Received session description " << message << " sending answer back";

    peer_connection_->SetRemoteDescription(
        NattySessionObserver::Create(), session_description);
    sleep(1);
    if (session_description->type() ==
        webrtc::SessionDescriptionInterface::kOffer) {
      peer_connection_->CreateAnswer(this, NULL);
      sleep(1);
    }
    return;
  }
  else {
    std::string sdp_mid;
    sdp_mlineindex = 0;
    std::string sdp;
    if (!GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid) ||
        !GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName,
          &sdp_mlineindex) ||
        !GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp)) {
      LOG(INFO) << "Can't parse received message";
      return;
    }
    rtc::scoped_ptr<webrtc::IceCandidateInterface> candidate(
        webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp));
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
    PickFinalCandidate();
    return;
  }
};

void Natty::Output5Tuple(const cricket::Candidate *cand) {
   const char kStreamLabel[] = "stream_label";
   Json::FastWriter writer;
   Json::Value jmessage;
   jmessage["remote"] = cand->address().ToString();
   jmessage["local"] = cand->related_address().ToString();
   jmessage["type"] = cand->protocol();
   outfile << writer.write(jmessage);
   outfile.flush();
   peer_connection_->RemoveStream(stream);
   Shutdown();
}

void Natty::PickFinalCandidate() {
  const webrtc::IceCandidateCollection* candidates = 
      session_description->candidates(sdp_mlineindex);

  for (size_t i = 0; i < candidates->count(); ++i) {
    const cricket::Candidate *cand = &candidates->at(i)->candidate();
    if (cand->type() == "stun") {
      Output5Tuple(cand);
      return;
    } 
  }
}

void Natty::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  LOG(INFO) << "New data channel created " << data_channel;
}

void Natty::OnIceComplete() {
  LOG(INFO) << "ICE finished gathering candidates!";
}

// PeerConnectionObserver implementation.
void Natty::OnServerConnectionFailure() {

}

std::string GetPeerName() {
  char computer_name[256];
  if (gethostname(computer_name, ARRAY_SIZE(computer_name)) != 0)
    strcpy(computer_name, "host");
  std::string ret(GetEnvVarOrDefault("USERNAME", "user"));
  ret += '@';
  ret += computer_name;
  return ret;
}   

void Natty::setMode(Natty::Mode m) {
  mode = m;
}

void Natty::Init(bool offer) {
  InitializePeerConnection();
  if (offer) {
    Natty::setMode(Natty::OFFER);
    webrtc::FakeConstraints constraints;
    constraints.SetAllowRtpDataChannels();
    peer_connection_->CreateOffer(this, &constraints);
    sleep(1);
  }
}

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

void Natty::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
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

void Natty::OnFailure(const std::string& error) {
  LOG(LERROR) << error;
}

void Natty::SendMessage(const std::string& json_object) {

}

