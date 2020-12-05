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
            const Addresses &loc) {
#pragma HLS INTERFACE axis port = data_out
#pragma HLS DISAGGREGATE variable = loc
#pragma HLS PIPELINE II = 1

  static DataSpotter dataSpotter;
  static DataBundler dataBundler;
  static AxisWordGenerator axisWordGenerator;
  static FCSValidator fcsValidator;
  static EthDataHandler ethDataHandler;
  static DataGate dataGate;
  static hls::stream<axis_word> data_buffer;
#pragma HLS STREAM variable = data_buffer depth = 1500
  static hls::stream<ap_uint<1> > valid_buffer;
#pragma HLS STREAM variable = valid_buffer depth = 6
  Optional<ap_uint<8> > bundled_data;
  Optional<axis_word> data_word;
  Optional<axis_word> validator_output;
  Optional<axis_word> payload;
  static ap_uint<1> bad_data = false;
  static ap_uint<1> data_written = false;

  dataSpotter.next(rxd, crsdv);
  if (dataSpotter.spotted() || dataSpotter.spotted_before()) {
    if (rxerr) {
      bad_data = true;
    }
    bundled_data = dataBundler.bundle(rxd);
    fcsValidator.add_to_fcs(bundled_data);
    data_word = axisWordGenerator.next(bundled_data, crsdv);
    validator_output = fcsValidator.validate(data_word, bad_data);
    payload = ethDataHandler.get_payload(validator_output, loc, bad_data);
    if (payload.is_some()) {
      data_buffer.write(payload.some);
      data_written = true;
    }
    if (validator_output.some.last && data_written) {
      valid_buffer.write(!bad_data);
    }
  } else {
    dataBundler.reset();
    axisWordGenerator.reset();
    fcsValidator.reset();
    ethDataHandler.reset();
    bad_data = false;
    data_written = false;
  }
  dataGate.handle(data_buffer, valid_buffer, data_out);
}
