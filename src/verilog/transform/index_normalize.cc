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

#include "verilog/transform/index_normalize.h"

#include <cassert>
#include "verilog/analyze/constant.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"

using namespace std;

namespace cascade {

void IndexNormalize::run(ModuleDeclaration* md) {
  FixUses fu;
  md->accept(&fu);
  FixDecls fd;
  md->accept(&fd);

  // We may have deleted some static constant variables inside declarations.
  Resolve().invalidate(md);
}

IndexNormalize::FixDecls::FixDecls() : Editor() { }

void IndexNormalize::FixDecls::edit(GenvarDeclaration* gd) {
  fix_arity(gd->get_id());
  if (gd->is_non_null_dim()) {
    fix_dim(gd->get_dim());
  }
}

void IndexNormalize::FixDecls::edit(LocalparamDeclaration* ld) {
  fix_arity(ld->get_id());
  if (ld->is_non_null_dim()) {
    fix_dim(ld->get_dim());
  }
}

void IndexNormalize::FixDecls::edit(NetDeclaration* nd) {
  fix_arity(nd->get_id());
  if (nd->is_non_null_dim()) {
    fix_dim(nd->get_dim());
  }
}

void IndexNormalize::FixDecls::edit(ParameterDeclaration* pd) {
  fix_arity(pd->get_id());
  if (pd->is_non_null_dim()) {
    fix_dim(pd->get_dim());
  }
}

void IndexNormalize::FixDecls::edit(RegDeclaration* rd) {
  fix_arity(rd->get_id());
  if (rd->is_non_null_dim()) {
    fix_dim(rd->get_dim());
  }
}

void IndexNormalize::FixDecls::fix_arity(Identifier* id) const {
  for (auto i = id->begin_dim(), ie = id->end_dim(); i != ie; ++i) {
    assert((*i)->is(Node::Tag::range_expression));
    auto* re = static_cast<RangeExpression*>(*i);
    fix_dim(re);
  }
}

void IndexNormalize::FixDecls::fix_dim(RangeExpression* re) const {
  assert(Constant().is_static_constant(re));
  const auto rng = Evaluate().get_range(re);
  assert(rng.first >= rng.second);
  if (rng.second == 0) {
    return;
  }

  re->set_type(RangeExpression::Type::CONSTANT);
  re->replace_upper(new Number(Bits(32, rng.first-rng.second)));
  re->replace_lower(new Number(Bits(32, 0)));
  Evaluate().invalidate(re);
}

IndexNormalize::FixUses::FixUses() : Editor() { }

void IndexNormalize::FixUses::edit(Attributes* a) {
  // Don't descend past here
  (void) a;
}

void IndexNormalize::FixUses::edit(Identifier* id) {
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  const auto* p = r->get_parent();
  assert(p != nullptr);
  assert(p->is_subclass_of(Node::Tag::declaration));
  const auto* d = static_cast<const Declaration*>(p);

  // Fix array indices. Typechecking should ensure that there are as many of
  // these as appear in this variable's declaration.
  size_t i = 0;
  for (size_t ie = r->size_dim(); i < ie; ++i) {
    assert(r->get_dim(i)->is(Node::Tag::range_expression));
    fix_use(id, i, static_cast<const RangeExpression*>(r->get_dim(i)));
  }
  // The presence of a bit-select doesn't gaurantee that a range appears in a
  // declaration (see parameters, for example). When a range doesn't exist in a
  // declaration, it defaults to having 0 as a lsb, and nothing needs to be
  // done here.
  if ((i < id->size_dim()) && d->is_non_null_dim()) {
    fix_use(id, i, d->get_dim());
  }
}

void IndexNormalize::FixUses::edit(ModuleDeclaration* md) {
  // Only descend on items
  md->accept_items(this);
}

void IndexNormalize::FixUses::edit(GenvarDeclaration* gd) {
  // Does nothing; don't descend past here.
  (void) gd;
}

void IndexNormalize::FixUses::edit(LocalparamDeclaration* ld) {
  // Does nothing; don't descend past here.
  (void) ld;
}

void IndexNormalize::FixUses::edit(NetDeclaration* nd) {
  // Does nothing; don't descend past here.
  (void) nd;
}

void IndexNormalize::FixUses::edit(ParameterDeclaration* pd) {
  // Does nothing; don't descend past here.
  (void) pd;
}

void IndexNormalize::FixUses::edit(RegDeclaration* rd) {
  // Does nothing; don't descend past here.
  (void) rd;
}

void IndexNormalize::FixUses::edit(DebugStatement* ds) {
  // Does nothing; don't descend past here.
  (void) ds;
}

void IndexNormalize::FixUses::fix_use(Identifier* id, size_t n, const RangeExpression* re) const {
  assert(Constant().is_static_constant(re));
  const auto rng = Evaluate().get_range(re);
  assert(rng.first >= rng.second);
  if (rng.second == 0) {
    return;
  }

  if (id->get_dim(n)->is(Node::Tag::range_expression)) {
    fix_range(id, n, rng.second);
  } else {
    fix_scalar(id, n, rng.second);
  }
}

void IndexNormalize::FixUses::fix_scalar(Identifier* id, size_t n, size_t delta) const {
  assert(!id->get_dim(n)->is(Node::Tag::range_expression));
  auto* expr = id->get_dim(n);

  if (Constant().is_static_constant(expr)) {
    id->replace_dim(n, new Number(Bits(32, Evaluate().get_value(expr).to_uint()-delta)));
  } else {
    id->replace_dim(n, new BinaryExpression(expr->clone(), BinaryExpression::Op::MINUS, new Number(Bits(32, delta))));
  }
}

void IndexNormalize::FixUses::fix_range(Identifier* id, size_t n, size_t delta) const {
  assert(id->get_dim(n)->is(Node::Tag::range_expression));
  auto* re = static_cast<RangeExpression*>(id->get_dim(n));

  switch (re->get_type()) {
    case RangeExpression::Type::CONSTANT:
      assert(Constant().is_static_constant(re->get_upper()));
      assert(Constant().is_static_constant(re->get_lower()));
      re->replace_upper(new Number(Bits(32, Evaluate().get_value(re->get_upper()).to_uint()-delta)));
      re->replace_lower(new Number(Bits(32, Evaluate().get_value(re->get_lower()).to_uint()-delta)));
      Evaluate().invalidate(re);
      break;
    case RangeExpression::Type::PLUS:
    case RangeExpression::Type::MINUS:
      if (Constant().is_static_constant(re->get_upper())) {
        re->replace_upper(new Number(Bits(32, Evaluate().get_value(re->get_upper()).to_uint()-delta)));
      } else {
        re->replace_upper(new BinaryExpression(re->get_upper()->clone(), BinaryExpression::Op::MINUS, new Number(Bits(32, delta))));
      }
      break;
    default:
      assert(false);
      break;
  }
}

} // namespace cascade
