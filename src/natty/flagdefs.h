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
