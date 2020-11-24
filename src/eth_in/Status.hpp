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

#ifndef STATUS_HPP
#define STATUS_HPP
#pragma once

#include <ap_int.h>

const ap_uint<8> ERROR_RXERR = 0;
const ap_uint<8> ERROR_BAD_FCS = 1;
const ap_uint<8> ERROR_BAD_MAC_ADDRESS = 2;
const ap_uint<8> ERROR_BAD_ETH_PROTOCOL = 3;
const ap_uint<8> ERROR_BAD_IP_ADDRESS = 4;
const ap_uint<8> ERROR_BAD_IP_PROTOCOL = 5;
const ap_uint<8> ERROR_BAD_UDP_PORT = 6;
const ap_uint<8> ERROR_BAD_UDP_CHECKSUM = 7;

const ap_uint<8> STATUS_IDLE = 0;
const ap_uint<8> STATUS_CHECKING_PREAMBLE = 1;

struct Status {
public:
  Status() : error(0), state(0), _has_no_error(true) {}
  ap_uint<1> has_no_error() const { return this->_has_no_error; }
  ap_uint<8> get_error() const { return this->error; }
  void set_rxerr() {
    this->error[ERROR_RXERR] = true;
    this->_has_no_error = false;
  }
  void set_bad_fcs() {
    this->error[ERROR_BAD_FCS] = true;
    this->_has_no_error = false;
  }
  void set_bad_mac_addr() {
    this->error[ERROR_BAD_MAC_ADDRESS] = true;
    this->_has_no_error = false;
  }
  void set_bad_eth_protocol() {
    this->error[ERROR_BAD_ETH_PROTOCOL] = true;
    this->_has_no_error = false;
  }
  void set_bad_ip_addr() {
    this->error[ERROR_BAD_IP_ADDRESS] = true;
    this->_has_no_error = false;
  }
  void set_bad_ip_protocol() {
    this->error[ERROR_BAD_IP_PROTOCOL] = true;
    this->_has_no_error = false;
  }
  void set_bad_udp_port() {
    this->error[ERROR_BAD_UDP_PORT] = true;
    this->_has_no_error = false;
  }
  void set_bad_udp_checksum() {
    this->error[ERROR_BAD_UDP_CHECKSUM] = true;
    this->_has_no_error = false;
  }
  ap_uint<8> get_state() const { return this->state; }
  void set_state(const ap_uint<8> &state) { this->state = state; }
  void reset() {
    this->error = 0;
    this->state = STATUS_CHECKING_PREAMBLE;
  }

private:
  ap_uint<8> error;
  ap_uint<1> _has_no_error;
  ap_uint<8> state;
};

#endif
