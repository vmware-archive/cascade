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
#include "src/verilog/ast/ast.h"
#include "src/verilog/analyze/resolve.h"

using namespace std;

namespace cascade {

const Bits& Evaluate::get_value(const Expression* e) {
  if (e->needs_update_) {
    const_cast<Expression*>(e)->accept(this);
    const_cast<Expression*>(e)->needs_update_ = false;
  }
  return e->bit_val_;
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

void Evaluate::init_value(const Declaration* d) {
  const_cast<Declaration*>(d)->accept(this);
}

void Evaluate::assign_value(const Identifier* id, const Bits& val) {
  if (id->bit_val_ != val) {
    const_cast<Identifier*>(id)->bit_val_.assign(val);
    invalidate(id);
  }
}

void Evaluate::assign_value(const Identifier* id, const Bits& val, size_t idx) {
  if (!const_cast<Bits*>(&id->bit_val_)->eq(val, idx)) {
    const_cast<Identifier*>(id)->bit_val_.assign(idx, val);
    invalidate(id);
  }
}

void Evaluate::assign_value(const Identifier* id, const Bits& val, size_t i, size_t j) {
  if (!const_cast<Bits*>(&id->bit_val_)->eq(val, i, j)) {
    const_cast<Identifier*>(id)->bit_val_.assign(i, j, val);
    invalidate(id);
  }
}

void Evaluate::edit(BinaryExpression* be) {
  be->bit_val_ = get_value(be->get_lhs());
  switch (be->get_op()) {
    case BinaryExpression::PLUS:
      be->bit_val_.arithmetic_plus(get_value(be->get_rhs()));
      break;
    case BinaryExpression::MINUS:
      be->bit_val_.arithmetic_minus(get_value(be->get_rhs()));
      break;
    case BinaryExpression::TIMES:
      be->bit_val_.arithmetic_multiply(get_value(be->get_rhs()));
      break;
    case BinaryExpression::DIV:
      be->bit_val_.arithmetic_divide(get_value(be->get_rhs()));
      break;
    case BinaryExpression::MOD:
      be->bit_val_.arithmetic_mod(get_value(be->get_rhs()));
      break;
    case BinaryExpression::EEEQ:
      // NOTE: These are equivalent only insofar as we don't support x and z
    case BinaryExpression::EEQ:
      be->bit_val_.logical_eq(get_value(be->get_rhs()));
      break;
    case BinaryExpression::BEEQ:
      // NOTE: These are equivalent only insofar as we don't support x and z
    case BinaryExpression::BEQ:
      be->bit_val_.logical_ne(get_value(be->get_rhs()));
      break;
    case BinaryExpression::AAMP:
      be->bit_val_.logical_and(get_value(be->get_rhs()));
      break;
    case BinaryExpression::PPIPE:
      be->bit_val_.logical_or(get_value(be->get_rhs()));
      break;
    case BinaryExpression::TTIMES:
      be->bit_val_.arithmetic_pow(get_value(be->get_rhs()));
      break;
    case BinaryExpression::LT:
      be->bit_val_.logical_lt(get_value(be->get_rhs()));
      break;
    case BinaryExpression::LEQ:
      be->bit_val_.logical_lte(get_value(be->get_rhs()));
      break;
    case BinaryExpression::GT:
      be->bit_val_.logical_gt(get_value(be->get_rhs()));
      break;
    case BinaryExpression::GEQ:
      be->bit_val_.logical_gte(get_value(be->get_rhs()));
      break;
    case BinaryExpression::AMP:
      be->bit_val_.bitwise_and(get_value(be->get_rhs()));
      break;
    case BinaryExpression::PIPE:
      be->bit_val_.bitwise_or(get_value(be->get_rhs()));
      break;
    case BinaryExpression::CARAT:
      be->bit_val_.bitwise_xor(get_value(be->get_rhs()));
      break;
    case BinaryExpression::TCARAT:
      be->bit_val_.bitwise_xnor(get_value(be->get_rhs()));
      break;
    case BinaryExpression::LLT:
      be->bit_val_.bitwise_sll(get_value(be->get_rhs()));
      break;
    case BinaryExpression::LLLT:
      be->bit_val_.bitwise_sal(get_value(be->get_rhs()));
      break;
    case BinaryExpression::GGT:
      be->bit_val_.bitwise_slr(get_value(be->get_rhs()));
      break;
    case BinaryExpression::GGGT:
      be->bit_val_.bitwise_sar(get_value(be->get_rhs()));
      break;

    default:
      assert(false);
      break;
  }
}

void Evaluate::edit(ConditionalExpression* ce) {
  if (get_value(ce->get_cond()).to_bool()) {
    ce->bit_val_ = get_value(ce->get_lhs());
  } else {
    ce->bit_val_ = get_value(ce->get_rhs());
  }
}

void Evaluate::edit(NestedExpression* ne) {
  ne->bit_val_ = get_value(ne->get_expr());
}

void Evaluate::edit(Concatenation* c) {
  auto i = c->get_exprs()->begin();
  c->bit_val_ = get_value(*i++);
  for (auto ie = c->get_exprs()->end(); i != ie; ++i) {
    c->bit_val_.concat(get_value(*i));
  }
}

void Evaluate::edit(Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);
  if (r != id) { 
    id->bit_val_ = r->bit_val_;
  }
  if (!id->get_dim()->null()) {
    const auto range = get_range(id->get_dim()->get());
    id->bit_val_.slice(range.first, range.second);
  } 
}

void Evaluate::edit(MultipleConcatenation* mc) {
  const auto lhs = get_value(mc->get_expr()).to_int();
  mc->bit_val_ = get_value(mc->get_concat());
  for (size_t i = 1; i < lhs; ++i) {
    mc->bit_val_.concat(mc->get_concat()->bit_val_);
  }
}

void Evaluate::edit(Number* n) {
  n->bit_val_ = n->get_val();
}

void Evaluate::edit(String* s) {
  // TODO: Support this language feature.
  assert(false);
  (void) s;
}

void Evaluate::edit(RangeExpression* re) {
  // Control should never reach here.
  assert(false);
  (void) re;
}

void Evaluate::edit(UnaryExpression* ue) {
  ue->bit_val_ = get_value(ue->get_lhs());
  switch (ue->get_op()) {
    case UnaryExpression::PLUS:
      ue->bit_val_.arithmetic_plus();
      break;
    case UnaryExpression::MINUS:
      ue->bit_val_.arithmetic_minus();
      break;
    case UnaryExpression::BANG:
      ue->bit_val_.logical_not();
      break;
    case UnaryExpression::TILDE:
      ue->bit_val_.bitwise_not();
      break;
    case UnaryExpression::AMP:
      ue->bit_val_.reduce_and();
      break;
    case UnaryExpression::TAMP:
      ue->bit_val_.reduce_nand();
      break;
    case UnaryExpression::PIPE:
      ue->bit_val_.reduce_or();
      break;
    case UnaryExpression::TPIPE:
      ue->bit_val_.reduce_nor();
      break;
    case UnaryExpression::CARAT:
      ue->bit_val_.reduce_xor();
      break;
    case UnaryExpression::TCARAT:
      ue->bit_val_.reduce_xnor();
      break;
    default:
      assert(false);
      break;
  }
}

void Evaluate::edit(GenvarDeclaration* gd) {
  set_value(gd->get_id(), Bits(32, 0));
}

void Evaluate::edit(IntegerDeclaration* id) {
  const auto rhs = !id->get_val()->null() ? get_value(id->get_val()->get()).to_int() : 0;
  set_value(id->get_id(), Bits(32, rhs));
}

void Evaluate::edit(LocalparamDeclaration* ld) {
  set_value(ld->get_id(), get_value(ld->get_val()));
}

void Evaluate::edit(NetDeclaration* nd) {
  size_t w = 1;
  if (!nd->get_dim()->null()) {
    const auto rng = get_range(nd->get_dim()->get());
    w = rng.first-rng.second+1;
  }
  set_value(nd->get_id(), Bits(w, 0));
}

void Evaluate::edit(ParameterDeclaration* pd) {
  set_value(pd->get_id(), get_value(pd->get_val()));
}

void Evaluate::edit(RegDeclaration* rd) {
  size_t w = 1;
  if (!rd->get_dim()->null()) {
    const auto rng = get_range(rd->get_dim()->get());
    w = rng.first-rng.second+1;
  }
  set_value(rd->get_id(), Bits(w, 0));
  if (!rd->get_val()->null()) {
    assign_value(rd->get_id(), get_value(rd->get_val()->get()), w-1, 0); 
  } 
}

void Evaluate::set_value(const Identifier* id, const Bits& val) {
  const_cast<Identifier*>(id)->bit_val_ = val;
  invalidate(id);
}

void Evaluate::invalidate(const Identifier* id) {
  const_cast<Identifier*>(id)->needs_update_ = true;
  for (auto i = Resolve().dep_begin(id), ie = Resolve().dep_end(id); i != ie; ++i) {
    (*i)->needs_update_ = true;
  }
}

} // namespace cascade
