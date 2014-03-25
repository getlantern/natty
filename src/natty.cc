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

#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/base/common.h"
#include "talk/app/webrtc/datachannelinterface.h"
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

  Natty::Natty(PeerConnectionClient* client, talk_base::Thread* thread,
      const std::string& server, int port

      )
    : peer_id_(-1),
      thread(talk_base::Thread::Current()),
      server_(server),
      port_(port),
      client_(client) {
    client_->RegisterObserver(this);
  }

  Natty::~Natty() {
    ASSERT(peer_connection_.get() == NULL);
  }

  bool Natty::connection_active() const {
    return peer_connection_.get() != NULL;
  }

  PeerConnectionClient* Natty::GetClient() {
    return client_;
  }


  void Natty::SetupSocketServer() {
    std::cout << "Setting up socket server";
    talk_base::InitializeSSL();
    Init();
  }

  void Natty::Shutdown() {
    talk_base::CleanupSSL();
    thread->Quit();
    thread->set_socketserver(NULL);
  }

void Natty::ConnectToPeer(int peer_id) {
  ASSERT(peer_id_ == -1);
  ASSERT(peer_id != -1);

  if (InitializePeerConnection()) {
    peer_id_ = peer_id;
    peer_connection_->CreateOffer(this, NULL);
  }
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
  //return GetEnvVarOrDefault("WEBRTC_CONNECT", "stun:stun.l.google.com:19302");
  return GetEnvVarOrDefault("WEBRTC_CONNECT", "stun:stun3.l.google.com:19302");
}

void Natty::SetLocalDescription() {
  std::string sdp;
  const webrtc::SessionDescriptionInterface* session = peer_connection_->local_description();
  std::cout << sdp;
}


bool Natty::InitializePeerConnection() {

  IceServers servers;
  IceServer server;

  ASSERT(peer_connection_factory_.get() == NULL);
  ASSERT(peer_connection_.get() == NULL);

  peer_connection_factory_  = webrtc::CreatePeerConnectionFactory();

  if (!peer_connection_factory_.get()) {
    LOG(INFO) << "Failed to initialize peer connection factory";
    DeletePeerConnection();
    return false;
  }
  LOG(INFO) << "Created peer connection factory";

  server.uri = GetPeerConnectionString();
  servers.push_back(server);

  peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers, NULL, NULL, this);

  if (!peer_connection_.get()) {
    LOG(INFO) << "Create peer connection failed";
    DeletePeerConnection();
  }

  return peer_connection_.get() != NULL;
}

void Natty::DeletePeerConnection() {
  printf("Deleting peer connection\n");
  DisconnectFromServer();
  peer_connection_ = NULL;
  peer_connection_factory_ = NULL;
  peer_id_ = -1;
  Natty::Shutdown();
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

void Natty::ShowCandidate(const webrtc::IceCandidateInterface* candidate) {
  std::string out;
  const cricket::Candidate& cand = candidate->candidate();
  const talk_base::SocketAddress & address = cand.address(); 
  candidate->ToString(&out);
  
  LOG(INFO) << "Ice candidate" << out.c_str();
}

void Natty::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
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
  LOG(INFO) << "SDP message " << sdp;
  //SendMessage(writer.write(jmessage));

  ShowCandidate(candidate);
}

void Natty::OnRenegotiationNeeded() {
  LOG(INFO) << "Renegotiation needed";
  //peer_connection_->CreateOffer(this, NULL);

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
  LOG(INFO) << "Another peer connected " << id << " " << name;
}

void Natty::OnPeerDisconnected(int id) {
  if (id == peer_id_) {
    LOG(INFO) << "Our peer disconnected";
  }
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
    webrtc::SessionDescriptionInterface* session_description(
        webrtc::CreateSessionDescription(type, sdp));
    if (!session_description) {
      LOG(INFO) << "Can't parse SDP message";
      return;
    }
    LOG(INFO) << "Received session description " << message << " sending answer back";
    peer_connection_->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create(), session_description);
    if (session_description->type() ==
        webrtc::SessionDescriptionInterface::kOffer) {
      peer_connection_->CreateAnswer(this, NULL);
    }
  }; 
};

void Natty::OnMessageFromPeer(int peer_id, const std::string& message) {
  Json::Reader reader;
  Json::Value jmessage;
  std::string type;
  std::string json_object;
  LOG(INFO) << "Received message " << message.c_str() << " from peer " << peer_id;


  ASSERT(peer_id_ == peer_id || peer_id_ == -1);
  ASSERT(!message.empty());

  if (!peer_connection_.get()) {
    ASSERT(peer_id_ == -1);
    peer_id_ = peer_id;

    if (!InitializePeerConnection()) {
      LOG(INFO) << "Failed to initialize our PeerConnection instance";
      client_->SignOut();
      return;
    }
  } else if (peer_id != peer_id_) {
    ASSERT(peer_id_ != -1);
    LOG(INFO) << "Received message from an unknown peer";
    return;
  }

  if (!reader.parse(message, jmessage)) {
    LOG(INFO) << "Received an unknown message";
    return;
  }

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

    LOG(INFO) << "Received session description " << message.c_str() << " sending answer back";
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
      LOG(INFO) << "Can't parse received message";
      return;
    }
    talk_base::scoped_ptr<webrtc::IceCandidateInterface> candidate(
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

    LOG(INFO) << " Received candidate :" << message;
    return;
  }
}

void Natty::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  LOG(INFO) << "New data channel created " << data_channel;
  //printf("New data channel created %s\n", data_channel->ToString().c_str());
}

void Natty::OnMessageSent(int err) {

}

void Natty::OnIceComplete() {
  printf("ICE finished gather candidates!\n");
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

void Natty::Init() {
  if (client_->is_connected())
    return;
  InitializePeerConnection();
  peer_connection_->CreateOffer(this, NULL);
}

void Natty::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  printf("setting local description\n");
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
  if (msg) {
    pending_messages_.push_back(msg);
    if (!pending_messages_.empty() && !client_->IsSendingMessage()) {
      msg = pending_messages_.front();
      pending_messages_.pop_front();
      printf("Sending message to %d %s\n", peer_id_, msg->c_str());

      if (!client_->SendToPeer(peer_id_, *msg) && peer_id_ != -1) {
        DisconnectFromServer();
      }
      delete msg;
    }

    if (!peer_connection_.get())
      peer_id_ = -1;
  }
}

void Natty::DisconnectFromServer() {
  if (client_->is_connected())
    client_->SignOut();
}
 
