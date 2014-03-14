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
#include "peer_connection_client.h"

#include "talk/base/thread.h"

class CustomSocketServer : public talk_base::PhysicalSocketServer {
 public:
  CustomSocketServer(talk_base::Thread* thread)
      : thread_(thread), natty_(NULL), client_(NULL) {}
  virtual ~CustomSocketServer() {}

  void set_client(PeerConnectionClient* client) { client_ = client; }
  void set_natty(Natty* natty) { natty_ = natty; }
  Natty* get_natty() { return natty_; }

  virtual bool Wait(int cms, bool process_io) {
    if (!natty_->connection_active() &&
        client_ != NULL && !client_->is_connected()) {
      thread_->Quit();
    }
    return talk_base::PhysicalSocketServer::Wait(0/*cms == -1 ? 1 : cms*/,
                                                 process_io);
  }

 protected:
  talk_base::Thread* thread_;
  Natty* natty_;
  PeerConnectionClient* client_;
};
 

int main(int argc, char* argv[]) {
  PeerConnectionClient client;
  talk_base::Thread* thread = talk_base::Thread::Current();
  talk_base::scoped_refptr<Natty> natty(
      new talk_base::RefCountedObject<Natty>(&client, thread));


  CustomSocketServer socket_server(thread);
  thread->set_socketserver(&socket_server);

  socket_server.set_client(natty->getClient());
  socket_server.set_natty(natty);

  thread->Run();
  natty->SetupSocketServer();

  sleep(3);
  
  natty->Shutdown();
  thread->set_socketserver(NULL);

  return 0;
}

