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

#include "eth_in.hpp"

void eth_in(const ap_uint<2> &rxd,
            const ap_uint<1> &rxerr,
            const ap_uint<1> &crsdv,
            hls::stream<axis_word> &data_out,
            const Addresses &loc,
            Status &status) {
#pragma HLS INTERFACE axis port = data_out
#pragma HLS PIPELINE II = 1

  static DataSpotter dataSpotter;
  static DataBundler dataBundler;
  static FCSValidator fcsValidator;
  static EthDataHandler ethDataHandler;
  static DataGate dataGate;
  static hls::stream<axis_word> data_buffer;
#pragma HLS STREAM variable = data_buffer depth = 1500
  static hls::stream<ap_uint<1> > valid_buffer;
#pragma HLS STREAM variable = valid_buffer depth = 6
  Optional<axis_word> bundled_word;
  Optional<axis_word> validated_word;
  Optional<axis_word> payload_word;
  ap_uint<1> is_end;

  dataSpotter.next(rxd, crsdv);
  if (dataSpotter.spotted() || dataSpotter.spotted_before()) {
    bundled_word = dataBundler.bundle(rxd, rxerr, crsdv, status);
    validated_word = fcsValidator.validate(bundled_word, status);
    payload_word = ethDataHandler.get_payload(validated_word, loc, status);
    if (payload_word.is_valid) {
      data_buffer.write(payload_word.value);
    }
    if (bundled_word.value.last) { // Last only set if is valid word
      valid_buffer.write(status.has_no_error());
    }
  } else {
    status.reset();
    dataBundler.reset();
    fcsValidator.reset();
    ethDataHandler.reset();
  }
  dataGate.handle(data_buffer, valid_buffer, data_out);
}
