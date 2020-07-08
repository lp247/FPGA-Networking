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

void pass_payload(hls::stream<axis_word> &buffer,
                  hls::stream<ap_uint<11> > &num_valid_bytes,
                  hls::stream<axis_word> &data_out) {
#pragma HLS INLINE

  static ap_uint<11> num_bytes_to_write;
  static ap_uint<1> working = false;

  if (!num_valid_bytes.empty() && !buffer.empty()) {
    working = true;
    num_bytes_to_write = num_valid_bytes.read();
  }
  if (working) {
    axis_word word = buffer.read();
    if (word.last) {
      working = false;
    }
    if (num_bytes_to_write > 0) {
      word.last = num_bytes_to_write == 1;
      data_out.write(word);
      num_bytes_to_write--;
    }
  }
}

void process_next_udp_pkt_word(const Addresses &loc,
                               Checksum &udp_checksum,
                               const axis_word &word,
                               hls::stream<axis_word> &buffer,
                               ap_uint<11> &num_valid_bytes) {
#pragma HLS INLINE

  static ap_uint<4> word_cnt = 0;
  static ap_uint<11> payload_cnt = 0;
  static ap_uint<11> valid_payload_bytes_cnt = 0;
  static ap_uint<16> udp_pkt_src_port;
  static ap_uint<16> udp_pkt_dst_port;
  static ap_uint<16> udp_pkt_length;
  static ap_uint<16> udp_pkt_checksum;

  switch (word_cnt) {
  case 0:
    udp_pkt_src_port(15, 8) = word.data;
    word_cnt = 1;
    break;
  case 1:
    udp_pkt_src_port(7, 0) = word.data;
    udp_checksum.add(udp_pkt_src_port);
    word_cnt = 2;
    break;
  case 2:
    udp_pkt_dst_port(15, 8) = word.data;
    word_cnt = 3;
    break;
  case 3:
    udp_pkt_dst_port(7, 0) = word.data;
    udp_checksum.add(udp_pkt_dst_port);
    word_cnt = 4;
    break;
  case 4:
    udp_pkt_length(15, 0) = word.data;
    udp_checksum.add(0x0011);
    word_cnt = 5;
    break;
  case 5:
    udp_pkt_length(7, 0) = word.data;
    udp_checksum.add(udp_pkt_length);
    word_cnt = 6;
    break;
  case 6:
    udp_pkt_checksum(15, 8) = word.data;
    udp_checksum.add(udp_pkt_length);
    word_cnt = 7;
    break;
  case 7:
    udp_pkt_checksum(7, 0) = word.data;
    udp_checksum.add(udp_pkt_checksum);
    word_cnt = 8;
    break;
  default:
    word.user(95, 80) = udp_pkt_src_port;
    buffer.write(word);
    udp_checksum.add(word.data, payload_cnt % 2 == 0);
    payload_cnt++;
    if (word.data != 0) {
      valid_payload_bytes_cnt = payload_cnt;
    }

    if (word.last) {
      if (loc.udp_port != udp_pkt_dst_port ||
          (udp_pkt_checksum != 0 && udp_checksum != 0)) {
        num_valid_bytes = 0;
      } else {
        num_valid_bytes = valid_payload_bytes_cnt;
      }
      word_cnt = 0;
      payload_cnt = 0;
      udp_checksum.reset();
    }
    break;
  }
}

