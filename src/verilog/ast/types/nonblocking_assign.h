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

#ifndef CASCADE_SRC_VERILOG_AST_NONBLOCKING_ASSIGN_H
#define CASCADE_SRC_VERILOG_AST_NONBLOCKING_ASSIGN_H

#include "verilog/ast/types/assign_statement.h"
#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/identifier.h"
#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/timing_control.h"

namespace cascade {

class NonblockingAssign : public AssignStatement {
  public:
    // Constructors:
    explicit NonblockingAssign(Identifier* lhs__, Expression* rhs__);
    NonblockingAssign(TimingControl* ctrl__, Identifier* lhs__, Expression* rhs__);
    ~NonblockingAssign() override;

    // Node Interface:
    NODE(NonblockingAssign)
    NonblockingAssign* clone() const override;

    // Get/Set:
    MAYBE_GET_SET(NonblockingAssign, TimingControl, ctrl)
    PTR_GET_SET(NonblockingAssign, Identifier, lhs)
    PTR_GET_SET(NonblockingAssign, Expression, rhs)

  private:
    MAYBE_ATTR(TimingControl, ctrl);
    PTR_ATTR(Identifier, lhs);
    PTR_ATTR(Expression, rhs);
};

inline NonblockingAssign::NonblockingAssign(Identifier* lhs__, Expression* rhs__) : AssignStatement(Node::Tag::nonblocking_assign) {
  MAYBE_DEFAULT_SETUP(ctrl);
  PTR_SETUP(lhs); 
  PTR_SETUP(rhs); 
  parent_ = nullptr;
}

inline NonblockingAssign::NonblockingAssign(TimingControl* ctrl__, Identifier* lhs__, Expression* rhs__) : NonblockingAssign(lhs__, rhs__) {
  MAYBE_SETUP(ctrl);
}

inline NonblockingAssign::~NonblockingAssign() {
  MAYBE_TEARDOWN(ctrl);
  PTR_TEARDOWN(lhs);
  PTR_TEARDOWN(rhs);
}

inline NonblockingAssign* NonblockingAssign::clone() const {
  auto* res = new NonblockingAssign(lhs_->clone(), rhs_->clone());
  MAYBE_CLONE(ctrl);
  return res;
}

} // namespace cascade 

#endif
