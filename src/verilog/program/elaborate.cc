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

#include "verilog/program/elaborate.h"

#include <cassert>
#include <unordered_map>
#include "base/bits/bits.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/indices.h"
#include "verilog/analyze/navigate.h"
#include "verilog/ast/ast.h"
#include "verilog/program/program.h"

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

GenerateBlock* Elaborate::elaborate(CaseGenerateConstruct* cgc) {
  if (cgc->gen_ != nullptr) {
    return cgc->gen_;
  }

  const auto& c = Evaluate().get_value(cgc->get_cond());
  for (auto i = cgc->begin_items(), ie = cgc->end_items(); i != ie; ++i) { 
    for (auto j = (*i)->begin_exprs(), je = (*i)->end_exprs(); j != je; ++j) { 
      const auto& v = Evaluate().get_value(*j);
      if (c == v) {
        elaborate(cgc, (*i)->get_block());
        return cgc->gen_;
      }
    }
    if ((*i)->empty_exprs()) {
      elaborate(cgc, (*i)->get_block());
      return cgc->gen_;
    }
  }

  // Control should never reach here. We'll always encounter a default.
  assert(false);
  return nullptr;
}

GenerateBlock* Elaborate::elaborate(IfGenerateConstruct* igc) {
  if (igc->gen_ != nullptr) {
    return igc->gen_;
  }
  for (auto i = igc->begin_clauses(), ie = igc->end_clauses(); i != ie; ++i) {
    if (Evaluate().get_value((*i)->get_if()).to_bool()) {
      elaborate(igc, (*i)->get_then());
      return igc->gen_;
    }
  } 
  elaborate(igc, igc->get_else());
  return igc->gen_;
}

Vector<GenerateBlock*>& Elaborate::elaborate(LoopGenerateConstruct* lgc) {
  if (!lgc->gen_.empty()) {
    return lgc->gen_;
  }

  auto* id = lgc->get_block()->is_null_id() ? get_name(lgc) : lgc->get_block()->get_id()->clone();
  const auto* itr = lgc->get_init()->get_lhs();
  for (
    Evaluate().assign_value(itr, Evaluate().get_value(lgc->get_init()->get_rhs()));
    Evaluate().get_value(lgc->get_cond()).to_bool();
    Evaluate().assign_value(itr, Evaluate().get_value(lgc->get_update()->get_rhs()))
  ) {
    auto* block = new GenerateBlock(true);
    block->replace_id(new Identifier(new Id(
      id->front_ids()->get_sid(), 
      new Number(Evaluate().get_value(itr))
    )));
    block->push_back_items(new LocalparamDeclaration(
      new Attributes(),
      itr->clone(), 
      false,
      new RangeExpression(32, 0),
      new Number(Evaluate().get_value(itr))
    ));
    for (auto i = lgc->get_block()->begin_items(), ie = lgc->get_block()->end_items(); i != ie; ++i) {
      block->push_back_items((*i)->clone());
    }
    lgc->gen_.push_back(block);
  }  
  delete id;

  for (auto* b : lgc->gen_) {
    b->parent_ = lgc;
  }
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
  return !lgc->gen_.empty();
}

ModuleDeclaration* Elaborate::get_elaboration(ModuleInstantiation* mi) {
  assert(mi->inst_ != nullptr);
  return mi->inst_;
}

GenerateBlock* Elaborate::get_elaboration(CaseGenerateConstruct* cgc) {
  assert(cgc->gen_ != nullptr);
  return cgc->gen_;
}

GenerateBlock* Elaborate::get_elaboration(IfGenerateConstruct* igc) {
  assert(igc->gen_ != nullptr);
  return igc->gen_;
}

Vector<GenerateBlock*>& Elaborate::get_elaboration(LoopGenerateConstruct* lgc) {
  assert(!lgc->gen_.empty());
  return lgc->gen_;
}

const ModuleDeclaration* Elaborate::get_elaboration(const ModuleInstantiation* mi) {
  assert(mi->inst_ != nullptr);
  return mi->inst_;
}

const GenerateBlock* Elaborate::get_elaboration(const CaseGenerateConstruct* cgc) {
  assert(cgc->gen_ != nullptr);
  return cgc->gen_;
}

const GenerateBlock* Elaborate::get_elaboration(const IfGenerateConstruct* igc) {
  assert(igc->gen_ != nullptr);
  return igc->gen_;
}

const Vector<GenerateBlock*>& Elaborate::get_elaboration(const LoopGenerateConstruct* lgc) {
  assert(!lgc->gen_.empty());
  return lgc->gen_;
}

void Elaborate::named_params(ModuleInstantiation* mi) {
  unordered_map<const Identifier*, const Expression*, HashId, EqId> params;
  for (auto i = mi->begin_params(), ie = mi->end_params(); i != ie; ++i) {
    assert((*i)->is_non_null_exp());
    assert((*i)->is_non_null_imp());
    params[(*i)->get_exp()] = (*i)->get_imp();
  }

  for (auto i = mi->inst_->begin_items(), ie = mi->inst_->end_items(); i != ie; ++i) {
    if ((*i)->is(Node::Tag::parameter_declaration)) {
      auto* pd = static_cast<ParameterDeclaration*>(*i);
      const auto itr = params.find(pd->get_id());
      if (itr != params.end()) {
        const_cast<ParameterDeclaration*>(pd)->replace_val(new Number(Evaluate().get_value(itr->second)));
      }
    }
  }
}

void Elaborate::ordered_params(ModuleInstantiation* mi) {
  size_t idx = 0;
  for (auto i = mi->inst_->begin_items(), ie = mi->inst_->end_items(); i != ie; ++i) {
    if ((*i)->is(Node::Tag::parameter_declaration)) {
      auto* pd = static_cast<ParameterDeclaration*>(*i);
      const auto* p = mi->get_params(idx++);
      const_cast<ParameterDeclaration*>(pd)->replace_val(new Number(Evaluate().get_value(p->get_imp())));
    }
  }
}

void Elaborate::elaborate(ConditionalGenerateConstruct* cgc, GenerateBlock* b) {
  cgc->gen_ = b;  
  if (b != nullptr) {
    b->parent_ = cgc;
  }

  // Nothing to do if this block was already named
  if (b == nullptr || b->is_non_null_id()) {
    return;
  }
  // Also nothing to do if this is a directly nested block without begin/ends
  if (cgc->get_parent()->is(Node::Tag::generate_block)) {
    auto* block = static_cast<GenerateBlock*>(cgc->get_parent());
    const auto only_item = block->size_items() == 1;
    const auto* p = block->get_parent();
    const auto nested_if = p->is(Node::Tag::if_generate_clause);
    const auto nested_else = p->is(Node::Tag::if_generate_construct);
    const auto nested_case = p->is(Node::Tag::case_generate_item);
    if (!block->get_scope() && only_item && (nested_if || nested_else || nested_case)) {
      return;
    }
  }
  // Otherwise, attach the next name for this scope
  b->replace_id(get_name(cgc));
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
