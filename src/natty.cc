  /*
   * libjingle
   * Copyright 2012, Google Inc.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions are met:
   *
   *  1. Redistributions of source code must retain the above copyright notice,
   *     this list of conditions and the following disclaimer.
   *  2. Redistributions in binary form must reproduce the above copyright notice,
   *     this list of conditions and the following disclaimer in the documentation
   *     and/or other materials provided with the distribution.
   *  3. The name of the author may not be used to endorse or promote products
   *     derived from this software without specific prior written permission.
   *
   * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
   * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
   * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   */

#include "natty.h"

#include <utility>

#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/base/common.h"
#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "talk/media/devices/devicemanager.h"

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

  class DummySetSessionDescriptionObserver
      : public webrtc::SetSessionDescriptionObserver {
   public:
    static DummySetSessionDescriptionObserver* Create() {
      return
          new talk_base::RefCountedObject<DummySetSessionDescriptionObserver>();
    }
    virtual void OnSuccess() {
      LOG(INFO) << __FUNCTION__;
    }
    virtual void OnFailure(const std::string& error) {
      LOG(INFO) << __FUNCTION__ << " " << error;
    }

   protected:
    DummySetSessionDescriptionObserver() {}
    ~DummySetSessionDescriptionObserver() {}
  };

  Natty::Natty(PeerConnectionClient* client, talk_base::Thread* thread)
    : peer_id_(-1),
      thread(talk_base::Thread::Current()),
      client_(client) {
    client_->RegisterObserver(this);
  }

  Natty::~Natty() {
    ASSERT(peer_connection_.get() == NULL);
  }

  bool Natty::connection_active() const {
    return peer_connection_.get() != NULL;
  }

  PeerConnectionClient* Natty::getClient() {
    return client_;
  }

  void Natty::SetupSocketServer() {
    talk_base::InitializeSSL();
    Init("127.0.0.1", 8888);
    InitializePeerConnection();
  }

  void Natty::Shutdown() {
    client_->SignOut();
    DeletePeerConnection();
    talk_base::CleanupSSL();
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


bool Natty::InitializePeerConnection() {

  IceServers servers;
  IceServer server;

  ASSERT(peer_connection_factory_.get() == NULL);
  ASSERT(peer_connection_.get() == NULL);

  peer_connection_factory_  = webrtc::CreatePeerConnectionFactory();

  if (!peer_connection_factory_.get()) {
    printf("Failed to initialize peer connection factory\n");
    DeletePeerConnection();
    return false;
  }

  server.uri = GetPeerConnectionString();
  servers.push_back(server);

  peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers, NULL, NULL, this);
  if (!peer_connection_.get()) {
    printf("create peer connection failed\n");
    DeletePeerConnection();
  }
  const webrtc::SessionDescriptionInterface* session = peer_connection_->local_description();

  printf("successfully created peer connection\n");
  /* create offer */
  peer_connection_->CreateOffer(this, NULL);
  return peer_connection_.get() != NULL;
}

void Natty::DeletePeerConnection() {
  printf("Deleting peer connection\n");
  peer_connection_ = NULL;
  peer_connection_factory_ = NULL;
  peer_id_ = -1;
}

//
// PeerConnectionObserver implementation.
//

void Natty::OnError() {
  LOG(LS_ERROR) << __FUNCTION__;
}

void Natty::OnAddStream(webrtc::MediaStreamInterface* stream) {

}

void Natty::OnRemoveStream(webrtc::MediaStreamInterface* stream) {
  LOG(INFO) << __FUNCTION__ << " " << stream->label();
  stream->AddRef();
}

void Natty::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
  Json::StyledWriter writer;
  Json::Value jmessage;

  jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
  jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
  std::string sdp;
  if (!candidate->ToString(&sdp)) {
    LOG(LS_ERROR) << "Failed to serialize candidate";
    return;
  }
  jmessage[kCandidateSdpName] = sdp;
  SendMessage(writer.write(jmessage));
}

//
// PeerConnectionClientObserver implementation.
//

void Natty::OnSignedIn() {
  
}

void Natty::OnDisconnected() {

  DeletePeerConnection();

}

void Natty::OnPeerConnected(int id, const std::string& name) {

}

void Natty::OnPeerDisconnected(int id) {
  if (id == peer_id_) {
    //DEBUG("Our peer is gone.\n");
  }
}

void Natty::OnMessageFromPeer(int peer_id, const std::string& message) {
  ASSERT(peer_id_ == peer_id || peer_id_ == -1);
  ASSERT(!message.empty());

  if (!peer_connection_.get()) {
    ASSERT(peer_id_ == -1);
    peer_id_ = peer_id;

    if (!InitializePeerConnection()) {
      LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
      client_->SignOut();
      return;
    }
  } else if (peer_id != peer_id_) {
    ASSERT(peer_id_ != -1);
    LOG(WARNING) << "Received a message from unknown peer while already in a "
                    "conversation with a different peer.";
    return;
  }

  Json::Reader reader;
  Json::Value jmessage;
  if (!reader.parse(message, jmessage)) {
    LOG(WARNING) << "Received unknown message. " << message;
    return;
  }
  std::string type;
  std::string json_object;

  GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);
  if (!type.empty()) {
    std::string sdp;
    if (!GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName, &sdp)) {
      LOG(WARNING) << "Can't parse received session description message.";
      return;
    }
    webrtc::SessionDescriptionInterface* session_description(
        webrtc::CreateSessionDescription(type, sdp));
    if (!session_description) {
      LOG(WARNING) << "Can't parse received session description message.";
      return;
    }
    LOG(INFO) << " Received session description :" << message;
    peer_connection_->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create(), session_description);
    if (session_description->type() ==
        webrtc::SessionDescriptionInterface::kOffer) {
      peer_connection_->CreateAnswer(this, NULL);
    }
    return;
  } else {
    std::string sdp_mid;
    int sdp_mlineindex = 0;
    std::string sdp;
    if (!GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid) ||
        !GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName,
                              &sdp_mlineindex) ||
        !GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp)) {
      LOG(WARNING) << "Can't parse received message.";
      return;
    }
    talk_base::scoped_ptr<webrtc::IceCandidateInterface> candidate(
        webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp));
    if (!candidate.get()) {
      LOG(WARNING) << "Can't parse received candidate message.";
      return;
    }
    if (!peer_connection_->AddIceCandidate(candidate.get())) {
      LOG(WARNING) << "Failed to apply the received candidate";
      return;
    }
    LOG(INFO) << " Received candidate :" << message;
    return;
  }
}

void Natty::OnMessageSent(int err) {

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

void Natty::Init(const std::string& server, int port) {
  if (client_->is_connected())
    return;
  server_ = server;
  client_->Connect(server, port, GetPeerName());
}

void Natty::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  peer_connection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create(), desc);
  Json::StyledWriter writer;
  Json::Value jmessage;
  jmessage[kSessionDescriptionTypeName] = desc->type();
  std::string sdp;
  desc->ToString(&sdp);
  jmessage[kSessionDescriptionSdpName] = sdp;
  SendMessage(writer.write(jmessage));
}

void Natty::OnFailure(const std::string& error) {
    LOG(LERROR) << error;
}

void Natty::SendMessage(const std::string& json_object) {
  std::string* msg = new std::string(json_object);
}
