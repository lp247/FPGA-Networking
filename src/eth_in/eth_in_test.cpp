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

#include "../utils/Addresses.hpp"
#include "../utils/axis_word.hpp"
#include "../utils/test/ITest.hpp"
#include "../utils/test/InputValueFeed.hpp"
#include "../utils/test/OutputStreamStore.hpp"
#include "../utils/test/UDPFrame.hpp"
#include "eth_in.hpp"
#include <ap_int.h>
#include <vector>

template <int L> class EthInTest : public ITest {
public:
  InputValueFeed<ap_uint<2>, L> rxd_feed;
  InputValueFeed<ap_uint<1>, L> rxerr_feed;
  InputValueFeed<ap_uint<1>, L> crsdv_feed;
  OutputStreamStore<axis_word> data_out_store;
  Addresses loc;
  EthInTest(const std::string &title,
            const std::vector<ap_uint<2> > &rxd_tv,
            const std::vector<ap_uint<1> > &rxerr_tv,
            const std::vector<ap_uint<1> > &crsdv_tv,
            const std::vector<TimedValue<axis_word> > &data_out_tv,
            const Addresses &loc)
      : ITest(title), rxd_feed(rxd_tv, 0), rxerr_feed(rxerr_tv, 0),
        crsdv_feed(crsdv_tv, 0), data_out_store("DATA", data_out_tv, 1),
        loc(loc) {}
  void feed_inputs(int step_index) override {
    this->rxd_feed.feed(step_index);
    this->rxerr_feed.feed(step_index);
    this->crsdv_feed.feed(step_index);
  }
  void store_outputs(int step_index) override {
    this->data_out_store.store(step_index);
  }

private:
  std::vector<Comparison> get_comparisons() override {
    return {this->data_out_store.get_comparison()};
  }
};

int main() {
  const int NUM_CYCLES = 400;
  std::vector<EthInTest<NUM_CYCLES> > tests;
  int errors = 0;

  const Addresses loc{0xbbbbbbbbbbbb, 0x22222222, 0x0035};
  const Addresses src{0x999999999999, 0x11111111, 0xde60};

  const Addresses real_loc{0xaaaaaaaaaaaa, 0xa9fecd01, 0x0035};
  const Addresses real_src{0xd89ef3fbcf15, 0xa9fecda9, 0xe5b8};
  Frame real({0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xd8, 0x9e, 0xf3, 0xfb, 0xcf,
              0x15, 0x08, 0x00, 0x45, 0x00, 0x00, 0x1d, 0x48, 0xeb, 0x00, 0x00,
              0x80, 0x11, 0x03, 0x3d, 0xa9, 0xfe, 0xcd, 0xa9, 0xa9, 0xfe, 0xcd,
              0x01, 0xe5, 0xb8, 0x00, 0x35, 0x00, 0x09, 0x81, 0x45, 0xaa, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x1e, 0x84, 0xd5});
  tests.push_back({"Real world example packet",
                   real,
                   {},
                   std::vector<ap_uint<1> >(288, 1),
                   {{288, {0xaa, true, real_src}}},
                   real_loc});

  tests.push_back({"Normal packet",
                   UDPFrame(src, loc, {0xaa}),
                   {},
                   std::vector<ap_uint<1> >(288, 1),
                   {{288, {0xaa, true, src}}},
                   loc});

  std::vector<ap_uint<2> > rxd_delay = UDPFrame(src, loc, {0xaa});
  rxd_delay.insert(rxd_delay.begin(), {0, 0, 0, 0});
  tests.push_back({"Normal packet - delayed rxd",
                   rxd_delay,
                   {},
                   std::vector<ap_uint<1> >(292, 1),
                   {{292, {0xaa, true, src}}},
                   loc});

  std::vector<ap_uint<2> > rxd_wrong_fcs(rxd_delay);
  rxd_wrong_fcs[254].b_not();
  tests.push_back({"Wrong frame check sequence - delayed rxd",
                   rxd_wrong_fcs,
                   {},
                   std::vector<ap_uint<1> >(292, 1),
                   {},
                   loc});

  const Addresses dst_wrong_mac = {0xbbbbbbbbbbbc, 0x22222222, 0x0035};
  std::vector<ap_uint<2> > rxd_wrong_mac = UDPFrame(src, dst_wrong_mac, {0xaa});
  rxd_wrong_mac.insert(rxd_wrong_mac.begin(), {0, 0, 0, 0});
  tests.push_back({"Wrong destination mac address",
                   rxd_wrong_mac,
                   {},
                   std::vector<ap_uint<1> >(292, 1),
                   {},
                   loc});

  const Addresses dst_wrong_ip = {0xbbbbbbbbbbbb, 0x22222223, 0x0035};
  std::vector<ap_uint<2> > rxd_wrong_ip = UDPFrame(src, dst_wrong_ip, {0xaa});
  rxd_wrong_ip.insert(rxd_wrong_ip.begin(), {0, 0, 0, 0});
  tests.push_back({"Wrong destination ip address",
                   rxd_wrong_ip,
                   {},
                   std::vector<ap_uint<1> >(292, 1),
                   {},
                   loc});

  const Addresses dst_wrong_udp = {0xbbbbbbbbbbbb, 0x22222222, 0x0040};
  std::vector<ap_uint<2> > rxd_wrong_udp = UDPFrame(src, dst_wrong_udp, {0xaa});
  tests.push_back({"Wrong destination udp port",
                   rxd_wrong_udp,
                   {},
                   std::vector<ap_uint<1> >(292, 1),
                   {},
                   loc});

  for (int i = 0; i < tests.size(); i++) {
    for (int j = 0; j < NUM_CYCLES; j++) {
      tests[i].feed_inputs(j);
      eth_in(tests[i].rxd_feed.value,
             tests[i].rxerr_feed.value,
             tests[i].crsdv_feed.value,
             tests[i].data_out_store.stream,
             tests[i].loc);
      tests[i].store_outputs(j);
    }
    errors += tests[i].get_result();
  }
  return errors;
}
