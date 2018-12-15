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

#ifndef CASCADE_SRC_VERILOG_AST_CASE_GENERATE_CONSTRUCT_H
#define CASCADE_SRC_VERILOG_AST_CASE_GENERATE_CONSTRUCT_H

#include "src/verilog/ast/types/case_generate_item.h"
#include "src/verilog/ast/types/conditional_generate_construct.h"
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"

namespace cascade {

class CaseGenerateConstruct : public ConditionalGenerateConstruct {
  public:
    // Constructors:
    CaseGenerateConstruct(Expression* cond__);
    template <typename ItemsItr>
    CaseGenerateConstruct(Expression* cond__, ItemsItr items_begin__, ItemsItr items_end__);
    ~CaseGenerateConstruct() override;

    // Node Interface:
    NODE(CaseGenerateConstruct)
    CaseGenerateConstruct* clone() const override;

    // Get/Set
    PTR_GET_SET(CaseGenerateConstruct, Expression, cond)
    MANY_GET_SET(CaseGenerateConstruct, CaseGenerateItem, items)

  private:
    PTR_ATTR(Expression, cond);
    MANY_ATTR(CaseGenerateItem, items);
};

inline CaseGenerateConstruct::CaseGenerateConstruct(Expression* cond__) : ConditionalGenerateConstruct() {
  PTR_SETUP(cond);
  MANY_DEFAULT_SETUP(items);
  parent_ = nullptr;
  gen_ = nullptr;
}

template <typename ItemsItr>
inline CaseGenerateConstruct::CaseGenerateConstruct(Expression* cond__, ItemsItr items_begin__, ItemsItr items_end__) : CaseGenerateConstruct(cond__) {
  MANY_SETUP(items);
}

inline CaseGenerateConstruct::~CaseGenerateConstruct() {
  PTR_TEARDOWN(cond);
  MANY_TEARDOWN(items);
  // Don't delete gen_; it points to one of items_
}

inline CaseGenerateConstruct* CaseGenerateConstruct::clone() const {
  auto res = new CaseGenerateConstruct(cond_->clone());
  MANY_CLONE(items);
  return res;
}

} // namespace cascade 

#endif
