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
      new talk_base::RefCountedObject<Natty>(&client, thread));

  natty.get()->OpenDumpFile(FLAG_out);
  natty.get()->Init(FLAG_offer); 

  if (!FLAG_offer) {
    natty.get()->ProcessInput();
  }

  thread->Run();

  return 0;
}

