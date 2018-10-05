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

#include "src/verilog/program/elaborate.h"

#include <cassert>
#include <unordered_map>
#include "src/base/bits/bits.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/indices.h"
#include "src/verilog/analyze/navigate.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/program/program.h"

using namespace std;

namespace cascade {

Elaborate::Elaborate(const Program* p) : Visitor() { 
  program_ = p;
}

ModuleDeclaration* Elaborate::elaborate(ModuleInstantiation* mi) {
  if (mi->inst_ != nullptr) {
    return mi->inst_;
  }

  assert(program_ != nullptr);
  const auto itr = program_->decl_find(mi->get_mid());
  assert(itr != program_->decl_end());

  mi->inst_ = itr->second->clone();
  mi->inst_->parent_ = mi;

  if (mi->uses_named_params()) {
    named_params(mi);
  } else {
    ordered_params(mi);
  }

  return mi->inst_;
}

Maybe<GenerateBlock>* Elaborate::elaborate(CaseGenerateConstruct* cgc) {
  if (cgc->gen_ != nullptr) {
    return cgc->gen_;
  }

  const auto& c = Evaluate().get_value(cgc->get_cond());
  for (auto ci : *cgc->get_items()) {
    for (auto e : *ci->get_exprs()) {
      const auto& v = Evaluate().get_value(e);
      if (c == v) {
        elaborate(cgc, ci->get_block());
        return cgc->gen_;
      }
    }
    if (ci->get_exprs()->empty()) {
      elaborate(cgc, ci->get_block());
      return cgc->gen_;
    }
  }

  // Control should never reach here. We'll always encounter a default.
  assert(false);
  return nullptr;
}

Maybe<GenerateBlock>* Elaborate::elaborate(IfGenerateConstruct* igc) {
  if (igc->gen_ != nullptr) {
    return igc->gen_;
  }
  for (auto c : *igc->get_clauses()) {
    if (Evaluate().get_value(c->get_if()).to_bool()) {
      elaborate(igc, c->get_then());
      return igc->gen_;
    }
  } 
  elaborate(igc, igc->get_else());
  return igc->gen_;
}

Many<GenerateBlock>* Elaborate::elaborate(LoopGenerateConstruct* lgc) {
  if (lgc->gen_ != nullptr) {
    return lgc->gen_;
  }

  auto id = lgc->get_block()->get_id()->null() ? get_name(lgc) : lgc->get_block()->get_id()->get()->clone();
  auto blocks = new Many<GenerateBlock>();

  const auto itr = Resolve().get_resolution(lgc->get_init()->get_lhs());
  for (
    Evaluate().assign_value(itr, Evaluate().get_value(lgc->get_init()->get_rhs()));
    Evaluate().get_value(lgc->get_cond()).to_bool();
    Evaluate().assign_value(itr, Evaluate().get_value(lgc->get_update()->get_rhs()))
  ) {
    // TODO: This localparam logic should move to Canonicalize()
    const auto lpd = new LocalparamDeclaration(
      new Attributes(new Many<AttrSpec>()),
      new Maybe<RangeExpression>(new RangeExpression(64, 0)),
      itr->clone(), 
      new Number(Evaluate().get_value(itr))
    );
    const auto block = new GenerateBlock(
      new Maybe<Identifier>(new Identifier(
        new Many<Id>(new Id(
          id->get_ids()->front()->get_sid(), 
          new Maybe<Expression>(new Number(Evaluate().get_value(itr)))
        )),
        new Maybe<Expression>()
      )),
      new Many<ModuleItem>(lpd)
    );
    block->get_items()->concat(lgc->get_block()->get_items()->clone());
    blocks->push_back(block);
  }  
  delete id;

  lgc->gen_ = blocks;
  blocks->parent_ = lgc;

  return lgc->gen_;
}

const ModuleDeclaration* Elaborate::get_elaboration(const ModuleInstantiation* mi) {
  return mi->inst_;
}

const Maybe<GenerateBlock>* Elaborate::get_elaboration(const CaseGenerateConstruct* cgc) {
  return cgc->gen_;
}

const Maybe<GenerateBlock>* Elaborate::get_elaboration(const IfGenerateConstruct* igc) {
  return igc->gen_;
}

const Many<GenerateBlock>* Elaborate::get_elaboration(const LoopGenerateConstruct* lgc) {
  return lgc->gen_;
}

bool Elaborate::is_elaborated(const ModuleInstantiation* mi) {
  return mi->inst_ != nullptr;
}

bool Elaborate::is_elaborated(const CaseGenerateConstruct* cgc) {
  return cgc->gen_ != nullptr;
}

bool Elaborate::is_elaborated(const IfGenerateConstruct* igc) {
  return igc->gen_ != nullptr;
}

bool Elaborate::is_elaborated(const LoopGenerateConstruct* lgc) {
  return lgc->gen_ != nullptr;
}

void Elaborate::named_params(ModuleInstantiation* mi) {
  unordered_map<const Identifier*, const Expression*, HashId, EqId> params;
  for (auto p : *mi->get_params()) {
    assert(!p->get_exp()->null());
    assert(!p->get_imp()->null());
    params[p->get_exp()->get()] = p->get_imp()->get();
  }

  for (auto inst : *mi->inst_->get_items()) {
    if (auto pd = dynamic_cast<ParameterDeclaration*>(inst)) {
      const auto itr = params.find(pd->get_id());
      if (itr != params.end()) {
        const_cast<ParameterDeclaration*>(pd)->replace_val(new Number(Evaluate().get_value(itr->second)));
      }
    }
  }
}

void Elaborate::ordered_params(ModuleInstantiation* mi) {
  size_t idx = 0;
  for (auto item : *mi->inst_->get_items()) {
    if (auto pd = dynamic_cast<ParameterDeclaration*>(item)) {
      const auto p = mi->get_params()->get(idx++);
      const_cast<ParameterDeclaration*>(pd)->replace_val(new Number(Evaluate().get_value(p->get_imp()->get())));
    }
  }
}

void Elaborate::elaborate(ConditionalGenerateConstruct* cgc, Maybe<GenerateBlock>* b) {
  cgc->gen_ = b;  
  // Nothing to do if this block was already named
  if (b->null() || !b->get()->get_id()->null()) {
    return;
  }
  // Also nothing to do if this is a directly nested block
  if (auto block = dynamic_cast<GenerateBlock*>(cgc->get_parent()->get_parent())) {
    const auto only_item = block->get_items()->size() == 1;
    const auto pp = block->get_parent()->get_parent();
    const auto nested_if = dynamic_cast<IfGenerateClause*>(pp);
    const auto nested_else = dynamic_cast<IfGenerateConstruct*>(pp);
    const auto nested_case = dynamic_cast<CaseGenerateItem*>(pp);
    if (only_item && (nested_if || nested_else || nested_case)) {
      return;
    }
  }
  // Otherwise, attach the next name for this scope
  b->get()->get_id()->replace(get_name(cgc));
}

Identifier* Elaborate::get_name(GenerateConstruct* gc) {
  // Automatically generated genblk ids begin with genblk1
  next_name_ = 1;
  here_ = gc;

  Navigate nav(gc);
  nav.where()->accept(this);

  stringstream ss;
  ss << "genblk" << next_name_;

  return new Identifier(ss.str());
}

void Elaborate::visit(const CaseGenerateConstruct* cgc) {
  if (here_ == nullptr) {
    return;
  } else if (here_ == cgc) {
    here_ = nullptr;
  } else {
    ++next_name_;
  }
}

void Elaborate::visit(const IfGenerateConstruct* igc) {
  if (here_ == nullptr) {
    return;
  } else if (here_ == igc) {
    here_ = nullptr;
  } else {
    ++next_name_;
  }
}

void Elaborate::visit(const LoopGenerateConstruct* lgc) {
  if (here_ == nullptr) {
    return;
  } else if (here_ == lgc) {
    here_ = nullptr;
  } else {
    ++next_name_;
  }
}

} // namespace cascade
