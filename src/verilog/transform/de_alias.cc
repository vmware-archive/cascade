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

#include "src/verilog/transform/de_alias.h"

#include <cassert>
#include "src/verilog/analyze/constant.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

DeAlias::DeAlias() : Rewriter() { }

void DeAlias::run(ModuleDeclaration* md) {
  // Build the alias table 
  table_ = new AliasTable(md);
  // Replace aliases and delete zero-time self-assignments 
  md->accept(this);
  for (auto i = md->begin_items(); i != md->end_items(); ) {
    if (auto ca = dynamic_cast<ContinuousAssign*>(*i)) {
      if (is_self_assign(ca)) {
        i = md->purge_items(i);
        continue;
      } 
    } 
    ++i;
  }
  // Delete the table
  delete table_;

  // Invalidate cached state (we haven't added or removed any declarations so
  // there's no need to invalidate the scope tree).
  Resolve().invalidate(md);
  ModuleInfo(md).invalidate();
}

DeAlias::AliasTable::AliasTable(const ModuleDeclaration* md) : Visitor() { 
  md->accept(this);
  for (auto& a : aliases_) {
    follow(a.second);
  }
  for (auto& a : aliases_) {
    collapse(a.second);
  }
}

DeAlias::AliasTable::~AliasTable() {
  for (auto& a : aliases_) {
    assert(a.second.slices_.size() <= 1);
    if (!a.second.slices_.empty()) {
      delete a.second.slices_.back();
    }
  }
}

Identifier* DeAlias::AliasTable::dealias(const Identifier* id) {
  // Nothing to do if we can't find this identifier in the alias table
  const auto r = Resolve().get_resolution(id);
  const auto itr = aliases_.find(r);
  if (itr == aliases_.end()) {
    return nullptr;
  }

  // Otherwise, we're going to replace this variable with the target that's
  // in the alias table. Careful though, it might still have its original slice
  // attached to it.
  auto res = itr->second.target_->clone();
  if (Resolve().is_slice(itr->second.target_)) {
    assert(!res->empty_dim());
    delete res->remove_back_dim();
  }

  // If either this id or the target is a slice, we'll need one last merge.
  // Otherwise, just append the slice that's attached to id or which appears in
  // the alias table.
  const auto st = !itr->second.slices_.empty();
  const auto si = Resolve().is_slice(id);
  if (st && si) {
    res->push_back_dim(merge(itr->second.slices_.back(), id->back_dim()));
  } else if (st) {
    res->push_back_dim(itr->second.slices_.back()->clone());
  } else if (si) {
    res->push_back_dim(id->back_dim()->clone());
  }
  return res;
}

void DeAlias::AliasTable::visit(const ContinuousAssign* ca) {
  // Ignore assignments with sliced left-hand-sides
  const auto lhs = ca->get_assign()->get_lhs();
  if (Resolve().is_slice(lhs)) {
    return;
  }
  // Ignore assignments with right-hand-sides which aren't identifiers
  const auto rhs = dynamic_cast<const Identifier*>(ca->get_assign()->get_rhs());
  if (rhs == nullptr) {
    return;
  }
  // Ignore assignments which don't preserve bit-width
  if (Resolve().is_slice(rhs)) {
    if (Evaluate().get_width(lhs) != Evaluate().get_width(rhs)) {
      return;
    }
  } else {
    const auto r = Resolve().get_resolution(rhs);
    assert(r != nullptr);
    if (Evaluate().get_width(lhs) != Evaluate().get_width(r)) {
      return;
    }
  }
  // Ignore assignments with left-hand-sides that point to arrays
  const auto r = Resolve().get_resolution(lhs);
  assert(r != nullptr);
  if (Resolve().is_array(r)) {
    return;
  }
  // Ignore non-const assignments
  for (auto i = rhs->begin_dim(), ie = rhs->end_dim(); i != ie; ++i) {
    if (!Constant().is_static_constant(*i)) {
      return;
    }
  }

  // If the right-hand-side is a slice, record its bounds
  vector<const Expression*> slices;
  if (Resolve().is_slice(rhs)) {
    slices.push_back(rhs->back_dim());
  }
  // Multiple assigns to the same variable are undefined. Keep the most recent.
  aliases_[r] = {rhs, slices, false};
}

void DeAlias::AliasTable::follow(Row& row) {
  // Base Case: This row is done, just return.
  if (row.done_) {
    return;
  }
  // Base Case: We can't follow this variable any further. Close
  // out this row and return.
  const auto r = Resolve().get_resolution(row.target_);
  assert(r != nullptr);
  const auto itr = aliases_.find(r);
  if (itr == aliases_.end()) {
    row.done_ = true;
    return;
  }
  // Inductive Case: Close out the row that this variable points to.
  // We inherit its information and append the slices that we had already
  // built up.
  follow(itr->second);
  const auto slices_ = row.slices_;
  row = itr->second;
  row.slices_.insert(row.slices_.end(), slices_.begin(), slices_.end());
}

