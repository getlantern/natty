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

typedef webrtc::DataChannelInterface::DataState DCState;

class NattyDataChannelObserver 
: public webrtc::DataChannelObserver {
 public:
  explicit NattyDataChannelObserver(webrtc::DataChannelInterface* channel)
      : channel_(channel), received_message_count_(0) {
        channel_->RegisterObserver(this);
        InitStates();
        state_ = channel_->state();
      }
  virtual ~NattyDataChannelObserver() {
    channel_->UnregisterObserver();
  }

  /* if data channel is open and writable, we know a successful 
   * traversal happened and the best connection has been settled on
   */                                                               
  virtual void OnStateChange() { 
    state_ = channel_->state(); 
    LOG(INFO) << "Data channel state changed " << dc_states[state_]; 
  }

  /* map data channel connection states enums to strings */
  virtual void InitStates() {
    const std::string data_states[] = {"Connecting", "Open", 
      "Closing", "Closed"};

    for ( int i = DCState::kConnecting; 
         i != DCState::kClosed; i++ )
    {
      DCState state = static_cast<DCState>(i);
      dc_states[state] = data_states[i].c_str();
    }
  }

  virtual void OnMessage(const webrtc::DataBuffer& buffer) {
    LOG(INFO) << "Received data buffer message";
    last_message_.assign(buffer.data.data(), buffer.data.length());
    ++received_message_count_;
  }

  bool IsOpen() const { return state_ == webrtc::DataChannelInterface::kOpen; }
  const std::string& last_message() const { return last_message_; }
  size_t received_message_count() const { return received_message_count_; }

 private:
  std::map<DCState, std::string> dc_states;
  rtc::scoped_refptr<webrtc::DataChannelInterface> channel_;
  webrtc::DataChannelInterface::DataState state_;
  std::string last_message_;
  size_t received_message_count_;
};


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


class Natty
  : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver {
 public:

  Natty(rtc::Thread* thread);

  bool connection_active() const;

  virtual void Init(bool mode);
  virtual void OpenDumpFile(const std::string& filename);
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
      webrtc::PeerConnectionObserver::StateType state_changed);
  virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state);

    virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
  virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);
  virtual void OnRenegotiationNeeded();
  virtual void OnIceConnectionChange(webrtc::PeerConnection::IceConnectionState new_state);
  virtual void OnIceComplete();

  virtual void OnDataChannel(webrtc::DataChannelInterface *data_channel);
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

  virtual void OnFailure(const std::string& msg);

  virtual void OnServerConnectionFailure();

  // CreateSessionDescriptionObserver implementation.
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
 
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;  
  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
  NattyDataChannelObserver* data_channel_observer_;

  rtc::Thread* thread_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;

  webrtc::SessionDescriptionInterface* session_description;

  /* stdout */
  std::ofstream outfile;

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
