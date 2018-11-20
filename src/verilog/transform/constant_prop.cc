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

#include "src/verilog/transform/constant_prop.h"

#include "src/verilog/analyze/constant.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"

namespace cascade {

ConstantProp::ConstantProp() : Rewriter() { }

void ConstantProp::run(ModuleDeclaration* md) {
  md->accept(this);
}

Expression* ConstantProp::rewrite(BinaryExpression* be) {
  if (Constant().is_static_constant(be)) {
    auto res = new Number(Evaluate().get_value(be), Number::HEX);
    Evaluate().invalidate(be);
    Resolve().invalidate(be);
    return res;
  }
  return Rewriter::rewrite(be);
}

Expression* ConstantProp::rewrite(ConditionalExpression* ce) {
  if (Constant().is_static_constant(ce->get_cond())) {
    Expression* res = nullptr;
    if (Evaluate().get_value(ce->get_cond()).to_bool()) {
      res = ce->get_lhs()->accept(this);
      if (res == ce->get_lhs()) {
        ce->set_lhs(new Identifier("ignore"));
      }
    } else {
      res = ce->get_rhs()->accept(this);
      if (res == ce->get_rhs()) {
        ce->set_rhs(new Identifier("ignore"));
      }
    }
    Evaluate().invalidate(ce);
    Resolve().invalidate(ce);
    return res;
  }
  return Rewriter::rewrite(ce);
}

Expression* ConstantProp::rewrite(NestedExpression* ne) {
  if (Constant().is_static_constant(ne)) {
    auto res = new Number(Evaluate().get_value(ne), Number::HEX);
    Evaluate().invalidate(ne);
    Resolve().invalidate(ne);
    return res;
  }
  return Rewriter::rewrite(ne);
}

Expression* ConstantProp::rewrite(Concatenation* c) {
  if (Constant().is_static_constant(c)) {
    auto res = new Number(Evaluate().get_value(c), Number::HEX);
    Evaluate().invalidate(c);
    Resolve().invalidate(c);
    return res;
  }
  return Rewriter::rewrite(c);
}

Expression* ConstantProp::rewrite(Identifier* i) {
  if (dynamic_cast<Declaration*>(i->get_parent())) {
    return Rewriter::rewrite(i);
  }
  if (Constant().is_static_constant(i)) {
    auto res = new Number(Evaluate().get_value(i), Number::HEX);
    Evaluate().invalidate(i);
    Resolve().invalidate(i);
    return res;
  }
  return Rewriter::rewrite(i);
}

Expression* ConstantProp::rewrite(MultipleConcatenation* mc) {
  if (Constant().is_static_constant(mc)) {
    auto res = new Number(Evaluate().get_value(mc), Number::HEX);
    Evaluate().invalidate(mc);
    Resolve().invalidate(mc);
    return res;
  }
  return Rewriter::rewrite(mc);
}

Expression* ConstantProp::rewrite(RangeExpression* re) {
  if (Constant().is_static_constant(re)) {
    const auto rng = Evaluate().get_range(re);
    auto res = new RangeExpression(rng.first+1, rng.second);
    Evaluate().invalidate(re);
    Resolve().invalidate(re);
    return res;
  }
  return Rewriter::rewrite(re);
}

Expression* ConstantProp::rewrite(UnaryExpression* ue) {
  if (Constant().is_static_constant(ue)) {
    auto res = new Number(Evaluate().get_value(ue), Number::HEX);
    Evaluate().invalidate(ue);
    Resolve().invalidate(ue);
    return res;
  }
  return Rewriter::rewrite(ue);
}

Statement* ConstantProp::rewrite(ConditionalStatement* cs) {
  if (Constant().is_static_constant(cs->get_if())) {
    Statement* res = nullptr;
    if (Evaluate().get_value(cs->get_if()).to_bool()) {
      res = cs->get_then()->accept(this);
      if (res == cs->get_then()) {
        cs->set_then(new SeqBlock(new Maybe<Identifier>(), new Many<Declaration>(), new Many<Statement>()));  
      }
    } else {
      res = cs->get_else()->accept(this);
      if (res == cs->get_else()) {
        cs->set_else(new SeqBlock(new Maybe<Identifier>(), new Many<Declaration>(), new Many<Statement>()));  
      }
    }
    Resolve().invalidate(cs);
    return res;
  }
  return Rewriter::rewrite(cs);
}

} // namespace cascade
