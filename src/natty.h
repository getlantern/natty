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

#include "webrtc/base/socketaddress.h"
#include "talk/app/webrtc/portallocatorfactory.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/peerconnection.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/datachannelinterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/json.h"

struct NattyMessage : public rtc::MessageData {
  explicit NattyMessage(std::string body) : body_(body) {}
  virtual ~NattyMessage() {}

  std::string body_;
};             

class MessageClient : public rtc::MessageHandler {
 public:
  MessageClient(rtc::Thread* pth, rtc::Socket* socket)
    : thread_(pth), socket_(socket) {
    }

  virtual void OnMessage(rtc::Message *pmsg);
  virtual ~MessageClient();

 private:
  rtc::Thread* thread_;
  rtc::Socket* socket_;
};


class Natty
  : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver {
 public:
  Natty(rtc::Thread* thread);

  static const int DELAY_INTERVAL = 3;

  bool connection_active() const;

  virtual void Init(bool mode);
  virtual void OpenDumpFile(const std::string& filename);
  bool InitializePeerConnection();
  void Shutdown();
  void ProcessInput();
  virtual void ReadMessage(const std::string& message);
  

 protected:
  ~Natty();
 
  void AddStreams(); 

  //
  // PeerConnectionObserver implementation.
  //
  virtual void InspectTransportChannel();
  virtual void OnError();
  virtual void OnStateChange(
      webrtc::PeerConnectionObserver::StateType state_changed);
  virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state);

    virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
  virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);
  virtual void OnRenegotiationNeeded();
  virtual void OnIceComplete();

  virtual void OnDataChannel(webrtc::DataChannelInterface *data_channel);
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

  virtual void PickFinalCandidate();

  virtual void Output5Tuple(const cricket::Candidate *cand);


  virtual void OnServerConnectionFailure();

  // CreateSessionDescriptionObserver implementation.
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
  virtual void OnFailure(const std::string& error);

 protected:
  // Send a message to the remote peer.
  void SendMessage(const std::string& json_object);

  int peer_id_;
  rtc::Thread* thread_;
  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
  std::deque<std::string*> pending_messages_;



  webrtc::SessionDescriptionInterface* session_description;
  int sdp_mlineindex;
  rtc::scoped_refptr<webrtc::PortAllocatorFactoryInterface> allocator_factory_;
  cricket::PortAllocator* allocator;

  uint32 highestPrioritySeen;
  const cricket::Candidate *bestCandidate;

  /* stdout */
  std::ofstream outfile;

  enum Mode { OFFER, TRAVERSE };

  void setMode(Mode m);
  //Mode getMode() const;
  Mode mode;

  Json::Value fivetuple;

  std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >
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
