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

#include "webrtc/base/thread.h"
#include "webrtc/base/logging.h"
#include "flagdefs.h"

#include <iostream>
#include <string>

static const int LOG_DEFAULT = rtc::LS_INFO;

using namespace std;

int main(int argc, char* argv[]) {

  rtc::FlagList::SetFlagsFromCommandLine(&argc, argv, true);
  if (FLAG_help) {
      rtc::FlagList::Print(NULL, false);
    return 0;
  }

  if (FLAG_debug) {
    rtc::LogMessage::LogTimestamps();
    rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
  }

  rtc::InitializeSSL();
  rtc::Thread* thread = rtc::Thread::Current();
  /* Scopers help maintain ownership of a pointer
   */
  rtc::scoped_refptr<Natty> natty(
      new rtc::RefCountedObject<Natty>(thread));

  natty.get()->OpenDumpFile(FLAG_out);
  natty.get()->Init(FLAG_offer); 
  
  natty.get()->ProcessInput();
  return 0;
}