void process_next_ip_pkt_word(const Addresses &loc,
                              const axis_word &word,
                              hls::stream<axis_word> &buffer,
                              ap_uint<11> &num_valid_bytes) {
#pragma HLS INLINE

  static ap_uint<5> word_cnt = 0;
  static ap_uint<4> ip_pkt_ihl;
  static ap_uint<8> ip_pkt_protocol;
  static ap_uint<32> ip_pkt_src_ip_addr;
  static ap_uint<32> ip_pkt_dst_ip_addr;
  static Checksum udp_checksum;

  switch (word_cnt) {
  case 0:
    ip_pkt_ihl = word.data(3, 0);
    word_cnt = 1;
    break;
  case 1:
    word_cnt = 2;
    break;
  case 2:
    word_cnt = 3;
    break;
  case 3:
    word_cnt = 4;
    break;
  case 4:
    word_cnt = 5;
    break;
  case 5:
    word_cnt = 6;
    break;
  case 6:
    word_cnt = 7;
    break;
  case 7:
    word_cnt = 8;
    break;
  case 8:
    word_cnt = 9;
    break;
  case 9:
    ip_pkt_protocol = word.data;
    word_cnt = 10;
    break;
  case 10:
    word_cnt = 11;
    break;
  case 11:
    word_cnt = 12;
    break;
  case 12:
    ip_pkt_src_ip_addr(31, 24) = word.data;
    word_cnt = 13;
    break;
  case 13:
    ip_pkt_src_ip_addr(23, 16) = word.data;
    udp_checksum.add(ip_pkt_src_ip_addr(31, 16));
    word_cnt = 14;
    break;
  case 14:
    ip_pkt_src_ip_addr(15, 8) = word.data;
    word_cnt = 15;
    break;
  case 15:
    ip_pkt_src_ip_addr(7, 0) = word.data;
    udp_checksum.add(ip_pkt_src_ip_addr(15, 0));
    word_cnt = 16;
    break;
  case 16:
    ip_pkt_dst_ip_addr(31, 24) = word.data;
    word_cnt = 17;
    break;
  case 17:
    ip_pkt_dst_ip_addr(23, 16) = word.data;
    udp_checksum.add(ip_pkt_dst_ip_addr(31, 16));
    word_cnt = 18;
    break;
  case 18:
    ip_pkt_dst_ip_addr(15, 8) = word.data;
    word_cnt = 19;
    break;
  case 19:
    ip_pkt_dst_ip_addr(7, 0) = word.data;
    udp_checksum.add(ip_pkt_dst_ip_addr(15, 0));
    word_cnt = 20;
    break;
  default:
    if (word_cnt >= ip_pkt_ihl * 4) {
      word.user(79, 48) = ip_pkt_src_ip_addr;
      switch (ip_pkt_protocol) {
      case UDP:
        process_next_udp_pkt_word(
            loc, udp_checksum, word, buffer, num_valid_bytes);
        break;
      }
    }
    if (word.last) {
      word_cnt = 0;
      if (ip_pkt_dst_ip_addr != loc.ip_addr) {
        num_valid_bytes = 0;
      }
    }
    break;
  }
}

void process_next_eth_pkt_word(const Addresses &loc,
                               const axis_word &word,
                               hls::stream<axis_word> &buffer,
                               ap_uint<11> &num_valid_bytes) {
#pragma HLS INLINE

  static ap_uint<4> word_cnt = 0;
  static ap_uint<48> frm_dst_addr;
  static ap_uint<48> frm_src_addr;
  static ap_uint<16> frm_protocol;

  switch (word_cnt) {
  case 0:
    frm_dst_addr(47, 40) = word.data;
    word_cnt = 1;
    break;
  case 1:
    frm_dst_addr(39, 32) = word.data;
    word_cnt = 2;
    break;
  case 2:
    frm_dst_addr(31, 24) = word.data;
    word_cnt = 3;
    break;
  case 3:
    frm_dst_addr(23, 16) = word.data;
    word_cnt = 4;
    break;
  case 4:
    frm_dst_addr(15, 8) = word.data;
    word_cnt = 5;
    break;
  case 5:
    frm_dst_addr(7, 0) = word.data;
    word_cnt = 6;
    break;
  case 6:
    frm_src_addr(47, 40) = word.data;
    word_cnt = 7;
    break;
  case 7:
    frm_src_addr(39, 32) = word.data;
    word_cnt = 8;
    break;
  case 8:
    frm_src_addr(31, 24) = word.data;
    word_cnt = 9;
    break;
  case 9:
    frm_src_addr(23, 16) = word.data;
    word_cnt = 10;
    break;
  case 10:
    frm_src_addr(15, 8) = word.data;
    word_cnt = 11;
    break;
  case 11:
    frm_src_addr(7, 0) = word.data;
    word_cnt = 12;
    break;
  case 12:
    frm_protocol(15, 8) = word.data;
    word_cnt = 13;
    break;
  case 13:
    frm_protocol(7, 0) = word.data;
    word_cnt = 14;
    break;
  default:
    word.user(47, 0) = frm_src_addr;
    switch (frm_protocol) {
    case IPv4:
      process_next_ip_pkt_word(loc, word, buffer, num_valid_bytes);
      break;
    }
    if (word.last) {
      word_cnt = 0;
      if (frm_dst_addr != loc.mac_addr) {
        num_valid_bytes = 0;
      }
    }
    break;
  }
}

