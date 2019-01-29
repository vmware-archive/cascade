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

#ifndef CASCADE_SRC_VERILOG_AST_ARG_ASSIGN_H
#define CASCADE_SRC_VERILOG_AST_ARG_ASSIGN_H

#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/identifier.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/node.h"

namespace cascade {

class ArgAssign : public Node {
  public:
    // Constructors:
    ArgAssign();
    ArgAssign(Identifier* exp__, Expression* imp__);
    ~ArgAssign() override;

    // Node Interface:
    NODE(ArgAssign)
    ArgAssign* clone() const override;

    // Get/Set
    MAYBE_GET_SET(ArgAssign, Identifier, exp)
    MAYBE_GET_SET(ArgAssign, Expression, imp)

  private:
    MAYBE_ATTR(Identifier, exp);
    MAYBE_ATTR(Expression, imp);
};

inline ArgAssign::ArgAssign() : Node(Node::Tag::arg_assign) { 
  MAYBE_DEFAULT_SETUP(exp);
  MAYBE_DEFAULT_SETUP(imp);
  parent_ = nullptr;
}

inline ArgAssign::ArgAssign(Identifier* exp__, Expression* imp__) : ArgAssign() { 
  MAYBE_SETUP(exp);
  MAYBE_SETUP(imp);
}

inline ArgAssign::~ArgAssign() {
  MAYBE_TEARDOWN(exp);
  MAYBE_TEARDOWN(imp);
}

inline ArgAssign* ArgAssign::clone() const {
  auto* res = new ArgAssign();
  MAYBE_CLONE(exp);
  MAYBE_CLONE(imp);
  return res;
}

} // namespace cascade

#endif
