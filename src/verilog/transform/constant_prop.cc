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

#include "src/verilog/transform/constant_prop.h"

#include "src/verilog/analyze/constant.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

ConstantProp::ConstantProp() : Rewriter() { }

void ConstantProp::run(ModuleDeclaration* md) {
  // Collect net assignments, we'll populate the runtime constant set lazily in
  // calls to RuntimeConstant::check().
  for (auto i = md->begin_items(), ie = md->end_items(); i != ie; ++i) {
    if (auto* ca = dynamic_cast<ContinuousAssign*>(*i)) {
      const auto* lhs = ca->get_assign()->get_lhs();
      const auto* r = Resolve().get_resolution(lhs);
      assert(r != nullptr);

      const auto is_net = dynamic_cast<const NetDeclaration*>(r->get_parent()) != nullptr;
      const auto is_slice = Resolve().is_slice(lhs);
      const auto is_array = Resolve().is_array(r);
      if (is_net && !is_slice && !is_array) {
        net_assigns_.insert(make_pair(r, ca));
      }  
    }
  }

  // Replace constant expressions with numbers
  md->accept(this);

  // Go back and delete every continuous assignment which was consumed when we
  // populated the runtime constants set.
  for (auto i = md->begin_items(); i != md->end_items(); ) {
    if (auto* ca = dynamic_cast<ContinuousAssign*>(*i)) {
      const auto* lhs = ca->get_assign()->get_lhs();
      const auto* r = Resolve().get_resolution(lhs);
      assert(r != nullptr);

      const auto is_slice = Resolve().is_slice(lhs);
      const auto is_array = Resolve().is_array(r);
      const auto consumed = runtime_constants_.find(r) != runtime_constants_.end();
      if (!is_slice && !is_array && consumed) {
        i = md->purge_items(i);
        continue;
      } 
    } 
    ++i;
  }

  // Invalidate cached state (we haven't added or deleted declarations, so
  // there's no need to invalidate the scope tree).
  Resolve().invalidate(md);
  ModuleInfo(md).invalidate();
}

bool ConstantProp::is_assign_target(const Identifier* i) const {
  const auto* p = i->get_parent();
  if (auto* va = dynamic_cast<const VariableAssign*>(p)) {
    const auto* pp = p->get_parent();
    if (dynamic_cast<const ContinuousAssign*>(pp)) {
      return i == va->get_lhs();
    }
  }
  return false;
}

ConstantProp::RuntimeConstant::RuntimeConstant(ConstantProp* cp) : Visitor() {
  cp_ = cp;
}

bool ConstantProp::RuntimeConstant::check(const Expression* e) {
  res_ = true;
  e->accept(this);
  return res_;
}

void ConstantProp::RuntimeConstant::visit(const Attributes* as) {
  // Descending on attributes will cause false negatives.
  (void) as;
}

void ConstantProp::RuntimeConstant::visit(const Identifier* i) {
  Visitor::visit(i);

  // All static constants are runtime constants
  if (Constant().is_static_constant(i)) {
    return;
  }
  // If we can't resolve this identifier, it's not a runtime constant
  const auto* r = Resolve().get_resolution(i);
  if (r == nullptr) {
    res_ = false;
    return;
  }
  // Nothing to do if this variable appears in the runtime constants set
  if (cp_->runtime_constants_.find(r) != cp_->runtime_constants_.end()) {
    return;
  }
  // If this variable is the target of a continuous assign it might belong
  // in the runtime constant set
  auto itr = cp_->net_assigns_.find(r);
  if (itr != cp_->net_assigns_.end()) {
    const auto sc = Constant().is_static_constant(itr->second->get_assign()->get_rhs());
    const auto rc = sc || RuntimeConstant(cp_).check(itr->second->get_assign()->get_rhs());
    itr = cp_->net_assigns_.find(r);
    if (sc || rc) {
      cp_->runtime_constants_.insert(r);
      Evaluate().assign_value(r, Evaluate().get_value(itr->second->get_assign()->get_rhs()));
    }
    cp_->net_assigns_.erase(itr);
    if (sc || rc) {
      return;
    }
  }

  // No such luck. This isn't a constant.
  res_ = false;
}

Expression* ConstantProp::rewrite(BinaryExpression* be) {
  if (RuntimeConstant(this).check(be)) {
    auto* res = new Number(Evaluate().get_value(be), Number::Format::HEX);
    Evaluate().invalidate(be);
    return res;
  }
  return Rewriter::rewrite(be);
}

Expression* ConstantProp::rewrite(ConditionalExpression* ce) {
  if (RuntimeConstant(this).check(ce->get_cond())) {
    Expression* res = nullptr;
    if (Evaluate().get_value(ce->get_cond()).to_bool()) {
      res = ce->accept_lhs(this);
      if (res == ce->get_lhs()) {
        ce->set_lhs(new Identifier("ignore"));
      }
    } else {
      res = ce->accept_rhs(this);
      if (res == ce->get_rhs()) {
        ce->set_rhs(new Identifier("ignore"));
      }
    }
    Evaluate().invalidate(ce);
    return res;
  }
  return Rewriter::rewrite(ce);
}

Expression* ConstantProp::rewrite(Concatenation* c) {
  if (RuntimeConstant(this).check(c)) {
    auto* res = new Number(Evaluate().get_value(c), Number::Format::HEX);
    Evaluate().invalidate(c);
    return res;
  }
  return Rewriter::rewrite(c);
}

Expression* ConstantProp::rewrite(Identifier* i) {
  if (dynamic_cast<Declaration*>(i->get_parent())) {
    return Rewriter::rewrite(i);
  }
  if (is_assign_target(i)) {
    return Rewriter::rewrite(i);
  }
  if (RuntimeConstant(this).check(i)) {
    auto* res = new Number(Evaluate().get_value(i), Number::Format::HEX);
    Evaluate().invalidate(i);
    return res;
  }
  return Rewriter::rewrite(i);
}

Expression* ConstantProp::rewrite(MultipleConcatenation* mc) {
  if (RuntimeConstant(this).check(mc)) {
    auto* res = new Number(Evaluate().get_value(mc), Number::Format::HEX);
    Evaluate().invalidate(mc);
    return res;
  }
  return Rewriter::rewrite(mc);
}

Expression* ConstantProp::rewrite(RangeExpression* re) {
  if (RuntimeConstant(this).check(re)) {
    const auto rng = Evaluate().get_range(re);
    auto* res = new RangeExpression(rng.first+1, rng.second);
    Evaluate().invalidate(re);
    return res;
  }
  return Rewriter::rewrite(re);
}

Expression* ConstantProp::rewrite(UnaryExpression* ue) {
  if (RuntimeConstant(this).check(ue)) {
    auto* res = new Number(Evaluate().get_value(ue), Number::Format::HEX);
    Evaluate().invalidate(ue);
    return res;
  }
  return Rewriter::rewrite(ue);
}

Statement* ConstantProp::rewrite(ConditionalStatement* cs) {
  if (RuntimeConstant(this).check(cs->get_if())) {
    Statement* res = nullptr;
    if (Evaluate().get_value(cs->get_if()).to_bool()) {
      res = cs->accept_then(this);
      if (res == cs->get_then()) {
        cs->set_then(new SeqBlock());  
      }
    } else {
      res = cs->accept_else(this);
      if (res == cs->get_else()) {
        cs->set_else(new SeqBlock());
      }
    }
    return res;
  }
  return Rewriter::rewrite(cs);
}

} // namespace cascade
