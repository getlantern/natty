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

#ifndef DEFAULTS_H_
#define DEFAULTS_H_
#pragma once

#include <string>

#include "webrtc/base/basictypes.h"

extern const char kAudioLabel[];
extern const char kVideoLabel[];
extern const char kStreamLabel[];
extern const uint16 kDefaultServerPort;

std::string GetEnvVarOrDefault(const char* env_var_name,
                               const char* default_value);
std::string GetPeerConnectionString();
std::string GetDefaultServerName();
std::string GetPeerName();

#endif  // DEFAULTS_H_
