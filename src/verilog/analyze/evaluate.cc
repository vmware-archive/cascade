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

#include "verilog/analyze/evaluate.h"

using namespace std;

namespace cascade {

Evaluate::Evaluate() {
  feof_ = nullptr;
  fopen_ = nullptr;
}

Evaluate& Evaluate::set_feof_handler(FeofHandler h) {
  feof_ = h;
  return *this;
}

Evaluate& Evaluate::set_fopen_handler(FopenHandler h) {
  fopen_ = h;
  return *this;
}

vector<size_t> Evaluate::get_arity(const Identifier* id) {
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);

  // Variable references are always scalar
  if (id != r) {
    return vector<size_t>();
  }
  // Otherwise, iterate over the declaration to determine arity
  vector<size_t> res;
  for (auto i = r->begin_dim(), ie = r->end_dim(); i != ie; ++i) {
    const auto rng = get_range(*i);
    res.push_back((rng.first-rng.second)+1);
  }
  return res;
}

size_t Evaluate::get_width(const Expression* e) {
  init(const_cast<Expression*>(e));
  return e->bit_val_[0].size();
}

bool Evaluate::get_signed(const Expression* e) {
  init(const_cast<Expression*>(e));
  return (e->bit_val_[0].get_type() != Bits::Type::UNSIGNED);
}

bool Evaluate::get_real(const Expression* e) {
  init(const_cast<Expression*>(e));
  return (e->bit_val_[0].get_type() == Bits::Type::REAL);
}

const Bits& Evaluate::get_value(const Expression* e) {
  init(const_cast<Expression*>(e));
  if (e->get_flag<0>()) {
    const_cast<Expression*>(e)->accept(this);
    const_cast<Expression*>(e)->set_flag<0>(false);
  }
  return e->bit_val_[0];
}

const Vector<Bits>& Evaluate::get_array_value(const Identifier* i) {
  init(const_cast<Identifier*>(i));
  if (i->get_flag<0>()) {
    const_cast<Identifier*>(i)->accept(this);
    const_cast<Identifier*>(i)->set_flag<0>(false);
  }
  return i->bit_val_;
}

pair<size_t, size_t> Evaluate::get_range(const Expression* e) {
  if (e->is(Node::Tag::range_expression)) {
    const auto* re = static_cast<const RangeExpression*>(e);
    const auto upper = get_value(re->get_upper()).to_int();
    const auto lower = get_value(re->get_lower()).to_int();
    switch (re->get_type()) {
      case RangeExpression::Type::CONSTANT:
        return make_pair(upper, lower);
      case RangeExpression::Type::PLUS:
        return make_pair((upper+lower)-1, upper);
      case RangeExpression::Type::MINUS:
        return make_pair(upper, (upper-lower)+1);
      default:
        assert(false);
        return make_pair(0,0);
    }
  }
  const auto idx = get_value(e).to_int();
  return make_pair(idx, idx);
}

void Evaluate::assign_value(const Identifier* id, const Bits& val) {
  // Find the variable that we're referring to. 
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  init(const_cast<Identifier*>(r));
  // Calculate its index
  const auto dres = dereference(r, id);
  const auto idx = static_cast<size_t>(get<0>(dres));

  // Corner Case: Ignore writes to out of range indices
  if (idx >= r->bit_val_.size()) {
    return;
  }
  // Simple Case: Full Assignment
  else if (get<1>(dres) == -1) {
    if (!r->bit_val_[idx].eq(val)) {
      const_cast<Identifier*>(r)->bit_val_[idx].assign(val);
      flag_changed(r);
    }
  } 
  // Corner Case: Ignore writes to bit ranges which are entirely out of range
  else if (static_cast<size_t>(get<2>(dres)) >= get_width(r)) {
    return;
  }
  // Partial Case: Write as much as possible to a partially valid range
  else {
    const auto msb = min(static_cast<size_t>(get<1>(dres)), get_width(r)-1);
    const auto lsb = min(static_cast<size_t>(get<2>(dres)), get_width(r)-1);
    if (!r->bit_val_[idx].eq(msb, lsb, val)) {
      const_cast<Identifier*>(r)->bit_val_[idx].assign(msb, lsb, val);
      flag_changed(r);
    }
  }
}

void Evaluate::assign_array_value(const Identifier* id, const Vector<Bits>& val) {
  // Find the variable that we're referring to. 
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  init(const_cast<Identifier*>(r));

  // Perform the assignment
  // TODO(eschkufz) Is it worthwhile to check whether anything has changed here?
  assert(val.size() == id->bit_val_.size());
  for (size_t i = 0, ie = id->bit_val_.size(); i < ie; ++i) {
    const_cast<Identifier*>(r)->bit_val_[i].assign(val[i]);
  }
  flag_changed(r);
}

