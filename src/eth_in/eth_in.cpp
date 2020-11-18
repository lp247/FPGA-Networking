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

class DataWriter {
public:
  DataWriter() {}
  void write(const Optional<axis_word> &word, hls::stream<axis_word> &buffer) {
    if (!word.is_valid) {
      return;
    }

    buffer.write(word.value);
  }
};

class PayloadSizeCounter {
public:
  PayloadSizeCounter() : payload_cnt(0), valid_payload_bytes_cnt(0) {}
  void count(const Optional<axis_word> &word,
             hls::stream<ap_uint<11> > &num_vld_bytes_buf,
             Status &status) {
#pragma HLS INLINE

    if (!word.is_valid) {
      return;
    }

    this->payload_cnt++;
    if (word.value.data != 0) {
      this->valid_payload_bytes_cnt = this->payload_cnt;
    }
    if (word.value.last) {
      if (status.get_error() == 0) {
        num_vld_bytes_buf.write(this->valid_payload_bytes_cnt);
      } else {
        num_vld_bytes_buf.write(0);
      }
    }
  }
  void reset() {
    this->payload_cnt = 0;
    this->valid_payload_bytes_cnt = 0;
  }

private:
  ap_uint<11> payload_cnt;
  ap_uint<11> valid_payload_bytes_cnt;
};

class UDPPacketHandler {
public:
  UDPPacketHandler() : cnt(0) {}
  Optional<axis_word> get_payload(const Optional<axis_word> &word,
                                  const Addresses &loc,
                                  Status &status,
                                  const ap_uint<32> &src_ip_addr) {
#pragma HLS INLINE

    if (!word.is_valid) {
      return NOTHING;
    }

    switch (this->cnt) { // ! CHECKSUM FIX
    case 0:
      this->udp_pkt_src_port(15, 8) = word.value.data;
      this->udp_checksum1.add(loc.ip_addr(31, 16));
      this->udp_checksum2.add(loc.ip_addr(15, 0));
      this->cnt = 1;
      return NOTHING;
      break;
    case 1:
      this->udp_pkt_src_port(7, 0) = word.value.data;
      this->udp_checksum1.add(src_ip_addr(31, 16));
      this->udp_checksum2.add(src_ip_addr(15, 0));
      this->cnt = 2;
      return NOTHING;
      break;
    case 2:
      this->udp_pkt_dst_port(15, 8) = word.value.data;
      this->udp_checksum1.add(this->udp_pkt_src_port);
      this->udp_checksum2.add(0x0011);
      this->cnt = 3;
      return NOTHING;
      break;
    case 3:
      this->udp_pkt_dst_port(7, 0) = word.value.data;
      this->udp_checksum1.add(this->udp_pkt_dst_port);
      this->cnt = 4;
      return NOTHING;
      break;
    case 4:
      this->udp_pkt_length(15, 0) = word.value.data;
      this->cnt = 5;
      return NOTHING;
      break;
    case 5:
      this->udp_pkt_length(7, 0) = word.value.data;
      this->udp_checksum1.add(this->udp_pkt_length);
      this->udp_checksum2.add(this->udp_pkt_length);
      this->cnt = 6;
      return NOTHING;
      break;
    case 6:
      this->udp_pkt_checksum(15, 8) = word.value.data;
      this->udp_checksum1.add(this->udp_checksum2);
      this->cnt = 7;
      return NOTHING;
      break;
    case 7:
      this->udp_pkt_checksum(7, 0) = word.value.data;
      this->udp_checksum1.add(this->udp_pkt_checksum);
      this->cnt = 8;
      return NOTHING;
      break;
    default:
      if (loc.udp_port != udp_pkt_dst_port) {
        status.register_error(ERROR_BAD_UDP_PORT);
        return NOTHING;
      }
      word.value.user(95, 80) = this->udp_pkt_src_port;
      this->udp_checksum1.add_half(word.value.data);
      if (word.value.last && udp_pkt_checksum != 0 &&
          this->udp_checksum1 != 0) {
        status.register_error(ERROR_BAD_UDP_CHECKSUM);
      }
      return word;
      break;
    }
  }
  void reset() {
    this->udp_checksum1.reset();
    this->udp_checksum2.reset();
    this->cnt = 0;
  }

private:
  Checksum udp_checksum1;
  Checksum udp_checksum2;
  ap_uint<4> cnt;
  ap_uint<16> udp_pkt_src_port;
  ap_uint<16> udp_pkt_dst_port;
  ap_uint<16> udp_pkt_length;
  ap_uint<16> udp_pkt_checksum;
};

