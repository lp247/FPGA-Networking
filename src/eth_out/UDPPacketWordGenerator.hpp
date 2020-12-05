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

#ifndef UDP_PACKET_WORD_GENERATOR
#define UDP_PACKET_WORD_GENERATOR
#pragma once

#include "../utils/Addresses.hpp"
#include "../utils/axis_word.hpp"
#include "../utils/checksums/Checksum.hpp"
#include "Meta.hpp"
#include "PayloadWordGenerator.hpp"
#include <hls_stream.h>

const int MIN_UDP_PAYLOAD_BYTE_SIZE = 18;
const int UDP_PKT_HEADER_BYTE_SIZE = 8;

class UDPPacketWordGenerator {
public:
  UDPPacketWordGenerator()
      : word_cnt(0), payloadWordGenerator(MIN_UDP_PAYLOAD_BYTE_SIZE) {}
  axis_word get_next_word(const Addresses &loc,
                          const Meta &meta,
                          hls::stream<axis_word> &buffer);
  void reset();

private:
  ap_uint<5> word_cnt;
  ap_uint<16> udp_pkt_length;
  Checksum udp_checksum1;
  Checksum udp_checksum2;
  PayloadWordGenerator payloadWordGenerator;
};

#endif