tuple<size_t,int,int> Evaluate::dereference(const Identifier* r, const Identifier* i) {
  // Nothing to do if this is a scalar variable
  if (r->empty_dim()) {
    const auto rng = i->empty_dim() ? 
      make_pair<size_t, size_t>(-1,-1) : 
      get_range(i->front_dim());
    return make_tuple(0, rng.first, rng.second);
  }

  // Otherwise, i had better have at most one more dimension than r
  assert(i->size_dim() - r->size_dim() <= 1);

  // Iterators
  auto iitr = i->begin_dim();
  auto ritr = r->begin_dim();
  // The index we're looking for
  size_t idx = 0;
  // Multiplier for multi-dimensional arrays
  size_t mul = r->bit_val_.size();

  // Walk along subscripts 
  for (auto re = r->end_dim(); ritr != re; ++iitr, ++ritr) {
    assert((*ritr)->is(Node::Tag::range_expression));
    assert(!(*iitr)->is(Node::Tag::range_expression));

    const auto rval = get_range(*ritr);
    const auto ival = get_value(*iitr).to_int();

    mul /= ((rval.first-rval.second)+1);
    idx += mul * ival;
  }
  const auto rng = (iitr == i->end_dim()) ?
    make_pair<size_t, size_t>(-1,-1) : 
    get_range(*iitr);
  return make_tuple(idx, rng.first, rng.second);
}

void Evaluate::assign_value(const Identifier* id, size_t idx, int msb, int lsb, const Bits& val) {
  init(const_cast<Identifier*>(id));

  // Corner Case: Ignore writes to out of bounds indices
  if (idx >= id->bit_val_.size()) {
    return;
  }
  // Fast Path: Single bit assignments are easy to check
  else if (msb == -1) {
    if (!id->bit_val_[idx].eq(val)) {
      const_cast<Identifier*>(id)->bit_val_[idx].assign(val);
      flag_changed(id);
    }
  } 
  // Corner Case: Ignore writes to bit ranges which are completely out of bounds
  else if (static_cast<size_t>(lsb) >= get_width(id)) {
    return;
  }
  // Partial Case: Perform as much of the assignment as possible
  else {
    const auto m = min(static_cast<size_t>(msb), get_width(id)-1);
    const auto l = min(static_cast<size_t>(lsb), get_width(id)-1);
    if (!id->bit_val_[idx].eq(m, l, val)) {
      const_cast<Identifier*>(id)->bit_val_[idx].assign(m, l, val);
      flag_changed(id);
    }
  }
}

void Evaluate::flag_changed(const Identifier* id) {
  for (auto i = Resolve().use_begin(id), ie = Resolve().use_end(id); i != ie; ++i) {
    const_cast<Expression*>(*i)->set_flag<0>(true);
  }
  const_cast<Identifier*>(id)->set_flag<0>(false);
}

void Evaluate::flag_changed(const FeofExpression* fe) {
  for (const Node* n = fe; n->is_subclass_of(Node::Tag::expression); n = n->get_parent()) {
    const auto* e = static_cast<const Expression*>(n);
    const_cast<Expression*>(e)->set_flag<0>(true);
  }
}

void Evaluate::invalidate(const Expression* e) {
  const auto* root = get_root(e);
  Invalidate i;
  const_cast<Node*>(root)->accept(&i);
}

