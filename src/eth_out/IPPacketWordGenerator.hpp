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

#ifndef IP_PACKET_WORD_GENERATOR
#define IP_PACKET_WORD_GENERATOR
#pragma once

#include "../utils/Addresses.hpp"
#include "../utils/axis_word.hpp"
#include "../utils/checksums/Checksum.hpp"
#include "../utils/protocols.hpp"
#include "Meta.hpp"
#include "UDPPacketWordGenerator.hpp"
#include <ap_int.h>
#include <hls_stream.h>

const ap_uint<8> IP_VERSION_AND_STD_IHL = 0x45;
const ap_uint<16> IP_VERSION_AND_STD_IHL_AND_NO_SPECIAL = 0x4500;
const ap_uint<8> IP_HOP_COUNT = 0x80;
const int IP_PKT_HEADER_BYTE_SIZE = 20;
const int IP_AND_UDP_HEADER_BYTE_SIZE =
    IP_PKT_HEADER_BYTE_SIZE + UDP_PKT_HEADER_BYTE_SIZE;

class IPPacketWordGenerator {
public:
  IPPacketWordGenerator() : word_cnt(0) {}
  axis_word get_next_word(const Addresses &loc,
                          const Meta &meta,
                          hls::stream<axis_word> &buffer);
  void reset();

private:
  ap_uint<32> word_cnt;
  Checksum ip_checksum;
  ap_uint<8> ip_pkt_protocol;
  ap_uint<11> ip_pkt_length;
  ap_uint<16> ip_hop_count_and_protocol;
  UDPPacketWordGenerator udpPacketWordGenerator;
};

#endif
