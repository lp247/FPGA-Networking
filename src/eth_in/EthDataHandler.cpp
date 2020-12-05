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

#include "EthDataHandler.hpp"

Optional<axis_word> EthDataHandler::get_payload(const Optional<axis_word> &word,
                                                const Addresses &loc,
                                                ap_uint<1> &bad_data) {
#pragma HLS INLINE

  if (word.is_none()) {
    return NOTHING;
  }

  switch (this->cnt) {
  case 0:
    frm_dst_addr(47, 40) = word.some.data;
    this->cnt = 1;
    return NOTHING;
    break;
  case 1:
    frm_dst_addr(39, 32) = word.some.data;
    this->cnt = 2;
    return NOTHING;
    break;
  case 2:
    frm_dst_addr(31, 24) = word.some.data;
    this->cnt = 3;
    return NOTHING;
    break;
  case 3:
    frm_dst_addr(23, 16) = word.some.data;
    this->cnt = 4;
    return NOTHING;
    break;
  case 4:
    frm_dst_addr(15, 8) = word.some.data;
    this->cnt = 5;
    return NOTHING;
    break;
  case 5:
    frm_dst_addr(7, 0) = word.some.data;
    this->cnt = 6;
    return NOTHING;
    break;
  case 6:
    frm_src_addr(47, 40) = word.some.data;
    this->cnt = 7;
    return NOTHING;
    break;
  case 7:
    frm_src_addr(39, 32) = word.some.data;
    this->cnt = 8;
    return NOTHING;
    break;
  case 8:
    frm_src_addr(31, 24) = word.some.data;
    this->cnt = 9;
    return NOTHING;
    break;
  case 9:
    frm_src_addr(23, 16) = word.some.data;
    this->cnt = 10;
    return NOTHING;
    break;
  case 10:
    frm_src_addr(15, 8) = word.some.data;
    this->cnt = 11;
    return NOTHING;
    break;
  case 11:
    frm_src_addr(7, 0) = word.some.data;
    this->cnt = 12;
    return NOTHING;
    break;
  case 12:
    frm_protocol(15, 8) = word.some.data;
    this->cnt = 13;
    return NOTHING;
    break;
  case 13:
    frm_protocol(7, 0) = word.some.data;
    this->cnt = 14;
    return NOTHING;
    break;
  default:
    if (loc.mac_addr != frm_dst_addr) {
      return NOTHING;
    }
    word.some.user(47, 0) = frm_src_addr;
    switch (frm_protocol) {
    case IPv4:
      return this->ipPacketHandler.get_payload(word, loc, bad_data);
      break;
    default:
      return NOTHING;
    }
    break;
  }
}

void EthDataHandler::reset() {
  this->ipPacketHandler.reset();
  this->cnt = 0;
}
