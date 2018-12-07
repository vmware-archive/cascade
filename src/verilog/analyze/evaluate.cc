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


#include "src/verilog/print/term/term_printer.h"


using namespace std;

namespace cascade {

bool Evaluate::is_scalar(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);
  return (id != r) || r->get_dim()->empty();
}

bool Evaluate::is_array(const Identifier* id) {
  return !is_scalar(id);
}

vector<size_t> Evaluate::get_arity(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);

  // Variable references are always scalar
  if (id != r) {
    return vector<size_t>();
  }
  // Otherwise, iterate over the declaration to determine arity
  vector<size_t> res;
  for (auto d : *r->get_dim()) {
    const auto rng = get_range(d);
    res.push_back(rng.first - rng.second + 1);
  }
  return res;
}

size_t Evaluate::get_width(const Expression* e) {
  init(const_cast<Expression*>(e));
  return e->bit_val_[0].size();
}

bool Evaluate::get_signed(const Expression* e) {
  init(const_cast<Expression*>(e));
  return e->bit_val_[0].is_signed();
}

const Bits& Evaluate::get_value(const Expression* e) {
  init(const_cast<Expression*>(e));
  if (e->needs_update_) {
    const_cast<Expression*>(e)->accept(this);
    const_cast<Expression*>(e)->needs_update_ = false;
  }
  return e->bit_val_[0];
}

const vector<Bits>& Evaluate::get_array_value(const Identifier* i) {
  init(const_cast<Identifier*>(i));
  if (i->needs_update_) {
    const_cast<Identifier*>(i)->accept(this);
    const_cast<Identifier*>(i)->needs_update_ = false;
  }
  return i->bit_val_;
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
  // Find the variable that we're referring to. 
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);
  init(const_cast<Identifier*>(r));

  // Perform the assignment
  const auto dres = dereference(r, id);
  if (get<1>(dres) == -1) {
    if (!r->bit_val_[get<0>(dres)].eq(val)) {
      const_cast<Identifier*>(r)->bit_val_[get<0>(dres)].assign(val);
      flag_changed(r);
    }
  } else {
    const auto msb = min((size_t) get<1>(dres), get_width(r)-1);
    const auto lsb = min((size_t) get<2>(dres), get_width(r)-1);
    if (!r->bit_val_[get<0>(dres)].eq(msb, lsb, val)) {
      const_cast<Identifier*>(r)->bit_val_[get<0>(dres)].assign(msb, lsb, val);
      flag_changed(r);
    }
  }
}

void Evaluate::assign_array_value(const Identifier* id, const vector<Bits>& val) {
  // Find the variable that we're referring to. 
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);
  init(const_cast<Identifier*>(r));

  // Perform the assignment
  // TODO: Is it worthwhile to check whether anything has changed here?
  assert(val.size() == id->bit_val_.size());
  for (size_t i = 0, ie = id->bit_val_.size(); i < ie; ++i) {
    const_cast<Identifier*>(r)->bit_val_[i].assign(val[i]);
  }
  flag_changed(r);
}

tuple<size_t,int,int> Evaluate::dereference(const Identifier* r, const Identifier* i) {
  // Nothing to do if this is a scalar variable
  if (r->get_dim()->empty()) {
    const auto rng = i->get_dim()->empty() ? 
      make_pair<size_t, size_t>(-1,-1) : 
      get_range(i->get_dim()->front());
    return make_tuple(0, rng.first, rng.second);
  }

  // Otherwise, i had better have at most one more dimension than r
  assert(i->get_dim()->size() - r->get_dim()->size() <= 1);

  // Iterators
  auto iitr = i->get_dim()->begin();
  auto ritr = r->get_dim()->begin();
  // The index we're looking for
  size_t idx = 0;
  // Multiplier for multi-dimensional arrays
  size_t mul = r->bit_val_.size();

  // Walk along subscripts 
  for (auto re = r->get_dim()->end(); ritr != re; ++iitr, ++ritr) {
    assert(dynamic_cast<RangeExpression*>(*ritr) != nullptr);
    assert(dynamic_cast<RangeExpression*>(*iitr) == nullptr);

    const auto rval = get_range(*ritr);
    const auto ival = get_value(*iitr).to_int();

    mul /= (rval.first-rval.second+1);
    idx += mul * ival;
  }
  // Out of bounds accesses are undefined, so we'll map them to a safe value
  const auto rng = iitr == i->get_dim()->end() ?
    make_pair<size_t, size_t>(-1,-1) : 
    get_range(*iitr);
  return make_tuple(idx >= r->bit_val_.size() ? 0 : idx, rng.first, rng.second);
}

