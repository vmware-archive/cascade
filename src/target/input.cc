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

#include "src/target/input.h"

using namespace std;

namespace cascade {

size_t Input::deserialize(istream& is) {
  input_.clear();

  // How many elements are in this input?
  uint32_t n = 0;
  is.read(reinterpret_cast<char*>(&n), 4);
  size_t res = 4;

  // Read that many id / bit pairs
  for (size_t i = 0; i < n; ++i) {
    VId id; 
    Bits bits;
    is.read(reinterpret_cast<char*>(&id), 4);
    res += (4 + bits.deserialize(is));
    input_.insert(make_pair(id, bits));
  }
  return res;
}

size_t Input::serialize(ostream& os) const {
  // How many elements are in this input?
  uint32_t n = input_.size();
  os.write(reinterpret_cast<char*>(&n), 4);
  size_t res = 4;

  // Write that many id / bit pairs
  for (const auto& i : input_) {
    os.write(const_cast<char*>(reinterpret_cast<const char*>(&i.first)), 4);
    res += (4 + i.second.serialize(os));
  }
  return res;
}

} // namespace cascade
