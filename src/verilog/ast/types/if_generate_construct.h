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

#ifndef CASCADE_SRC_VERILOG_AST_IF_GENERATE_CONSTRUCT_H
#define CASCADE_SRC_VERILOG_AST_IF_GENERATE_CONSTRUCT_H

#include "verilog/ast/types/attributes.h"
#include "verilog/ast/types/conditional_generate_construct.h"
#include "verilog/ast/types/if_generate_clause.h"
#include "verilog/ast/types/generate_block.h"
#include "verilog/ast/types/macro.h"

namespace cascade {

class IfGenerateConstruct : public ConditionalGenerateConstruct {
  public:
    // Constructors:
    explicit IfGenerateConstruct(Attributes* attrs__);
    IfGenerateConstruct(Attributes* attrs__, IfGenerateClause* clause__, GenerateBlock* else__);
    template <typename ClausesItr>
    IfGenerateConstruct(Attributes* attrs__, ClausesItr clauses_begin__, ClausesItr clauses_end__, GenerateBlock* else__);
    ~IfGenerateConstruct() override;

    // Node Interface:
    NODE(IfGenerateConstruct)
    IfGenerateConstruct* clone() const override;

    // Get/Set:
    PTR_GET_SET(IfGenerateConstruct, Attributes, attrs)
    MANY_GET_SET(IfGenerateConstruct, IfGenerateClause, clauses)
    MAYBE_GET_SET(IfGenerateConstruct, GenerateBlock, else)

  private:
    friend class Inline;
    PTR_ATTR(Attributes, attrs);
    MANY_ATTR(IfGenerateClause, clauses);
    MAYBE_ATTR(GenerateBlock, else);
};

inline IfGenerateConstruct::IfGenerateConstruct(Attributes* attrs__) : ConditionalGenerateConstruct(Node::Tag::if_generate_construct) {
  PTR_SETUP(attrs);
  MANY_DEFAULT_SETUP(clauses);
  MAYBE_DEFAULT_SETUP(else);
  parent_ = nullptr;
  gen_ = nullptr;
}

inline IfGenerateConstruct::IfGenerateConstruct(Attributes* attrs__, IfGenerateClause* clause__, GenerateBlock* else__) : IfGenerateConstruct(attrs__) { 
  push_back_clauses(clause__);
  MAYBE_SETUP(else);
}

template <typename ClausesItr>
inline IfGenerateConstruct::IfGenerateConstruct(Attributes* attrs__, ClausesItr clauses_begin__, ClausesItr clauses_end__, GenerateBlock* else__) : IfGenerateConstruct(attrs__) {
  MANY_SETUP(clauses);
  MAYBE_SETUP(else);
}

inline IfGenerateConstruct::~IfGenerateConstruct() {
  PTR_TEARDOWN(attrs);
  MANY_TEARDOWN(clauses);
  MAYBE_TEARDOWN(else);
  // Don't delete gen_; it points to then_ or else_
}

inline IfGenerateConstruct* IfGenerateConstruct::clone() const {
  auto* res = new IfGenerateConstruct(attrs_->clone());
  MANY_CLONE(clauses);
  MAYBE_CLONE(else);
  return res;
}

} // namespace cascade 

#endif
