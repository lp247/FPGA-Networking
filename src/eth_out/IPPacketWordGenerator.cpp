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

#include "IPPacketWordGenerator.hpp"

axis_word IPPacketWordGenerator::get_next_word(const Addresses &loc,
                                               const Meta &meta,
                                               hls::stream<axis_word> &buffer) {
#pragma HLS INLINE

  switch (word_cnt) {
  case 0:
    ip_checksum.add(0x4500);
    ip_pkt_protocol = UDP;
    ip_pkt_length = meta.payload_length + IP_AND_UDP_HEADER_BYTE_SIZE;
    ip_hop_count_and_protocol(15, 8) = IP_HOP_COUNT;
    ip_hop_count_and_protocol(7, 0) = ip_pkt_protocol;
    return counted(0x45, word_cnt);
    break;
  case 1: // DSCP + ECN
    ip_checksum.add(loc.ip_addr(31, 16));
    return counted(0, word_cnt);
    break;
  case 2: // Packet total length high byte
    ip_checksum.add(loc.ip_addr(15, 0));
    return counted(ip_pkt_length(10, 8), word_cnt);
    break;
  case 3: // Packet total length low byte
    ip_checksum.add(ip_pkt_length);
    return counted(ip_pkt_length(7, 0), word_cnt);
    break;
  case 4: // ID high byte
    ip_checksum.add(meta.dst_ip_addr(31, 16));
    return counted(0, word_cnt);
    break;
  case 5: // ID low byte
    ip_checksum.add(meta.dst_ip_addr(15, 0));
    return counted(0, word_cnt);
    break;
  case 6: // Flags + fragment offset highest 3 bits
    return counted(0, word_cnt);
    break;
  case 7: // Fragment offset low byte
    return counted(0, word_cnt);
    break;
  case 8: // Hop count
    return counted(IP_HOP_COUNT, word_cnt);
    break;
  case 9: // IP protocol
    ip_checksum.add(ip_hop_count_and_protocol);
    return counted(ip_pkt_protocol, word_cnt);
    break;
  case 10: // IP header checksum high byte
    return counted(ip_checksum(15, 8), word_cnt);
    break;
  case 11: // IP header checksum low byte
    return counted(ip_checksum(7, 0), word_cnt);
    break;
  case 12:
    return counted(loc.ip_addr(31, 24), word_cnt);
    break;
  case 13:
    return counted(loc.ip_addr(23, 16), word_cnt);
    break;
  case 14:
    return counted(loc.ip_addr(15, 8), word_cnt);
    break;
  case 15:
    return counted(loc.ip_addr(7, 0), word_cnt);
    break;
  case 16:
    return counted(meta.dst_ip_addr(31, 24), word_cnt);
    break;
  case 17:
    return counted(meta.dst_ip_addr(23, 16), word_cnt);
    break;
  case 18:
    return counted(meta.dst_ip_addr(15, 8), word_cnt);
    break;
  case 19:
    return counted(meta.dst_ip_addr(7, 0), word_cnt);
    break;
  default:
    switch (ip_pkt_protocol) {
    case UDP:
      return udpPacketWordGenerator.get_next_word(loc, meta, buffer);
      break;
    default:
      return {true, 0, 0};
      break;
    }
    break;
  }
}

void IPPacketWordGenerator::reset() {
  word_cnt = 0;
  ip_checksum.reset();
  udpPacketWordGenerator.reset();
}
