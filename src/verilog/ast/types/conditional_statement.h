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

#ifndef CASCADE_SRC_VERILOG_AST_CONDITIONAL_STATEMENT_H
#define CASCADE_SRC_VERILOG_AST_CONDITIONAL_STATEMENT_H

#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/statement.h"

namespace cascade {

class ConditionalStatement : public Statement {
  public:
    // Constructors:
    ConditionalStatement(Expression* if__, Statement* then__, Statement* else__);
    ~ConditionalStatement() override;

    // Node Interface:
    NODE(ConditionalStatement)
    ConditionalStatement* clone() const override;

    // Get/Set:
    PTR_GET_SET(ConditionalStatement, Expression, if)
    PTR_GET_SET(ConditionalStatement, Statement, then)
    PTR_GET_SET(ConditionalStatement, Statement, else)

  private:
    PTR_ATTR(Expression, if);
    PTR_ATTR(Statement, then);
    PTR_ATTR(Statement, else);
};

inline ConditionalStatement::ConditionalStatement(Expression* if__, Statement* then__, Statement* else__) : Statement(Node::Tag::conditional_statement) {
  PTR_SETUP(if);
  PTR_SETUP(then);
  PTR_SETUP(else);
  parent_ = nullptr;
}

inline ConditionalStatement::~ConditionalStatement() {
  PTR_TEARDOWN(if);
  PTR_TEARDOWN(then);
  PTR_TEARDOWN(else);
}

inline ConditionalStatement* ConditionalStatement::clone() const {
  return new ConditionalStatement(if_->clone(), then_->clone(), else_->clone());
}

} // namespace cascade 

#endif
