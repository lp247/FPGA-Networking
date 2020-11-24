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

#ifndef IP_PACKET_HANDLER_HPP
#define IP_PACKET_HANDLER_HPP
#pragma once

#include "../utils/Addresses.hpp"
#include "../utils/Optional.hpp"
#include "../utils/axis_word.hpp"
#include "../utils/constants.hpp"
#include "Status.hpp"
#include "UDPPacketHandler.hpp"
#include <ap_int.h>

class IPPacketHandler {
public:
  IPPacketHandler() : cnt(0) {}
  Optional<axis_word> get_payload(const Optional<axis_word> &word,
                                  const Addresses &loc,
                                  Status &status);
  void reset();

private:
  UDPPacketHandler udpPacketHandler;
  ap_uint<5> cnt;
  ap_uint<4> ip_pkt_ihl;
  ap_uint<8> ip_pkt_protocol;
  ap_uint<32> ip_pkt_src_ip_addr;
  ap_uint<32> ip_pkt_dst_ip_addr;
};

#endif
