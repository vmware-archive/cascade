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

#include "src/verilog/analyze/evaluate.h"

#include <cassert>
#include "src/verilog/analyze/resolve.h"

using namespace std;

namespace cascade {

size_t Evaluate::get_width(const Expression* e) {
  init(const_cast<Expression*>(e));
  return e->bit_val_->size();
}

bool Evaluate::get_signed(const Expression* e) {
  init(const_cast<Expression*>(e));
  return e->bit_val_->is_signed();
}

const Bits& Evaluate::get_value(const Expression* e) {
  init(const_cast<Expression*>(e));
  if (e->needs_update_) {
    const_cast<Expression*>(e)->accept(this);
    const_cast<Expression*>(e)->needs_update_ = false;
  }
  return *e->bit_val_;
}

pair<size_t, size_t> Evaluate::get_range(const Expression* e) {
  if (const auto re = dynamic_cast<const RangeExpression*>(e)) {
    const auto upper = get_value(re->get_upper()).to_int();
    const auto lower = get_value(re->get_lower()).to_int();
    switch (re->get_type()) {
      case RangeExpression::CONSTANT:
        return make_pair(upper, lower);
      case RangeExpression::PLUS:
        return make_pair(upper+lower-1, upper);
      case RangeExpression::MINUS:
        return make_pair(upper, upper-lower+1);
      default:
        assert(false);
        return make_pair(0,0);
    }
  }
  const auto idx = get_value(e).to_int();
  return make_pair(idx, idx);
}

void Evaluate::assign_value(const Identifier* id, const Bits& val) {
  init(const_cast<Identifier*>(id));
  if (!id->bit_val_->eq(val)) {
    const_cast<Identifier*>(id)->bit_val_->assign(val);
    flag_changed(id);
  }
}

void Evaluate::assign_value(const Identifier* id, size_t idx, const Bits& val) {
  init(const_cast<Identifier*>(id));
  if (!id->bit_val_->eq(idx, val)) {
    const_cast<Identifier*>(id)->bit_val_->assign(idx, val);
    flag_changed(id);
  }
}

void Evaluate::assign_value(const Identifier* id, size_t i, size_t j, const Bits& val) {
  init(const_cast<Identifier*>(id));
  if (!id->bit_val_->eq(i, j, val)) {
    const_cast<Identifier*>(id)->bit_val_->assign(i, j, val);
    flag_changed(id);
  }
}

void Evaluate::invalidate(const Expression* e) {
  const auto root = get_root(e);
  Invalidate i;
  const_cast<Node*>(root)->accept(&i);
}

void Evaluate::edit(BinaryExpression* be) {
  switch (be->get_op()) {
    case BinaryExpression::PLUS:
      get_value(be->get_lhs()).arithmetic_plus(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::MINUS:
      get_value(be->get_lhs()).arithmetic_minus(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::TIMES:
      get_value(be->get_lhs()).arithmetic_multiply(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::DIV:
      get_value(be->get_lhs()).arithmetic_divide(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::MOD:
      get_value(be->get_lhs()).arithmetic_mod(get_value(be->get_rhs()), *be->bit_val_);
      break;
    // NOTE: These are equivalent insofar as we don't support x and z
    case BinaryExpression::EEEQ:
    case BinaryExpression::EEQ:
      get_value(be->get_lhs()).logical_eq(get_value(be->get_rhs()), *be->bit_val_);
      break;
    // NOTE: These are equivalent insofar as we don't support x and z
    case BinaryExpression::BEEQ:
    case BinaryExpression::BEQ:
      get_value(be->get_lhs()).logical_ne(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::AAMP:
      get_value(be->get_lhs()).logical_and(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::PPIPE:
      get_value(be->get_lhs()).logical_or(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::TTIMES:
      get_value(be->get_lhs()).arithmetic_pow(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::LT:
      get_value(be->get_lhs()).logical_lt(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::LEQ:
      get_value(be->get_lhs()).logical_lte(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::GT:
      get_value(be->get_lhs()).logical_gt(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::GEQ:
      get_value(be->get_lhs()).logical_gte(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::AMP:
      get_value(be->get_lhs()).bitwise_and(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::PIPE:
      get_value(be->get_lhs()).bitwise_or(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::CARAT:
      get_value(be->get_lhs()).bitwise_xor(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::TCARAT:
      get_value(be->get_lhs()).bitwise_xnor(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::LLT:
      get_value(be->get_lhs()).bitwise_sll(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::LLLT:
      get_value(be->get_lhs()).bitwise_sal(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::GGT:
      get_value(be->get_lhs()).bitwise_slr(get_value(be->get_rhs()), *be->bit_val_);
      break;
    case BinaryExpression::GGGT:
      get_value(be->get_lhs()).bitwise_sar(get_value(be->get_rhs()), *be->bit_val_);
      break;

    default:
      assert(false);
      break;
  }
}

void Evaluate::edit(ConditionalExpression* ce) {
  if (get_value(ce->get_cond()).to_bool()) {
    ce->bit_val_->assign(get_value(ce->get_lhs()));
  } else {
    ce->bit_val_->assign(get_value(ce->get_rhs()));
  }
}

void Evaluate::edit(NestedExpression* ne) {
  ne->bit_val_->assign(get_value(ne->get_expr()));
}

void Evaluate::edit(Concatenation* c) {
  auto i = c->get_exprs()->begin();
  c->bit_val_->assign(get_value(*i++));
  for (auto ie = c->get_exprs()->end(); i != ie; ++i) {
    c->bit_val_->concat( get_value(*i));
  }
}

void Evaluate::edit(Identifier* id) {
  // Nothing to do if this is a self-reference
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);
  if (r == id) {
    return;
  }
  // Otherwise copy or slice 
  // TODO ISSUE-20: This needs to be a different check: is the number of
  // subscripts equal to the arity of this variable?
  if (id->get_dim()->empty()) {
    id->bit_val_->assign(get_value(r));
  } else {
    const auto range = get_range(id->get_dim()->back());
    id->bit_val_->assign(get_value(r), range.first, range.second);
  }
}

void Evaluate::edit(MultipleConcatenation* mc) {
  const auto lhs = get_value(mc->get_expr()).to_int();
  mc->bit_val_->assign(get_value(mc->get_concat()));
  for (size_t i = 1; i < lhs; ++i) {
    mc->bit_val_->concat(*mc->get_concat()->bit_val_);
  }
}

void Evaluate::edit(Number* n) {
  // Does nothing. We assigned a value to this expression in init.
  (void) n;
}

void Evaluate::edit(String* s) {
  // TODO: Support this language feature.
  assert(false);
  (void) s;
}

void Evaluate::edit(UnaryExpression* ue) {
  switch (ue->get_op()) {
    case UnaryExpression::PLUS:
      get_value(ue->get_lhs()).arithmetic_plus(*ue->bit_val_);
      break;
    case UnaryExpression::MINUS:
      get_value(ue->get_lhs()).arithmetic_minus(*ue->bit_val_);
      break;
    case UnaryExpression::BANG:
      get_value(ue->get_lhs()).logical_not(*ue->bit_val_);
      break;
    case UnaryExpression::TILDE:
      get_value(ue->get_lhs()).bitwise_not(*ue->bit_val_);
      break;
    case UnaryExpression::AMP:
      get_value(ue->get_lhs()).reduce_and(*ue->bit_val_);
      break;
    case UnaryExpression::TAMP:
      get_value(ue->get_lhs()).reduce_nand(*ue->bit_val_);
      break;
    case UnaryExpression::PIPE:
      get_value(ue->get_lhs()).reduce_or(*ue->bit_val_);
      break;
    case UnaryExpression::TPIPE:
      get_value(ue->get_lhs()).reduce_nor(*ue->bit_val_);
      break;
    case UnaryExpression::CARAT:
      get_value(ue->get_lhs()).reduce_xor(*ue->bit_val_);
      break;
    case UnaryExpression::TCARAT:
      get_value(ue->get_lhs()).reduce_xnor(*ue->bit_val_);
      break;
    default:
      assert(false);
      break;
  }
}

const Node* Evaluate::get_root(const Expression* e) const {
  // Walk up the AST until we find something other than an expression.  Our
  // goal is to find the root of the expression subtree containing e.
  // 
  // We treat assignments as subtrees. We also treat expressions inside of
  // identifier and id subscripts, declaration subscripts, and the multiplier
  // in multiple concatenations as subtrees. This will simplify some things in
  // a few places where we'll want to compute a final context-determined value
  // during self-determination.
  const Node* root = nullptr;
  for (root = e; ; root = root->get_parent()) {
    // Continuous, non-blocking, or blocking assigns
    if (dynamic_cast<VariableAssign*>(root->get_parent())) {
      return root->get_parent();
    }
    // Parameter declarations
    else if (dynamic_cast<LocalparamDeclaration*>(root->get_parent())) {
      return root->get_parent();
    }
    else if (dynamic_cast<ParameterDeclaration*>(root->get_parent())) {
      return root->get_parent();
    }
    // Multiple Concatenation
    else if (dynamic_cast<MultipleConcatenation*>(root->get_parent())) {
      return root;
    }
    // Subscripts inside of ids
    else if (dynamic_cast<Maybe<Expression>*>(root->get_parent()) && dynamic_cast<Id*>(root->get_parent()->get_parent())) {
      return root->get_parent();
    }
    // Subscripts inside of identifiers
    else if (dynamic_cast<Maybe<Expression>*>(root->get_parent()) && dynamic_cast<Identifier*>(root->get_parent()->get_parent())) {
      return root->get_parent();
    }
    // Ranges inside of declarations 
    else if (dynamic_cast<Maybe<RangeExpression>*>(root->get_parent()) && dynamic_cast<Declaration*>(root->get_parent()->get_parent())) {
      return root->get_parent();
    }
    // Variables inside of declarations
    else if (dynamic_cast<Declaration*>(root->get_parent())) {
      return root->get_parent();
    }
    else if (!dynamic_cast<Expression*>(root->get_parent())) {
      return root;
    }
  } 
  assert(false);
  return nullptr;
}

void Evaluate::init(Expression* e) {
  // Nothing to do if this expression has bits allocated for it.
  if (e->bit_val_ != nullptr) {
    return;
  }

  // Find the root of this subtree
  const auto root = get_root(e);
  // Use rules of self-determination to allocate bits, sizes, and signs
  SelfDetermine sd;
  const_cast<Node*>(root)->accept(&sd);
  // Use the rules of context-determination to update sizes in some cases
  ContextDetermine cd;
  const_cast<Node*>(root)->accept(&cd);
}

void Evaluate::flag_changed(const Identifier* id) {
  const_cast<Identifier*>(id)->needs_update_ = false;
  for (auto i = Resolve().dep_begin(id), ie = Resolve().dep_end(id); i != ie; ++i) {
    (*i)->needs_update_ = true;
  }
}

void Evaluate::Invalidate::edit(BinaryExpression* be) {
  if (be->bit_val_ != nullptr) {
    delete be->bit_val_;
    be->bit_val_ = nullptr;
    be->needs_update_ = true;
  }
  Editor::edit(be);
}

void Evaluate::Invalidate::edit(ConditionalExpression* ce) {
  if (ce->bit_val_ != nullptr) {
    delete ce->bit_val_;
    ce->bit_val_ = nullptr;
    ce->needs_update_ = true;
  }
  Editor::edit(ce);
}

void Evaluate::Invalidate::edit(NestedExpression* ne) {
  if (ne->bit_val_ != nullptr) {
    delete ne->bit_val_;
    ne->bit_val_ = nullptr;
    ne->needs_update_ = true;
  }
  Editor::edit(ne);
}

void Evaluate::Invalidate::edit(Concatenation* c) {
  if (c->bit_val_ != nullptr) {
    delete c->bit_val_;
    c->bit_val_ = nullptr;
    c->needs_update_ = true;
  }
  Editor::edit(c);
}

void Evaluate::Invalidate::edit(Identifier* id) {
  if (id->bit_val_ != nullptr) {
    delete id->bit_val_;
    id->bit_val_ = nullptr;
    id->needs_update_ = true;
  }
  // Don't descend into a different subtree
}

void Evaluate::Invalidate::edit(MultipleConcatenation* mc) {
  if (mc->bit_val_ != nullptr) {
    delete mc->bit_val_;
    mc->bit_val_ = nullptr;
    mc->needs_update_ = true;
  }
  // Don't descend into a different subtree
  mc->get_concat()->accept(this);
}

void Evaluate::Invalidate::edit(Number* n) {
  if (n->bit_val_ != nullptr) {
    delete n->bit_val_;
    n->bit_val_ = nullptr;
    n->needs_update_ = true;
  }
}

void Evaluate::Invalidate::edit(String* s) {
  // TODO: Support for this language feature
  assert(false);
  (void) s;
}

void Evaluate::Invalidate::edit(UnaryExpression* ue) {
  if (ue->bit_val_ != nullptr) {
    delete ue->bit_val_;
    ue->bit_val_ = nullptr;
    ue->needs_update_ = false;
  }
  Editor::edit(ue);
}

void Evaluate::Invalidate::edit(GenvarDeclaration* gd) {
  Editor::edit(gd);
}

void Evaluate::Invalidate::edit(IntegerDeclaration* id) {
  Editor::edit(id);
}

void Evaluate::Invalidate::edit(LocalparamDeclaration* ld) {
  // Don't descend into a different subtree
  ld->get_id()->accept(this);
  ld->get_val()->accept(this);
}

void Evaluate::Invalidate::edit(NetDeclaration* nd) { 
  // Don't descend into a different subtree
  nd->get_id()->accept(this);
}

void Evaluate::Invalidate::edit(ParameterDeclaration* pd) {
  // Don't descend into a different subtree
  pd->get_id()->accept(this);
  pd->get_val()->accept(this);
}

void Evaluate::Invalidate::edit(RegDeclaration* rd) {
  // Don't descend into a different subtree
  rd->get_id()->accept(this);
  rd->get_val()->accept(this);
}

void Evaluate::Invalidate::edit(VariableAssign* va) {
  Editor::edit(va);
}

void Evaluate::SelfDetermine::edit(BinaryExpression* be) {
  Editor::edit(be);

  size_t w = 0;
  bool s = false;
  switch (be->get_op()) {
    case BinaryExpression::PLUS:
    case BinaryExpression::MINUS:
    case BinaryExpression::TIMES:
    case BinaryExpression::DIV:
    case BinaryExpression::MOD:
      s = be->get_lhs()->bit_val_->is_signed() && be->get_rhs()->bit_val_->is_signed();
      w = max(be->get_lhs()->bit_val_->size(), be->get_rhs()->bit_val_->size());
      break;
    case BinaryExpression::AMP:
    case BinaryExpression::PIPE:
    case BinaryExpression::CARAT:
    case BinaryExpression::TCARAT:
      w = max(be->get_lhs()->bit_val_->size(), be->get_rhs()->bit_val_->size());
      s = false;
      break;
    case BinaryExpression::EEEQ:
    case BinaryExpression::BEEQ:
    case BinaryExpression::EEQ:
    case BinaryExpression::BEQ:
    case BinaryExpression::GT:
    case BinaryExpression::GEQ:
    case BinaryExpression::LT:
    case BinaryExpression::LEQ:
    case BinaryExpression::AAMP:
    case BinaryExpression::PPIPE:
      w = 1;
      s = false;
      break;
    case BinaryExpression::GGT:
    case BinaryExpression::LLT:
    case BinaryExpression::TTIMES:
    case BinaryExpression::GGGT:
    case BinaryExpression::LLLT:
      w = be->get_lhs()->bit_val_->size();
      s = false;
      break;
    default:
      assert(false);
  }
  be->bit_val_ = new Bits(w, 0);
  be->bit_val_->set_signed(s);
}

void Evaluate::SelfDetermine::edit(ConditionalExpression* ce) {
  Editor::edit(ce);

  // TODO: Are we sure that this is the right calculus for sign?  I haven't
  // been able to find anything to this point in the spec.
  bool s = ce->get_lhs()->bit_val_->is_signed() && ce->get_rhs()->bit_val_->is_signed();
  size_t w = max(ce->get_lhs()->bit_val_->size(), ce->get_rhs()->bit_val_->size());
  ce->bit_val_ = new Bits(w, 0);
  ce->bit_val_->set_signed(s);
}

void Evaluate::SelfDetermine::edit(NestedExpression* ne) {
  Editor::edit(ne);

  size_t w = ne->get_expr()->bit_val_->size();
  bool s = ne->get_expr()->bit_val_->is_signed();
  ne->bit_val_ = new Bits(w, 0);
  ne->bit_val_->set_signed(s);
}

void Evaluate::SelfDetermine::edit(Concatenation* c) {
  Editor::edit(c);

  size_t w = 0;
  for (auto e : *c->get_exprs()) {
    w += e->bit_val_->size();
  }
  c->bit_val_ = new Bits(w, 0);
  c->bit_val_->set_signed(false);
}

void Evaluate::SelfDetermine::edit(Identifier* id) {
  // Don't descend on dim. We treat it as a separate subtree.

  size_t w = 0;
  bool s = false;
  // TODO: This needs to be a different check: is the number of subscripts
  // equal to the arity of this variable?
  if (id->get_dim()->empty()) {
    const auto r = Resolve().get_resolution(id);
    assert(r != nullptr);
    w = Evaluate().get_width(r);
    s = Evaluate().get_signed(r);
  } else if (auto re = dynamic_cast<RangeExpression*>(id->get_dim()->back())) {
    const auto lower = Evaluate().get_value(re->get_lower()).to_int();
    if (re->get_type() == RangeExpression::CONSTANT) {
      const auto upper = Evaluate().get_value(re->get_upper()).to_int();
      w = upper-lower+1;
    } else {
      w = lower;
    }
    s = false;
  } else {
    w = 1;
    s = false;
  }
  id->bit_val_ = new Bits(w, 0);
  id->bit_val_->set_signed(s);
}

void Evaluate::SelfDetermine::edit(MultipleConcatenation* mc) {
  // Don't descend on expr, this is a separate expression tree.
  mc->get_concat()->accept(this);

  size_t w = Evaluate().get_value(mc->get_expr()).to_int() * mc->get_concat()->bit_val_->size();
  mc->bit_val_ = new Bits(w, 0);
  mc->bit_val_->set_signed(false);
}

void Evaluate::SelfDetermine::edit(Number* n) {
  // Copying the underlying value of n sets value, width, and sign
  n->bit_val_ = new Bits(n->get_val());
}

void Evaluate::SelfDetermine::edit(String* s) {
  // TODO: Support for this language feature
  assert(false);
  (void) s;
}

void Evaluate::SelfDetermine::edit(UnaryExpression* ue) {
  Editor::edit(ue);

  size_t w = 0;
  auto s = false;
  switch (ue->get_op()) {
    case UnaryExpression::PLUS:
    case UnaryExpression::MINUS:
      w = ue->get_lhs()->bit_val_->size();
      s = ue->get_lhs()->bit_val_->is_signed();
      break;
    case UnaryExpression::TILDE:
      w = ue->get_lhs()->bit_val_->size();
      s = false;
      break;
    case UnaryExpression::AMP:
    case UnaryExpression::TAMP:
    case UnaryExpression::PIPE:
    case UnaryExpression::TPIPE:
    case UnaryExpression::CARAT:
    case UnaryExpression::TCARAT:
    case UnaryExpression::BANG:
      w = 1;
      s = false;
      break;
    default:
     assert(false); 
  }
  ue->bit_val_ = new Bits(w, 0);
  ue->bit_val_->set_signed(s);
}

void Evaluate::SelfDetermine::edit(GenvarDeclaration* gd) {
  // Don't descend on id, we handle it below

  // Genvars are materialized as localparams and follow the same rules.  For
  // lack of information in this declaration though, we have to assume 32 bit
  // unsigned. It's not like it even matters. Nothing should ever try to
  // dereference this value.
  gd->get_id()->bit_val_ = new Bits(32, 0);
  gd->get_id()->bit_val_->set_signed(false);
}

void Evaluate::SelfDetermine::edit(IntegerDeclaration* id) {
  // Don't descend on id, we handle it below
  id->get_val()->accept(this);

  // Integers must be a minimum of 32 bits and are always signed
  id->get_id()->bit_val_ = new Bits(32, 0);
  id->get_id()->bit_val_->set_signed(true);

  // Hold off on initial assignment here. We may be doing some size extending
  // in context-determination. We'll want to wait until then to compute the
  // value of this variable.
}

void Evaluate::SelfDetermine::edit(LocalparamDeclaration* ld) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  ld->get_val()->accept(this);

  // Start out with a basic allocation of bits
  ld->get_id()->bit_val_ = new Bits(false);
  // Parameter declaration may override size and sign
  ld->get_id()->bit_val_->set_signed(ld->get_signed());
  if (!ld->get_dim()->null()) {
    const auto rng = Evaluate().get_range(ld->get_dim()->get());
    ld->get_id()->bit_val_->resize(rng.first-rng.second+1);
  } 

  // Hold off on initial assignment here. We may be doing some size extending
  // in context-determination. We'll want to wait until then to compute the
  // value of this variable.
}

void Evaluate::SelfDetermine::edit(NetDeclaration* nd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  nd->get_ctrl()->accept(this);

  size_t w = 1;
  if (!nd->get_dim()->null()) {
    const auto rng = Evaluate().get_range(nd->get_dim()->get());
    w = rng.first-rng.second+1;
  }
  nd->get_id()->bit_val_ = new Bits(w, 0);
  nd->get_id()->bit_val_->set_signed(nd->get_signed());

  // For whatever reason, we've chosen to materialize net declarations as
  // declarations followed by continuous assigns. So there's no initial value
  // to worry about here.
}

void Evaluate::SelfDetermine::edit(ParameterDeclaration* pd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  pd->get_val()->accept(this);

  // Start out with a basic allocation of bits
  pd->get_id()->bit_val_ = new Bits(false);
  // Parameter declaration may override size and sign
  pd->get_id()->bit_val_->set_signed(pd->get_signed());
  if (!pd->get_dim()->null()) {
    const auto rng = Evaluate().get_range(pd->get_dim()->get());
    pd->get_id()->bit_val_->resize(rng.first-rng.second+1);
  }

  // Hold off on initial assignment here. We may be doing some size extending
  // in context-determination. We'll want to wait until then to compute the
  // value of this variable.
}

void Evaluate::SelfDetermine::edit(RegDeclaration* rd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  rd->get_val()->accept(this);

  size_t w = 1;
  if (!rd->get_dim()->null()) {
    const auto rng = Evaluate().get_range(rd->get_dim()->get());
    w = rng.first-rng.second+1;
  }
  rd->get_id()->bit_val_ = new Bits(w, 0);
  rd->get_id()->bit_val_->set_signed(rd->get_signed());

  // Hold off on initial assignment here. We may be doing some size extending
  // in context-determination. We'll want to wait until then to compute the
  // value of this variable.
}

void Evaluate::SelfDetermine::edit(VariableAssign* va) {
  // There's nothing special to be done here. All the weird magic for
  // assignments happens in context determination.
  Editor::edit(va);
}

void Evaluate::ContextDetermine::edit(BinaryExpression* be) {
  size_t w = 0;
  bool s = false;
  switch (be->get_op()) {
    case BinaryExpression::PLUS:
    case BinaryExpression::MINUS:
    case BinaryExpression::TIMES:
    case BinaryExpression::DIV:
    case BinaryExpression::MOD:
    case BinaryExpression::AMP:
    case BinaryExpression::PIPE:
    case BinaryExpression::CARAT:
    case BinaryExpression::TCARAT:
      // Both operands are context dependent
      be->get_lhs()->bit_val_->set_signed(be->bit_val_->is_signed());
      be->get_lhs()->bit_val_->resize(be->bit_val_->size());
      be->get_rhs()->bit_val_->set_signed(be->bit_val_->is_signed());
      be->get_rhs()->bit_val_->resize(be->bit_val_->size());
      break;
    case BinaryExpression::EEEQ:
    case BinaryExpression::BEEQ:
    case BinaryExpression::EEQ:
    case BinaryExpression::BEQ:
    case BinaryExpression::GT:
    case BinaryExpression::GEQ:
    case BinaryExpression::LT:
    case BinaryExpression::LEQ:
      // Operands are sort-of context dependent. They affect each other
      // independently of what's going on here.
      w = max(Evaluate().get_width(be->get_lhs()), Evaluate().get_width(be->get_rhs()));
      s = Evaluate().get_signed(be->get_lhs()) && Evaluate().get_signed(be->get_rhs());
      be->get_lhs()->bit_val_->set_signed(s);
      be->get_lhs()->bit_val_->resize(w);
      be->get_rhs()->bit_val_->set_signed(s);
      be->get_rhs()->bit_val_->resize(w);
      break;
    case BinaryExpression::AAMP:
    case BinaryExpression::PPIPE:
      // Both operands are self-determined
      break;
    case BinaryExpression::GGT:
    case BinaryExpression::LLT:
    case BinaryExpression::TTIMES:
    case BinaryExpression::GGGT:
    case BinaryExpression::LLLT:
      // The right-hand side of these expressions is self-determined
      be->get_lhs()->bit_val_->set_signed(be->bit_val_->is_signed());
      be->get_lhs()->bit_val_->resize(be->bit_val_->size());
      break;
    default:
      assert(false);
  }

  Editor::edit(be);        
}

void Evaluate::ContextDetermine::edit(ConditionalExpression* ce) {
  // Conditions are self-determined
  ce->get_lhs()->bit_val_->set_signed(ce->bit_val_->is_signed());
  ce->get_lhs()->bit_val_->resize(ce->bit_val_->size());
  ce->get_rhs()->bit_val_->set_signed(ce->bit_val_->is_signed());
  ce->get_rhs()->bit_val_->resize(ce->bit_val_->size());

  Editor::edit(ce);
}

void Evaluate::ContextDetermine::edit(NestedExpression* ne) {
  ne->get_expr()->bit_val_->set_signed(ne->bit_val_->is_signed());
  ne->get_expr()->bit_val_->resize(ne->bit_val_->size());
  
  Editor::edit(ne);
}

void Evaluate::ContextDetermine::edit(Concatenation* c) {
  // Pass on through. All operands are self-determined.
  Editor::edit(c);
}

void Evaluate::ContextDetermine::edit(Identifier* id) {
  // Nothing to do here. The only expressions we can reach from here are subscripts,
  // which we treat as separate subtrees.
  (void) id;
}

void Evaluate::ContextDetermine::edit(MultipleConcatenation* mc) {
  // Don't descend on expr, this is a separate expression tree.
  mc->get_concat()->accept(this);
}

void Evaluate::ContextDetermine::edit(Number* n) {
  // Nothing left to do here. This is a primary with no nested expressions.
  (void) n;
}

void Evaluate::ContextDetermine::edit(String* s) {
  // Nothing left to do here. This is a primary with no nested expressions.
  (void) s;
}

void Evaluate::ContextDetermine::edit(UnaryExpression* ue) {
  switch (ue->get_op()) {
    case UnaryExpression::PLUS:
    case UnaryExpression::MINUS:
    case UnaryExpression::TILDE:
      ue->get_lhs()->bit_val_->set_signed(ue->bit_val_->is_signed());
      ue->get_lhs()->bit_val_->resize(ue->bit_val_->size());
      break;
    case UnaryExpression::AMP:
    case UnaryExpression::TAMP:
    case UnaryExpression::PIPE:
    case UnaryExpression::TPIPE:
    case UnaryExpression::CARAT:
    case UnaryExpression::TCARAT:
    case UnaryExpression::BANG:
      // All operands are self-determined
      break;
    default:
     assert(false); 
  }

  Editor::edit(ue);
}

void Evaluate::ContextDetermine::edit(GenvarDeclaration* gd) {
  // There's nothing left to do here. There's no rhs to worry about.
  (void) gd;
}

void Evaluate::ContextDetermine::edit(IntegerDeclaration* id) { 
  // Nothing to do if there's no assignment happening here
  if (id->get_val()->null()) {
    return;
  }
  // Assignments impose larger sizes but not sign constraints
  if (id->get_val()->get()->bit_val_->size() < 32) {
    id->get_val()->get()->bit_val_->resize(32);
  }
  id->get_val()->accept(this);

  // Now that we're context determined, we can perform initial assignment
  id->get_id()->bit_val_->assign(Evaluate().get_value(id->get_val()->get()));
}

void Evaluate::ContextDetermine::edit(LocalparamDeclaration* ld) {
  // Parameters don't impose constraints on their rhs
  ld->get_val()->accept(this);
  // But they inherit size and width from their rhs unless otherwise specified
  if (!ld->get_signed()) {
    ld->get_id()->bit_val_->set_signed(ld->get_val()->bit_val_->is_signed());
  }
  if (ld->get_dim()->null()) {
    ld->get_id()->bit_val_->resize(ld->get_val()->bit_val_->size());
  }

  // Now that we're context determined, we can perform initial assignment
  ld->get_id()->bit_val_->assign(Evaluate().get_value(ld->get_val()));
}

void Evaluate::ContextDetermine::edit(NetDeclaration* nd) { 
  // There's nothing left to do here. There's no rhs to worry about.
  (void) nd;
}

void Evaluate::ContextDetermine::edit(ParameterDeclaration* pd) {
  // Parameters don't impose constraints on their rhs
  pd->get_val()->accept(this);
  // But they inherit size and width from their rhs unless otherwise specified
  if (!pd->get_signed()) {
    pd->get_id()->bit_val_->set_signed(pd->get_val()->bit_val_->is_signed());
  }
  if (pd->get_dim()->null()) {
    pd->get_id()->bit_val_->resize(pd->get_val()->bit_val_->size());
  }

  // Now that we're context determined, we can perform initial assignment
  pd->get_id()->bit_val_->assign(Evaluate().get_value(pd->get_val()));
}

void Evaluate::ContextDetermine::edit(RegDeclaration* rd) {
  // Nothing to do if there's no assignment happening here
  if (rd->get_val()->null()) {
    return;
  }
  // Assignments impose larger sizes but not sign constraints
  if (rd->get_id()->bit_val_->size() > rd->get_val()->get()->bit_val_->size()) {
    rd->get_val()->get()->bit_val_->resize(rd->get_id()->bit_val_->size());
  }
  rd->get_val()->accept(this);

  // Now that we're context determined, we can perform initial assignment
  rd->get_id()->bit_val_->assign(Evaluate().get_value(rd->get_val()->get()));
}

void Evaluate::ContextDetermine::edit(VariableAssign* va) {
  // Assignments impose larger sizes but not sign constraints
  if (va->get_lhs()->bit_val_->size() > va->get_rhs()->bit_val_->size()) {
    va->get_rhs()->bit_val_->resize(va->get_lhs()->bit_val_->size());
  }
  va->get_rhs()->accept(this);

  // We're context determined, but these assignments happen dynamically.
  // Nothing more to do here.
}

} // namespace cascade
