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

#ifndef CASCADE_SRC_VERILOG_AST_ID_H
#define CASCADE_SRC_VERILOG_AST_ID_H

#include <cassert>
#include <string>
#include "src/base/token/tokenize.h"
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/maybe.h"
#include "src/verilog/ast/types/node.h"
#include "src/verilog/ast/types/string.h"

namespace cascade {

class Id : public Node {
  public:
    // Constructors:
    Id(const std::string& sid__, Maybe<Expression>* isel__);
    Id(Tokenize::Token sid__, Maybe<Expression>* isel__);
    ~Id() override;

    // Node Interface:
    NODE(Id, LEAF(sid), TREE(isel))
    // Get/Set:
    LEAF_GET_SET(sid)
    TREE_GET_SET(isel)
    // Additional Get/Set:
    const std::string& get_readable_sid();
    const std::string& get_readable_sid() const;
    void set_sid(const std::string& sid);
    void assign_sid(const std::string& sid);

    // Comparison Operators:
    bool eq(const std::string& rhs) const;
    bool eq(const String* rhs) const;

  private:
    LEAF_ATTR(Tokenize::Token, sid);
    TREE_ATTR(Maybe<Expression>*, isel);
};

inline Id::Id(const std::string& sid__, Maybe<Expression>* isel__) : Id(Tokenize().map(sid__), isel__) { }

inline Id::Id(Tokenize::Token sid__, Maybe<Expression>* isel__) : Node() {
  parent_ = nullptr;
  LEAF_SETUP(sid);
  TREE_SETUP(isel);
}

inline Id::~Id() {
  LEAF_TEARDOWN(sid);
  TREE_TEARDOWN(isel);
}

inline const std::string& Id::get_readable_sid() {
  return Tokenize().unmap(sid_);
}

inline const std::string& Id::get_readable_sid() const {
  return Tokenize().unmap(sid_);
}

inline void Id::set_sid(const std::string& sid) {
  sid_ = Tokenize().map(sid);
}

inline void Id::assign_sid(const std::string& sid) {
  sid_ = Tokenize().map(sid);
}

inline bool Id::eq(const std::string& rhs) const {
  return sid_ == Tokenize().map(rhs) && isel_->null();
}

inline bool Id::eq(const String* rhs) const {
  return sid_ == rhs->get_val() && isel_->null();
}

} // namespace cascade 

#endif
