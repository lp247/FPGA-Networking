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

#ifndef ETH_IN_HPP
#define ETH_IN_HPP
#pragma once

#include "../utils/Addresses.hpp"
#include "../utils/axis_word.hpp"
#include "../utils/checksums/CRC32.hpp"
#include "../utils/checksums/Checksum.hpp"
#include "../utils/constants.hpp"
#include <ap_int.h>
#include <ap_shift_reg.h>
#include <hls_stream.h>

template <typename T> struct Optional {
  T value;
  ap_uint<1> is_valid;
};

const ap_uint<8> ERROR_RXERR = 1;
const ap_uint<8> ERROR_BAD_FCS = 2;
const ap_uint<8> ERROR_BAD_MAC_ADDRESS = 3;
const ap_uint<8> ERROR_UNKNOWN_ETH_PROTOCOL = 4;
const ap_uint<8> ERROR_BAD_IP_ADDRESS = 5;
const ap_uint<8> ERROR_UNKNOWN_IP_PROTOCOL = 6;
const ap_uint<8> ERROR_BAD_UDP_PORT = 7;
const ap_uint<8> ERROR_BAD_UDP_CHECKSUM = 8;

const ap_uint<8> STATUS_IDLE = 0;
const ap_uint<8> STATUS_CHECKING_PREAMBLE = 1;

const Optional<axis_word> NOTHING = {{0, false, 0}, false};

struct Status {
public:
  Status() : error(0), state(0) {}
  ap_uint<1> has_error() const { return this->error > 0; }
  ap_uint<8> get_error() const { return this->error; }
  void register_error(const ap_uint<8> &error) {
    if (this->error == 0) {
      this->error = error;
    }
  }
  ap_uint<8> get_state() const { return this->state; }
  void set_state(const ap_uint<8> &state) { this->state = state; }

private:
  ap_uint<8> error;
  ap_uint<8> state;
};

void eth_in(const ap_uint<2> &rxd,
            const ap_uint<1> &rxerr,
            const ap_uint<1> &crsdv,
            hls::stream<axis_word> &data_out,
            const Addresses &loc,
            Status &status);

#endif
