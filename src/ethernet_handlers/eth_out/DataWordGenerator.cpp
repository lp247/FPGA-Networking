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

#include "DataWordGenerator.hpp"

axis_word DataWordGenerator::get_next_word(const Addresses &loc,
                                           const Meta &meta,
                                           hls::stream<axis_word> &buffer) {
#pragma HLS INLINE

  switch (state) {
  case PREAMBLE:
    word = preambleWordGenerator.get_next_word();
    return {word.data, false, 0};
    break;
  case DATA:
    word = ethPacketWordGenerator.get_next_word(loc, meta, buffer);
    return {word.data, false, 0};
    break;
  case FCS:
    word = fcsWordGenerator.get_next_word();
    return word;
    break;
  default:
    return {true, 0, 0};
    break;
  }
}

void DataWordGenerator::maintenance() {
#pragma HLS INLINE
  switch (state) {
  case PREAMBLE:
    if (word.last) {
      state = DATA;
    }
    break;
  case DATA:
    fcsWordGenerator.add_to_fcs(word.data);
    if (word.last) {
      state = FCS;
    }
    break;
  case FCS:
    if (word.last) {
      state = PREAMBLE;
    }
    break;
  default:
    break;
  }
}

void DataWordGenerator::reset() {
  preambleWordGenerator.reset();
  ethPacketWordGenerator.reset();
  fcsWordGenerator.reset();
  state = PREAMBLE;
}