void Evaluate::assign_value(const Identifier* id, size_t idx, int msb, int lsb, const Bits& val) {
  init(const_cast<Identifier*>(id));
  assert(idx < id->bit_val_.size());

  if (msb == -1) {
    if (!id->bit_val_[idx].eq(val)) {
      const_cast<Identifier*>(id)->bit_val_[idx].assign(val);
      flag_changed(id);
    }
  } else {
    const auto m = min((size_t) msb, get_width(id)-1);
    const auto l = min((size_t) lsb, get_width(id)-1);
    if (!id->bit_val_[idx].eq(m, l, val)) {
      const_cast<Identifier*>(id)->bit_val_[idx].assign(m, l, val);
      flag_changed(id);
    }
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
      get_value(be->get_lhs()).arithmetic_plus(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::MINUS:
      get_value(be->get_lhs()).arithmetic_minus(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::TIMES:
      get_value(be->get_lhs()).arithmetic_multiply(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::DIV:
      get_value(be->get_lhs()).arithmetic_divide(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::MOD:
      get_value(be->get_lhs()).arithmetic_mod(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    // NOTE: These are equivalent because we don't support x and z
    case BinaryExpression::EEEQ:
    case BinaryExpression::EEQ:
      get_value(be->get_lhs()).logical_eq(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    // NOTE: These are equivalent because we don't support x and z
    case BinaryExpression::BEEQ:
    case BinaryExpression::BEQ:
      get_value(be->get_lhs()).logical_ne(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::AAMP:
      get_value(be->get_lhs()).logical_and(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::PPIPE:
      get_value(be->get_lhs()).logical_or(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::TTIMES:
      get_value(be->get_lhs()).arithmetic_pow(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::LT:
      get_value(be->get_lhs()).logical_lt(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::LEQ:
      get_value(be->get_lhs()).logical_lte(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::GT:
      get_value(be->get_lhs()).logical_gt(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::GEQ:
      get_value(be->get_lhs()).logical_gte(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::AMP:
      get_value(be->get_lhs()).bitwise_and(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::PIPE:
      get_value(be->get_lhs()).bitwise_or(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::CARAT:
      get_value(be->get_lhs()).bitwise_xor(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::TCARAT:
      get_value(be->get_lhs()).bitwise_xnor(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::LLT:
      get_value(be->get_lhs()).bitwise_sll(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::LLLT:
      get_value(be->get_lhs()).bitwise_sal(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::GGT:
      get_value(be->get_lhs()).bitwise_slr(get_value(be->get_rhs()), be->bit_val_[0]);
      break;
    case BinaryExpression::GGGT:
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

void Evaluate::edit(NestedExpression* ne) {
  ne->bit_val_[0].assign(get_value(ne->get_expr()));
}

void Evaluate::edit(Concatenation* c) {
  auto i = c->get_exprs()->begin();
  c->bit_val_[0].assign(get_value(*i++));
  for (auto ie = c->get_exprs()->end(); i != ie; ++i) {
    c->bit_val_[0].concat( get_value(*i));
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
  const auto dres = dereference(r, id);
  if (get<1>(dres) == -1) {
    id->bit_val_[0].assign(get_array_value(r)[get<0>(dres)]);
  } else {
    const auto msb = min((size_t) get<1>(dres), get_width(r)-1);
    const auto lsb = min((size_t) get<2>(dres), get_width(r)-1);
    id->bit_val_[0].assign(get_array_value(r)[get<0>(dres)], msb, lsb);
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
  // TODO: Support this language feature.
  assert(false);
  (void) s;
}

void Evaluate::edit(UnaryExpression* ue) {
  switch (ue->get_op()) {
    case UnaryExpression::PLUS:
      get_value(ue->get_lhs()).arithmetic_plus(ue->bit_val_[0]);
      break;
    case UnaryExpression::MINUS:
      get_value(ue->get_lhs()).arithmetic_minus(ue->bit_val_[0]);
      break;
    case UnaryExpression::BANG:
      get_value(ue->get_lhs()).logical_not(ue->bit_val_[0]);
      break;
    case UnaryExpression::TILDE:
      get_value(ue->get_lhs()).bitwise_not(ue->bit_val_[0]);
      break;
    case UnaryExpression::AMP:
      get_value(ue->get_lhs()).reduce_and(ue->bit_val_[0]);
      break;
    case UnaryExpression::TAMP:
      get_value(ue->get_lhs()).reduce_nand(ue->bit_val_[0]);
      break;
    case UnaryExpression::PIPE:
      get_value(ue->get_lhs()).reduce_or(ue->bit_val_[0]);
      break;
    case UnaryExpression::TPIPE:
      get_value(ue->get_lhs()).reduce_nor(ue->bit_val_[0]);
      break;
    case UnaryExpression::CARAT:
      get_value(ue->get_lhs()).reduce_xor(ue->bit_val_[0]);
      break;
    case UnaryExpression::TCARAT:
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
  if (!e->bit_val_.empty()) {
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
  be->bit_val_.clear();
  be->needs_update_ = true;
  Editor::edit(be);
}

void Evaluate::Invalidate::edit(ConditionalExpression* ce) {
  ce->bit_val_.clear();
  ce->needs_update_ = true;
  Editor::edit(ce);
}

void Evaluate::Invalidate::edit(NestedExpression* ne) {
  ne->bit_val_.clear();
  ne->needs_update_ = true;
  Editor::edit(ne);
}

void Evaluate::Invalidate::edit(Concatenation* c) {
  c->bit_val_.clear();
  c->needs_update_ = true;
  Editor::edit(c);
}

void Evaluate::Invalidate::edit(Identifier* id) {
  id->bit_val_.clear();
  id->needs_update_ = true;
  // Don't descend into a different subtree
}

void Evaluate::Invalidate::edit(MultipleConcatenation* mc) {
  mc->bit_val_.clear();
  mc->needs_update_ = true;
  // Don't descend into a different subtree
  mc->get_concat()->accept(this);
}

void Evaluate::Invalidate::edit(Number* n) {
  n->bit_val_.clear();
  n->needs_update_ = true;
}

void Evaluate::Invalidate::edit(String* s) {
  // TODO: Support for this language feature
  assert(false);
  (void) s;
}

void Evaluate::Invalidate::edit(UnaryExpression* ue) {
  ue->bit_val_.clear();
  ue->needs_update_ = true;
  Editor::edit(ue);
}

void Evaluate::Invalidate::edit(GenvarDeclaration* gd) {
  Editor::edit(gd);
}

void Evaluate::Invalidate::edit(IntegerDeclaration* id) {
  Editor::edit(id);
}

void Evaluate::Invalidate::edit(LocalparamDeclaration* ld) {
  ld->get_id()->accept(this);
  // Ignore dim: Don't descend into a different subtree
  ld->get_val()->accept(this);
}

void Evaluate::Invalidate::edit(NetDeclaration* nd) { 
  nd->get_id()->accept(this);
  // Ignore dim: Don't descend into a different subtree
}

void Evaluate::Invalidate::edit(ParameterDeclaration* pd) {
  pd->get_id()->accept(this);
  // Ignore dim: Don't descend into a different subtree
  pd->get_val()->accept(this);
}

void Evaluate::Invalidate::edit(RegDeclaration* rd) {
  rd->get_id()->accept(this);
  // Ignore dim: Don't descend into a different subtree
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
      s = be->get_lhs()->bit_val_[0].is_signed() && be->get_rhs()->bit_val_[0].is_signed();
      w = max(be->get_lhs()->bit_val_[0].size(), be->get_rhs()->bit_val_[0].size());
      break;
    case BinaryExpression::AMP:
    case BinaryExpression::PIPE:
    case BinaryExpression::CARAT:
    case BinaryExpression::TCARAT:
      w = max(be->get_lhs()->bit_val_[0].size(), be->get_rhs()->bit_val_[0].size());
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
      w = be->get_lhs()->bit_val_[0].size();
      s = be->get_lhs()->bit_val_[0].is_signed();
      break;
    case BinaryExpression::TTIMES:
      w = be->get_lhs()->bit_val_[0].size();
      s = false;
      break;
    case BinaryExpression::GGGT:
    case BinaryExpression::LLLT:
      w = be->get_lhs()->bit_val_[0].size();
      s = be->get_lhs()->bit_val_[0].is_signed();
      break;
    default:
      assert(false);
  }
  be->bit_val_.emplace_back(Bits(w, 0));
  be->bit_val_[0].set_signed(s);
}

void Evaluate::SelfDetermine::edit(ConditionalExpression* ce) {
  Editor::edit(ce);

  // TODO: Are we sure that this is the right calculus for sign?  I haven't
  // been able to find anything to this point in the spec.
  bool s = ce->get_lhs()->bit_val_[0].is_signed() && ce->get_rhs()->bit_val_[0].is_signed();
  size_t w = max(ce->get_lhs()->bit_val_[0].size(), ce->get_rhs()->bit_val_[0].size());
  ce->bit_val_.emplace_back(Bits(w, 0));
  ce->bit_val_[0].set_signed(s);
}

void Evaluate::SelfDetermine::edit(NestedExpression* ne) {
  Editor::edit(ne);

  size_t w = ne->get_expr()->bit_val_[0].size();
  bool s = ne->get_expr()->bit_val_[0].is_signed();
  ne->bit_val_.emplace_back(Bits(w, 0));
  ne->bit_val_[0].set_signed(s);
}

void Evaluate::SelfDetermine::edit(Concatenation* c) {
  Editor::edit(c);

  size_t w = 0;
  for (auto e : *c->get_exprs()) {
    w += e->bit_val_[0].size();
  }
  c->bit_val_.emplace_back(Bits(w, 0));
  c->bit_val_[0].set_signed(false);
}

void Evaluate::SelfDetermine::edit(Identifier* id) {
  // Don't descend on dim. We treat it as a separate subtree.

  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);

  size_t w = 0;
  bool s = false;
  if (id->get_dim()->size() == r->get_dim()->size()) {
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
  id->bit_val_.emplace_back(Bits(w, 0));
  id->bit_val_[0].set_signed(s);
}

void Evaluate::SelfDetermine::edit(MultipleConcatenation* mc) {
  // Don't descend on expr, this is a separate expression tree.
  mc->get_concat()->accept(this);

  size_t w = Evaluate().get_value(mc->get_expr()).to_int() * mc->get_concat()->bit_val_[0].size();
  mc->bit_val_.emplace_back(Bits(w, 0));
  mc->bit_val_[0].set_signed(false);
}

void Evaluate::SelfDetermine::edit(Number* n) {
  // Copying the underlying value of n sets value, width, and sign
  n->bit_val_.push_back(n->get_val());
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
      w = ue->get_lhs()->bit_val_[0].size();
      s = ue->get_lhs()->bit_val_[0].is_signed();
      break;
    case UnaryExpression::TILDE:
      w = ue->get_lhs()->bit_val_[0].size();
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
  ue->bit_val_.emplace_back(Bits(w, 0));
  ue->bit_val_[0].set_signed(s);
}

void Evaluate::SelfDetermine::edit(GenvarDeclaration* gd) {
  // Don't descend on id, we handle it below

  // Genvars are materialized as localparams and follow the same rules.  For
  // lack of information in this declaration though, we have to assume 32 bit
  // unsigned. 
  gd->get_id()->bit_val_.emplace_back(Bits(32, 0));
  gd->get_id()->bit_val_[0].set_signed(false);
}

void Evaluate::SelfDetermine::edit(IntegerDeclaration* id) {
  // Don't descend on id, we handle it below
  id->get_val()->accept(this);

  // Calculate arity
  size_t arity = 1;
  for (auto dim : *id->get_id()->get_dim()) {
    const auto rng = Evaluate().get_range(dim);
    arity *= (rng.first-rng.second+1);
  }
  // Allocate bits: Integers must be a minimum of 32 bits and are always signed
  id->get_id()->bit_val_.resize(arity);
  for (size_t i = 0; i < arity; ++i) { 
    id->get_id()->bit_val_[i].resize(32);
    id->get_id()->bit_val_[i].set_signed(true);
  }

  // Hold off on initial assignment here. We may be doing some size extending
  // in context-determination. We'll want to wait until then to compute the
  // value of this variable.
}

void Evaluate::SelfDetermine::edit(LocalparamDeclaration* ld) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  ld->get_val()->accept(this);

  // Start out with a basic allocation of bits
  ld->get_id()->bit_val_.emplace_back(Bits(false));
  // Parameter declaration may override size and sign
  ld->get_id()->bit_val_[0].set_signed(ld->get_signed());
  if (!ld->get_dim()->null()) {
    const auto rng = Evaluate().get_range(ld->get_dim()->get());
    ld->get_id()->bit_val_[0].resize(rng.first-rng.second+1);
  } 

  // Hold off on initial assignment here. We may be doing some size extending
  // in context-determination. We'll want to wait until then to compute the
  // value of this variable.
}

void Evaluate::SelfDetermine::edit(NetDeclaration* nd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  nd->get_ctrl()->accept(this);

  // Calculate arity
  size_t arity = 1;
  for (auto dim : *nd->get_id()->get_dim()) {
    const auto rng = Evaluate().get_range(dim);
    arity *= (rng.first-rng.second+1);
  }
  // Calculate width
  size_t w = 1;
  if (!nd->get_dim()->null()) {
    const auto rng = Evaluate().get_range(nd->get_dim()->get());
    w = rng.first-rng.second+1;
  }
  // Allocate bits
  nd->get_id()->bit_val_.resize(arity);
  for (size_t i = 0; i < arity; ++i) {
    nd->get_id()->bit_val_[i].resize(w);
    nd->get_id()->bit_val_[i].set_signed(nd->get_signed());
  }

  // For whatever reason, we've chosen to materialize net declarations as
  // declarations followed by continuous assigns. So there's no initial value
  // to worry about here.
}

void Evaluate::SelfDetermine::edit(ParameterDeclaration* pd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  pd->get_val()->accept(this);

  // Start out with a basic allocation of bits
  pd->get_id()->bit_val_.emplace_back(Bits(false));
  // Parameter declaration may override size and sign
  pd->get_id()->bit_val_[0].set_signed(pd->get_signed());
  if (!pd->get_dim()->null()) {
    const auto rng = Evaluate().get_range(pd->get_dim()->get());
    pd->get_id()->bit_val_[0].resize(rng.first-rng.second+1);
  }

  // Hold off on initial assignment here. We may be doing some size extending
  // in context-determination. We'll want to wait until then to compute the
  // value of this variable.
}

void Evaluate::SelfDetermine::edit(RegDeclaration* rd) {
  // Don't descend on id or dim (id we handle below, dim is a separate subtree)
  rd->get_val()->accept(this);

  // Calculate arity
  size_t arity = 1;
  for (auto dim : *rd->get_id()->get_dim()) {
    const auto rng = Evaluate().get_range(dim);
    arity *= (rng.first-rng.second+1);
  }
  // Calculate width
  size_t w = 1;
  if (!rd->get_dim()->null()) {
    const auto rng = Evaluate().get_range(rd->get_dim()->get());
    w = rng.first-rng.second+1;
  }
  // Allocate bits:
  rd->get_id()->bit_val_.resize(arity);
  for (size_t i = 0; i < arity; ++i) {
    rd->get_id()->bit_val_[i].resize(w);
    rd->get_id()->bit_val_[i].set_signed(rd->get_signed());
  }

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
      be->get_lhs()->bit_val_[0].set_signed(be->bit_val_[0].is_signed());
      be->get_lhs()->bit_val_[0].resize(be->bit_val_[0].size());
      be->get_rhs()->bit_val_[0].set_signed(be->bit_val_[0].is_signed());
      be->get_rhs()->bit_val_[0].resize(be->bit_val_[0].size());
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
      be->get_lhs()->bit_val_[0].set_signed(s);
      be->get_lhs()->bit_val_[0].resize(w);
      be->get_rhs()->bit_val_[0].set_signed(s);
      be->get_rhs()->bit_val_[0].resize(w);
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
      be->get_lhs()->bit_val_[0].set_signed(be->bit_val_[0].is_signed());
      be->get_lhs()->bit_val_[0].resize(be->bit_val_[0].size());
      break;
    default:
      assert(false);
  }

  Editor::edit(be);        
}

void Evaluate::ContextDetermine::edit(ConditionalExpression* ce) {
  // Conditions are self-determined
  ce->get_lhs()->bit_val_[0].set_signed(ce->bit_val_[0].is_signed());
  ce->get_lhs()->bit_val_[0].resize(ce->bit_val_[0].size());
  ce->get_rhs()->bit_val_[0].set_signed(ce->bit_val_[0].is_signed());
  ce->get_rhs()->bit_val_[0].resize(ce->bit_val_[0].size());

  Editor::edit(ce);
}

void Evaluate::ContextDetermine::edit(NestedExpression* ne) {
  ne->get_expr()->bit_val_[0].set_signed(ne->bit_val_[0].is_signed());
  ne->get_expr()->bit_val_[0].resize(ne->bit_val_[0].size());
  
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
      ue->get_lhs()->bit_val_[0].set_signed(ue->bit_val_[0].is_signed());
      ue->get_lhs()->bit_val_[0].resize(ue->bit_val_[0].size());
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
  // The parser should guarantee that only scalar declarations
  // have initial values.
  assert(id->get_id()->bit_val_.size() == 1);

  // Assignments impose larger sizes but not sign constraints
  if (id->get_val()->get()->bit_val_[0].size() < 32) {
    id->get_val()->get()->bit_val_[0].resize(32);
  }
  id->get_val()->accept(this);

  // Now that we're context determined, we can perform initial assignment
  id->get_id()->bit_val_[0].assign(Evaluate().get_value(id->get_val()->get()));
}

void Evaluate::ContextDetermine::edit(LocalparamDeclaration* ld) {
  // Parameters don't impose constraints on their rhs
  ld->get_val()->accept(this);
  // But they inherit size and width from their rhs unless otherwise specified
  if (!ld->get_signed()) {
    ld->get_id()->bit_val_[0].set_signed(ld->get_val()->bit_val_[0].is_signed());
  }
  if (ld->get_dim()->null()) {
    ld->get_id()->bit_val_[0].resize(ld->get_val()->bit_val_[0].size());
  }

  // Now that we're context determined, we can perform initial assignment
  ld->get_id()->bit_val_[0].assign(Evaluate().get_value(ld->get_val()));
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
    pd->get_id()->bit_val_[0].set_signed(pd->get_val()->bit_val_[0].is_signed());
  }
  if (pd->get_dim()->null()) {
    pd->get_id()->bit_val_[0].resize(pd->get_val()->bit_val_[0].size());
  }

  // Now that we're context determined, we can perform initial assignment
  pd->get_id()->bit_val_[0].assign(Evaluate().get_value(pd->get_val()));
}

void Evaluate::ContextDetermine::edit(RegDeclaration* rd) {
  // Nothing to do if there's no assignment happening here
  if (rd->get_val()->null()) {
    return;
  }
  // The parser should guarantee that only scalar declarations
  // have initial values.
  assert(rd->get_id()->bit_val_.size() == 1);

  // Assignments impose larger sizes but not sign constraints
  if (rd->get_id()->bit_val_[0].size() > rd->get_val()->get()->bit_val_[0].size()) {
    rd->get_val()->get()->bit_val_[0].resize(rd->get_id()->bit_val_[0].size());
  }
  rd->get_val()->accept(this);

  // Now that we're context determined, we can perform initial assignment
  rd->get_id()->bit_val_[0].assign(Evaluate().get_value(rd->get_val()->get()));
}

void Evaluate::ContextDetermine::edit(VariableAssign* va) {
  // Assignments impose larger sizes but not sign constraints
  if (va->get_lhs()->bit_val_[0].size() > va->get_rhs()->bit_val_[0].size()) {
    va->get_rhs()->bit_val_[0].resize(va->get_lhs()->bit_val_[0].size());
  }
  va->get_rhs()->accept(this);

  // We're context determined, but these assignments happen dynamically.
  // Nothing more to do here.
}

} // namespace cascade
