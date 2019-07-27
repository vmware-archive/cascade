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

#ifndef CASCADE_SRC_COMMON_TOKENIZE_H
#define CASCADE_SRC_COMMON_TOKENIZE_H

#include <cassert>
#include <limits>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace cascade {

// This class is used to represent the overhead of keeping track of string
// variables by replacing them with integer tokens. 

class Tokenize {
  public:
    // Typedefs:
    typedef uint32_t Token;

    // Map/Unmap:
    Token map(const std::string& s);
    const std::string& unmap(Token t);

  private:
    static std::mutex lock_;
    static std::vector<std::string> t2s_;
    static std::unordered_map<std::string, Token> s2t_;
};

inline Tokenize::Token Tokenize::map(const std::string& s) {
  std::lock_guard<std::mutex> lg(lock_);
  const auto res = s2t_.insert(make_pair(s, t2s_.size()));
  if (res.second) {
    assert(t2s_.size() < std::numeric_limits<Token>::max());
    t2s_.push_back(s);
  }
  return res.first->second;
}

inline const std::string& Tokenize::unmap(Token t) {
  assert(t < t2s_.size());
  std::lock_guard<std::mutex> lg(lock_);
  return t2s_[t];
}

} // namespace cascade

#endif
