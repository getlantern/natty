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
#include "talk/base/logging.h"
#include "flagdefs.h"

#include <iostream>
#include <string>

static const int LOG_DEFAULT = talk_base::LS_INFO;
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
      LOG(INFO) << "Quitting!";
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

  string getStream() const { return ss.str(); }

  void read() {
    while (getline(std::cin, input)) {
      /* need to remove new lines or the SDP won't be valid */

      if (input.empty()) {
        /* terminate input on empty line */
        break;
      }
      ss << input;
    }
  }

  string build() const { 
    string str = ss.str();
    str.erase(
      std::remove(str.begin(), str.end(), '\n'), str.end()
    ); 
    return str;
  }

 protected:
  stringstream ss;
  string input;
};

void init(talk_base::Thread* thread, talk_base::scoped_refptr<Natty> natty) {

  NattySocket socket_server(thread);

  thread->set_socketserver(&socket_server);

  socket_server.set_client(natty->GetClient());
  socket_server.set_natty(natty);

  natty->SetupSocketServer();
  thread->Run();
}

int main(int argc, char* argv[]) {

  FlagList::SetFlagsFromCommandLine(&argc, argv, true);
  if (FLAG_help) {
    FlagList::Print(NULL, false);
    return 0;
  }

  if (FLAG_debug) {
    talk_base::LogMessage::LogTimestamps();
    talk_base::LogMessage::LogToDebug(talk_base::LS_VERBOSE);
  }

  // Abort if the user specifies a port that is outside the allowed
  // range [1, 65535].
  if ((FLAG_port < 1) || (FLAG_port > 65535)) {
    LOG(INFO) << "Error: %i is not a valid port.\n" << FLAG_port;
    return -1;
  }


  PeerConnectionClient client;
  talk_base::Thread* thread = talk_base::Thread::Current();
  talk_base::scoped_refptr<Natty> natty(
      new talk_base::RefCountedObject<Natty>(&client, thread, FLAG_server, FLAG_port));

  if (FLAG_offer) {
    init(thread, natty);
    return 0;
  }

  InputStream is;
  is.read();
  natty.get()->InitializePeerConnection();
  natty.get()->ReadMessage(is.build());
  thread->Run();
  natty.get()->DeletePeerConnection();
  return 0;
}

