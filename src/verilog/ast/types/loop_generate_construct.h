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

#ifndef CASCADE_SRC_VERILOG_AST_LOOP_GENERATE_CONSTRUCT_H
#define CASCADE_SRC_VERILOG_AST_LOOP_GENERATE_CONSTRUCT_H

#include <cassert>
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/generate_block.h"
#include "src/verilog/ast/types/generate_construct.h"
#include "src/verilog/ast/types/loop_statement.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/many.h"
#include "src/verilog/ast/types/variable_assign.h"

namespace cascade {

class LoopGenerateConstruct : public GenerateConstruct {
  public:
    // Constructors:
    LoopGenerateConstruct(VariableAssign* init__, Expression* cond__, VariableAssign* update__, GenerateBlock* block__);
    ~LoopGenerateConstruct() override;

    // Node Interface:
    NODE(LoopGenerateConstruct, TREE(init), TREE(cond), TREE(update), TREE(block))
    // Get/Set:
    TREE_GET_SET(init)
    TREE_GET_SET(cond)
    TREE_GET_SET(update)
    TREE_GET_SET(block)

  private:
    TREE_ATTR(VariableAssign*, init);
    TREE_ATTR(Expression*, cond);
    TREE_ATTR(VariableAssign*, update);
    TREE_ATTR(GenerateBlock*, block);

    friend class Elaborate;
    DECORATION(Many<GenerateBlock>*, gen);
};

inline LoopGenerateConstruct::LoopGenerateConstruct(VariableAssign* init__, Expression* cond__, VariableAssign* update__, GenerateBlock* block__) : GenerateConstruct() {
  parent_ = nullptr;
  TREE_SETUP(init);
  TREE_SETUP(cond);
  TREE_SETUP(update);
  TREE_SETUP(block);
  gen_ = nullptr;
}

inline LoopGenerateConstruct::~LoopGenerateConstruct() {
  TREE_TEARDOWN(init);
  TREE_TEARDOWN(cond);
  TREE_TEARDOWN(update);
  TREE_TEARDOWN(block);
  if (gen_ != nullptr) {
    delete gen_;
  }
}

} // namespace cascade 

#endif
