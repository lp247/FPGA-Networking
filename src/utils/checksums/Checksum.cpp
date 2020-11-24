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

#include "Checksum.hpp"

ap_uint<16> Checksum::get_value() const {
  ap_uint<16> ret = accumulator(26, 16) + accumulator(15, 0);
  ret.b_not();
  return ret;
}

void Checksum::add(const ap_uint<16> &next) { this->accumulator += next; }

void Checksum::add(const Checksum &other) {
  this->accumulator += other.accumulator;
}

void Checksum::add_half(const ap_uint<8> &next) {
  ap_uint<16> full_next = next;
  if (this->next_byte_high) {
    full_next <<= 8;
  }
  this->add(full_next);
  this->next_byte_high = !this->next_byte_high;
}

void Checksum::reset() {
  IChecksum::reset();
  this->next_byte_high = true;
}

ap_uint<16> Checksum::operator()(int high, int low) const {
  ap_uint<16> checksum = this->get_value();
  return checksum(high, low);
}

ap_uint<1> Checksum::operator==(const ap_uint<16> &other) const {
  ap_uint<16> checksum = this->get_value();
  return checksum == other;
}

ap_uint<1> Checksum::operator!=(const ap_uint<16> &other) const {
  ap_uint<16> checksum = this->get_value();
  return checksum != other;
}
