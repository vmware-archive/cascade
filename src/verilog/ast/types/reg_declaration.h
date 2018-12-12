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

#ifndef CASCADE_SRC_VERILOG_AST_REG_DECLARATION_H
#define CASCADE_SRC_VERILOG_AST_REG_DECLARATION_H

#include <cassert>
#include "src/verilog/ast/types/declaration.h"
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/maybe.h"
#include "src/verilog/ast/types/range_expression.h"

namespace cascade {

class RegDeclaration : public Declaration {
  public:
    // Constructors:
    RegDeclaration(Attributes* attrs__, Identifier* id__, bool signed__, Maybe<RangeExpression>* dim__, Maybe<Expression>* val__);
    ~RegDeclaration() override;

    // Node Interface:
    NODE(RegDeclaration, TREE(attrs), TREE(id), LEAF(signed), TREE(dim), TREE(val))
    // Get/Set:
    LEAF_GET_SET(signed)
    TREE_GET_SET(dim)
    TREE_GET_SET(val)

  private:
    LEAF_ATTR(bool, signed);
    TREE_ATTR(Maybe<RangeExpression>*, dim);
    TREE_ATTR(Maybe<Expression>*, val);
};

inline RegDeclaration::RegDeclaration(Attributes* attrs__, Identifier* id__, bool signed__, Maybe<RangeExpression>* dim__, Maybe<Expression>* val__) : Declaration() {
  parent_ = nullptr;
  TREE_SETUP(attrs);
  TREE_SETUP(id);
  LEAF_SETUP(signed);
  TREE_SETUP(dim);
  TREE_SETUP(val);
}

inline RegDeclaration::~RegDeclaration() {
  TREE_TEARDOWN(attrs);
  TREE_TEARDOWN(id);
  LEAF_TEARDOWN(signed);
  TREE_TEARDOWN(dim);
  TREE_TEARDOWN(val);
}

} // namespace cascade 

#endif
