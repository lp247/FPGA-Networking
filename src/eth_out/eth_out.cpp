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

#include "eth_out.hpp"

struct Meta {
  ap_uint<16> payload_checksum;
  ap_uint<11> payload_length;
  ap_uint<48> dst_mac_addr;
  ap_uint<32> dst_ip_addr;
  ap_uint<16> dst_udp_port;
};

void calculate_length_checksum(hls::stream<axis_word> &data_in,
                               hls::stream<axis_word> &buffer,
                               hls::stream<Meta> &meta_buffer) {
#pragma HLS INLINE

  static ap_uint<11> byte_cnt = 0;
  static Checksum checksum;

  if (!data_in.empty()) {
    axis_word tmp = data_in.read();
    buffer.write(tmp);
    checksum.add(tmp.data, byte_cnt % 2 == 0);
    byte_cnt++;
    if (tmp.last) {
      meta_buffer.write({checksum.accu(),
                         byte_cnt,
                         tmp.user(47, 0),
                         tmp.user(79, 48),
                         tmp.user(95, 80)});
      byte_cnt = 0;
      checksum.reset();
    }
  }
}

template <int I>
axis_word counted(const ap_uint<8> &data, ap_uint<I> &cnt, bool last = false) {
#pragma HLS INLINE
  cnt++;
  return {data, last, 0};
}

class PayloadWordGenerator {
public:
  PayloadWordGenerator(const ap_uint<6> &min_payload_size)
      : word_cnt(0), min_payload_size(min_payload_size), state(READING_BUFFER) {
  }
  axis_word get_next_word(hls::stream<axis_word> &buffer) {
#pragma HLS INLINE
    axis_word word;
    switch (state) {
    case READING_BUFFER:
      buffer.read(word);
      if (word.last) {
        if (word_cnt >= min_payload_size - 1) {
          return counted(word.data, word_cnt, true);
        }
        state = FILLING_OUTPUT;
      }
      return counted(word.data, word_cnt);
      break;
    case FILLING_OUTPUT:
      return counted(0, word_cnt, word_cnt == min_payload_size - 1);
      break;
    default:
      return {true, 0, 0};
      break;
    }
  }
  void reset() {
    word_cnt = 0;
    state = READING_BUFFER;
  }

private:
  ap_uint<5> word_cnt;
  ap_uint<6> min_payload_size;
  enum state_type { READING_BUFFER, FILLING_OUTPUT };
  state_type state;
};

class UDPPacketWordGenerator {
public:
  UDPPacketWordGenerator()
      : word_cnt(0), udp_pkt_total_length(0), payloadWordGenerator(18) {}
  axis_word get_next_word(const Addresses &loc,
                          const Meta &meta,
                          hls::stream<axis_word> &buffer) {
#pragma HLS INLINE
    switch (word_cnt) {
    case 0:
      if (meta.payload_length < MIN_UDP_PAYLOAD_BYTE_SIZE) {
        udp_pkt_total_length = MIN_UDP_PKT_BYTE_SIZE;
      } else {
        udp_pkt_total_length = meta.payload_length + UDP_PKT_HEADER_BYTE_SIZE;
      }
      udp_checksum1 = Checksum(loc.ip_addr(31, 16));
      udp_checksum2 = Checksum(meta.dst_ip_addr(31, 16));
      return counted(loc.udp_port(15, 8), word_cnt);
      break;
    case 1:
      udp_checksum1.add(loc.ip_addr(15, 0));
      udp_checksum2.add(meta.dst_ip_addr(15, 0));
      return counted(loc.udp_port(7, 0), word_cnt);
      break;
    case 2:
      udp_checksum1.add(udp_pkt_total_length);
      udp_checksum2.add(udp_pkt_total_length);
      return counted(meta.dst_udp_port(15, 8), word_cnt);
      break;
    case 3:
      udp_checksum1.add(loc.udp_port);
      udp_checksum2.add(meta.dst_udp_port);
      return counted(meta.dst_udp_port(7, 0), word_cnt);
      break;
    case 4:
      udp_checksum1.add(0x0011);
      udp_checksum2.add(meta.payload_checksum);
      return counted(udp_pkt_total_length(10, 8), word_cnt);
      break;
    case 5:
      udp_checksum1.add(udp_checksum2.accu());
      return counted(udp_pkt_total_length(7, 0), word_cnt);
      break;
    case 6:
      return counted(udp_checksum1.get()(15, 8), word_cnt);
      break;
    case 7:
      return counted(udp_checksum1.get()(7, 0), word_cnt);
      break;
    default:
      return payloadWordGenerator.get_next_word(buffer);
      break;
    }
  }
  void reset() {
    word_cnt = 0;
    udp_checksum1.reset();
    udp_checksum2.reset();
    payloadWordGenerator.reset();
  }

private:
  ap_uint<5> word_cnt;
  ap_uint<16> udp_pkt_total_length;
  Checksum udp_checksum1;
  Checksum udp_checksum2;
  PayloadWordGenerator payloadWordGenerator;
};

