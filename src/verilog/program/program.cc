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

#include "src/verilog/program/program.h"

#include <algorithm>
#include <cassert>
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/navigate.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/inline.h"
#include "src/verilog/program/type_check.h"

using namespace std;

namespace cascade {

Program::Program() : Editor() {
  root_inst_ = nullptr;
  root_ditr_ = decls_.end();
  root_eitr_ = elabs_.end();
  typecheck(true);
}

Program::Program(ModuleDeclaration* md) : Program() {
  declare_and_instantiate(md);
}

Program::Program(ModuleDeclaration* md, ModuleInstantiation* mi) : Program() {
  declare(md);
  if (log_.error()) {
    return;
  }
  eval(mi);
}

Program::~Program() {
  if (root_inst_ != nullptr) {
    delete root_inst_;
  }
}

Program& Program::typecheck(bool tc) {
  checker_off_ = !tc;
  return *this;
}

void Program::declare(ModuleDeclaration* md) {
  log_.clear();

  // Propage default annotations
  if (md->get_attrs()->get_as()->empty() && root_decl() != decl_end()) {
    md->replace_attrs(root_decl()->second->get_attrs()->clone());
  }
  // Elaborate
  decl_check_ = true;
  local_only_ = true;
  expand_insts_ = false;
  expand_gens_ = false;
  elaborate(md);

  // Fail on error
  if (decl_find(md->get_id()) != decl_end()) {
    log_.error("Previous declaration already exists for this module");
  }
  if (log_.error()) {
    delete md;
    return;
  } 

  // Insert the new declaration
  decls_.checkpoint();
  decls_.insert(md->get_id(), md);
  decls_.commit();
  if (decls_.size() == 1) {
    root_ditr_ = decls_.begin();
  }
}

void Program::declare_and_instantiate(ModuleDeclaration* md) {
  declare(md);
  if (log_.error()) {
    return;
  }

  auto iid = md->get_id()->get_ids()->front()->get_readable_sid();
  transform(iid.begin(), iid.end(), iid.begin(), [](unsigned char c){return tolower(c);});

  auto mi = new ModuleInstantiation(
    new Attributes(new Many<AttrSpec>()),
    md->get_id()->clone(),
    new Identifier(iid),
    new Maybe<RangeExpression>(),
    new Many<ArgAssign>(),
    new Many<ArgAssign>()
  );
  eval(mi);
}

void Program::eval(ModuleItem* mi) {
  log_.clear();
  if (elab_begin() == elab_end()) {
    eval_root(mi);
  } else {
    eval_item(mi);
  }
}

void Program::inline_all() {
  if (root_eitr_ != elabs_.end()) {
    inline_all(root_eitr_->second);
  }
}

void Program::outline_all() {
  if (root_eitr_ != elabs_.end()) {
    outline_all(root_eitr_->second);
  }
}

const ModuleDeclaration* Program::src() const {
  return root_eitr_ == elab_end() ? nullptr : root_eitr_->second;
}

const Log& Program::get_log() const {
  return log_;
}

Program::decl_iterator Program::root_decl() const {
  return root_ditr_;
}

Program::decl_iterator Program::decl_find(const Identifier* id) const {
  return decls_.find(id);
}

Program::decl_iterator Program::decl_begin() const {
  return decls_.begin();
}

Program::decl_iterator Program::decl_end() const {
  return decls_.end();
}

Program::elab_iterator Program::root_elab() const {
  return root_eitr_;
}

Program::elab_iterator Program::elab_find(const Identifier* id) const {
  return elabs_.find(id);
}

Program::elab_iterator Program::elab_begin() const {
  return elabs_.begin();
}

Program::elab_iterator Program::elab_end() const {
  return elabs_.end();
}

void Program::elaborate(Node* n) {
  TypeCheck tc(this, &log_);
  tc.deactivate(checker_off_);
  tc.declaration_check(decl_check_);
  tc.local_only(local_only_);

  inst_queue_.clear();
  gen_queue_.clear();
  n->accept(this);

  while (!log_.error() && (!inst_queue_.empty() || !gen_queue_.empty())) {
    for (size_t i = 0; !log_.error() && i < inst_queue_.size(); ++i) {
      auto mi = inst_queue_[i];
      tc.pre_elaboration_check(mi);
      if (!log_.error() && expand_insts_) {
        Elaborate(this).elaborate(mi)->accept(this);
        if (!Navigate(mi).lost()) {
          Navigate(mi).invalidate();
        }

        auto inst = Elaborate().elaborate(mi);
        if (mi->get_attrs()->get_as()->empty()) {
          inst->get_attrs()->set_or_replace(root_inst_->get_attrs());
        } else {
          inst->get_attrs()->set_or_replace(mi->get_attrs());
        }
        elabs_.insert(Resolve().get_full_id(mi->get_iid()), inst);
      }
    }
    inst_queue_.clear();

    // TODO: Technically, we're not supposed to elaborate any new generate
    // statements that we create here until after we've recleared the
    // instantiation queue. In practice because don't support defparams
    // I don't *think* it makes a difference.

    for (size_t i = 0; !log_.error() && i < gen_queue_.size(); ++i) {
      auto gc = gen_queue_[i];
      if (auto cgc = dynamic_cast<CaseGenerateConstruct*>(gc)) {
        tc.pre_elaboration_check(cgc);
        if (!log_.error() && expand_gens_) {
          Elaborate().elaborate(cgc)->accept(this);
          Navigate(cgc).invalidate();
        }
      } else if (auto igc = dynamic_cast<IfGenerateConstruct*>(gc)) {
        tc.pre_elaboration_check(igc);
        if (!log_.error() && expand_gens_) {
          Elaborate().elaborate(igc)->accept(this);
          Navigate(igc).invalidate();
        }
      } else if (auto lgc = dynamic_cast<LoopGenerateConstruct*>(gc)) {
        tc.pre_elaboration_check(lgc);
        if (!log_.error() && expand_gens_) {
          Elaborate().elaborate(lgc)->accept(this);
          Navigate(lgc).invalidate();
        }
      }
    }
    gen_queue_.clear();
  }

  if (!log_.error()) {
    tc.post_elaboration_check(n);
  }
}

void Program::elaborate_item(ModuleItem* mi) {
  decl_check_ = false;
  local_only_ = false;
  expand_insts_ = true;
  expand_gens_ = true;
  elaborate(mi);
}

void Program::eval_root(ModuleItem* mi) {
  elabs_.checkpoint();
  auto inst = dynamic_cast<ModuleInstantiation*>(mi);
  if ((inst == nullptr) || !EqId()(inst->get_mid(), root_decl()->first)) {
    log_.error("Cannot evaluate code without first instantiating the root module");
  } else {
    elaborate_item(inst);
  }
  if (log_.error()) {
    elabs_.undo();
    delete mi;
    return;
  }
  elabs_.commit();

  root_inst_ = inst;
  root_eitr_ = elabs_.begin();
}

void Program::eval_item(ModuleItem* mi) {
  auto src = root_eitr_->second;
  src->get_items()->push_back(mi);

  elabs_.checkpoint();
  elaborate_item(mi);

  if (log_.error()) {
    elabs_.undo();
    // Invalidate any references to this module item
    Resolve().invalidate(src->get_items()->back());
    // Invalidate any scope references to this module item
    Navigate(src).invalidate();
    // Delete the module item
    src->get_items()->purge_to(src->get_items()->size()-1);
  } else {
    elabs_.commit();
  }

  // One or more modules may have been affected by this eval. This is true
  // regardless of whether succeeded or failed. Invalidate module info for the
  // entire hierarchy. This is overkill, but it works for now.
  for (auto i = elab_begin(), ie = elab_end(); i != ie; ++i) {
    ModuleInfo(i->second).invalidate();
  }
}

void Program::edit(ModuleInstantiation* mi) {
  inst_queue_.push_back(mi);
}

void Program::edit(CaseGenerateConstruct* cgc) {
  gen_queue_.push_back(cgc);
}

void Program::edit(IfGenerateConstruct* igc) {
  gen_queue_.push_back(igc);
}

void Program::edit(LoopGenerateConstruct* lgc) {
  gen_queue_.push_back(lgc);
}

void Program::inline_all(ModuleDeclaration* md) {
  if (!Inline().can_inline(md)) {
    return;
  }
  for (auto& c : ModuleInfo(md).children()) {
    auto itr = elabs_.find(Resolve().get_full_id(c.first));
    assert(itr != elabs_.end());
    inline_all(itr->second);
  }
  Inline().inline_source(md);
  //DebugTermPrinter(cout, true, true) << "AFTER INLINING:\n" << md << "\n";
}

void Program::outline_all(ModuleDeclaration* md) {
  if (!Inline().can_inline(md)) {
    return;
  }
  Inline().outline_source(md);
  for (auto& c : ModuleInfo(md).children()) {
    auto itr = elabs_.find(Resolve().get_full_id(c.first));
    assert(itr != elabs_.end());
    outline_all(itr->second);
  }
  //DebugTermPrinter(cout, true, true) << "AFTER OUTLINING:\n" << md << "\n";
}

} // namespace cascade
