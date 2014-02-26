/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
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

#include <iostream>  // NOLINT

#include "talk/base/thread.h"
#include "talk/p2p/base/relayserver.h"
#include "talk/examples/natty/natty_server.h"
#include "talk/app/webrtc/portallocatorfactory.h"
#include "talk/base/crc32.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/base/natserver.h"
#include "talk/base/natsocketfactory.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/socketaddress.h"
#include "talk/base/ssladapter.h"
#include "talk/examples/natty/natty_socket.h"
#include "talk/base/flags.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"
#include "talk/p2p/base/portproxy.h"
#include "talk/p2p/base/relayport.h"
#include "talk/p2p/base/stunport.h"
#include "talk/p2p/base/tcpport.h"
#include "talk/p2p/base/testrelayserver.h"
#include "talk/p2p/base/teststunserver.h"
#include "talk/p2p/base/testturnserver.h"
#include "talk/p2p/base/transport.h"
#include "talk/p2p/base/turnport.h"

#define LOG2(fmt, ...) printf("%s:%d: \n" fmt, __FILE__, __LINE__, __VA_ARGS__);

typedef std::vector<NattySocket*> SocketArray;
typedef talk_base::BasicPacketSocketFactory NattySocketFactory;

using talk_base::PacketSocketFactory;
using talk_base::SocketAddress;
using talk_base::AsyncPacketSocket;
using talk_base::scoped_ptr;
using talk_base::Socket;
using talk_base::ByteBuffer;
using namespace cricket;

static const SocketAddress kLocalAddr1("192.168.1.2", 0);
static const SocketAddress kLocalAddr2("192.168.1.3", 0);

class FakePacketSocketFactory : public talk_base::PacketSocketFactory {
 public:
  FakePacketSocketFactory()
      : next_udp_socket_(NULL),
        next_server_tcp_socket_(NULL),
        next_client_tcp_socket_(NULL) {
  }
  virtual ~FakePacketSocketFactory() { }

  virtual AsyncPacketSocket* CreateUdpSocket(
      const SocketAddress& address, int min_port, int max_port) {
    //EXPECT_TRUE(next_udp_socket_ != NULL);
    AsyncPacketSocket* result = next_udp_socket_;
    next_udp_socket_ = NULL;
    return result;
  }

  virtual AsyncPacketSocket* CreateServerTcpSocket(
      const SocketAddress& local_address, int min_port, int max_port,
      int opts) {
    //EXPECT_TRUE(next_server_tcp_socket_ != NULL);
    AsyncPacketSocket* result = next_server_tcp_socket_;
    next_server_tcp_socket_ = NULL;
    return result;
  }

  // TODO: |proxy_info| and |user_agent| should be set
  // per-factory and not when socket is created.
  virtual AsyncPacketSocket* CreateClientTcpSocket(
      const SocketAddress& local_address, const SocketAddress& remote_address,
      const talk_base::ProxyInfo& proxy_info,
      const std::string& user_agent, int opts) {
    //EXPECT_TRUE(next_client_tcp_socket_ != NULL);
    AsyncPacketSocket* result = next_client_tcp_socket_;
    next_client_tcp_socket_ = NULL;
    return result;
  }

  void set_next_udp_socket(AsyncPacketSocket* next_udp_socket) {
    next_udp_socket_ = next_udp_socket;
  }
  void set_next_server_tcp_socket(AsyncPacketSocket* next_server_tcp_socket) {
    next_server_tcp_socket_ = next_server_tcp_socket;
  }
  void set_next_client_tcp_socket(AsyncPacketSocket* next_client_tcp_socket) {
    next_client_tcp_socket_ = next_client_tcp_socket;
  }
  talk_base::AsyncResolverInterface* CreateAsyncResolver() {
    return NULL;
  }

 private:
  AsyncPacketSocket* next_udp_socket_;
  AsyncPacketSocket* next_server_tcp_socket_;
  AsyncPacketSocket* next_client_tcp_socket_;
};
 
class FakeAsyncPacketSocket : public AsyncPacketSocket {
 public:
  // Returns current local address. Address may be set to NULL if the
  // socket is not bound yet (GetState() returns STATE_BINDING).
  virtual SocketAddress GetLocalAddress() const {
    return SocketAddress();
  }

