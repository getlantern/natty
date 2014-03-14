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

