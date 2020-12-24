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

#ifndef AXIS_WORD_HPP
#define AXIS_WORD_HPP
#pragma once

#include "Addresses.hpp"
#include <ap_int.h>
#include <iostream>

struct axis_word {
  ap_uint<8> data;
  ap_uint<1> last;
  ap_uint<96> user;
  axis_word() : data(0), last(false), user(0) {}
  axis_word(const ap_uint<8> &data,
            const ap_uint<1> &last,
            const ap_uint<96> &user)
      : data(data), last(last), user(user) {}
  axis_word(const ap_uint<8> &data,
            const ap_uint<1> &last,
            const Addresses &addr)
      : data(data), last(last), user(to_user(addr)) {}
  bool operator==(const axis_word other) const {
    return this->data == other.data && this->last == other.last &&
           this->user == other.user;
  }
  bool operator!=(const axis_word other) const {
    return (this->data != other.data) || (this->last != other.last) ||
           (this->user != other.user);
  }
  static ap_uint<96> to_user(const Addresses &addr) {
    ap_uint<96> ret;
    ret(95, 80) = addr.udp_port;
    ret(79, 48) = addr.ip_addr;
    ret(47, 0) = addr.mac_addr;
    return ret;
  }
};

std::ostream &operator<<(std::ostream &os, const axis_word &word);

#endif
