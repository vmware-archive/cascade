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

#ifndef CASCADE_SRC_VERILOG_BUILD_AST_BUILDER_H
#define CASCADE_SRC_VERILOG_BUILD_AST_BUILDER_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "verilog/ast/ast.h"
#include "verilog/parse/parser.h"

namespace cascade {

template <typename T>
class AstBuilder : public std::ostream {
  public:
    typedef typename std::vector<T*>::iterator iterator;

    AstBuilder();
    AstBuilder(const std::string& s);
    ~AstBuilder() override = default;

    // Discards all previous results and parses everything which has been
    // passed to this stream since the last invocation of begin(). Ownership of
    // any resulting objects passes on to the caller.  On error, begin == end.
    iterator begin();
    // Returns a pointer to the end of the result range.
    iterator end();

    // Invokes begin, and returns the first element or nullptr if none exist.
    // All other elements are deleted.
    T* get();

  private:
    std::stringbuf sb_;
    std::vector<T*> res_;
};

using NodeBuilder = AstBuilder<Node>;
using DeclBuilder = AstBuilder<ModuleDeclaration>;
using ItemBuilder = AstBuilder<ModuleItem>;

template <typename T>
inline AstBuilder<T>::AstBuilder() : std::ostream(&sb_), sb_() { }

template <typename T>
inline AstBuilder<T>::AstBuilder(const std::string& s) : AstBuilder() {
  sb_.str(s);
}

template <typename T>
inline typename AstBuilder<T>::iterator AstBuilder<T>::begin() {
  // Clear any previous results
  res_.clear();
  // Create a new log and parser
  Log log;
  Parser parser(&log);

  // Read from this stream until error or end of file
  for (auto done = false; !done; ) { 
    std::istream is(&sb_);
    done = parser.parse(is) || log.error();
    if (!log.error()) {
      for (auto* n : parser) {
        res_.push_back(static_cast<T*>(n));
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

template <typename T>
inline typename  AstBuilder<T>::iterator AstBuilder<T>::end() {
  return res_.end();
}

template <typename T>
inline T* AstBuilder<T>::get() {
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

} // namespace caascade

#endif
