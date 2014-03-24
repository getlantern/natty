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
#include "flagdefs.h"

#include <iostream>

const uint16 kDefaultServerPort = 8888;

using namespace std;

class NattySocket : public talk_base::PhysicalSocketServer {
 public:
  NattySocket(talk_base::Thread* thread)
      : thread_(thread), natty_(NULL), client_(NULL) {}
  virtual ~NattySocket() {}

  void set_client(PeerConnectionClient* client) { client_ = client; }
  void set_natty(Natty* natty) { natty_ = natty; }
  Natty* get_natty() { return natty_; }

  virtual bool Wait(int cms, bool process_io) {
    //if (!natty_->connection_active() ||
    if (client_ == NULL) {
        //client_ == NULL || !client_->is_connected()) {
      printf("quitting..\n");
      thread_->Quit();
    }
    return talk_base::PhysicalSocketServer::Wait(-1,
                                                 process_io);
  }

 protected:
  talk_base::Thread* thread_;
  Natty* natty_;
  PeerConnectionClient* client_;
};

class InputStream {
 public:
  InputStream() {}
  ~InputStream() {}

  void read() {
    while (!std::cin.eof()) {
      if (!getline(std::cin, input)) {
        /* error processing stdin */
        //cerr << "Encountered a problem processing stdin!" << endl;
        break;
      }

      if (input.empty()) {
        /* terminate input on empty line */
        break;
      }
      ss << input;
    }
  }

  stringstream ss;
  string input;
};
 

int main(int argc, char* argv[]) {

  FlagList::SetFlagsFromCommandLine(&argc, argv, true);
  if (FLAG_help) {
    FlagList::Print(NULL, false);
    return 0;
  }

  // Abort if the user specifies a port that is outside the allowed
  // range [1, 65535].
  if ((FLAG_port < 1) || (FLAG_port > 65535)) {
    printf("Error: %i is not a valid port.\n", FLAG_port);
    return -1;
  }

  PeerConnectionClient client;
  talk_base::Thread* thread = talk_base::Thread::Current();
  talk_base::scoped_refptr<Natty> natty(
      new talk_base::RefCountedObject<Natty>(&client, thread, FLAG_server, FLAG_port));

  InputStream is;
  is.read();
  natty.get()->ReadMessage(is.ss.str());
  return 0;


  NattySocket socket_server(thread);
  //talk_base::PhysicalSocketServer* pss = new talk_base::PhysicalSocketServer();
  thread->set_socketserver(&socket_server);
  //thread->set_socketserver(pss);

  socket_server.set_client(natty->GetClient());
  socket_server.set_natty(natty);

  natty->SetupSocketServer();
  thread->Run();

  return 0;
}

