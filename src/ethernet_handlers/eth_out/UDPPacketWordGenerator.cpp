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

#include "UDPPacketWordGenerator.hpp"

axis_word UDPPacketWordGenerator::get_next_word(
    const Addresses &loc, const Meta &meta, hls::stream<axis_word> &buffer) {
#pragma HLS INLINE

  switch (word_cnt) {
  case 0:
    udp_pkt_length = meta.payload_length + UDP_PKT_HEADER_BYTE_SIZE;
    udp_checksum1 = Checksum(loc.ip_addr(31, 16));
    udp_checksum2 = Checksum(meta.dst_ip_addr(31, 16));
    return counted(loc.udp_port(15, 8), word_cnt);
    break;
  case 1:
    udp_checksum1.add(loc.ip_addr(15, 0));
    udp_checksum2.add(meta.dst_ip_addr(15, 0));
    return counted(loc.udp_port(7, 0), word_cnt);
    break;
  case 2:
    udp_checksum1.add(udp_pkt_length);
    udp_checksum2.add(udp_pkt_length);
    return counted(meta.dst_udp_port(15, 8), word_cnt);
    break;
  case 3:
    udp_checksum1.add(loc.udp_port);
    udp_checksum2.add(meta.dst_udp_port);
    return counted(meta.dst_udp_port(7, 0), word_cnt);
    break;
  case 4:
    udp_checksum1.add(0x0011);
    udp_checksum2.add(meta.payload_checksum);
    return counted(udp_pkt_length(10, 8), word_cnt);
    break;
  case 5:
    udp_checksum1.add(udp_checksum2);
    return counted(udp_pkt_length(7, 0), word_cnt);
    break;
  case 6:
    return counted(udp_checksum1(15, 8), word_cnt);
    break;
  case 7:
    return counted(udp_checksum1(7, 0), word_cnt);
    break;
  default:
    return payloadWordGenerator.get_next_word(buffer);
    break;
  }
}

void UDPPacketWordGenerator::reset() {
  word_cnt = 0;
  udp_checksum1.reset();
  udp_checksum2.reset();
  payloadWordGenerator.reset();
}
