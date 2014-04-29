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
 
#ifndef NATTY_H_
#define NATTY_H_
#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>

#include "talk/base/socketaddress.h"
#include "peer_connection_client.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/datachannelinterface.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/ssladapter.h"
#include "talk/base/json.h"

struct NattyMessage : public talk_base::MessageData {
  explicit NattyMessage(std::string body) : body_(body) {}
  virtual ~NattyMessage() {}

  std::string body_;
};             

class MessageClient : public talk_base::MessageHandler {
 public:
  MessageClient(talk_base::Thread* pth, talk_base::Socket* socket)
    : thread_(pth), socket_(socket) {
    }

  virtual void OnMessage(talk_base::Message *pmsg);
  virtual ~MessageClient();

 private:
  talk_base::Thread* thread_;
  talk_base::Socket* socket_;
};


class NattySocket : public talk_base::PhysicalSocketServer {
 public:
  NattySocket(talk_base::Thread* thread) :
     thread_(thread) {

  }

  virtual ~NattySocket() {};

  virtual bool Wait(int cms, bool process_io);

 protected:
  talk_base::Thread* thread_;
};

class Natty
  : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver,
    public PeerConnectionClientObserver {
 public:
  Natty(PeerConnectionClient* client, talk_base::Thread* thread
      );

  static const int DELAY_INTERVAL = 3;

  bool connection_active() const;

  virtual void Init(bool mode);
  virtual void OpenDumpFile(const std::string& filename);
  PeerConnectionClient* GetClient();
  bool InitializePeerConnection();
  void Shutdown();
  void ProcessInput();
  virtual void ReadMessage(const std::string& message);
  

 protected:
  ~Natty();

  //
  // PeerConnectionObserver implementation.
  //
  virtual void OnError();
  virtual void OnStateChange(
      webrtc::PeerConnectionObserver::StateType state_changed) {};
  virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
  virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);
  virtual void OnRenegotiationNeeded();
  virtual void OnIceChange() {};
  virtual void OnIceComplete();

  virtual void OnDataChannel(webrtc::DataChannelInterface *data_channel);
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

  virtual void OnSignedIn();

  virtual void OnDisconnected();
  virtual void DisconnectFromServer();

  virtual void OnPeerConnected(int id, const std::string& name);

  virtual void OnPeerDisconnected(int id);

  virtual void OnMessageFromPeer(int peer_id, const std::string& message);

  virtual void OnMessageSent(int err);


  virtual void OnServerConnectionFailure();

  // CreateSessionDescriptionObserver implementation.
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
  virtual void OnFailure(const std::string& error);

 protected:
  // Send a message to the remote peer.
  void SendMessage(const std::string& json_object);

  int peer_id_;
  talk_base::Thread* thread_;
  talk_base::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
  PeerConnectionClient* client_;
  std::deque<std::string*> pending_messages_;

  /* stdout */
  std::ofstream outfile;

  enum Mode { OFFER, TRAVERSE };

  void setMode(Mode m);
  //Mode getMode() const;
  Mode mode;

  Json::Value fivetuple;

  std::map<std::string, talk_base::scoped_refptr<webrtc::MediaStreamInterface> >
    active_streams_;


  class InputStream {
    public:
      InputStream() {}
      ~InputStream() {}

      void read(Natty* natty);

      std::string build() const;

    protected:
      std::stringstream ss;
      std::string input;
  };    


};

#endif  // NATTY_H_
