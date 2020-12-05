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

#include "ETHPacketWordGenerator.hpp"

axis_word ETHPacketWordGenerator::get_next_word(
    const Addresses &loc, const Meta &meta, hls::stream<axis_word> &buffer) {
#pragma HLS INLINE

  switch (word_cnt) {
  case 0:
    frm_protocol = IPv4;
    return counted(meta.dst_mac_addr(7, 0), word_cnt);
    break;
  case 1:
    return counted(meta.dst_mac_addr(15, 8), word_cnt);
    break;
  case 2:
    return counted(meta.dst_mac_addr(23, 16), word_cnt);
    break;
  case 3:
    return counted(meta.dst_mac_addr(31, 24), word_cnt);
    break;
  case 4:
    return counted(meta.dst_mac_addr(39, 32), word_cnt);
    break;
  case 5:
    return counted(meta.dst_mac_addr(47, 40), word_cnt);
    break;
  case 6:
    return counted(loc.mac_addr(7, 0), word_cnt);
    break;
  case 7:
    return counted(loc.mac_addr(15, 8), word_cnt);
    break;
  case 8:
    return counted(loc.mac_addr(23, 16), word_cnt);
    break;
  case 9:
    return counted(loc.mac_addr(31, 24), word_cnt);
    break;
  case 10:
    return counted(loc.mac_addr(39, 32), word_cnt);
    break;
  case 11:
    return counted(loc.mac_addr(47, 40), word_cnt);
    break;
  case 12:
    return counted(frm_protocol(15, 8), word_cnt);
    break;
  case 13:
    return counted(frm_protocol(7, 0), word_cnt);
    break;
  default:
    switch (frm_protocol) {
    case IPv4:
      return ipPacketWordGenerator.get_next_word(loc, meta, buffer);
      break;
    default:
      return {true, 0, 0};
      break;
    }
    break;
  }
}

void ETHPacketWordGenerator::reset() {
  word_cnt = 0;
  ipPacketWordGenerator.reset();
}
