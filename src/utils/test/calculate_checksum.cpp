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

#include "calculate_checksum.hpp"

void extended_to_even_byte_num(std::vector<ap_uint<8> > &bytes) {
  if (bytes.size() % 2 != 0) {
    bytes.push_back(0);
  }
}

std::vector<ap_uint<16> > group_to_byte_pairs(std::vector<ap_uint<8> > &bytes) {
  std::vector<ap_uint<16> > byte_pairs;
  for (int i = 0; i < bytes.size() / 2; i++) {
    ap_uint<16> byte_pair;
    byte_pair(15, 8) = bytes[2 * i];
    byte_pair(7, 0) = bytes[2 * i + 1];
    byte_pairs.push_back(byte_pair);
  }
  return byte_pairs;
}

ap_uint<32> sum(std::vector<ap_uint<16> > &byte_pairs) {
  ap_uint<32> ret = 0;
  for (auto v : byte_pairs) {
    ret += v;
  }
  return ret;
}

ap_uint<16> calculate_checksum(std::vector<ap_uint<8> > bytes) {
  extended_to_even_byte_num(bytes);
  std::vector<ap_uint<16> > byte_pairs = group_to_byte_pairs(bytes);
  ap_uint<32> summed = sum(byte_pairs);
  ap_uint<16> folded = summed(31, 16) + summed(15, 0);
  ap_uint<16> inverted = folded ^ 0xFFFF;
  return inverted;
}