void process_next_data_word(const Addresses &loc,
                            const axis_word &word,
                            hls::stream<axis_word> &buffer,
                            ap_uint<11> &num_valid_bytes) {
#pragma HLS INLINE

  static ap_uint<3> shift_cnt = 0;
  static ap_shift_reg<ap_uint<8>, 4> crc_buf;
  static CRC32 fcs;
  axis_word shifted_word;

  if (shift_cnt < 4) {
    shift_cnt++;
    crc_buf.shift(word.data);
  } else {
    shifted_word.data = crc_buf.shift(word.data, 3);
    shifted_word.last = word.last;
    process_next_eth_pkt_word(loc, shifted_word, buffer, num_valid_bytes);
  }
  if (!word.last) {
    fcs.add(word.data);
  } else {
    shift_cnt = 0;
    if (crc32_preview(word.data, fcs.accu(), 5) != INV_CRC32_RESIDUE) {
      num_valid_bytes = 0;
    }
    fcs.reset();
  }
}

void process_next_word(const Addresses &loc,
                       const axis_word &word,
                       hls::stream<axis_word> &buffer,
                       hls::stream<ap_uint<11> > &num_vld_bytes_buf) {
#pragma HLS INLINE

  static ap_uint<11> num_valid_bytes = 0;

  process_next_data_word(loc, word, buffer, num_valid_bytes);
  if (word.last) {
    num_vld_bytes_buf.write(num_valid_bytes);
    num_valid_bytes = 0;
  }
}

void read_pair_into_word(axis_word &word,
                         ap_uint<2> &pair_cnt,
                         const ap_uint<2> &rxd,
                         const ap_uint<1> &rxerr,
                         ap_uint<1> &bad) {
#pragma HLS INLINE

  word.data(2 * pair_cnt + 1, 2 * pair_cnt) = rxd;
  bad |= rxerr;
  pair_cnt++;
}

enum read_state_type { PREAMBLE_CHECK, PREDATA, DATA };

void handle_preamble(ap_uint<5> &cnt,
                     const ap_uint<2> &rxd,
                     read_state_type &read_state) {
#pragma HLS INLINE

  if (cnt < 31 && rxd == 2) {
    cnt++;
  } else {
    if (cnt == 31 && rxd == 3) {
      read_state = PREDATA;
    }
    cnt = 0;
  }
}

void process_mac_input(const ap_uint<2> &rxd,
                       const ap_uint<1> &rxerr,
                       const ap_uint<1> &crsdv,
                       const Addresses &loc,
                       hls::stream<axis_word> &buffer,
                       hls::stream<ap_uint<11> > &num_vld_bytes_buf) {
#pragma HLS INLINE

  static ap_uint<1> bad_rx;
  static ap_uint<2> pair_cnt = 0;
  static axis_word word;
  static read_state_type read_state = PREAMBLE_CHECK;
  static ap_uint<5> preamble_cnt = 0;

  if (pair_cnt == 0 && read_state == DATA) {
    word.last = !crsdv;
    process_next_word(loc, word, buffer, num_vld_bytes_buf);
  }
  if (crsdv) {
    switch (read_state) {
    case PREAMBLE_CHECK:
      handle_preamble(preamble_cnt, rxd, read_state);
      break;
    case PREDATA:
      read_state = DATA;
    case DATA:
      read_pair_into_word(word, pair_cnt, rxd, rxerr, bad_rx);
      break;
    default:
      break;
    }
  } else {
    read_state = PREAMBLE_CHECK;
    preamble_cnt = 0;
    bad_rx = false;
  }
}

void eth_in(const ap_uint<2> &rxd,
            const ap_uint<1> &rxerr,
            const ap_uint<1> &crsdv,
            hls::stream<axis_word> &data_out,
            const Addresses &loc) {
#pragma HLS INTERFACE axis port = data_out

  static hls::stream<axis_word> buffer;
#pragma HLS STREAM variable = buffer depth = 1500
  static hls::stream<ap_uint<11> > num_vld_bytes_buf;
#pragma HLS STREAM variable = num_vld_bytes_buf depth = 6

  process_mac_input(rxd, rxerr, crsdv, loc, buffer, num_vld_bytes_buf);
  pass_payload(buffer, num_vld_bytes_buf, data_out);
}
