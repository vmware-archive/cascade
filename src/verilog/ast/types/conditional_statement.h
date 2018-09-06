// Copyright 2017-2018 VMware, Inc.
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

#ifndef CASCADE_SRC_VERILOG_AST_CONDITIONAL_STATEMENT_H
#define CASCADE_SRC_VERILOG_AST_CONDITIONAL_STATEMENT_H

#include <cassert>
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/statement.h"

namespace cascade {

class ConditionalStatement : public Statement {
  public:
    // Constructors:
    ConditionalStatement(Expression* if__, Statement* then__, Statement* else__);
    ~ConditionalStatement() override;

    // Node Interface:
    NODE(ConditionalStatement, TREE(if), TREE(then), TREE(else))
    // Get/Set:
    TREE_GET_SET(if)
    TREE_GET_SET(then)
    TREE_GET_SET(else)

  private:
    TREE_ATTR(Expression*, if);
    TREE_ATTR(Statement*, then);
    TREE_ATTR(Statement*, else);
};

inline ConditionalStatement::ConditionalStatement(Expression* if__, Statement* then__, Statement* else__) : Statement() {
  parent_ = nullptr;
  TREE_SETUP(if);
  TREE_SETUP(then);
  TREE_SETUP(else);
}

inline ConditionalStatement::~ConditionalStatement() {
  TREE_TEARDOWN(if);
  TREE_TEARDOWN(then);
  TREE_TEARDOWN(else);
}

} // namespace cascade 

#endif
