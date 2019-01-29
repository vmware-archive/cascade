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

#ifndef CASCADE_SRC_VERILOG_AST_LOOP_GENERATE_CONSTRUCT_H
#define CASCADE_SRC_VERILOG_AST_LOOP_GENERATE_CONSTRUCT_H

#include "src/base/container/vector.h"
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/generate_block.h"
#include "src/verilog/ast/types/generate_construct.h"
#include "src/verilog/ast/types/loop_statement.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/variable_assign.h"

namespace cascade {

class LoopGenerateConstruct : public GenerateConstruct {
  public:
    // Constructors:
    LoopGenerateConstruct(VariableAssign* init__, Expression* cond__, VariableAssign* update__, GenerateBlock* block__);
    ~LoopGenerateConstruct() override;

    // Node Interface:
    NODE(LoopGenerateConstruct)
    LoopGenerateConstruct* clone() const override;

    // Get/Set:
    PTR_GET_SET(LoopGenerateConstruct, VariableAssign, init)
    PTR_GET_SET(LoopGenerateConstruct, Expression, cond)
    PTR_GET_SET(LoopGenerateConstruct, VariableAssign, update)
    PTR_GET_SET(LoopGenerateConstruct, GenerateBlock, block)

  private:
    PTR_ATTR(VariableAssign, init);
    PTR_ATTR(Expression, cond);
    PTR_ATTR(VariableAssign, update);
    PTR_ATTR(GenerateBlock, block);

    friend class Elaborate;
    DECORATION(Vector<GenerateBlock*>, gen);
};

inline LoopGenerateConstruct::LoopGenerateConstruct(VariableAssign* init__, Expression* cond__, VariableAssign* update__, GenerateBlock* block__) : GenerateConstruct(Node::Tag::loop_generate_construct) {
  PTR_SETUP(init);
  PTR_SETUP(cond);
  PTR_SETUP(update);
  PTR_SETUP(block);
  parent_ = nullptr;
}

inline LoopGenerateConstruct::~LoopGenerateConstruct() {
  PTR_TEARDOWN(init);
  PTR_TEARDOWN(cond);
  PTR_TEARDOWN(update);
  PTR_TEARDOWN(block);
  for (auto* g : gen_) {
    delete g;
  }
}

inline LoopGenerateConstruct* LoopGenerateConstruct::clone() const {
  return new LoopGenerateConstruct(init_->clone(), cond_->clone(), update_->clone(), block_->clone());
}

} // namespace cascade 

#endif