class IPPacketWordGenerator {
public:
  IPPacketWordGenerator()
      : word_cnt(0), ip_pkt_protocol(0), ip_pkt_total_length(0),
        ip_hop_count_and_protocol(0) {}
  axis_word get_next_word(const Addresses &loc,
                          const Meta &meta,
                          hls::stream<axis_word> &buffer) {
#pragma HLS INLINE
    switch (word_cnt) {
    case 0:
      ip_checksum.add(0x45, true);
      ip_pkt_protocol = UDP;
      if (meta.payload_length < MIN_UDP_PAYLOAD_BYTE_SIZE) {
        ip_pkt_total_length = MIN_IP_PKT_BYTE_SIZE;
      } else {
        ip_pkt_total_length = meta.payload_length + IP_PKT_HEADER_BYTE_SIZE +
                              UDP_PKT_HEADER_BYTE_SIZE;
      }
      ip_hop_count_and_protocol = (IP_HOP_COUNT << 8) + ip_pkt_protocol;
      return counted(0x45, word_cnt);
      break;
    case 1: // DSCP + ECN
      ip_checksum.add(loc.ip_addr(31, 16));
      return counted(0, word_cnt);
      break;
    case 2: // Packet total length high byte
      ip_checksum.add(loc.ip_addr(15, 0));
      return counted(ip_pkt_total_length(10, 8), word_cnt);
      break;
    case 3: // Packet total length low byte
      ip_checksum.add(ip_pkt_total_length);
      return counted(ip_pkt_total_length(7, 0), word_cnt);
      break;
    case 4: // ID high byte
      ip_checksum.add(meta.dst_ip_addr(31, 16));
      return counted(0, word_cnt);
      break;
    case 5: // ID low byte
      ip_checksum.add(meta.dst_ip_addr(15, 0));
      return counted(0, word_cnt);
      break;
    case 6: // Flags + fragment offset highest 3 bits
      return counted(0x40, word_cnt);
      break;
    case 7: // Fragment offset low byte
      ip_checksum.add(0x4000);
      return counted(0, word_cnt);
      break;
    case 8: // Hop count
      return counted(IP_HOP_COUNT, word_cnt);
      break;
    case 9: // IP protocol
      ip_checksum.add(ip_hop_count_and_protocol);
      return counted(ip_pkt_protocol, word_cnt);
      break;
    case 10: // IP header checksum high byte
      return counted(ip_checksum.get()(15, 8), word_cnt);
      break;
    case 11: // IP header checksum low byte
      return counted(ip_checksum.get()(7, 0), word_cnt);
      break;
    case 12:
      return counted(loc.ip_addr(31, 24), word_cnt);
      break;
    case 13:
      return counted(loc.ip_addr(23, 16), word_cnt);
      break;
    case 14:
      return counted(loc.ip_addr(15, 8), word_cnt);
      break;
    case 15:
      return counted(loc.ip_addr(7, 0), word_cnt);
      break;
    case 16:
      return counted(meta.dst_ip_addr(31, 24), word_cnt);
      break;
    case 17:
      return counted(meta.dst_ip_addr(23, 16), word_cnt);
      break;
    case 18:
      return counted(meta.dst_ip_addr(15, 8), word_cnt);
      break;
    case 19:
      return counted(meta.dst_ip_addr(7, 0), word_cnt);
      break;
    default:
      switch (ip_pkt_protocol) {
      case UDP:
        return udpPacketWordGenerator.get_next_word(loc, meta, buffer);
        break;
      default:
        return {true, 0, 0};
        break;
      }
      break;
    }
  }
  void reset() {
    word_cnt = 0;
    ip_checksum.reset();
    udpPacketWordGenerator.reset();
  }

private:
  ap_uint<32> word_cnt;
  Checksum ip_checksum;
  ap_uint<8> ip_pkt_protocol;
  ap_uint<11> ip_pkt_total_length;
  ap_uint<16> ip_hop_count_and_protocol;
  UDPPacketWordGenerator udpPacketWordGenerator;
};

