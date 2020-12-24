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

#ifndef TEST_ITEST_HPP
#define TEST_ITEST_HPP
#pragma once

#include "Comparison.hpp"
#include "color_codes.hpp"
#include <iostream>
#include <string>
#include <vector>

struct TestResult {
  int is_good;
  std::string note;
};

class ITest {
public:
  ITest(const std::string &title) : title(title) {}
  int get_result() {
    TestResult result = this->is_good();
    this->print_summary(result);
    return result.is_good ? 0 : 1;
  }
  virtual void feed_inputs(int step_index) = 0;
  virtual void store_outputs(int step_index) = 0;

protected:
  virtual std::vector<Comparison> get_comparisons() = 0;

private:
  std::string title;
  TestResult is_good() {
    std::vector<Comparison> comparisons = this->get_comparisons();
    int num_shifts = comparisons[0].get_latency();
    for (int i = 0; i < comparisons.size(); i++) {
      if (!comparisons[i].matches()) {
        return {false, "Mismatching ref and values"};
      }
      if (comparisons[i].get_latency() != comparisons[0].get_latency()) {
        return {false, "Mismatching latency between test outputs"};
      }
    }
    return {true, ""};
  }
  void print_summary(TestResult result) {
    if (result.is_good) {
      std::cout << FG_GREEN << this->title << ": "
                << "PASSED" << FG_WHITE;
    } else {
      std::cout << FG_RED << this->title << ": "
                << "FAILED (Reason: " << result.note << ")" << FG_WHITE;
    }
    std::vector<Comparison> comparisons = this->get_comparisons();
    if (result.is_good) {
      std::cout << " (Latency: " << comparisons[0].get_latency() << " cycles)";
    }
    std::cout << std::endl;
    for (int i = 0; i < comparisons.size(); i++) {
      if (!comparisons[i].matches()) {
        comparisons[i].print();
      }
    }
  }
};

#endif
