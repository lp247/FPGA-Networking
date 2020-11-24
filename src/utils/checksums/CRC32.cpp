/*
 * Copyright [c] 2020, Peter Lehnhardt
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
 * CONSEQUENTIAL DAMAGES [INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION] HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT [INCLUDING NEGLIGENCE OR OTHERWISE]
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "CRC32.hpp"

ap_uint<32> crc32_preview(const ap_uint<8> &next,
                          const ap_uint<32> &prev_crc32,
                          const ap_uint<3> &add_cnt) {
#pragma HLS INLINE

  ap_uint<32> c = prev_crc32;
  ap_uint<8> d = next;
  if (add_cnt < 4) {
    d.b_not();
  }
  ap_uint<32> n;
  n[0] = d[6] ^ d[0] ^ c[24] ^ c[30];
  n[1] = d[7] ^ d[6] ^ d[1] ^ d[0] ^ c[24] ^ c[25] ^ c[30] ^ c[31];
  n[2] =
      d[7] ^ d[6] ^ d[2] ^ d[1] ^ d[0] ^ c[24] ^ c[25] ^ c[26] ^ c[30] ^ c[31];
  n[3] = d[7] ^ d[3] ^ d[2] ^ d[1] ^ c[25] ^ c[26] ^ c[27] ^ c[31];
  n[4] =
      d[6] ^ d[4] ^ d[3] ^ d[2] ^ d[0] ^ c[24] ^ c[26] ^ c[27] ^ c[28] ^ c[30];
  n[5] = d[7] ^ d[6] ^ d[5] ^ d[4] ^ d[3] ^ d[1] ^ d[0] ^ c[24] ^ c[25] ^
         c[27] ^ c[28] ^ c[29] ^ c[30] ^ c[31];
  n[6] = d[7] ^ d[6] ^ d[5] ^ d[4] ^ d[2] ^ d[1] ^ c[25] ^ c[26] ^ c[28] ^
         c[29] ^ c[30] ^ c[31];
  n[7] =
      d[7] ^ d[5] ^ d[3] ^ d[2] ^ d[0] ^ c[24] ^ c[26] ^ c[27] ^ c[29] ^ c[31];
  n[8] = d[4] ^ d[3] ^ d[1] ^ d[0] ^ c[0] ^ c[24] ^ c[25] ^ c[27] ^ c[28];
  n[9] = d[5] ^ d[4] ^ d[2] ^ d[1] ^ c[1] ^ c[25] ^ c[26] ^ c[28] ^ c[29];
  n[10] = d[5] ^ d[3] ^ d[2] ^ d[0] ^ c[2] ^ c[24] ^ c[26] ^ c[27] ^ c[29];
  n[11] = d[4] ^ d[3] ^ d[1] ^ d[0] ^ c[3] ^ c[24] ^ c[25] ^ c[27] ^ c[28];
  n[12] = d[6] ^ d[5] ^ d[4] ^ d[2] ^ d[1] ^ d[0] ^ c[4] ^ c[24] ^ c[25] ^
          c[26] ^ c[28] ^ c[29] ^ c[30];
  n[13] = d[7] ^ d[6] ^ d[5] ^ d[3] ^ d[2] ^ d[1] ^ c[5] ^ c[25] ^ c[26] ^
          c[27] ^ c[29] ^ c[30] ^ c[31];
  n[14] = d[7] ^ d[6] ^ d[4] ^ d[3] ^ d[2] ^ c[6] ^ c[26] ^ c[27] ^ c[28] ^
          c[30] ^ c[31];
  n[15] = d[7] ^ d[5] ^ d[4] ^ d[3] ^ c[7] ^ c[27] ^ c[28] ^ c[29] ^ c[31];
  n[16] = d[5] ^ d[4] ^ d[0] ^ c[8] ^ c[24] ^ c[28] ^ c[29];
  n[17] = d[6] ^ d[5] ^ d[1] ^ c[9] ^ c[25] ^ c[29] ^ c[30];
  n[18] = d[7] ^ d[6] ^ d[2] ^ c[10] ^ c[26] ^ c[30] ^ c[31];
  n[19] = d[7] ^ d[3] ^ c[11] ^ c[27] ^ c[31];
  n[20] = d[4] ^ c[12] ^ c[28];
  n[21] = d[5] ^ c[13] ^ c[29];
  n[22] = d[0] ^ c[14] ^ c[24];
  n[23] = d[6] ^ d[1] ^ d[0] ^ c[15] ^ c[24] ^ c[25] ^ c[30];
  n[24] = d[7] ^ d[2] ^ d[1] ^ c[16] ^ c[25] ^ c[26] ^ c[31];
  n[25] = d[3] ^ d[2] ^ c[17] ^ c[26] ^ c[27];
  n[26] = d[6] ^ d[4] ^ d[3] ^ d[0] ^ c[18] ^ c[24] ^ c[27] ^ c[28] ^ c[30];
  n[27] = d[7] ^ d[5] ^ d[4] ^ d[1] ^ c[19] ^ c[25] ^ c[28] ^ c[29] ^ c[31];
  n[28] = d[6] ^ d[5] ^ d[2] ^ c[20] ^ c[26] ^ c[29] ^ c[30];
  n[29] = d[7] ^ d[6] ^ d[3] ^ c[21] ^ c[27] ^ c[30] ^ c[31];
  n[30] = d[7] ^ d[4] ^ c[22] ^ c[28] ^ c[31];
  n[31] = d[5] ^ c[23] ^ c[29];
  return n;
}

ap_uint<32> CRC32::get_value() const {
  ap_uint<32> ret;
  ap_uint<8> nibble_0 = this->accumulator(7, 0);
  ap_uint<8> nibble_1 = this->accumulator(15, 8);
  ap_uint<8> nibble_2 = this->accumulator(23, 16);
  ap_uint<8> nibble_3 = this->accumulator(31, 24);
  nibble_0.reverse();
  nibble_1.reverse();
  nibble_2.reverse();
  nibble_3.reverse();
  ret(31, 24) = nibble_3;
  ret(23, 16) = nibble_2;
  ret(15, 8) = nibble_1;
  ret(7, 0) = nibble_0;
  ret.b_not();
  return ret;
}

void CRC32::add(const ap_uint<8> &next) {
#pragma HLS INLINE

  ap_uint<8> rev_next = next;
  rev_next.reverse();
  this->accumulator = crc32_preview(rev_next, this->accumulator, this->add_cnt);
  if (this->add_cnt < 4) {
    this->add_cnt++;
  }
}

void CRC32::reset() {
  IChecksum::reset();
  this->add_cnt = 0;
}

ap_uint<32> CRC32::operator()(int high, int low) {
  ap_uint<32> checksum = this->get_value();
  return checksum(high, low);
}

ap_uint<1> CRC32::operator==(const ap_uint<32> &value) {
  ap_uint<32> checksum = this->get_value();
  return checksum == value;
}
