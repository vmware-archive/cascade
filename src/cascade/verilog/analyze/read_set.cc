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

#include "verilog/analyze/read_set.h"

#include "verilog/ast/ast.h"

namespace cascade {

ReadSet::ReadSet(const Expression* e) : Visitor() {
  lhs_ = false;
  e->accept(this);  
}

ReadSet::ReadSet(const Statement* s) : Visitor() {
  lhs_ = false;
  s->accept(this);  
}

ReadSet::const_iterator ReadSet::begin() const {
  return reads_.begin();
}

ReadSet::const_iterator ReadSet::end() const {
  return reads_.end();
}

void ReadSet::visit(const FeofExpression* fe) {
  Visitor::visit(fe);
  reads_.insert(fe);
}

void ReadSet::visit(const Identifier* id) {
  // Descend on this node regardless of which side we're on; indices are reads
  // regardless of where they appear. But don't any ids from the lhs of an
  // assignment.

  const auto prev_lhs = lhs_;
  lhs_ = false;
  id->accept_ids(this);
  id->accept_dim(this);
  lhs_ = prev_lhs;

  if (!lhs_) {
    reads_.insert(id);
  }
}

void ReadSet::visit(const BlockingAssign* ba) {
  const auto prev_lhs = lhs_;
  lhs_ = true;
  ba->accept_lhs(this);
  lhs_ = prev_lhs;
  ba->accept_rhs(this);
}

void ReadSet::visit(const NonblockingAssign* na) {
  const auto prev_lhs = lhs_;
  lhs_ = true;
  na->accept_lhs(this);
  lhs_ = prev_lhs;
  na->accept_rhs(this);
}

void ReadSet::visit(const VariableAssign* va) {
  const auto prev_lhs = lhs_;
  lhs_ = true;
  va->accept_lhs(this);
  lhs_ = prev_lhs;
  va->accept_rhs(this);
}

} // namespace cascade
