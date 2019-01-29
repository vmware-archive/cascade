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

#include "src/verilog/analyze/constant.h"

#include "src/verilog/analyze/resolve.h"

namespace cascade {

Constant::Constant() : Visitor() { }

bool Constant::is_static_constant(const Expression* e) {
  res_ = true;
  genvar_ok_ = false;
  e->accept(this);
  return res_;
}

bool Constant::is_genvar_constant(const Expression* e) {
  res_ = true;
  genvar_ok_ = true;
  e->accept(this);
  return res_;
}

void Constant::visit(const Identifier* i) {
  Visitor::visit(i);

  const auto* r = Resolve().get_resolution(i);
  if (r == nullptr) {
    res_ = false;
    return;
  } 


  const auto* p = r->get_parent();
  const auto is_param = p->is(Node::Tag::parameter_declaration);
  const auto is_localparam = p->is(Node::Tag::localparam_declaration);
  if (is_param || is_localparam) {
    return;
  }
  const auto is_genvar = p->is(Node::Tag::genvar_declaration);
  if (is_genvar && genvar_ok_) { 
    return;
  }

  res_ = false;
}

} // namespace cascade
