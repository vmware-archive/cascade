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

#include "src/verilog/transform/de_alias.h"

#include <cassert>
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

DeAlias::DeAlias() : Rewriter() { }

void DeAlias::run(ModuleDeclaration* md) {
  // Build the alias table and replace variable names
  table_ = new AliasTable(md);
  md->accept(this);

  // Now that we're done we'll have a bunch of identity assignments assign X =
  // X, which are are zero time loops that need to be deleted. We can also
  // delete declarations for aliases.
  for (auto i = md->get_items()->begin(); i != md->get_items()->end(); ) {
    if (auto ca = dynamic_cast<ContinuousAssign*>(*i)) {
      if (is_self_assign(ca)) {
        Resolve().invalidate(ca);
        i = md->get_items()->purge(i);
        continue;
      } 
    } 
    if (auto d = dynamic_cast<Declaration*>(*i)) {
      if (table_->is_alias(d->get_id())) {
        Resolve().invalidate(d);
        i = md->get_items()->purge(i);
        continue;
      }
    }
    ++i;
  }

  delete table_;
}

DeAlias::AliasTable::AliasTable(const ModuleDeclaration* md) : Visitor() { 
  md->accept(this);
  for (auto& a : assigns_) {
    resolve(a.first);
  }
}

bool DeAlias::AliasTable::is_alias(const Identifier* id) {
  return results_.find(id) != results_.end();
}

const Identifier* DeAlias::AliasTable::get(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);
  
  const auto itr = results_.find(r);
  return itr != results_.end() ? itr->second : id;  
}

const Identifier* DeAlias::AliasTable::resolve(const Identifier* id) {
  // First off, resolve this identifier.
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);

  // If we already have a result we're done.
  auto ritr = results_.find(r);
  if (ritr != results_.end()) {
    return ritr->second;
  }
  
  // Look up this identifier in the assignments. If there's nothing there, 
  // just return id. We don't need to change this identifier.
  const auto aitr = assigns_.find(r);
  if (aitr == assigns_.end()) {
    return id;
  } 
  
  // TODO: For now let's only consider trivial aliases: 
  // TODO: Give up if either identifier uses a dimension subscript
  if (!aitr->first->get_dim()->null() || !aitr->second->get_dim()->null()) {
    return id;
  }
  // TODO: Give up if these two variables are different widths
  if (Evaluate().get_width(aitr->first) != Evaluate().get_width(aitr->second)) {
    return id;
  }

  // Now the recursive part. This variable is an alias for something else.  So
  // the resolution for this variable is the same as the resolution for that
  // variable.
  auto res = resolve(aitr->second);
  results_.insert(make_pair(r, res));
  
  return res;
}

void DeAlias::AliasTable::visit(const ContinuousAssign* ca) {
  const auto lhs = dynamic_cast<const Identifier*>(ca->get_assign()->get_lhs());
  if ((lhs == nullptr) || !lhs->get_dim()->null()) {
    return;
  }
  const auto rhs = dynamic_cast<const Identifier*>(ca->get_assign()->get_rhs());
  if (rhs == nullptr) {
    return;
  }
  const auto rlhs = Resolve().get_resolution(lhs);
  assert(rlhs != nullptr);

  // Note: Multiple assignments to the same variable are undefined, but
  // allowed.  We prefer the most recent assignment.
  assigns_[rlhs] = rhs;
}

bool DeAlias::is_self_assign(const ContinuousAssign* ca) {
  const auto lhs = ca->get_assign()->get_lhs();
  const auto rhs = dynamic_cast<const Identifier*>(ca->get_assign()->get_rhs());
  if (rhs == nullptr) {
    return false;
  }
  return Resolve().get_resolution(lhs) == Resolve().get_resolution(rhs);
}

Attributes* DeAlias::rewrite(Attributes* as) {
  // Don't descend past here.
  return as;
}

Expression* DeAlias::rewrite(Identifier* id) {
  // Don't rewrite anything in a declaration!
  if (dynamic_cast<Declaration*>(id->get_parent())) {
    return Rewriter::rewrite(id);
  }
  // Don't rewrite things we can't resolve like scope names
  const auto resl = Resolve().get_resolution(id);
  if (resl == nullptr) {
    return Rewriter::rewrite(id);
  }
  // And don't rewrite outputs! They might just be aliases, but we need to
  // preserve them.
  if (auto pd = dynamic_cast<PortDeclaration*>(resl->get_parent()->get_parent())) {
    if (pd->get_type() != PortDeclaration::INPUT) {
      return Rewriter::rewrite(id);
    }
  }
  // Lookup the alias resolution for this variable. If we get back the same
  // variable, there are no changes to make.
  const auto r = table_->get(id);  
  if (id == r) {
    return Rewriter::rewrite(id);
  }

  // Otherwise, we can use this name instead. Make sure to carry over
  // dimensions that were attached to this variable, though.
  auto res = r->clone();
  res->replace_dim(id->get_dim()->clone());

  // Invalidate resolution information that was attached to this variable and
  // return the replacement.
  Resolve().invalidate(id);
  return res;
}

ModuleDeclaration* DeAlias::rewrite(ModuleDeclaration* md) {
  // Only mess with module items. We're not going to rename anything anywhere else
  md->get_items()->accept(this);
  return md;
}

} // namespace cascade