  // Returns remote address. Returns zeroes if this is not a client TCP socket.
  virtual SocketAddress GetRemoteAddress() const {
    return SocketAddress();
  }

  // Send a packet.
  virtual int Send(const void *pv, size_t cb,
                   const talk_base::PacketOptions& options) {
    return static_cast<int>(cb);
  }
  virtual int SendTo(const void *pv, size_t cb, const SocketAddress& addr,
                     const talk_base::PacketOptions& options) {
    return static_cast<int>(cb);
  }
  virtual int Close() {
    return 0;
  }

  virtual State GetState() const { return state_; }
  virtual int GetOption(Socket::Option opt, int* value) { return 0; }
  virtual int SetOption(Socket::Option opt, int value) { return 0; }
  virtual int GetError() const { return 0; }
  virtual void SetError(int error) { }

  void set_state(State state) { state_ = state; }

 private:
  State state_;
};

static Candidate GetCandidate(Port* port) {
  assert(port->Candidates().size() == 1);
  return port->Candidates()[0];
}
 
static IceMessage* CopyStunMessage(const IceMessage* src) {
  IceMessage* dst = new IceMessage();
  ByteBuffer buf;
  src->Write(&buf);
  dst->Read(&buf);
  return dst;
}

class TestChannel : public sigslot::has_slots<> {
 public:
  // Takes ownership of |p1| (but not |p2|).
  TestChannel(Port* p1, Port* p2)
      : ice_mode_(ICEMODE_FULL), src_(p1), dst_(p2), complete_count_(0),
	conn_(NULL), remote_request_(), nominated_(false) {
    src_->SignalPortComplete.connect(
        this, &TestChannel::OnPortComplete);
    src_->SignalUnknownAddress.connect(this, &TestChannel::OnUnknownAddress);
    src_->SignalDestroyed.connect(this, &TestChannel::OnSrcPortDestroyed);
  }

  int complete_count() { return complete_count_; }
  Connection* conn() { return conn_; }
  const SocketAddress& remote_address() { return remote_address_; }
  const std::string remote_fragment() { return remote_frag_; }

  void Start() {
    src_->PrepareAddress();
  }
  void CreateConnection() {
    conn_ = src_->CreateConnection(GetCandidate(dst_), Port::ORIGIN_MESSAGE);
    IceMode remote_ice_mode =
        (ice_mode_ == ICEMODE_FULL) ? ICEMODE_LITE : ICEMODE_FULL;
    conn_->set_remote_ice_mode(remote_ice_mode);
    conn_->set_use_candidate_attr(remote_ice_mode == ICEMODE_FULL);
    conn_->SignalStateChange.connect(
        this, &TestChannel::OnConnectionStateChange);
  }
  void OnConnectionStateChange(Connection* conn) {
    if (conn->write_state() == Connection::STATE_WRITABLE) {
      conn->set_use_candidate_attr(true);
      nominated_ = true;
    }
  }
  void AcceptConnection() {
    //ASSERT_TRUE(remote_request_.get() != NULL);
    Candidate c = GetCandidate(dst_);
    c.set_address(remote_address_);
    conn_ = src_->CreateConnection(c, Port::ORIGIN_MESSAGE);
    src_->SendBindingResponse(remote_request_.get(), remote_address_);
    remote_request_.reset();
  }
  void Ping() {
    Ping(0);
  }
  void Ping(uint32 now) {
    conn_->Ping(now);
  }
  void Stop() {
    conn_->SignalDestroyed.connect(this, &TestChannel::OnDestroyed);
    conn_->Destroy();
  }

  void OnPortComplete(Port* port) {
    complete_count_++;
  }
  void SetIceMode(IceMode ice_mode) {
    ice_mode_ = ice_mode;
  }

