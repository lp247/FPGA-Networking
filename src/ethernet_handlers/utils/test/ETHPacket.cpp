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

#include "ETHPacket.hpp"

ap_uint<32> calculate_fcs(std::vector<ap_uint<8> > bytes) {
  ap_uint<33> polynomial = 0x104C11DB7;
  ap_uint<33> crc = 0;
  std::vector<ap_uint<8> > modified_bytes(bytes);
  modified_bytes.insert(modified_bytes.end(), {0, 0, 0, 0});
  for (int i = 0; i < modified_bytes.size(); i++) {
    modified_bytes[i].reverse();
  }
  for (int i = 0; i < 4; i++) {
    modified_bytes[i].b_not();
  }
  for (auto byte : modified_bytes) {
    for (int i = 0; i < 8; i++) {
      crc = crc << 1;
      ap_uint<1> new_bit = (byte >> (7 - i)) & 1;
      crc[0] = new_bit;
      if ((crc[32]) == 1) {
        crc = crc ^ polynomial;
      }
    }
  }
  ap_uint<32> ret;
  ret(31, 24) = crc(7, 0);
  ret(23, 16) = crc(15, 8);
  ret(15, 8) = crc(23, 16);
  ret(7, 0) = crc(31, 24);
  ret.reverse();
  return ret ^ 0xFFFFFFFF;
}

std::vector<ap_uint<8> >
ETHPacket::compute_bytes(const Addresses src,
                         const Addresses dst,
                         const ap_uint<16> ether_type,
                         const std::vector<ap_uint<8> > payload) {
  std::vector<ap_uint<8> > header{dst.mac_addr(47, 40),
                                  dst.mac_addr(39, 32),
                                  dst.mac_addr(31, 24),
                                  dst.mac_addr(23, 16),
                                  dst.mac_addr(15, 8),
                                  dst.mac_addr(7, 0),
                                  src.mac_addr(47, 40),
                                  src.mac_addr(39, 32),
                                  src.mac_addr(31, 24),
                                  src.mac_addr(23, 16),
                                  src.mac_addr(15, 8),
                                  src.mac_addr(7, 0),
                                  ether_type(15, 8),
                                  ether_type(7, 0)};

  std::vector<ap_uint<8> > packet(header);
  packet.insert(packet.end(), payload.begin(), payload.end());
  while (packet.size() < 60) {
    packet.push_back(0);
  }

  ap_uint<32> fcs = calculate_fcs(packet);

  packet.push_back(fcs(31, 24));
  packet.push_back(fcs(23, 16));
  packet.push_back(fcs(15, 8));
  packet.push_back(fcs(7, 0));

  return packet;
}
