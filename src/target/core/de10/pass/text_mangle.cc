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

#include "target/core/de10/pass/text_mangle.h"

#include <sstream>
#include "target/core/de10/de10_logic.h"
#include "verilog/analyze/module_info.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

TextMangle::TextMangle(const ModuleDeclaration* md, const De10Logic* de) : Builder() {
  md_ = md;
  de_ = de;
  task_index_ = 1;
}

Attributes* TextMangle::build(const Attributes* as) {
  (void) as;
  return new Attributes();
}

ModuleItem* TextMangle::build(const RegDeclaration* rd) {
  return ModuleInfo(md_).is_stateful(rd->get_id()) ? nullptr : rd->clone();
}

ModuleItem* TextMangle::build(const PortDeclaration* pd) {
  ModuleInfo info(md_);
  if (info.is_stateful(pd->get_decl()->get_id()) || info.is_input(pd->get_decl()->get_id())) {
    return nullptr;
  } else {
    return pd->get_decl()->clone();
  }
}

Expression* TextMangle::build(const FeofExpression* fe) {
  // Feof expressions are replaced by references to a location in the variable
  // table. The software-side of the de10 logic manages the value of this variable
  // based on invocations of other io tasks.

  const auto itr = de_->get_table().expr_find(fe);
  assert(itr != de_->get_table().expr_end());
  return new Identifier(new Id("__expr"), new Number(Bits(32, itr->second.begin)));
}

Statement* TextMangle::build(const BlockingAssign* ba) {
  // Look up the target of this assignment and its entry in the variable table
  const auto* r = Resolve().get_resolution(ba->get_lhs());
  assert(r != nullptr);
  const auto titr = de_->get_table().var_find(r);
  assert(titr != de_->get_table().var_end());

  // Replace the original assignment with an assignment to a concatenation
  vector<Identifier*> lhs;
  for (size_t i = 0, ie = titr->second.words_per_element; i < ie; ++i) {
    lhs.push_back(new Identifier(new Id("__var"), new Number(Bits(32, titr->second.begin+ie-i-1))));
  }
  return new BlockingAssign(lhs.begin(), lhs.end(), ba->get_rhs()->clone());
}

Statement* TextMangle::build(const NonblockingAssign* na) {
  auto* res = new SeqBlock();

  // Look up the target of this assignment 
  const auto* lhs = na->get_lhs();
  const auto* r = Resolve().get_resolution(lhs);
  assert(r != nullptr);
  
  // Replace the original assignment with an assignment to a temporary variable
  auto* next = lhs->clone();
  next->purge_ids();
  next->push_back_ids(new Id(lhs->front_ids()->get_readable_sid() + "_next"));
  res->push_back_stmts(new NonblockingAssign(
    na->clone_ctrl(),
    next,
    na->get_rhs()->clone()
  ));

  // Insert a new assignment to the next mask
  res->push_back_stmts(new NonblockingAssign(
    new Identifier(
      new Id("__update_mask"),
      get_table_range(r, lhs)
    ),
    new UnaryExpression(
      UnaryExpression::Op::TILDE,
      new Identifier(
        new Id("__prev_update_mask"),
        get_table_range(r, lhs)
      )
    )
  ));

  return res;
}

Statement* TextMangle::build(const DebugStatement* ds) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Statement* TextMangle::build(const FflushStatement* fs) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Statement* TextMangle::build(const FinishStatement* fs) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Statement* TextMangle::build(const FseekStatement* fs) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Statement* TextMangle::build(const GetStatement* gs) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Statement* TextMangle::build(const PutStatement* ps) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Statement* TextMangle::build(const RestartStatement* rs) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Statement* TextMangle::build(const RetargetStatement* rs) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Statement* TextMangle::build(const SaveStatement* ss) {
  return new NonblockingAssign(
    new Identifier("__task_id"), 
    new Number(Bits(32, task_index_++))
  );
}

Expression* TextMangle::get_table_range(const Identifier* r, const Identifier* i) {
  // Look up r in the variable table
  const auto titr = de_->get_table().var_find(r);
  assert(titr != de_->get_table().var_end());

  // Start with an expression for where this variable begins in the variable table
  Expression* idx = new Number(Bits(32, titr->second.begin));

  // Now iterate over the arity of r and compute a symbolic expression 
  auto mul = titr->second.elements;
  auto iitr = i->begin_dim();
  for (auto a : Evaluate().get_arity(titr->first)) {
    mul /= a;
    idx = new BinaryExpression(
      idx,
      BinaryExpression::Op::PLUS,
      new BinaryExpression(
        (*iitr++)->clone(),
        BinaryExpression::Op::TIMES,
        new Number(Bits(32, mul*titr->second.words_per_element))
      )
    );
  }
  return new RangeExpression(idx, RangeExpression::Type::PLUS, new Number(Bits(32, titr->second.words_per_element)));
}

} // namespace cascade
