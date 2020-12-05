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

#include "DataSender.hpp"

void write_data_bit_pair(const axis_word &word,
                         ap_uint<2> &data_bit_pair_cnt,
                         ap_uint<2> &txd,
                         ap_uint<1> &txen) {
#pragma HLS INLINE

  txd = word.data(2 * data_bit_pair_cnt + 1, 2 * data_bit_pair_cnt);
  txen = true;
  data_bit_pair_cnt++;
}

void write_idle_bit_pair(ap_uint<2> &txd, ap_uint<1> &txen) {
#pragma HLS INLINE

  txd = 0;
  txen = false;
}

void DataSender::handle(ap_uint<2> &txd,
                        ap_uint<1> &txen,
                        hls::stream<axis_word> &buffer,
                        hls::stream<Meta> &meta_buffer,
                        const Addresses &loc) {
#pragma HLS INLINE

  switch (state) {
  case IDLE:
    if (!meta_buffer.empty()) {
      meta = meta_buffer.read();
      state = SENDING_PACKET;
      word = dataWordGenerator.get_next_word(loc, meta, buffer);
      write_data_bit_pair(word, data_bit_pair_cnt, txd, txen);
    } else {
      write_idle_bit_pair(txd, txen);
    }
    break;
  case SENDING_PACKET:
    switch (data_bit_pair_cnt) {
    case 0:
      word = dataWordGenerator.get_next_word(loc, meta, buffer);
      break;
    case 1:
      dataWordGenerator.maintenance();
      break;
    case 3:
      if (word.last) {
        dataWordGenerator.reset();
        state = WAITING_FOR_INTER_PACKAGE_GAP;
      }
      break;
    default:
      break;
    }
    write_data_bit_pair(word, data_bit_pair_cnt, txd, txen);
    break;
  case WAITING_FOR_INTER_PACKAGE_GAP:
    if (ipg_cnt < 95) {
      ipg_cnt++;
    } else {
      ipg_cnt = 0;
      state = IDLE;
    }
    write_idle_bit_pair(txd, txen);
    break;
  }
}
