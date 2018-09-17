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

#ifndef CASCADE_SRC_VERILOG_AST_RANGE_EXPRESSION_H
#define CASCADE_SRC_VERILOG_AST_RANGE_EXPRESSION_H

#include <cassert>
#include <sstream>
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"

namespace cascade {

class RangeExpression : public Expression {
  public:
    // Supporting Concepts:
    enum Type {
      CONSTANT = 0,
      PLUS,
      MINUS
    };

    // Constructors:
    RangeExpression(size_t i__, size_t j__ = 0);
    RangeExpression(Expression* upper__, Type type__, Expression* lower__);
    ~RangeExpression() override;

    // Node Interface:
    NODE(RangeExpression, TREE(upper), LEAF(type), TREE(lower))
    // Get/Set:
    TREE_GET_SET(upper)
    LEAF_GET_SET(type)
    TREE_GET_SET(lower)

  private:
    TREE_ATTR(Expression*, upper);
    LEAF_ATTR(Type, type);
    TREE_ATTR(Expression*, lower);
};

inline RangeExpression::RangeExpression(size_t i__, size_t j__) {
  std::stringstream ssu;
  ssu << i__-1;
  std::stringstream ssl;
  ssl << j__;

  upper_ = new Number(ssu.str(), Number::UNSIGNED, 32);
  type_ = RangeExpression::CONSTANT;
  lower_ = new Number(ssl.str(), Number::UNSIGNED, 32);
}

inline RangeExpression::RangeExpression(Expression* upper__, Type type__, Expression* lower__) : Expression() {
  parent_ = nullptr;
  TREE_SETUP(upper);
  LEAF_SETUP(type);
  TREE_SETUP(lower);
}

inline RangeExpression::~RangeExpression() {
  TREE_TEARDOWN(upper);
  LEAF_TEARDOWN(type);
  TREE_TEARDOWN(lower);
}

} // namespace cascade 

#endif
