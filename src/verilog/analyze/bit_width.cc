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

#include "src/verilog/analyze/bit_width.h"

#include <cassert>
#include <algorithm>
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

size_t BitWidth::get_width(const Expression* e) {
  e->accept(this);
  return res_;
}

void BitWidth::visit(const BinaryExpression* be) {
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
      res_ = max(get_width(be->get_lhs()), get_width(be->get_rhs()));
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
      res_ = 1;
      break;
    case BinaryExpression::GGT:
    case BinaryExpression::LLT:
    case BinaryExpression::TTIMES:
    case BinaryExpression::GGGT:
    case BinaryExpression::LLLT:
      res_ = get_width(be->get_lhs());
      break;
    default:
      assert(false);
  }
}

void BitWidth::visit(const ConditionalExpression* ce) {
  res_ = max(get_width(ce->get_lhs()), get_width(ce->get_rhs()));
}

void BitWidth::visit(const Concatenation* c) {
  auto i = c->get_exprs()->begin();
  auto temp = get_width(*i++);
  for (auto ie = c->get_exprs()->end(); i != ie; ++i) {
    temp += get_width(*i);
  }
  res_ = temp;
}

void BitWidth::visit(const Identifier* id) {
  if (id->get_dim()->null()) {
    res_ = get_decl_width(id);
  } else if (dynamic_cast<Number*>(id->get_dim()->get())) {
    res_ = 1;
  } else if (auto re = dynamic_cast<RangeExpression*>(id->get_dim()->get())) {
    const auto lower = Evaluate().get_value(re->get_lower()).to_int();
    if (re->get_type() == RangeExpression::CONSTANT) {
      res_ = lower;
    } else {
      const auto upper = Evaluate().get_value(re->get_upper()).to_int();
      res_ = upper-lower+1;
    }
  }
}

void BitWidth::visit(const MultipleConcatenation* c) {
  const auto rhs = get_width(c->get_concat());
  res_ = Evaluate().get_value(c->get_expr()).to_int() * rhs;
}

void BitWidth::visit(const Number* n) {
  res_ = Evaluate().get_value(n).size();
}

void BitWidth::visit(const String* s) {
  res_ = 8 * s->get_readable_val().length();
}

void BitWidth::visit(const UnaryExpression* ue) {
  switch (ue->get_op()) {
    case UnaryExpression::PLUS:
    case UnaryExpression::MINUS:
    case UnaryExpression::TILDE:
      res_ = get_width(ue->get_lhs());
      break;
    case UnaryExpression::AMP:
    case UnaryExpression::TAMP:
    case UnaryExpression::PIPE:
    case UnaryExpression::TPIPE:
    case UnaryExpression::CARAT:
    case UnaryExpression::TCARAT:
    case UnaryExpression::BANG:
      res_ = 1;
      break;
    default:
     assert(false); 
  }
}

size_t BitWidth::get_decl_width(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  if (r == nullptr) {
    return 0;
  }
  const auto p = r->get_parent();
  if (p == nullptr) {
    return 0;
  }

  if (dynamic_cast<const GenvarDeclaration*>(p) != nullptr) {
    return 64;
  } else if (dynamic_cast<const IntegerDeclaration*>(p) != nullptr) {
    return 64;
  } else if (auto ld = dynamic_cast<const LocalparamDeclaration*>(p)) {
    return ld->get_dim()->null() ? get_width(ld->get_val()) : get_decl_width(ld->get_dim()->get());
  } else if (auto nd = dynamic_cast<const NetDeclaration*>(p)) {
    return nd->get_dim()->null() ? 1 : get_decl_width(nd->get_dim()->get());
  } else if (auto pd = dynamic_cast<const ParameterDeclaration*>(p)) {
    return pd->get_dim()->null() ? get_width(pd->get_val()) : get_decl_width(pd->get_dim()->get());
  } else if (auto rd = dynamic_cast<const RegDeclaration*>(p)) {
    return rd->get_dim()->null() ? 1 : get_decl_width(rd->get_dim()->get());
  } else {
    assert(false);
    return 0;
  }
}

size_t BitWidth::get_decl_width(const RangeExpression* re) {
  const auto rng = Evaluate().get_range(re);
  return rng.first - rng.second + 1;
}

} // namespace cascade
