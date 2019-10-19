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

#include "verilog/transform/assign_unpack.h"

using namespace std;

namespace cascade {

AssignUnpack::AssignUnpack() : Rewriter() { 
  next_id_ = 0;
}

void AssignUnpack::run(ModuleDeclaration* md) {
  md->accept(this);
  if (next_id_ == 0) {
    return;
  }

  for (auto i = decls_.begin(), ie = decls_.end(); i != ie; ++i) {
    md->push_front_items(*i);
  }
  for (auto i = cas_.begin(), ie = cas_.end(); i != ie; ++i) {
    md->push_back_items(*i);
  }
  Resolve().invalidate(md);
  Navigate(md).invalidate();
  ModuleInfo(md).invalidate();
}

ModuleItem* AssignUnpack::rewrite(ContinuousAssign* ca) {
  // Nothing to do if this isn't a pack
  if (ca->size_lhs() == 1) {
    return ca;
  }

  // Create a new pack variable and populate the rhs array
  unpack(ca->begin_lhs(), ca->end_lhs(), ca->get_rhs());
  
  // Create new continuous assignments for each lhs/rhs pair
  size_t idx = 0;
  for (auto i = ca->begin_lhs(), ie = ca->end_lhs(); i != ie; ++i) {
    cas_.push_back(new ContinuousAssign((*i)->clone(), rhs_[idx++]));
  }
  rhs_.clear();

  // We have to leave one continuous assign hehind here
  auto* res = cas_.back();
  cas_.pop_back();
  return res;
}

Statement* AssignUnpack::rewrite(BlockingAssign* ba) {
  // Nothing to do if this isn't a pack
  if (ba->size_lhs() == 1) {
    return ba;
  }

  // Create a new pack variable and populate the rhs array
  unpack(ba->begin_lhs(), ba->end_lhs(), ba->get_rhs());
  
  // Create new assignments for each lhs/rhs pair
  auto* res = new SeqBlock();
  size_t idx = 0;
  for (auto i = ba->begin_lhs(), ie = ba->end_lhs(); i != ie; ++i) {
    res->push_back_stmts(new BlockingAssign((*i)->clone(), rhs_[idx++]));
  }
  rhs_.clear();
  return res;
}

Statement* AssignUnpack::rewrite(NonblockingAssign* na) {
  if (na->size_lhs() == 1) {
    return na;
  }

  // Create a new pack variable and populate the rhs array
  unpack(na->begin_lhs(), na->end_lhs(), na->get_rhs());
  
  // Create new assignments for each lhs/rhs pair
  auto* res = new SeqBlock();
  size_t idx = 0;
  for (auto i = na->begin_lhs(), ie = na->end_lhs(); i != ie; ++i) {
    res->push_back_stmts(new NonblockingAssign((*i)->clone(), rhs_[idx++]));
  }
  rhs_.clear();
  return res;
}

} // namespace cascade
