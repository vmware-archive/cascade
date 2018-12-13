// Copyright 2017-2019 VMware, Inc.
// SPDX-License-Identifier: BSD-2-Clause
//
// The BSD-2 license (the License) set forth below applies to all parts of the
// Cascade project.  You may not use this file except in compliance with the
// License.
//
// BSD-2 License
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/target/state.h"

using namespace std;

namespace cascade {

size_t State::deserialize(istream& is) {
  state_.clear();

  // How many elements are in this state?
  uint32_t n = 0;
  is.read((char*)&n, 4);
  size_t res = 4;

  // Read that many id / bit pairs
  for (size_t i = 0; i < n; ++i) {
    VId id; 
    is.read((char*)&id, 4);
    res += 4;

    uint32_t arity = 0;
    is.read((char*)&arity, 4);
    res += 4;

    for (size_t i = 0; i < arity; ++i) {
      Bits bits;
      res += bits.deserialize(is);
      state_[id].push_back(bits);
    }
  }
  return res;
}

size_t State::serialize(ostream& os) const {
  // How many elements are in this state?
  const uint32_t n = state_.size();
  os.write((char*)&n, 4);
  size_t res = 4;

  // Write that many id / bit pairs
  for (const auto& s : state_) {
    os.write((char*)&s.first, 4);
    res += 4;
    
    const uint32_t arity = s.second.size();
    os.write((char*)&arity, 4);
    res += 4; 
           
    for (const auto& b : s.second) {
      res += b.serialize(os);
    }
  }
  return res;
}

} // namespace cascade