void Evaluate::edit(BinaryExpression* be) {
  switch (be->get_op()) {
    case BinaryExpression::Op::PLUS:
      get_value(be->get_lhs()).arithmetic_plus(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::MINUS:
      get_value(be->get_lhs()).arithmetic_minus(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::TIMES:
      get_value(be->get_lhs()).arithmetic_multiply(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::DIV:
      get_value(be->get_lhs()).arithmetic_divide(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::MOD:
      get_value(be->get_lhs()).arithmetic_mod(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    // NOTE: These are equivalent because we don't support x and z
    case BinaryExpression::Op::EEEQ:
    case BinaryExpression::Op::EEQ:
      get_value(be->get_lhs()).logical_eq(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    // NOTE: These are equivalent because we don't support x and z
    case BinaryExpression::Op::BEEQ:
    case BinaryExpression::Op::BEQ:
      get_value(be->get_lhs()).logical_ne(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::AAMP:
      get_value(be->get_lhs()).logical_and(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::PPIPE:
      get_value(be->get_lhs()).logical_or(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::TTIMES:
      get_value(be->get_lhs()).arithmetic_pow(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::LT:
      get_value(be->get_lhs()).logical_lt(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::LEQ:
      get_value(be->get_lhs()).logical_lte(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::GT:
      get_value(be->get_lhs()).logical_gt(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::GEQ:
      get_value(be->get_lhs()).logical_gte(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::AMP:
      get_value(be->get_lhs()).bitwise_and(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::PIPE:
      get_value(be->get_lhs()).bitwise_or(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::CARAT:
      get_value(be->get_lhs()).bitwise_xor(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::TCARAT:
      get_value(be->get_lhs()).bitwise_xnor(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::LLT:
      get_value(be->get_lhs()).bitwise_sll(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::LLLT:
      get_value(be->get_lhs()).bitwise_sal(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::GGT:
      get_value(be->get_lhs()).bitwise_slr(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::Op::GGGT:
      get_value(be->get_lhs()).bitwise_sar(get_value(be->get_rhs()), be->bit_val_[0]);
      break;

    default:
      assert(false);
      break;
  }
}

void Evaluate::edit(ConditionalExpression* ce) {
  if (get_value(ce->get_cond()).to_bool()) {
    ce->bit_val_[0].assign(get_value(ce->get_lhs()));
  } else {
    ce->bit_val_[0].assign(get_value(ce->get_rhs()));
  }
}

void Evaluate::edit(FeofExpression* fe) {
  // Relies on target-specific logic:
  if (feof_ != nullptr) {
    fe->bit_val_[0].set(0, feof_(this, fe));
  } else {
    fe->bit_val_[0].set(0, true);
  }
}

void Evaluate::edit(FopenExpression* fe) {
  // Relies on target-specific logic:
  if (fopen_ != nullptr) {
    fe->bit_val_[0].assign(Bits(32, fopen_(this, fe)));
  } else {
    fe->bit_val_[0].assign(Bits(32, 0));
  }
}

void Evaluate::edit(Concatenation* c) {
  auto i = c->begin_exprs();
  c->bit_val_[0].assign(get_value(*i++));
  for (auto ie = c->end_exprs(); i != ie; ++i) {
    c->bit_val_[0].concat(get_value(*i));
  }
}

void Evaluate::edit(Identifier* id) {
  // Nothing to do if this is a self-reference
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  if (r == id) {
    return;
  }
  // Compute the index of this dereference
  const auto dres = dereference(r, id);
  const auto idx = static_cast<size_t>(get<0>(dres));

  // Corner Case: Ignore reads from out of bounds indices
  if (idx >= get_array_value(r).size()) {
    return;
  }
  // Simple Case: Full value assignment
  else if (get<1>(dres) == -1) {
    id->bit_val_[0].assign(get_array_value(r)[idx]);
  } 
  // Partial Case: Ignore reads from bit ranges which are out of bounds
  else {
    const auto msb = min(static_cast<size_t>(get<1>(dres)), get_width(r)-1);
    const auto lsb = min(static_cast<size_t>(get<2>(dres)), get_width(r)-1);
    id->bit_val_[0].assign(get_array_value(r)[idx], msb, lsb);
  }
}

void Evaluate::edit(MultipleConcatenation* mc) {
  const auto lhs = get_value(mc->get_expr()).to_int();
  mc->bit_val_[0].assign(get_value(mc->get_concat()));
  for (size_t i = 1; i < lhs; ++i) {
    mc->bit_val_[0].concat(mc->get_concat()->bit_val_[0]);
  }
}

void Evaluate::edit(Number* n) {
  // Does nothing. We assigned a value to this expression in init.
  (void) n;
}

void Evaluate::edit(String* s) {
  // Does nothing. We assigned a value to this expression in init.
  (void) s;
}

void Evaluate::edit(UnaryExpression* ue) {
  switch (ue->get_op()) {
    case UnaryExpression::Op::PLUS:
      get_value(ue->get_lhs()).arithmetic_plus(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::MINUS:
      get_value(ue->get_lhs()).arithmetic_minus(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::BANG:
      get_value(ue->get_lhs()).logical_not(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::TILDE:
      get_value(ue->get_lhs()).bitwise_not(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::AMP:
      get_value(ue->get_lhs()).reduce_and(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::TAMP:
      get_value(ue->get_lhs()).reduce_nand(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::PIPE:
      get_value(ue->get_lhs()).reduce_or(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::TPIPE:
      get_value(ue->get_lhs()).reduce_nor(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::CARAT:
      get_value(ue->get_lhs()).reduce_xor(ue->bit_val_[0]);
      break;
    case UnaryExpression::Op::TCARAT:
      get_value(ue->get_lhs()).reduce_xnor(ue->bit_val_[0]);
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
    // Subscripts inside of identifiers 
    if (root->is_subclass_of(Node::Tag::expression) && root->get_parent()->is(Node::Tag::identifier)) {
      return root;
    }
    // Ranges inside of declarations 
    else if (root->is(Node::Tag::range_expression) && root->get_parent()->is_subclass_of(Node::Tag::declaration)) {
      return root;
    }
    // Variables inside of declarations
    else if (root->get_parent()->is_subclass_of(Node::Tag::declaration)) {
      return root->get_parent();
    }
    // Continuous, non-blocking, or blocking assigns
    else if (root->get_parent()->is(Node::Tag::variable_assign)) {
      return root->get_parent();
    }
    // Multiple Concatenation
    else if (root->get_parent()->is(Node::Tag::multiple_concatenation)) {
      return root;
    }
    // Subscripts inside of ids
    else if (root->get_parent()->is(Node::Tag::id)) {
      return root;
    }
    else if (!root->get_parent()->is_subclass_of(Node::Tag::expression)) {
      return root;
    }
  } 
  assert(false);
  return nullptr;
}

void Evaluate::init(Expression* e) {
  // Nothing to do if this expression has bits allocated for it.
  if (!e->bit_val_.empty()) {
    return;
  }

  // Find the root of this subtree
  const auto* root = get_root(e);
  // Use rules of self-determination to allocate bits, sizes, and signs
  SelfDetermine sd(this);
  const_cast<Node*>(root)->accept(&sd);
  // Use the rules of context-determination to update sizes in some cases
  ContextDetermine cd(this);
  const_cast<Node*>(root)->accept(&cd);
}

void Evaluate::Invalidate::edit(BinaryExpression* be) {
  be->bit_val_.clear();
  be->set_flag<0>(true);
  Editor::edit(be);
}

void Evaluate::Invalidate::edit(ConditionalExpression* ce) {
  ce->bit_val_.clear();
  ce->set_flag<0>(true);
  Editor::edit(ce);
}

void Evaluate::Invalidate::edit(FeofExpression* fe) {
  fe->bit_val_.clear();
  fe->set_flag<0>(true);
  Editor::edit(fe);
}

void Evaluate::Invalidate::edit(FopenExpression* fe) {
  fe->bit_val_.clear();
  fe->set_flag<0>(true);
  Editor::edit(fe);
}

void Evaluate::Invalidate::edit(Concatenation* c) {
  c->bit_val_.clear();
  c->set_flag<0>(true);
  Editor::edit(c);
}

void Evaluate::Invalidate::edit(Identifier* id) {
  id->bit_val_.clear();
  id->set_flag<0>(true);
  // Don't descend into a different subtree
}

void Evaluate::Invalidate::edit(MultipleConcatenation* mc) {
  mc->bit_val_.clear();
  mc->set_flag<0>(true);
  // Don't descend into a different subtree
  mc->accept_concat(this);
}

void Evaluate::Invalidate::edit(Number* n) {
  // Reset this number to its default size and sign.
  n->bit_val_[0].set_type(static_cast<Bits::Type>(n->Node::get_val<5,2>()));
  n->bit_val_[0].resize(n->Node::get_val<7,25>());
  n->set_flag<0>(true);
}

void Evaluate::Invalidate::edit(String* s) {
  // Nowhere left to descend to. This is a primary.
  s->bit_val_.clear();
  s->set_flag<0>(true);
}

void Evaluate::Invalidate::edit(UnaryExpression* ue) {
  ue->bit_val_.clear();
  ue->set_flag<0>(true);
  Editor::edit(ue);
}

void Evaluate::Invalidate::edit(LocalparamDeclaration* ld) {
  ld->accept_id(this);
  // Ignore dim: Don't descend into a different subtree
  ld->accept_val(this);
}

void Evaluate::Invalidate::edit(NetDeclaration* nd) { 
  nd->accept_id(this);
  // Ignore dim: Don't descend into a different subtree
}

void Evaluate::Invalidate::edit(ParameterDeclaration* pd) {
  pd->accept_id(this);
  // Ignore dim: Don't descend into a different subtree
  pd->accept_val(this);
}

void Evaluate::Invalidate::edit(RegDeclaration* rd) {
  rd->accept_id(this);
  // Ignore dim: Don't descend into a different subtree
  rd->accept_val(this);
}

Evaluate::SelfDetermine::SelfDetermine(Evaluate* eval) {
  eval_ = eval;
}

void Evaluate::SelfDetermine::edit(BinaryExpression* be) {
  Editor::edit(be);

  size_t w = 0;
  bool r = false;
  bool s = false;
  switch (be->get_op()) {
    case BinaryExpression::Op::PLUS:
    case BinaryExpression::Op::MINUS:
    case BinaryExpression::Op::TIMES:
    case BinaryExpression::Op::DIV:
    case BinaryExpression::Op::MOD:
      w = max(be->get_lhs()->bit_val_[0].size(), be->get_rhs()->bit_val_[0].size());
      r = be->get_lhs()->bit_val_[0].is_real() || be->get_rhs()->bit_val_[0].is_real();
      s = be->get_lhs()->bit_val_[0].is_signed() && be->get_rhs()->bit_val_[0].is_signed();
      break;
    case BinaryExpression::Op::AMP:
    case BinaryExpression::Op::PIPE:
    case BinaryExpression::Op::CARAT:
    case BinaryExpression::Op::TCARAT:
      w = max(be->get_lhs()->bit_val_[0].size(), be->get_rhs()->bit_val_[0].size());
      break;
    case BinaryExpression::Op::EEEQ:
    case BinaryExpression::Op::BEEQ:
    case BinaryExpression::Op::EEQ:
    case BinaryExpression::Op::BEQ:
    case BinaryExpression::Op::GT:
    case BinaryExpression::Op::GEQ:
    case BinaryExpression::Op::LT:
    case BinaryExpression::Op::LEQ:
    case BinaryExpression::Op::AAMP:
    case BinaryExpression::Op::PPIPE:
      w = 1;
      break;
    case BinaryExpression::Op::GGT:
    case BinaryExpression::Op::LLT:
      w = be->get_lhs()->bit_val_[0].size();
      s = be->get_lhs()->bit_val_[0].is_signed();
      break;
    case BinaryExpression::Op::TTIMES:
      w = be->get_lhs()->bit_val_[0].size();
      break;
    case BinaryExpression::Op::GGGT:
    case BinaryExpression::Op::LLLT:
      w = be->get_lhs()->bit_val_[0].size();
      s = be->get_lhs()->bit_val_[0].is_signed();
      break;
    default:
      assert(false);
  }
  be->bit_val_.push_back(Bits(w, 0));
  if (r) {
    be->bit_val_[0].set_type(Bits::Type::REAL);
  } else if (s) {
    be->bit_val_[0].set_signed();
  } 
}

void Evaluate::SelfDetermine::edit(ConditionalExpression* ce) {
  Editor::edit(ce);

  // TODO(eschkufz) Are we sure that this is the right calculus for type?  I
  // haven't been able to find anything to this point in the spec.
  size_t w = max(ce->get_lhs()->bit_val_[0].size(), ce->get_rhs()->bit_val_[0].size());
  const auto r = ce->get_lhs()->bit_val_[0].is_real() || ce->get_rhs()->bit_val_[0].is_real();
  const auto s = ce->get_lhs()->bit_val_[0].is_signed() && ce->get_rhs()->bit_val_[0].is_signed();

  ce->bit_val_.push_back(Bits(w, 0));
  if (r) {
    ce->bit_val_[0].set_type(Bits::Type::REAL);
  } else {
    ce->bit_val_[0].set_signed();
  } 
}

void Evaluate::SelfDetermine::edit(FeofExpression* fe) {
  Editor::edit(fe);

  // $eof() expressions return 1-bit unsigned flags
  fe->bit_val_.push_back(Bits(1, 0));
  fe->bit_val_[0].set_type(Bits::Type::UNSIGNED);
}

void Evaluate::SelfDetermine::edit(FopenExpression* fe) {
  Editor::edit(fe);

  // $fopen() expressions return 32-bit unsigned integer file values.
  fe->bit_val_.push_back(Bits(32, 0));
  fe->bit_val_[0].set_type(Bits::Type::UNSIGNED);
}

void Evaluate::SelfDetermine::edit(Concatenation* c) {
  Editor::edit(c);

  size_t w = 0;
  for (auto i = c->begin_exprs(), ie = c->end_exprs(); i != ie; ++i) {
    w += (*i)->bit_val_[0].size();
  }
  c->bit_val_.push_back(Bits(w, 0));
  c->bit_val_[0].set_type(Bits::Type::UNSIGNED);
}

void Evaluate::SelfDetermine::edit(Identifier* id) {
  // Don't descend on dim. We treat it as a separate subtree.
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);

  size_t w = 0;
  auto s = false;
  auto t = false;
  if (id->size_dim() == r->size_dim()) {
    w = eval_->get_width(r);
    s = eval_->get_signed(r);
    t = eval_->get_real(r);
  } else if (id->back_dim()->is(Node::Tag::range_expression)) {
    auto* re = static_cast<RangeExpression*>(id->back_dim());
    const auto lower = eval_->get_value(re->get_lower()).to_int();
    if (re->get_type() == RangeExpression::Type::CONSTANT) {
      const auto upper = eval_->get_value(re->get_upper()).to_int();
      w = (upper-lower)+1;
    } else {
      w = lower;
    }
  } else {
    w = 1;
  }
  id->bit_val_.push_back(Bits(w, 0));
  if (t) {
    id->bit_val_[0].set_real();
  } else if (s) {
    id->bit_val_[0].set_signed();
  }
}

void Evaluate::SelfDetermine::edit(MultipleConcatenation* mc) {
  // Don't descend on expr, this is a separate expression tree.
  mc->accept_concat(this);

  size_t w = eval_->get_value(mc->get_expr()).to_int() * mc->get_concat()->bit_val_[0].size();
  mc->bit_val_.push_back(Bits(w, 0));
  mc->bit_val_[0].set_type(Bits::Type::UNSIGNED);
}

void Evaluate::SelfDetermine::edit(Number* n) {
  // Does nothing. We already have the value of this expression stored along
  // with its default size and type in the ast.
  (void) n; 
}

void Evaluate::SelfDetermine::edit(String* s) {
  // Strings are always unsigned. 
  s->bit_val_.push_back(Bits(s->get_readable_val()));
  s->bit_val_[0].set_type(Bits::Type::UNSIGNED);
}

void Evaluate::SelfDetermine::edit(UnaryExpression* ue) {
  Editor::edit(ue);

  size_t w = 0;
  auto r = false;
  auto s = false;
  switch (ue->get_op()) {
    case UnaryExpression::Op::PLUS:
    case UnaryExpression::Op::MINUS:
      w = ue->get_lhs()->bit_val_[0].size();
      r = ue->get_lhs()->bit_val_[0].is_real();
      s = ue->get_lhs()->bit_val_[0].is_signed();
      break;
    case UnaryExpression::Op::TILDE:
      w = ue->get_lhs()->bit_val_[0].size();
      break;
    case UnaryExpression::Op::AMP:
    case UnaryExpression::Op::TAMP:
    case UnaryExpression::Op::PIPE:
    case UnaryExpression::Op::TPIPE:
    case UnaryExpression::Op::CARAT:
    case UnaryExpression::Op::TCARAT:
    case UnaryExpression::Op::BANG:
      w = 1;
      break;
    default:
     assert(false); 
  }
  ue->bit_val_.push_back(Bits(w, 0));
  if (r) {
    ue->bit_val_[0].set_type(Bits::Type::REAL);
  } else if (s) {
    ue->bit_val_[0].set_signed();
  }
}

void Evaluate::SelfDetermine::edit(GenvarDeclaration* gd) {
  // Don't descend on id, we handle it below

  // Genvars are treated as 32-bit signed integers
  gd->get_id()->bit_val_.push_back(Bits(32, 0));
  gd->get_id()->bit_val_[0].set_type(Bits::Type::SIGNED);
}

void Evaluate::SelfDetermine::edit(LocalparamDeclaration* ld) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  ld->accept_val(this);
  // Hold off on initial assignment until context-determination
}

void Evaluate::SelfDetermine::edit(NetDeclaration* nd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)

  // Calculate arity
  size_t arity = 1;
  for (auto i = nd->get_id()->begin_dim(), ie = nd->get_id()->end_dim(); i != ie; ++i) {
    const auto rng = eval_->get_range(*i);
    arity *= ((rng.first-rng.second)+1);
  }
  // Calculate width
  size_t w = 1;
  if (nd->is_non_null_dim()) {
    const auto rng = eval_->get_range(nd->get_dim());
    w = (rng.first-rng.second)+1;
  }
  // Allocate bits
  nd->get_id()->bit_val_.resize(arity);
  for (size_t i = 0; i < arity; ++i) {
    nd->get_id()->bit_val_[i].resize(w);
    if (nd->get_type() == Declaration::Type::UNTYPED) {
      nd->get_id()->bit_val_[i].set_type(Bits::Type::UNSIGNED);
    } else {
      nd->get_id()->bit_val_[i].set_type(static_cast<Bits::Type>(nd->get_type()));
    }
  }

  // TODO(eschkufz) For whatever reason, we've chosen to materialize net
  // declarations as declarations followed by continuous assigns. So there's no
  // initial value to worry about here.
}

void Evaluate::SelfDetermine::edit(ParameterDeclaration* pd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  pd->accept_val(this);
  // Hold off on initial assignment until context-determination
}

void Evaluate::SelfDetermine::edit(RegDeclaration* rd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  rd->accept_val(this);

  // Calculate arity
  size_t arity = 1;
  for (auto i = rd->get_id()->begin_dim(), ie = rd->get_id()->end_dim(); i != ie; ++i) {
    const auto rng = eval_->get_range(*i);
    arity *= ((rng.first-rng.second)+1);
  }
  // Calculate width
  size_t w = 1;
  if (rd->is_non_null_dim()) {
    const auto rng = eval_->get_range(rd->get_dim());
    w = (rng.first-rng.second)+1;
  }
  // Allocate bits:
  rd->get_id()->bit_val_.resize(arity);
  for (size_t i = 0; i < arity; ++i) {
    rd->get_id()->bit_val_[i].resize(w);
    if (rd->get_type() == Declaration::Type::UNTYPED) {
      rd->get_id()->bit_val_[i].set_type(Bits::Type::UNSIGNED);
    } else {
      rd->get_id()->bit_val_[i].set_type(static_cast<Bits::Type>(rd->get_type()));
    }
  }

  // Hold off on initial assignment here. We may be doing some size extending
  // in context-determination. We'll want to wait until then to compute the
  // value of this variable.
}

Evaluate::ContextDetermine::ContextDetermine(Evaluate* eval) {
  eval_ = eval;
}

void Evaluate::ContextDetermine::edit(BinaryExpression* be) {
  size_t w = 0;
  switch (be->get_op()) {
    case BinaryExpression::Op::PLUS:
    case BinaryExpression::Op::MINUS:
    case BinaryExpression::Op::TIMES:
    case BinaryExpression::Op::DIV:
    case BinaryExpression::Op::MOD:
    case BinaryExpression::Op::AMP:
    case BinaryExpression::Op::PIPE:
    case BinaryExpression::Op::CARAT:
    case BinaryExpression::Op::TCARAT:
      // Both operands are context dependent
      be->get_lhs()->bit_val_[0].resize(be->bit_val_[0].size());
      be->get_rhs()->bit_val_[0].resize(be->bit_val_[0].size());
      break;
    case BinaryExpression::Op::EEEQ:
    case BinaryExpression::Op::BEEQ:
    case BinaryExpression::Op::EEQ:
    case BinaryExpression::Op::BEQ:
    case BinaryExpression::Op::GT:
    case BinaryExpression::Op::GEQ:
    case BinaryExpression::Op::LT:
    case BinaryExpression::Op::LEQ:
      // Operands are sort-of context dependent. They affect each other
      // independently of what's going on here.
      w = max(eval_->get_width(be->get_lhs()), eval_->get_width(be->get_rhs()));
      be->get_lhs()->bit_val_[0].resize(w);
      be->get_rhs()->bit_val_[0].resize(w);
      break;
    case BinaryExpression::Op::AAMP:
    case BinaryExpression::Op::PPIPE:
      // Both operands are self-determined
      break;
    case BinaryExpression::Op::GGT:
    case BinaryExpression::Op::LLT:
    case BinaryExpression::Op::TTIMES:
    case BinaryExpression::Op::GGGT:
    case BinaryExpression::Op::LLLT:
      // The right-hand side of these expressions is self-determined
      be->get_lhs()->bit_val_[0].resize(be->bit_val_[0].size());
      break;
    default:
      assert(false);
  }

  Editor::edit(be);        
}

void Evaluate::ContextDetermine::edit(ConditionalExpression* ce) {
  // Conditions are self-determined
  ce->get_lhs()->bit_val_[0].resize(ce->bit_val_[0].size());
  ce->get_rhs()->bit_val_[0].resize(ce->bit_val_[0].size());

  Editor::edit(ce);
}

void Evaluate::ContextDetermine::edit(FeofExpression* fe) {
  // Nothing to do here. Feof doesn't determine the size or type of its
  // argument.
  (void) fe;
}

void Evaluate::ContextDetermine::edit(FopenExpression* fe) {
  // Nothing to do here. Fopen doesn't determine the size or type of its
  // argument.
  (void) fe;
}

void Evaluate::ContextDetermine::edit(Identifier* id) {
  // Nothing to do here. The only expressions we can reach from here are subscripts,
  // which we treat as separate subtrees.
  (void) id;
}

void Evaluate::ContextDetermine::edit(MultipleConcatenation* mc) {
  // Don't descend on expr, this is a separate expression tree.
  mc->accept_concat(this);
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
    case UnaryExpression::Op::PLUS:
    case UnaryExpression::Op::MINUS:
    case UnaryExpression::Op::TILDE:
      ue->get_lhs()->bit_val_[0].resize(ue->bit_val_[0].size());
      break;
    case UnaryExpression::Op::AMP:
    case UnaryExpression::Op::TAMP:
    case UnaryExpression::Op::PIPE:
    case UnaryExpression::Op::TPIPE:
    case UnaryExpression::Op::CARAT:
    case UnaryExpression::Op::TCARAT:
    case UnaryExpression::Op::BANG:
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

void Evaluate::ContextDetermine::edit(LocalparamDeclaration* ld) {
  // Parameters don't impose constraints on their rhs
  ld->accept_val(this);
  // But they inherit size and width from their rhs according to some pretty
  // baroque rules.
  ld->get_id()->bit_val_.push_back(Bits(false));
  switch (ld->get_type()) {
    case Declaration::Type::UNSIGNED:
    case Declaration::Type::SIGNED:
      ld->get_id()->bit_val_[0].set_type(static_cast<Bits::Type>(ld->get_type()));
      if (ld->is_null_dim()) {
        ld->get_id()->bit_val_[0].resize(min(static_cast<size_t>(32), ld->get_val()->bit_val_[0].size()));
      } else {
        const auto rng = eval_->get_range(ld->get_dim());
        ld->get_id()->bit_val_[0].resize((rng.first-rng.second)+1);
      }
      break;
    case Declaration::Type::REAL:
      ld->get_id()->bit_val_[0].set_type(Bits::Type::REAL);
      ld->get_id()->bit_val_[0].resize(64);
      break;
    default:
      if (ld->is_null_dim()) {
        ld->get_id()->bit_val_[0].set_type(ld->get_val()->bit_val_[0].get_type());
        ld->get_id()->bit_val_[0].resize(ld->get_val()->bit_val_[0].size());
      } else {
        ld->get_id()->bit_val_[0].set_type(Bits::Type::UNSIGNED);     
        const auto rng = eval_->get_range(ld->get_dim());
        ld->get_id()->bit_val_[0].resize((rng.first-rng.second)+1);
      }
      break;
  }
  // Now that we're all done, we can perform the initial assignment
  ld->get_id()->bit_val_[0].assign(eval_->get_value(ld->get_val()));
}

void Evaluate::ContextDetermine::edit(NetDeclaration* nd) { 
  // There's nothing left to do here. There's no rhs to worry about.
  (void) nd;
}

void Evaluate::ContextDetermine::edit(ParameterDeclaration* pd) {
  // Parameters don't impose constraints on their rhs
  pd->accept_val(this);
  // But they inherit size and width from their rhs according to some pretty
  // baroque rules.
  pd->get_id()->bit_val_.push_back(Bits(false));
  switch (pd->get_type()) {
    case Declaration::Type::UNSIGNED:
    case Declaration::Type::SIGNED:
      pd->get_id()->bit_val_[0].set_type(static_cast<Bits::Type>(pd->get_type()));
      if (pd->is_null_dim()) {
        pd->get_id()->bit_val_[0].resize(min(static_cast<size_t>(32), pd->get_val()->bit_val_[0].size()));
      } else {
        const auto rng = eval_->get_range(pd->get_dim());
        pd->get_id()->bit_val_[0].resize((rng.first-rng.second)+1);
      }
      break;
    case Declaration::Type::REAL:
      pd->get_id()->bit_val_[0].set_type(Bits::Type::REAL);
      pd->get_id()->bit_val_[0].resize(64);
      break;
    default:
      if (pd->is_null_dim()) {
        pd->get_id()->bit_val_[0].set_type(pd->get_val()->bit_val_[0].get_type());
        pd->get_id()->bit_val_[0].resize(pd->get_val()->bit_val_[0].size());
      } else {
        pd->get_id()->bit_val_[0].set_type(Bits::Type::UNSIGNED);     
        const auto rng = eval_->get_range(pd->get_dim());
        pd->get_id()->bit_val_[0].resize((rng.first-rng.second)+1);
      }
      break;
  }
  // Now that we're all done, we can perform the initial assignment
  pd->get_id()->bit_val_[0].assign(eval_->get_value(pd->get_val()));
}

void Evaluate::ContextDetermine::edit(RegDeclaration* rd) {
  // Nothing to do if there's no assignment happening here
  if (rd->is_null_val()) {
    return;
  }
  // The parser should guarantee that only scalar declarations
  // have initial values.
  assert(rd->get_id()->bit_val_.size() == 1);

  // Assignments impose larger sizes but not type constraints
  if (rd->get_id()->bit_val_[0].size() > rd->get_val()->bit_val_[0].size()) {
    rd->get_val()->bit_val_[0].resize(rd->get_id()->bit_val_[0].size());
  }
  rd->accept_val(this);

  // Now that we're context determined, we can perform initial assignment
  rd->get_id()->bit_val_[0].assign(eval_->get_value(rd->get_val()));
}

void Evaluate::ContextDetermine::edit(VariableAssign* va) {
  // Assignments impose larger sizes but not type constraints
  if (va->get_lhs()->bit_val_[0].size() > va->get_rhs()->bit_val_[0].size()) {
    va->get_rhs()->bit_val_[0].resize(va->get_lhs()->bit_val_[0].size());
  }
  va->accept_rhs(this);

  // We're context determined, but these assignments happen dynamically.
  // Nothing more to do here.
}

} // namespace cascade
