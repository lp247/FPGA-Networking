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
#include "../utils/test/Comparison.hpp"
#include "../utils/test/ITest.hpp"
#include "../utils/test/InputStreamFeed.hpp"
#include "../utils/test/OutputValueStore.hpp"
#include "../utils/test/TimedValue.hpp"
#include "../utils/test/UDPFrame.hpp"
#include "eth_out.hpp"
#include <ap_int.h>
#include <initializer_list>
#include <string>
#include <vector>

template <int L> class EthOutTest : public ITest {
public:
  InputStreamFeed<axis_word> data_in_feed;
  OutputValueStore<ap_uint<2>, L> txd_store;
  OutputValueStore<ap_uint<1>, L> txen_store;
  Addresses loc;
  EthOutTest(const std::string &title,
             const std::vector<TimedValue<axis_word> > &data_in_tv,
             const std::vector<ap_uint<2> > &txd_tv,
             const std::vector<ap_uint<1> > &txen_tv,
             const Addresses &loc)
      : ITest(title), data_in_feed(data_in_tv), txd_store("TXD", txd_tv, 0, 4),
        txen_store("TXEN", txen_tv, 0, 8), loc(loc) {}
  void feed_inputs(int step_index) override {
    this->data_in_feed.feed(step_index);
  }
  void store_outputs(int step_index) override {
    this->txd_store.store(step_index);
    this->txen_store.store(step_index);
  }

private:
  std::vector<Comparison> get_comparisons() override {
    return {this->txd_store.get_comparison(),
            this->txen_store.get_comparison()};
  }
};

int main() {
  const int NUM_CYCLES = 700;
  std::vector<EthOutTest<NUM_CYCLES> > tests;
  int errors = 0;

  const Addresses loc = {0x123456789abc, 0x13579bdf, 0xde60};
  const Addresses dst = {0xfedcba987654, 0x98765432, 0x0035};

  std::vector<ap_uint<2> > packet_d(UDPFrame(loc, dst, {0xAA}));
  std::vector<ap_uint<1> > packet_en(288, 1);
  std::vector<ap_uint<2> > ipg_d(96, 0);
  std::vector<ap_uint<1> > ipg_en(96, 0);
  std::vector<ap_uint<2> > output_d(packet_d);
  std::vector<ap_uint<1> > output_en(packet_en);
  output_d.insert(output_d.end(), ipg_d.begin(), ipg_d.end());
  output_en.insert(output_en.end(), ipg_en.begin(), ipg_en.end());
  output_d.insert(output_d.end(), packet_d.begin(), packet_d.end());
  output_en.insert(output_en.end(), packet_en.begin(), packet_en.end());
  tests.push_back({"Normal packets with IPG",
                   {{0, {0xaa, true, dst}}, {1, {0xaa, true, dst}}},
                   output_d,
                   output_en,
                   loc});

  for (int i = 0; i < tests.size(); i++) {
    for (int j = 0; j < NUM_CYCLES; j++) {
      tests[i].feed_inputs(j);
      eth_out(tests[i].data_in_feed.stream,
              tests[i].txd_store.value,
              tests[i].txen_store.value,
              loc);
      tests[i].store_outputs(j);
    }
    errors += tests[i].get_result();
  }
  return errors;
}
