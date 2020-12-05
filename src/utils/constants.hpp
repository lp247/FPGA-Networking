/*
 * Copyright (c) 2020, Peter Lehnhardt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#pragma once

#include "stdint.h"
#include <string>
#include "Optional.hpp"

const ap_uint<8> IPG = 96;

// Ethernet protocols
const uint16_t ARP = 0x0806;
const uint16_t IPv4 = 0x0800;
const uint16_t IPv6 = 0x86DD;

// IPv4 protocols
const uint8_t ICMP = 0x1;
const uint8_t TCP = 0x6;
const uint8_t UDP = 0x11;

// IPv4 settings
const ap_uint<8> IP_IHL = 0x5;
const ap_uint<8> IP_VERSION = 0x4;
const ap_uint<8> IP_HOP_COUNT = 0x80;

const ap_uint<32> CRC32_RESIDUE = 0x1CDF4421;
const ap_uint<32> CRC32_RESIDUE_INV_BREV = 0xC704DD7B;

// Console text color codes
const std::string FG_RED = "\033[31m";
const std::string FG_WHITE = "\033[37m";
const std::string FG_GREEN = "\033[32m";
const std::string BG_GRAY = "\033[48;5;8m";
const std::string BG_BLACK = "\033[48;5;0m";
const std::string COLOR_RESET = "\033[0m";

const Optional<axis_word> NOTHING = {None, {0, false, 0}};

#endif
