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
    rtc::LogMessage::LogToDebug(LOG_DEFAULT);
  }

  /* InitializeSSL is where you confirm which TLS implementation you want to use
   * for peer connection sessions 
   */
  rtc::InitializeSSL();
  rtc::Thread* thread = rtc::Thread::Current();

  /* Scopers help maintain ownership of a pointer
   */
  rtc::scoped_refptr<Natty> natty(
      new rtc::RefCountedObject<Natty>(thread));

  /* OpenDumpFile checks if stdout should be redirected to an outfile */
  natty.get()->OpenDumpFile(FLAG_out);
  natty.get()->Init(FLAG_offer); 
  
  natty.get()->ProcessInput();
  return 0;
}