  void OnUnknownAddress(PortInterface* port, const SocketAddress& addr,
                        ProtocolType proto,
                        IceMessage* msg, const std::string& rf,
                        bool /*port_muxed*/) {
    //ASSERT_EQ(src_.get(), port);
    if (!remote_address_.IsNil()) {
      //ASSERT_EQ(remote_address_, addr);
    }
    // MI and PRIORITY attribute should be present in ping requests when port
    // is in ICEPROTO_RFC5245 mode.
    const cricket::StunUInt32Attribute* priority_attr =
        msg->GetUInt32(STUN_ATTR_PRIORITY);
    const cricket::StunByteStringAttribute* mi_attr =
        msg->GetByteString(STUN_ATTR_MESSAGE_INTEGRITY);
    const cricket::StunUInt32Attribute* fingerprint_attr =
        msg->GetUInt32(STUN_ATTR_FINGERPRINT);
    if (src_->IceProtocol() == cricket::ICEPROTO_RFC5245) {
      //EXPECT_TRUE(priority_attr != NULL);
      //EXPECT_TRUE(mi_attr != NULL);
      //EXPECT_TRUE(fingerprint_attr != NULL);
    } else {
      //EXPECT_TRUE(priority_attr == NULL);
      //EXPECT_TRUE(mi_attr == NULL);
      //EXPECT_TRUE(fingerprint_attr == NULL);
    }
    remote_address_ = addr;
    remote_request_.reset(CopyStunMessage(msg));
    remote_frag_ = rf;
  }

  void OnDestroyed(Connection* conn) {
    //ASSERT_EQ(conn_, conn);
    conn_ = NULL;
  }

  void OnSrcPortDestroyed(PortInterface* port) {
    Port* destroyed_src = src_.release();
    //ASSERT_EQ(destroyed_src, port);
  }

  bool nominated() const { return nominated_; }

 private:
  IceMode ice_mode_;
  talk_base::scoped_ptr<Port> src_;
  Port* dst_;

  int complete_count_;
  Connection* conn_;
  SocketAddress remote_address_;
  talk_base::scoped_ptr<StunMessage> remote_request_;
  std::string remote_frag_;
  bool nominated_;
};
 
 

void cleanConnections(SocketArray *sockets) {
  for (SocketArray::iterator i = sockets->begin(); i != sockets->end(); ++i)
    delete (*i);
  sockets->clear();
}

void acceptConnections(ListeningSocket *listener) {
  SocketArray sockets;
  bool quit = false;
  while (!quit) {
    fd_set socket_set;
    FD_ZERO(&socket_set);
    if (listener->valid())
      FD_SET(listener->socket(), &socket_set);
    for (SocketArray::iterator i = sockets.begin(); i != sockets.end(); ++i)
      FD_SET((*i)->socket(), &socket_set);

    struct timeval timeout = { 10, 0 };
    if (select(FD_SETSIZE, &socket_set, NULL, NULL, &timeout) == SOCKET_ERROR) {
      printf("select failed\n");
      break;
    }

    if (FD_ISSET(listener->socket(), &socket_set)) {
      NattySocket* s = listener->Accept();
      sockets.push_back(s);
      printf("New connection made!\n");
    }
  }
  printf("Exiting accept connections loop\n");
  cleanConnections(&sockets);
}

UDPPort* CreateUdpPort(const SocketAddress& addr,
    PacketSocketFactory* socket_factory,
    talk_base::Network *network
    ) {
  std::string username_ = talk_base::CreateRandomString(ICE_UFRAG_LENGTH);
  std::string password_ = talk_base::CreateRandomString(ICE_PWD_LENGTH);
  //std::cout << "username : " << username_;
  //std::cout << "password : " << password_;
  UDPPort* port = UDPPort::Create(talk_base::Thread::Current(), socket_factory, network,
      addr.ipaddr(), 0, 3000, "", "");
  port->SetIceProtocolType(cricket::ICEPROTO_GOOGLE);
  port->set_component(cricket::ICE_CANDIDATE_COMPONENT_DEFAULT);
  port->PrepareAddress();
  
  return NULL;
  //return port;
}




int main(int argc, char **argv) {
  FakeAsyncPacketSocket *s1 = new FakeAsyncPacketSocket();
  FakePacketSocketFactory socket_factory;

  socket_factory.set_next_udp_socket(s1);
  talk_base::Network *network = new talk_base::Network("unittest", "unittest", talk_base::IPAddress(INADDR_ANY), 32);

  delete network;


  return 0;
}
