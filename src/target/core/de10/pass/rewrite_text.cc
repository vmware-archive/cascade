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

#include "src/target/core/de10/pass/rewrite_text.h"

#include "src/target/core/de10/de10_logic.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"

namespace cascade {

RewriteText::RewriteText(const ModuleDeclaration* md, const De10Logic* de) : Builder() { 
  md_ = md;
  de_ = de;
}

Attributes* RewriteText::build(const Attributes* as) {
  (void) as;
  return new Attributes();
}

ModuleItem* RewriteText::build(const RegDeclaration* rd) {
  return ModuleInfo(md_).is_stateful(rd->get_id()) ? nullptr : rd->clone();
}

ModuleItem* RewriteText::build(const PortDeclaration* pd) {
  ModuleInfo info(md_);
  if (info.is_stateful(pd->get_decl()->get_id()) || info.is_input(pd->get_decl()->get_id())) {
    return nullptr;
  } else {
    return pd->get_decl()->clone();
  }
}

Statement* RewriteText::build(const NonblockingAssign* na) {
  // Create empty blocks for true and false branches (we'll never populate the
  // false branch)
  auto* t = new SeqBlock();
  auto* f = new SeqBlock();

  // Look up the target of this assignment and the indices it spans in the
  // variable table
  const auto* lhs = na->get_assign()->get_lhs();
  const auto* r = Resolve().get_resolution(lhs);
  assert(r != nullptr);
  
  // Replace the original assignment with an assignment to a temporary variable
  auto* next = lhs->clone();
  next->purge_ids();
  next->push_back_ids(new Id(lhs->front_ids()->get_readable_sid() + "_next"));
  t->push_back_stmts(new NonblockingAssign(
    na->clone_ctrl(),
    new VariableAssign(
      next,
      na->get_assign()->get_rhs()->clone()
    )
  ));

  // Insert a new assignment to the next mask
  t->push_back_stmts(new NonblockingAssign(
    new VariableAssign(
      new Identifier(
        new Id("__next_update_mask"),
        get_table_range(r, lhs)
      ),
      new UnaryExpression(
        UnaryExpression::Op::TILDE,
        new Identifier(
          new Id("__next_update_mask"),
          get_table_range(r, lhs)
        )
      )
    )
  ));

  // Return a conditional statement in place of the original assignment
  return new ConditionalStatement(new Identifier("__live"), t, f);
}

Expression* RewriteText::get_table_range(const Identifier* r, const Identifier* i) {
  // Look up r in the variable table
  const auto titr = de_->table_find(r);
  assert(titr != de_->table_end());
  assert(titr->second.materialized());

  // Start with an expression for where this variable begins in the variable table
  Expression* idx = new Number(Bits(32, titr->second.index()));

  // Now iterate over the arity of r and compute a symbolic expression 
  auto mul = titr->second.elements();
  auto iitr = i->begin_dim();
  for (auto a : titr->second.arity()) {
    mul /= a;
    idx = new BinaryExpression(
      idx,
      BinaryExpression::Op::PLUS,
      new BinaryExpression(
        (*iitr++)->clone(),
        BinaryExpression::Op::TIMES,
        new Number(Bits(32, mul*titr->second.element_size()))
      )
    );
  }
  return new RangeExpression(idx, RangeExpression::Type::PLUS, new Number(Bits(32, titr->second.element_size())));
}

} // namespace cascade
