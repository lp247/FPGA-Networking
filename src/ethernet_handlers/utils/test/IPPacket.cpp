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

#include "IPPacket.hpp"

std::vector<ap_uint<8> >
IPPacket::compute_bytes(const Addresses &src,
                        const Addresses &dst,
                        const ap_uint<8> &ip_protocol,
                        const std::vector<ap_uint<8> > &payload,
                        ap_uint<16> id) {
  ap_uint<16> packet_length = payload.size() + 20;
  ap_uint<8> version_ihl = 0x45;

  std::vector<ap_uint<8> > header_no_checksum{version_ihl,
                                              0,
                                              packet_length(15, 8),
                                              packet_length(7, 0),
                                              id(15, 8),
                                              id(7, 0),
                                              0,
                                              0,
                                              0x80,
                                              ip_protocol,
                                              0,
                                              0,
                                              src.ip_addr(31, 24),
                                              src.ip_addr(23, 16),
                                              src.ip_addr(15, 8),
                                              src.ip_addr(7, 0),
                                              dst.ip_addr(31, 24),
                                              dst.ip_addr(23, 16),
                                              dst.ip_addr(15, 8),
                                              dst.ip_addr(7, 0)};

  std::vector<ap_uint<8> > packet(header_no_checksum);
  packet.insert(packet.end(), payload.begin(), payload.end());

  ap_uint<16> checksum = calculate_checksum(header_no_checksum);

  packet[10] = checksum(15, 8);
  packet[11] = checksum(7, 0);

  return packet;
}