class IPPacketHandler {
public:
  IPPacketHandler() : cnt(0) {}
  Optional<axis_word> get_payload(const Optional<axis_word> &word,
                                  const Addresses &loc,
                                  Status &status) {
#pragma HLS INLINE

    if (!word.is_valid) {
      return NOTHING;
    }

    switch (this->cnt) {
    case 0:
      this->ip_pkt_ihl = word.value.data(3, 0);
      this->cnt = 1;
      return NOTHING;
      break;
    case 1:
      this->cnt = 2;
      return NOTHING;
      break;
    case 2:
      this->cnt = 3;
      return NOTHING;
      break;
    case 3:
      this->cnt = 4;
      return NOTHING;
      break;
    case 4:
      this->cnt = 5;
      return NOTHING;
      break;
    case 5:
      this->cnt = 6;
      return NOTHING;
      break;
    case 6:
      this->cnt = 7;
      return NOTHING;
      break;
    case 7:
      this->cnt = 8;
      return NOTHING;
      break;
    case 8:
      this->cnt = 9;
      return NOTHING;
      break;
    case 9:
      this->ip_pkt_protocol = word.value.data;
      this->cnt = 10;
      return NOTHING;
      break;
    case 10:
      this->cnt = 11;
      return NOTHING;
      break;
    case 11:
      this->cnt = 12;
      return NOTHING;
      break;
    case 12:
      this->ip_pkt_src_ip_addr(31, 24) = word.value.data;
      this->cnt = 13;
      return NOTHING;
      break;
    case 13:
      this->ip_pkt_src_ip_addr(23, 16) = word.value.data;
      this->cnt = 14;
      return NOTHING;
      break;
    case 14:
      this->ip_pkt_src_ip_addr(15, 8) = word.value.data;
      this->cnt = 15;
      return NOTHING;
      break;
    case 15:
      this->ip_pkt_src_ip_addr(7, 0) = word.value.data;
      this->cnt = 16;
      return NOTHING;
      break;
    case 16:
      this->ip_pkt_dst_ip_addr(31, 24) = word.value.data;
      this->cnt = 17;
      return NOTHING;
      break;
    case 17:
      this->ip_pkt_dst_ip_addr(23, 16) = word.value.data;
      this->cnt = 18;
      return NOTHING;
      break;
    case 18:
      this->ip_pkt_dst_ip_addr(15, 8) = word.value.data;
      this->cnt = 19;
      return NOTHING;
      break;
    case 19:
      this->ip_pkt_dst_ip_addr(7, 0) = word.value.data;
      this->cnt = 20;
      return NOTHING;
      break;
    default:
      if (loc.ip_addr != ip_pkt_dst_ip_addr) {
        status.register_error(ERROR_BAD_IP_ADDRESS);
        return NOTHING;
      }
      if (this->cnt >= this->ip_pkt_ihl * 4) {
        word.value.user(79, 48) = this->ip_pkt_src_ip_addr;
        switch (this->ip_pkt_protocol) {
        case UDP:
          return this->udpPacketHandler.get_payload(
              word, loc, status, this->ip_pkt_src_ip_addr);
          break;
        default:
          status.register_error(ERROR_UNKNOWN_IP_PROTOCOL);
          return NOTHING;
        }
      }
      return NOTHING;
      break;
    }
  }
  void reset() {
    this->udpPacketHandler.reset();
    this->cnt = 0;
  }

private:
  UDPPacketHandler udpPacketHandler;
  ap_uint<5> cnt;
  ap_uint<4> ip_pkt_ihl;
  ap_uint<8> ip_pkt_protocol;
  ap_uint<32> ip_pkt_src_ip_addr;
  ap_uint<32> ip_pkt_dst_ip_addr;
};