class ETHPacketWordGenerator {
public:
  ETHPacketWordGenerator() : word_cnt(0), frm_protocol(IPv4) {}
  axis_word get_next_word(const Addresses &loc,
                          const Meta &meta,
                          hls::stream<axis_word> &buffer) {
#pragma HLS INLINE
    switch (word_cnt) {
    case 0:
      return counted(meta.dst_mac_addr(7, 0), word_cnt);
      break;
    case 1:
      return counted(meta.dst_mac_addr(15, 8), word_cnt);
      break;
    case 2:
      return counted(meta.dst_mac_addr(23, 16), word_cnt);
      break;
    case 3:
      return counted(meta.dst_mac_addr(31, 24), word_cnt);
      break;
    case 4:
      return counted(meta.dst_mac_addr(39, 32), word_cnt);
      break;
    case 5:
      return counted(meta.dst_mac_addr(47, 40), word_cnt);
      break;
    case 6:
      return counted(loc.mac_addr(7, 0), word_cnt);
      break;
    case 7:
      return counted(loc.mac_addr(15, 8), word_cnt);
      break;
    case 8:
      return counted(loc.mac_addr(23, 16), word_cnt);
      break;
    case 9:
      return counted(loc.mac_addr(31, 24), word_cnt);
      break;
    case 10:
      return counted(loc.mac_addr(39, 32), word_cnt);
      break;
    case 11:
      return counted(loc.mac_addr(47, 40), word_cnt);
      break;
    case 12:
      return counted(frm_protocol(15, 8), word_cnt);
      break;
    case 13:
      return counted(frm_protocol(7, 0), word_cnt);
      break;
    default:
      switch (frm_protocol) {
      case IPv4:
        return ipPacketWordGenerator.get_next_word(loc, meta, buffer);
        break;
      default:
        return {true, 0, 0};
        break;
      }
      break;
    }
  }
  void reset() {
    word_cnt = 0;
    ipPacketWordGenerator.reset();
  }

private:
  ap_uint<5> word_cnt;
  ap_uint<16> frm_protocol;
  IPPacketWordGenerator ipPacketWordGenerator;
};

class FCSWordGenerator {
public:
  FCSWordGenerator() : word_cnt(0) {}
  axis_word get_next_word() {
#pragma HLS INLINE
    switch (word_cnt) {
    case 0:
      return counted(fcs(31, 24), word_cnt);
      break;
    case 1:
      return counted(fcs(23, 16), word_cnt);
      break;
    case 2:
      return counted(fcs(15, 8), word_cnt);
      break;
    case 3:
      return counted(fcs(7, 0), word_cnt, true);
      break;
    default:
      return {true, 0, 0};
      break;
    }
  }
  void add_to_fcs(const ap_uint<8> &word) { fcs.add(word); }
  void reset() {
    word_cnt = 0;
    fcs.reset();
  }

private:
  ap_uint<2> word_cnt;
  CRC32 fcs;
};

class PreambleWordGenerator {
public:
  PreambleWordGenerator() : word_cnt(0) {}
  axis_word get_next_word() {
#pragma HLS INLINE
    if (word_cnt == 7) {
      return counted(0xEA, word_cnt, true);
    }
    return counted(0xAA, word_cnt);
  }
  void reset() { word_cnt = 0; }

private:
  ap_uint<3> word_cnt;
};

class DataWordGenerator {
public:
  DataWordGenerator() : state(PREAMBLE) {}
  axis_word get_next_word(const Addresses &loc,
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
  void maintenance() {
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
  void reset() {
    preambleWordGenerator.reset();
    ethPacketWordGenerator.reset();
    fcsWordGenerator.reset();
    state = PREAMBLE;
  }

private:
  enum state_type {
    PREAMBLE,
    DATA,
    FCS,
  };
  state_type state;
  PreambleWordGenerator preambleWordGenerator;
  ETHPacketWordGenerator ethPacketWordGenerator;
  FCSWordGenerator fcsWordGenerator;
  axis_word word;
};

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

void send_data(ap_uint<2> &txd,
               ap_uint<1> &txen,
               hls::stream<axis_word> &buffer,
               hls::stream<Meta> &meta_buffer,
               const Addresses &loc) {
#pragma HLS INLINE

  enum state_type { IDLE, SENDING_PACKET, WAITING_FOR_INTER_PACKAGE_GAP };
  static state_type state = IDLE;
  static Meta meta;
  static axis_word word;
  static ap_uint<2> data_bit_pair_cnt = 0;
  static ap_uint<7> ipg_cnt = 0;
  static DataWordGenerator dataWordGenerator;

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

void eth_out(hls::stream<axis_word> &data_in,
             ap_uint<2> &txd,
             ap_uint<1> &txen,
             const Addresses &loc) {
#pragma HLS INTERFACE axis port = data_in

  static hls::stream<axis_word> buffer;
#pragma HLS STREAM variable = buffer depth = 1500
  static hls::stream<Meta> meta_buffer;
#pragma HLS STREAM variable = meta_buffer depth = 6

  calculate_length_checksum(data_in, buffer, meta_buffer);
  send_data(txd, txen, buffer, meta_buffer, loc);
}
