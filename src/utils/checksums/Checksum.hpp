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

#ifndef CHECKSUMS_CHECKSUM_HPP
#define CHECKSUMS_CHECKSUM_HPP
#pragma once

#include "IChecksum.hpp"
#include <ap_int.h>

// log2((16^4 - 1) * 1536) = 26,5... = 27 Maximum number of bits of maximum sum
class Checksum : public IChecksum<16, 27> {
public:
  Checksum() : IChecksum(), next_byte_high(true){};
  Checksum(const ap_uint<27> &accu) : IChecksum(accu), next_byte_high(true) {}
  ap_uint<16> get_value() const;
  void add(const ap_uint<16> &next);
  void add(const Checksum &other);
  void add_half(const ap_uint<8> &next);
  void reset();
  ap_uint<16> operator()(int high, int low) const;
  ap_uint<1> operator==(const ap_uint<16> &other) const;
  ap_uint<1> operator!=(const ap_uint<16> &other) const;

private:
  void ensure();
  ap_uint<1> next_byte_high;
};

#endif
