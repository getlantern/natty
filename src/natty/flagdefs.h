#ifndef NATTY_FLAGDEFS_H_
#define NATTY_FLAGDEFS_H_
#pragma once

#include "webrtc/base/flags.h"

extern const uint16 kDefaultServerPort;  // From defaults.[h|cc]

DEFINE_bool(help, false, "Prints this message");
DEFINE_bool(offer, false, "Generates an offer");
DEFINE_bool(debug, false, "Log debugging info");
DEFINE_bool(autoconnect, false, "Connect to the server without user "
                                "intervention.");
DEFINE_string(out, "", "Natty stdout");  
DEFINE_string(server, "localhost", "The server to connect to.");
DEFINE_int(port, 8000,
           "The port on which the server is listening.");
DEFINE_bool(autocall, false, "Call the first available other client on "
  "the server without user intervention.  Note: this flag should only be set "
  "to true on one of the two clients.");

#endif  // NATTY_FLAGDEFS_H_
