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

#ifndef CASCADE_SRC_VERILOG_AST_CASE_ITEM_H
#define CASCADE_SRC_VERILOG_AST_CASE_ITEM_H

#include <cassert>
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/node.h"
#include "src/verilog/ast/types/many.h"
#include "src/verilog/ast/types/statement.h"

namespace cascade {

class CaseItem : public Node {
  public:
    // Constructors:
    CaseItem(Many<Expression>* exprs__, Statement* stmt__);
    ~CaseItem() override;

    // Node Interface:
    NODE(CaseItem, TREE(exprs), TREE(stmt))
    // Get/Set:
    TREE_GET_SET(exprs)
    TREE_GET_SET(stmt)

  private:
    TREE_ATTR(Many<Expression>*, exprs);
    TREE_ATTR(Statement*, stmt);
};

inline CaseItem::CaseItem(Many<Expression>* exprs__, Statement* stmt__) : Node() {
  parent_ = nullptr;
  TREE_SETUP(exprs);
  TREE_SETUP(stmt);
}

inline CaseItem::~CaseItem() {
  TREE_TEARDOWN(exprs);
  TREE_TEARDOWN(stmt);
}

} // namespace cascade 

#endif
