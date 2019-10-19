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

#ifndef CASCADE_SRC_VERILOG_AST_VARIABLE_ASSIGN_H
#define CASCADE_SRC_VERILOG_AST_VARIABLE_ASSIGN_H

#include "verilog/ast/types/identifier.h"
#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/node.h"

namespace cascade {

class VariableAssign : public ModuleItem {
  public:
    // Constructors:
    explicit VariableAssign(Identifier* id__, Expression* rhs__);
    template <typename LhsItr>
    VariableAssign(LhsItr lhs_begin__, LhsItr lhs_end__, Expression* rhs__);
    ~VariableAssign() override;

    // Node Interface:
    NODE(VariableAssign)
    VariableAssign* clone() const override;

    // Get/Set:
    MANY_GET_SET(VariableAssign, Identifier, lhs)
    PTR_GET_SET(VariableAssign, Expression, rhs)

    // Returns front_lhs(), undefined for objects where size_lhs() != 1.
    Identifier* get_lhs();
    const Identifier* get_lhs() const;

  private:
    MANY_ATTR(Identifier, lhs);
    PTR_ATTR(Expression, rhs);

    explicit VariableAssign(Expression* rhs__);
};

inline VariableAssign::VariableAssign(Identifier* id__, Expression* rhs__) : VariableAssign(rhs__) {
  push_back_lhs(id__);
}

template <typename LhsItr>
inline VariableAssign::VariableAssign(LhsItr lhs_begin__, LhsItr lhs_end__, Expression* rhs__) : VariableAssign(rhs__) {
  MANY_SETUP(lhs); 
}

inline VariableAssign::~VariableAssign() {
  MANY_TEARDOWN(lhs);
  PTR_TEARDOWN(rhs);
}

inline VariableAssign* VariableAssign::clone() const {
  auto* res = new VariableAssign(rhs_->clone());
  MANY_CLONE(lhs);
  return res;
}

inline Identifier* VariableAssign::get_lhs() {
  assert(size_lhs() == 1);
  return front_lhs();
}

inline const Identifier* VariableAssign::get_lhs() const {
  assert(size_lhs() == 1);
  return front_lhs();
}

inline VariableAssign::VariableAssign(Expression* rhs__) : ModuleItem(Node::Tag::continuous_assign) {
  MANY_DEFAULT_SETUP(lhs);
  PTR_SETUP(rhs);
  parent_ = nullptr;
}

} // namespace cascade

#endif
