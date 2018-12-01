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

#ifndef CASCADE_SRC_VERILOG_AST_FOR_STATEMENT_H
#define CASCADE_SRC_VERILOG_AST_FOR_STATEMENT_H

#include <cassert>
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/loop_statement.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/statement.h"
#include "src/verilog/ast/types/variable_assign.h"

namespace cascade {

class ForStatement : public LoopStatement {
  public:
    // Constructors:
    ForStatement(VariableAssign* init__, Expression* cond__, VariableAssign* update__, Statement* stmt__);
    ~ForStatement() override;

    // Node Interface:
    NODE(ForStatement, PTR(init), PTR(cond), PTR(update), PTR(stmt))
    // Get/Set:
    PTR_GET_SET(VariableAssign*, init)
    PTR_GET_SET(Expression*, cond)
    PTR_GET_SET(VariableAssign*, update)
    PTR_GET_SET(Statement*, stmt)

  private:
    PTR_ATTR(VariableAssign*, init);
    PTR_ATTR(Expression*, cond);
    PTR_ATTR(VariableAssign*, update);
    PTR_ATTR(Statement*, stmt);
};

inline ForStatement::ForStatement(VariableAssign* init__, Expression* cond__, VariableAssign* update__, Statement* stmt__) : LoopStatement() {
  parent_ = nullptr;
  PTR_SETUP(init);
  PTR_SETUP(cond);
  PTR_SETUP(update);
  PTR_SETUP(stmt);
}

inline ForStatement::~ForStatement() {
  PTR_TEARDOWN(init);
  PTR_TEARDOWN(cond);
  PTR_TEARDOWN(update);
  PTR_TEARDOWN(stmt);
}

} // namespace cascade 

#endif
