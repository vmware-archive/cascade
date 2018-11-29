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

#ifndef CASCADE_SRC_VERILOG_AST_LOCALPARAM_DECLARATION_H
#define CASCADE_SRC_VERILOG_AST_LOCALPARAM_DECLARATION_H

#include <cassert>
#include "src/verilog/ast/types/declaration.h"
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/range_expression.h"

namespace cascade {

class LocalparamDeclaration : public Declaration {
  public:
    // Constructors:
    LocalparamDeclaration(Attributes* attrs__, bool signed__, RangeExpression* dim__, Identifier* id__, Expression* val__);
    ~LocalparamDeclaration() override;

    // Node Interface:
    NODE(LocalparamDeclaration, TREE(attrs), LEAF(signed), MAYBE(dim), TREE(id), TREE(val))
    // Get/Set:
    LEAF_GET_SET(signed)
    MAYBE_GET_SET(RangeExpression*, dim)
    TREE_GET_SET(val)

  private:
    MAYBE_ATTR(RangeExpression*, dim);
    LEAF_ATTR(bool, signed);
    TREE_ATTR(Expression*, val);
};

inline LocalparamDeclaration::LocalparamDeclaration(Attributes* attrs__, bool signed__, RangeExpression* dim__, Identifier* id__, Expression* val__) : Declaration() {
  parent_ = nullptr;
  TREE_SETUP(attrs);
  LEAF_SETUP(signed);
  MAYBE_SETUP(dim);
  TREE_SETUP(id);
  TREE_SETUP(val);
}

inline LocalparamDeclaration::~LocalparamDeclaration() {
  TREE_TEARDOWN(attrs);
  LEAF_TEARDOWN(signed);
  MAYBE_TEARDOWN(dim);
  TREE_TEARDOWN(id);
  TREE_TEARDOWN(val);
}

} // namespace cascade 

#endif
