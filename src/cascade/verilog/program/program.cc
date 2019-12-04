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

#include "verilog/program/program.h"

#include <algorithm>
#include <cassert>
#include "common/log.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/navigate.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/parse/parser.h"
#include "verilog/program/elaborate.h"
#include "verilog/program/inline.h"
#include "verilog/program/type_check.h"

using namespace std;

namespace cascade {

Program::Program() : Editor() {
  root_inst_ = nullptr;
  root_ditr_ = decls_.end();
  root_eitr_ = elabs_.end();
  typecheck(true);
}

Program::Program(ModuleDeclaration* md) : Program() {
  Log log;
  declare_and_instantiate(md, &log);
  assert(!log.error());
}

Program::Program(ModuleDeclaration* md, ModuleInstantiation* mi) : Program() {
  Log log;
  declare(md, &log);
  assert(!log.error());
  eval(mi, &log); 
  assert(!log.error());
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

bool Program::declare(ModuleDeclaration* md, Log* log, const Parser* p) {
  // Declarations inherit defaults from the root declaration. 
  if (root_decl() != decl_end()) {
    auto* attrs = root_decl()->second->get_attrs()->clone();
    attrs->set_or_replace(md->get_attrs());
    md->replace_attrs(attrs);
  }
  // Elaborate
  decl_check_ = true;
  local_only_ = true;
  expand_insts_ = false;
  expand_gens_ = false;
  elaborate(md, log, p);

  // Fail on error
  if (decl_find(md->get_id()) != decl_end()) {
    log->error("Previous declaration already exists for this module");
  }
  if (log->error()) {
    delete md;
    return false;
  } 

  // Insert the new declaration
  decls_.checkpoint();
  decls_.insert(md->get_id(), md);
  decls_.commit();
  if (decls_.size() == 1) {
    root_ditr_ = decls_.begin();
  }
  return log->error();
}

bool Program::declare_and_instantiate(ModuleDeclaration* md, Log* log, const Parser* p) {
  if (!declare(md, log, p)) {
    return false;
  }

  auto iid = md->get_id()->front_ids()->get_readable_sid();
  transform(iid.begin(), iid.end(), iid.begin(), [](unsigned char c){return tolower(c);});

  auto* mi = new ModuleInstantiation(
    new Attributes(),
    md->get_id()->clone(),
    new Identifier(iid)
  );

  return eval(mi, log, p);
}

bool Program::eval(ModuleItem* mi, Log* log, const Parser* p) {
  if (elab_begin() == elab_end()) {
    eval_root(mi, log, p);
  } else {
    eval_item(mi, log, p);
  }
  return log->error();
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
  return (root_eitr_ == elab_end()) ? nullptr : root_eitr_->second;
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

void Program::elaborate(Node* n, Log* log, const Parser* p) {
  TypeCheck tc(this, log, p);
  tc.deactivate(checker_off_);
  tc.declaration_check(decl_check_);
  tc.local_only(local_only_);

  inst_queue_.clear();
  gen_queue_.clear();
  n->accept(this);

  while (!log->error() && (!inst_queue_.empty() || !gen_queue_.empty())) {
    for (size_t i = 0; !log->error() && i < inst_queue_.size(); ++i) {
      auto* mi = inst_queue_[i];
      tc.pre_elaboration_check(mi);
      if (!log->error() && expand_insts_) {
        auto* e = Elaborate(this).elaborate(mi);
        assert(e != nullptr);
        e->accept(this);
        if (!Navigate(mi).lost()) {
          Navigate(mi).invalidate();
        }

        // Instantiations inherit attributes from their declarations. User
        // logic also inherits attributes from the root instantiation.
        assert(decl_find(mi->get_mid()) != decl_end());
        auto* attrs = decl_find(mi->get_mid())->second->get_attrs()->clone();
        if ((root_elab() != elab_end()) && attrs->get<String>("__std")->eq("logic")) {
          attrs->set_or_replace(root_elab()->second->get_attrs());
        }
        attrs->set_or_replace(mi->get_attrs());

        e->replace_attrs(attrs);
        elabs_.insert(Resolve().get_full_id(mi->get_iid()), e);
      }
    }
    inst_queue_.clear();

    // TODO(eschkufz) Technically, we're not supposed to elaborate any new
    // generate statements which we create here until after we've recleared the
    // instantiation queue. In practice because we don't support defparams I
    // don't *think* it makes a difference.

    for (size_t i = 0; !log->error() && i < gen_queue_.size(); ++i) {
      auto* gc = gen_queue_[i];
      if (gc->is(Node::Tag::case_generate_construct)) {
        auto* cgc = static_cast<CaseGenerateConstruct*>(gc);
        tc.pre_elaboration_check(cgc);
        if (!log->error() && expand_gens_) {
          if (auto* e = Elaborate().elaborate(cgc)) {
            e->accept(this);
          }
          Navigate(cgc).invalidate();
        }
      } else if (gc->is(Node::Tag::if_generate_construct)) {
        auto* igc = static_cast<IfGenerateConstruct*>(gc);
        tc.pre_elaboration_check(igc);
        if (!log->error() && expand_gens_) {
          if (auto* e = Elaborate().elaborate(igc)) {
            e->accept(this);
          }
          Navigate(igc).invalidate();
        }
      } else if (gc->is(Node::Tag::loop_generate_construct)) {
        auto* lgc = static_cast<LoopGenerateConstruct*>(gc);
        tc.pre_elaboration_check(lgc);
        if (!log->error() && expand_gens_) {
          const auto* itr = lgc->get_init()->get_lhs();
          const auto* r = Resolve().get_resolution(itr);
          Resolve().invalidate(r->get_parent());
          for (auto* b : Elaborate().elaborate(lgc)) {
            b->accept(this);
          }
          Navigate(lgc).invalidate();
        }
      }
    }
    gen_queue_.clear();
  }

  if (!log->error()) {
    tc.post_elaboration_check(n);
  }
}

void Program::elaborate_item(ModuleItem* mi, Log* log, const Parser* p) {
  decl_check_ = false;
  local_only_ = false;
  expand_insts_ = true;
  expand_gens_ = true;
  elaborate(mi, log, p);
}

void Program::eval_root(ModuleItem* mi, Log* log, const Parser* p) {
  elabs_.checkpoint();
  if (!mi->is(Node::Tag::module_instantiation)) { 
    log->error("Cannot evaluate code without first instantiating the root module");
  }
  auto* inst = static_cast<ModuleInstantiation*>(mi);
  if (!EqId()(inst->get_mid(), root_decl()->first)) {
    log->error("Cannot evaluate code without first instantiating the root module");
  } else {
    elaborate_item(inst, log, p);
  }
  if (log->error()) {
    elabs_.undo();
    delete mi;
    return;
  }
  elabs_.commit();

  root_inst_ = inst;
  root_eitr_ = elabs_.begin();
}

void Program::eval_item(ModuleItem* mi, Log* log, const Parser* p) {
  auto* src = root_eitr_->second;
  src->push_back_items(mi);

  elabs_.checkpoint();
  elaborate_item(mi, log, p);

  // Several modules may have been affected by this eval. This is true
  // regardless of succees or failure. Invalidate the entire hierarchy. 
  for (auto i = elab_begin(), ie = elab_end(); i != ie; ++i) {
    Navigate(i->second).invalidate();
    Resolve().invalidate(i->second);
    ModuleInfo(i->second).invalidate();
  }

  if (log->error()) {
    elabs_.undo();
    src->purge_to_items(src->size_items()-1);
  } else {
    elabs_.commit();
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
  for (auto& c : ModuleInfo(md).children()) {
    const auto* fid = Resolve().get_full_id(c.first);
    auto itr = elabs_.find(fid);
    delete fid;
    assert(itr != elabs_.end());
    inline_all(itr->second);
  }
  Inline().inline_source(md);
}

void Program::outline_all(ModuleDeclaration* md) {
  Inline().outline_source(md);
  for (auto& c : ModuleInfo(md).children()) {
    const auto* fid = Resolve().get_full_id(c.first);
    auto itr = elabs_.find(fid);
    delete fid;
    assert(itr != elabs_.end());
    outline_all(itr->second);
  }
}

} // namespace cascade
