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

#ifndef CASCADE_SRC_VERILOG_AST_CONDITIONAL_EXPRESSION_H
#define CASCADE_SRC_VERILOG_AST_CONDITIONAL_EXPRESSION_H

#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"

namespace cascade {

class ConditionalExpression : public Expression {
  public:
    // Constructors:
    ConditionalExpression(Expression* cond__, Expression* lhs__, Expression* rhs__);
    ~ConditionalExpression() override;

    // Node Interface:
    NODE(ConditionalExpression)
    ConditionalExpression* clone() const override;

    // Get/Set:
    PTR_GET_SET(ConditionalExpression, Expression, cond)
    PTR_GET_SET(ConditionalExpression, Expression, lhs)
    PTR_GET_SET(ConditionalExpression, Expression, rhs)

  private:
    PTR_ATTR(Expression, cond);
    PTR_ATTR(Expression, lhs);
    PTR_ATTR(Expression, rhs);
};

inline ConditionalExpression::ConditionalExpression(Expression* cond__, Expression* lhs__, Expression* rhs__) : Expression(Node::Tag::conditional_expression) {
  PTR_SETUP(cond);
  PTR_SETUP(lhs);
  PTR_SETUP(rhs);
  parent_ = nullptr;
}

inline ConditionalExpression::~ConditionalExpression() {
  PTR_TEARDOWN(cond);
  PTR_TEARDOWN(lhs);
  PTR_TEARDOWN(rhs);
}

inline ConditionalExpression* ConditionalExpression::clone() const {
  return new ConditionalExpression(cond_->clone(), lhs_->clone(), rhs_->clone());
}

} // namespace cascade 

#endif