class EthFrameWithoutFCSHandler {
public:
  EthFrameWithoutFCSHandler() : cnt(0) {}
  Optional<axis_word> get_payload(const Optional<axis_word> &word,
                                  const Addresses &loc,
                                  Status &status) {
#pragma HLS INLINE

    if (!word.is_valid) {
      return NOTHING;
    }

    switch (this->cnt) {
    case 0:
      frm_dst_addr(47, 40) = word.value.data;
      this->cnt = 1;
      return NOTHING;
      break;
    case 1:
      frm_dst_addr(39, 32) = word.value.data;
      this->cnt = 2;
      return NOTHING;
      break;
    case 2:
      frm_dst_addr(31, 24) = word.value.data;
      this->cnt = 3;
      return NOTHING;
      break;
    case 3:
      frm_dst_addr(23, 16) = word.value.data;
      this->cnt = 4;
      return NOTHING;
      break;
    case 4:
      frm_dst_addr(15, 8) = word.value.data;
      this->cnt = 5;
      return NOTHING;
      break;
    case 5:
      frm_dst_addr(7, 0) = word.value.data;
      this->cnt = 6;
      return NOTHING;
      break;
    case 6:
      frm_src_addr(47, 40) = word.value.data;
      this->cnt = 7;
      return NOTHING;
      break;
    case 7:
      frm_src_addr(39, 32) = word.value.data;
      this->cnt = 8;
      return NOTHING;
      break;
    case 8:
      frm_src_addr(31, 24) = word.value.data;
      this->cnt = 9;
      return NOTHING;
      break;
    case 9:
      frm_src_addr(23, 16) = word.value.data;
      this->cnt = 10;
      return NOTHING;
      break;
    case 10:
      frm_src_addr(15, 8) = word.value.data;
      this->cnt = 11;
      return NOTHING;
      break;
    case 11:
      frm_src_addr(7, 0) = word.value.data;
      this->cnt = 12;
      return NOTHING;
      break;
    case 12:
      frm_protocol(15, 8) = word.value.data;
      this->cnt = 13;
      return NOTHING;
      break;
    case 13:
      frm_protocol(7, 0) = word.value.data;
      this->cnt = 14;
      return NOTHING;
      break;
    default:
      if (loc.mac_addr != frm_dst_addr) {
        status.register_error(ERROR_BAD_MAC_ADDRESS);
        return NOTHING;
      }
      word.value.user(47, 0) = frm_src_addr;
      switch (frm_protocol) {
      case IPv4:
        return this->ipPacketHandler.get_payload(word, loc, status);
        break;
      default:
        status.register_error(ERROR_UNKNOWN_ETH_PROTOCOL);
        return NOTHING;
      }
      break;
    }
  }
  void reset() {
    this->ipPacketHandler.reset();
    this->cnt = 0;
  }

private:
  IPPacketHandler ipPacketHandler;
  ap_uint<4> cnt;
  ap_uint<48> frm_dst_addr;
  ap_uint<48> frm_src_addr;
  ap_uint<16> frm_protocol;
};

class FCSValidator {
public:
  FCSValidator() : shift_cnt(0) {}
  Optional<axis_word> validate(const Optional<axis_word> &word,
                               Status &status) {
#pragma HLS INLINE

    if (!word.is_valid) {
      return NOTHING;
    }

    if (this->shift_cnt < 4) {
      this->shift_cnt++;
      this->crc_buf.shift(word.value.data);
      return NOTHING;
    }

    axis_word shifted_word;
    shifted_word.data = this->crc_buf.shift(word.value.data);
    shifted_word.last = word.value.last;
    this->fcs.add(shifted_word.data);
    if (word.value.last && !this->is_good()) {
      status.register_error(ERROR_BAD_FCS);
    }
    return {shifted_word, true};
  }
  void reset() {
#pragma HLS INLINE

    this->shift_cnt = 0;
    this->fcs.reset();
  }
  ap_uint<1> is_good() {
#pragma HLS INLINE

    return this->fcs(31, 24) == this->crc_buf.read(3) &&
           this->fcs(23, 16) == this->crc_buf.read(2) &&
           this->fcs(15, 8) == this->crc_buf.read(1) &&
           this->fcs(7, 0) == this->crc_buf.read(0);
  }

private:
  ap_uint<3> shift_cnt;
  ap_shift_reg<ap_uint<8>, 4> crc_buf;
  CRC32 fcs;
};

