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

#ifndef TEST_OUTPUT_STREAM_STORE_HPP
#define TEST_OUTPUT_STREAM_STORE_HPP
#pragma once

#include "OutputStore.hpp"
#include "StreamContainer.hpp"
#include "TimedValue.hpp"
#include <string>
#include <vector>

template <typename T>
class OutputStreamStore : public OutputStore<TimedValue<T> >,
                          public StreamContainer<T> {
public:
  OutputStreamStore(std::string name,
                    std::vector<TimedValue<T> > refs,
                    int print_group_size)
      : OutputStore<TimedValue<T> >(name, refs, print_group_size),
        StreamContainer<T>() {}
  OutputStreamStore(OutputStreamStore<T> &&other)
      : OutputStore<TimedValue<T> >(std::move(other)), StreamContainer<T>(
                                                           std::move(other)) {}
  void store(int index) override {
    if (!this->stream.empty()) {
      T value = this->stream.read();
      this->values.push_back({index, value});
    }
  }
};

#endif