void DeAlias::AliasTable::collapse(Row& row) {
  // Base Case: No slices; just return
  if (row.slices_.empty()) {
    return;
  }
  // Base Case: One slices; replace it with a copy and return
  if (row.slices_.size() == 1) {
    row.slices_[0] = row.slices_[0]->clone();
    return;
  }
  // Inductive Case: Two or more slices:
  auto res = merge(row.slices_[0], row.slices_[1]);
  for (size_t i = 2, ie = row.slices_.size(); i < ie; ++i) {
    auto temp = merge(res, row.slices_[i]);
    delete res;
    res = temp;
  }
  row.slices_.resize(1);
  row.slices_[0] = res;
}

Expression* DeAlias::AliasTable::merge(const Expression* x, const Expression* y) {
  // Sanity Check: x can't be a single-bit, because if it were we wouldn't be
  // able to slice it any further.
  const auto xre = dynamic_cast<const RangeExpression*>(x);
  assert(xre != nullptr);

  // Compute x's least significant bit
  Expression* lsb = nullptr;
  if (xre->get_type() == RangeExpression::Type::MINUS) {
    lsb = new BinaryExpression(
      xre->get_upper()->clone(),
      BinaryExpression::Op::MINUS,
      new BinaryExpression(
        xre->get_lower()->clone(),
        BinaryExpression::Op::PLUS,
        new Number("1")
      )
    );
  } else if (xre->get_type() == RangeExpression::Type::PLUS) {
    lsb = xre->get_upper()->clone();      
  } else {
    lsb = xre->get_lower()->clone();
  }

  // Easy Case: If y is a single bit, then we can just add this value to x's lsb
  const auto yre = dynamic_cast<const RangeExpression*>(y);
  if (yre == nullptr) {
    return new BinaryExpression(lsb, BinaryExpression::Op::PLUS, y->clone());
  }
  // Easier Case: If y is a plus or minus range, we just need to change its offset
  else if (yre->get_type() != RangeExpression::Type::CONSTANT) {
    return new RangeExpression(
      new BinaryExpression(lsb, BinaryExpression::Op::PLUS, yre->get_upper()->clone()),
      yre->get_type(),
      yre->get_lower()->clone()
    );
  }
  // Hard Case: If y is a constant range, we need to add lsb to both of its bounds
  else {
    return new RangeExpression(
      new BinaryExpression(lsb, BinaryExpression::Op::PLUS, yre->get_upper()->clone()),
      RangeExpression::Type::CONSTANT,
      new BinaryExpression(lsb->clone(), BinaryExpression::Op::PLUS, yre->get_lower()->clone())
    );
  } 
}

bool DeAlias::is_self_assign(const ContinuousAssign* ca) {
  const auto lhs = ca->get_assign()->get_lhs();
  const auto rhs = dynamic_cast<const Identifier*>(ca->get_assign()->get_rhs());
  // An rhs which isn't an identifier can't be a self-assignment
  if (rhs == nullptr) {
    return false;
  } 
  // Identifiers which point to different variables can't be self assignments
  if (Resolve().get_resolution(lhs) != Resolve().get_resolution(rhs)) {
    return false;
  }
  // Identifiers which aren't both slices can't be self assignments
  if (Resolve().is_slice(lhs) != Resolve().is_slice(rhs)) {
    return false;
  }
  // Identifiers with different dimension arities can't be self assignments
  if (lhs->size_dim() != rhs->size_dim()) {
    return false;
  }
  // If the identifiers have subscripts which disagree, this isn't a self assignment
  for (auto i = lhs->begin_dim(), j = rhs->begin_dim(), ie = lhs->end_dim(); i != ie; ++i, ++j) {
    if (!Constant().is_static_constant(*i) || !Constant().is_static_constant(*j)) {
      return false;
    }
    const auto r1 = Evaluate().get_range(*i);
    const auto r2 = Evaluate().get_range(*j);
    if ((r1.first != r2.first) || (r1.second != r2.second)) {
      return false;
    }
  }
  // It's a self-assignment
  return true;
}

Attributes* DeAlias::rewrite(Attributes* as) {
  // Don't descend past here.
  return as;
}

Expression* DeAlias::rewrite(Identifier* id) {
  // Don't rewrite anything in a declaration
  if (dynamic_cast<Declaration*>(id->get_parent())) {
    return Rewriter::rewrite(id);
  }
  // Don't rewrite things we can't resolve like scope names
  const auto r = Resolve().get_resolution(id);
  if (r == nullptr) {
    return Rewriter::rewrite(id);
  }
  // Don't rewrite array operations
  if (Resolve().is_array(r)) {
    return Rewriter::rewrite(id);
  }
  // Don't rewrite references to ports. Mostly we just care about output ports.
  // We'll never find an aliasing relationship for an input, since it would have
  // to appear on the left-hand-side of an assignment.
  if (dynamic_cast<const PortDeclaration*>(r->get_parent()->get_parent())) {
    return Rewriter::rewrite(id);
  }
  // Don't rewrite variables we don't have a de-aliasing relationship for.
  // Otherwise, return the replacement.
  const auto res = table_->dealias(id);  
  return (res == nullptr) ? Rewriter::rewrite(id) : res;
}

ModuleDeclaration* DeAlias::rewrite(ModuleDeclaration* md) {
  // Only mess with module items. 
  md->accept_items(this);
  return md;
}

} // namespace cascade
