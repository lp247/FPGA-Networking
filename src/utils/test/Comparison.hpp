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

#ifndef TEST_COMPARISON_HPP
#define TEST_COMPARISON_HPP
#pragma once

#include "../constants.hpp"
#include <ap_int.h>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

void handle_group_boundary(std::stringstream &ss, int index, int group_size) {
  if ((index - group_size + 1) % group_size == 0) {
    ss << " ";
  }
}

template <typename T>
std::string get_colored_vector_string(const std::vector<T> &v,
                                      const std::vector<std::string> &colors,
                                      int group_size) {
  std::stringstream ss;
  for (int i = 0; i < v.size(); i++) {
    std::cout << std::hex;
    ss << colors[i] << std::hex << v[i] << std::dec << FG_WHITE;
    std::cout << std::dec;
    handle_group_boundary(ss, i, group_size);
  }
  std::string ret = ss.str();
  ret = std::regex_replace(ret, std::regex("0x"), "");
  return ret;
}

class Comparison {
public:
  int shifts_to_match;
  template <typename T>
  Comparison(const std::string &name,
             const std::vector<T> &refs,
             const std::vector<T> &values,
             int group_size)
      : name(name), shifts_to_match(get_shifts_to_match(values, refs)),
        refs(get_refs_string(refs, group_size)),
        values(get_values_string(values, refs, group_size)) {}
  bool matches() { return this->shifts_to_match > -1; }
  int get_latency() { return this->shifts_to_match; }
  void print() {
    std::cout << BG_GRAY << this->name << " - REFERENCE:" << COLOR_RESET << " "
              << this->refs << std::endl
              << BG_GRAY << this->name << " - VALUES:   " << COLOR_RESET << " "
              << this->values << std::endl;
  }

private:
  std::string name;
  std::string refs;
  std::string values;
  template <typename T>
  static std::string get_refs_string(const std::vector<T> &refs,
                                     int group_size) {
    std::vector<std::string> colors(refs.size(), FG_WHITE);
    return get_colored_vector_string(refs, colors, group_size);
  }
  template <typename T>
  static std::string get_values_string(const std::vector<T> &values,
                                       const std::vector<T> &refs,
                                       int group_size) {
    if (values.size() == 0)
      return "";

    std::vector<std::string> colors;
    if (refs.size() == 0) {
      colors = std::vector<std::string>(values.size(), FG_RED);
    } else {
      for (int i = 0; i < std::max(values.size(), refs.size()); i++) {
        bool bad_values_index = i > values.size() - 1;
        bool bad_refs_index = i > refs.size() - 1;
        if (bad_values_index || bad_refs_index || values[i] != refs[i]) {
          colors.push_back(FG_RED);
        } else {
          colors.push_back(FG_WHITE);
        }
      }
    }
    return get_colored_vector_string(values, colors, group_size);
  }
  template <typename T>
  static int get_shifts_to_match(const std::vector<T> &values,
                                 const std::vector<T> &refs) {
    if (values == refs)
      return 0;

    if (values.size() != refs.size())
      return -1;

    for (int num_shifts = 1; num_shifts < refs.size(); num_shifts++) {
      bool equal = true;
      for (int i = 0; i < values.size(); i++) {
        int v_index = (i + num_shifts) % values.size();
        equal = equal && values[v_index] == refs[i];
      }
      if (equal) {
        return num_shifts;
      }
    }

    return -1;
  }
};

#endif
