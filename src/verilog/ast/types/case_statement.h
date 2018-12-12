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

#ifndef CASCADE_SRC_VERILOG_AST_CASE_STATEMENT_H
#define CASCADE_SRC_VERILOG_AST_CASE_STATEMENT_H

#include <cassert>
#include "src/verilog/ast/types/case_item.h"
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/many.h"
#include "src/verilog/ast/types/statement.h"

namespace cascade {

class CaseStatement : public Statement {
  public:
    // Supporting Concepts:
    enum Type {
      CASE = 0,
      CASEX,
      CASEZ 
    };

    // Constructors:
    CaseStatement(Type type__, Expression* cond__, Many<CaseItem>* items__);
    ~CaseStatement() override;

    // Node Interface:
    NODE(CaseStatement, LEAF(type), TREE(cond), TREE(items))
    // Get/Set:
    LEAF_GET_SET(type)
    TREE_GET_SET(cond)
    TREE_GET_SET(items)

  private:
    LEAF_ATTR(Type, type);
    TREE_ATTR(Expression*, cond);
    TREE_ATTR(Many<CaseItem>*, items);
};

inline CaseStatement::CaseStatement(Type type__, Expression* cond__, Many<CaseItem>* items__) : Statement() {
  parent_ = nullptr;
  LEAF_SETUP(type);
  TREE_SETUP(cond);
  TREE_SETUP(items);
}

inline CaseStatement::~CaseStatement() {
  LEAF_TEARDOWN(type);
  TREE_TEARDOWN(cond);
  TREE_TEARDOWN(items);
}

} // namespace cascade 

#endif
