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

#include "UDPPacketHandler.hpp"

Optional<axis_word>
UDPPacketHandler::get_payload(const Optional<axis_word> &word,
                              const Addresses &loc,
                              Status &status,
                              const ap_uint<32> &src_ip_addr) {
#pragma HLS INLINE

  if (!word.is_valid) {
    return NOTHING;
  }

  switch (this->cnt) {
  case 0:
    this->udp_pkt_src_port(15, 8) = word.value.data;
    this->udp_checksum1.add(loc.ip_addr(31, 16));
    this->udp_checksum2.add(loc.ip_addr(15, 0));
    this->cnt = 1;
    return NOTHING;
    break;
  case 1:
    this->udp_pkt_src_port(7, 0) = word.value.data;
    this->udp_checksum1.add(src_ip_addr(31, 16));
    this->udp_checksum2.add(src_ip_addr(15, 0));
    this->cnt = 2;
    return NOTHING;
    break;
  case 2:
    this->udp_pkt_dst_port(15, 8) = word.value.data;
    this->udp_checksum1.add(this->udp_pkt_src_port);
    this->udp_checksum2.add(0x0011);
    this->cnt = 3;
    return NOTHING;
    break;
  case 3:
    this->udp_pkt_dst_port(7, 0) = word.value.data;
    this->udp_checksum1.add(this->udp_pkt_dst_port);
    this->cnt = 4;
    return NOTHING;
    break;
  case 4:
    this->udp_pkt_length(15, 0) = word.value.data;
    this->cnt = 5;
    return NOTHING;
    break;
  case 5:
    this->udp_pkt_length(7, 0) = word.value.data;
    this->udp_checksum1.add(this->udp_pkt_length);
    this->udp_checksum2.add(this->udp_pkt_length);
    this->cnt = 6;
    return NOTHING;
    break;
  case 6:
    this->udp_pkt_checksum(15, 8) = word.value.data;
    this->udp_checksum1.add(this->udp_checksum2);
    this->cnt = 7;
    return NOTHING;
    break;
  case 7:
    this->udp_pkt_checksum(7, 0) = word.value.data;
    this->udp_checksum1.add(this->udp_pkt_checksum);
    this->cnt = 8;
    return NOTHING;
    break;
  default:
    if (loc.udp_port != udp_pkt_dst_port) {
      status.set_bad_udp_port();
    }

    if (this->cnt >= this->udp_pkt_length) {
      return NOTHING;
    }

    ap_uint<1> is_last_word = this->cnt == (this->udp_pkt_length - 1);
    ap_uint<96> new_user = word.value.user;
    new_user(95, 80) = this->udp_pkt_src_port;
    this->udp_checksum1.add_half(word.value.data);
    if (is_last_word && udp_pkt_checksum != 0 && this->udp_checksum1 != 0) {
      status.set_bad_udp_checksum();
    }
    this->cnt++;
    return {{word.value.data, is_last_word, new_user}, true};
    break;
  }
}

void UDPPacketHandler::reset() {
  this->udp_checksum1.reset();
  this->udp_checksum2.reset();
  this->cnt = 0;
}