class DataBundler {
public:
  DataBundler() : pair_cnt(0), first_run(true) {}
  Optional<axis_word> bundle(const ap_uint<2> &rxd,
                             const ap_uint<1> &rxerr,
                             const ap_uint<1> &crsdv,
                             Status &status) {
#pragma HLS INLINE

    if (rxerr) {
      status.register_error(ERROR_RXERR);
    }

    if (this->first_run) {
      this->first_run = false;
      this->stage.shift(rxd);
      return NOTHING;
    }

    this->data(2 * pair_cnt + 1, 2 * pair_cnt) = this->stage.shift(rxd);
    this->pair_cnt++;
    if (this->pair_cnt != 0) {
      return NOTHING;
    }

    return {{data, !crsdv, 0}, true};
  }
  void reset() {
#pragma HLS INLINE

    this->pair_cnt = 0;
    this->first_run = true;
  }

private:
  ap_shift_reg<ap_uint<2>, 1> stage;
  ap_uint<8> data;
  ap_uint<2> pair_cnt;
  ap_uint<1> first_run;
};

class DataSpotter {
public:
  DataSpotter() : cnt(0), valid_data(false), state(PREAMBLE_CHECK) {}
  void next(const ap_uint<2> &rxd, const ap_uint<1> &crsdv) {
#pragma HLS INLINE

    this->state_before = this->state;
    if (!crsdv) {
      this->state = PREAMBLE_CHECK;
      this->cnt = 0;
    } else {
      switch (this->state) {
      case PREAMBLE_CHECK:
        if (rxd == 1 && this->cnt < 31) {
          this->cnt++;
        } else if (rxd == 3 && this->cnt == 31) {
          this->state = PREAMBLE_END;
        }
        break;
      case PREAMBLE_END:
        this->state = DATA;
      case DATA:
        break;
      }
    }
  }
  ap_uint<1> spotted() { return this->state == DATA; }
  ap_uint<1> spotted_before() { return this->state_before == DATA; }

private:
  enum state_type { PREAMBLE_CHECK, PREAMBLE_END, DATA };
  state_type state;
  state_type state_before;
  ap_uint<5> cnt;
  ap_uint<1> valid_data;
};

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
  static EthFrameWithoutFCSHandler ethFrameWithoutFCSHandler;
  static PayloadSizeCounter payloadSizeCounter;
  static DataWriter dataWriter;
  static hls::stream<axis_word> buffer;
#pragma HLS STREAM variable = buffer depth = 1500
  static hls::stream<ap_uint<11> > num_vld_bytes_buf;
#pragma HLS STREAM variable = num_vld_bytes_buf depth = 6

  dataSpotter.next(rxd, crsdv);
  if (dataSpotter.spotted() || dataSpotter.spotted_before()) {
    Optional<axis_word> word;
    word = dataBundler.bundle(rxd, rxerr, crsdv, status);
    word = fcsValidator.validate(word, status);
    word = ethFrameWithoutFCSHandler.get_payload(word, loc, status);
    payloadSizeCounter.count(word, num_vld_bytes_buf, status);
    dataWriter.write(word, buffer);
  } else {
    status.set_state(STATUS_CHECKING_PREAMBLE);
    dataBundler.reset();
    fcsValidator.reset();
    ethFrameWithoutFCSHandler.reset();
    payloadSizeCounter.reset();
  }
  pass_payload(buffer, num_vld_bytes_buf, data_out);
}
