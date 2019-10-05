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

#include "verilog/build/ast_builder.h"

#include "verilog/parse/parser.h"

using namespace std;

namespace cascade {

AstBuilder::AstBuilder() : ostream(&sb_), sb_() { }

AstBuilder::AstBuilder(const string& s) : AstBuilder() {
  sb_.str(s);
}

AstBuilder::iterator AstBuilder::begin() {
  // Clear any previous results
  res_.clear();
  // Create a new log and parser
  Log log;
  Parser parser(&log);

  // Read from this stream until error or end of file
  for (auto done = false; !done; ) { 
    istream is(&sb_);
    done = parser.parse(is) || log.error();
    if (!log.error()) {
      for (auto* n : parser) {
        res_.push_back(n);
      }
    }
  }
  // If we encountered a parser error, free any partial results
  if (log.error()) {
    for (auto* n : res_) {
      delete n;
    }
    res_.clear();
  }
  // Either way, clear the stream 
  sb_.str("");

  return res_.begin();
}

AstBuilder::iterator AstBuilder::end() {
  return res_.end();
}

AstBuilder::operator Node*() {
  auto itr = begin();
  if (itr == end()) {
    return nullptr;
  }
  auto* res = *itr;
  for (++itr; itr != end(); ++itr) {
    delete *itr;
  }
  return res;
}

} // namespace cascade
