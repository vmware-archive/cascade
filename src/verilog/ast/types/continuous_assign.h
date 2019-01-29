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

#ifndef CASCADE_SRC_VERILOG_AST_CONTINUOUS_ASSIGN_H
#define CASCADE_SRC_VERILOG_AST_CONTINUOUS_ASSIGN_H

#include "src/verilog/ast/types/delay_control.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/module_item.h"
#include "src/verilog/ast/types/variable_assign.h"

namespace cascade {

class ContinuousAssign : public ModuleItem {
  public:
    // Constructors:
    explicit ContinuousAssign(VariableAssign* assign__);
    ContinuousAssign(DelayControl* ctrl__, VariableAssign* assign__);
    ~ContinuousAssign() override;

    // Node Interface:
    NODE(ContinuousAssign)
    ContinuousAssign* clone() const override;

    // Get/Set:
    MAYBE_GET_SET(ContinuousAssign, DelayControl, ctrl)
    PTR_GET_SET(ContinuousAssign, VariableAssign, assign)

  private:
    MAYBE_ATTR(DelayControl, ctrl);
    PTR_ATTR(VariableAssign, assign);
};

inline ContinuousAssign::ContinuousAssign(VariableAssign* assign__) : ModuleItem(Node::Tag::continuous_assign) {
  MAYBE_DEFAULT_SETUP(ctrl);
  PTR_SETUP(assign);
  parent_ = nullptr;
}

inline ContinuousAssign::ContinuousAssign(DelayControl* ctrl__, VariableAssign* assign__) : ContinuousAssign(assign__) {
  MAYBE_SETUP(ctrl);
}

inline ContinuousAssign::~ContinuousAssign() {
  MAYBE_TEARDOWN(ctrl);
  PTR_TEARDOWN(assign);
}

inline ContinuousAssign* ContinuousAssign::clone() const {
  auto* res = new ContinuousAssign(assign_->clone());
  MAYBE_CLONE(ctrl);
  return res;
}

} // namespace cascade 

#endif
