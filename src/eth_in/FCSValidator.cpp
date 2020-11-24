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

#include "FCSValidator.hpp"

Optional<axis_word> FCSValidator::validate(const Optional<axis_word> &word,
                                           Status &status) {
#pragma HLS INLINE

  if (!word.is_valid) {
    return NOTHING;
  }

  this->fcs.add(word.value.data);
  ap_uint<8> next_data = this->crc_buf.shift(word.value.data);

  if (this->shift_cnt < 4) {
    this->shift_cnt++;
    return NOTHING;
  }

  if (word.value.last &&
      this->fcs.get_accumulator() != CRC32_RESIDUE_INV_BREV) {
    status.set_bad_fcs();
  }
  
  return {{next_data, word.value.last, 0}, true};
}

void FCSValidator::reset() {
  this->shift_cnt = 0;
  this->fcs.reset();
}

ap_uint<1> FCSValidator::is_good() {
  return this->fcs.get_accumulator() == CRC32_RESIDUE_INV_BREV;
}
