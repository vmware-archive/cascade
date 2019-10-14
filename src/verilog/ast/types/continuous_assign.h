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

#ifndef CASCADE_SRC_VERILOG_AST_CONTINUOUS_ASSIGN_H
#define CASCADE_SRC_VERILOG_AST_CONTINUOUS_ASSIGN_H

#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/identifier.h"
#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/module_item.h"

namespace cascade {

class ContinuousAssign : public ModuleItem {
  public:
    // Constructors:
    explicit ContinuousAssign(Identifier* id__, Expression* rhs__);
    template <typename LhsItr>
    ContinuousAssign(LhsItr lhs_begin__, LhsItr lhs_end__, Expression* rhs__);
    ~ContinuousAssign() override;

    // Node Interface:
    NODE(ContinuousAssign)
    ContinuousAssign* clone() const override;

    // Get/Set:
    MANY_GET_SET(ContinuousAssign, Identifier, lhs)
    PTR_GET_SET(ContinuousAssign, Expression, rhs)

    // Extended Get/Set:
    // 
    // TODO(eschkufz) These methods are deprecated, and will be supported only
    // until we complete the transition to supporting concatenations on the
    // left hand side of continuous assigns.
    Identifier* get_lhs();
    const Identifier* get_lhs() const;

  private:
    MANY_ATTR(Identifier, lhs);
    PTR_ATTR(Expression, rhs);

    explicit ContinuousAssign(Expression* rhs__);
};

inline ContinuousAssign::ContinuousAssign(Identifier* id__, Expression* rhs__) : ContinuousAssign(rhs__) {
  push_back_lhs(id__);
}

template <typename LhsItr>
inline ContinuousAssign::ContinuousAssign(LhsItr lhs_begin__, LhsItr lhs_end__, Expression* rhs__) : ContinuousAssign(rhs__) {
  MANY_SETUP(lhs); 
}

inline ContinuousAssign::~ContinuousAssign() {
  MANY_TEARDOWN(lhs);
  PTR_TEARDOWN(rhs);
}

inline ContinuousAssign* ContinuousAssign::clone() const {
  auto* res = new ContinuousAssign(rhs_->clone());
  MANY_CLONE(lhs);
  return res;
}

inline Identifier* ContinuousAssign::get_lhs() {
  assert(size_lhs() == 1);
  return front_lhs();
}

inline const Identifier* ContinuousAssign::get_lhs() const {
  assert(lhs_size() == 1);
  return front_lhs();
}

inline ContinuousAssign::ContinuousAssign(Expression* rhs__) : ModuleItem(Node::Tag::continuous_assign) {
  MANY_DEFAULT_SETUP(lhs);
  PTR_SETUP(rhs);
  parent_ = nullptr;
}

} // namespace cascade 

#endif
