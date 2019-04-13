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

#include "src/runtime/module.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include "src/runtime/data_plane.h"
#include "src/runtime/isolate.h"
#include "src/runtime/runtime.h"
#include "src/target/compiler.h"
#include "src/target/engine.h"
#include "src/target/state.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/inline.h"
#include "src/verilog/transform/constant_prop.h"
#include "src/verilog/transform/de_alias.h"
#include "src/verilog/transform/dead_code_eliminate.h"
#include "src/verilog/transform/loop_unroll.h"


#include "src/verilog/print/term/term_printer.h"

using namespace std;

namespace cascade {

Module* Module::iterator::operator*() const {
  return path_.front();
}

Module::iterator& Module::iterator::operator++() {
  if (path_.front() == nullptr) {
    return *this;
  }
  const auto* ptr = path_.front();
  path_.pop_front();

  // Sort children lexicographically to enforce deterministic iteration
  // orderings.
  for (auto i = ptr->children_.rbegin(), ie = ptr->children_.rend(); i != ie; ++i) {
    path_.push_front(*i);
  }
  return *this;
}

bool Module::iterator::operator==(const Module::iterator& rhs) const {
  assert(!path_.empty());
  assert(!rhs.path_.empty());
  return path_.front() == rhs.path_.front();
}

bool Module::iterator::operator!=(const Module::iterator& rhs) const {
  assert(!path_.empty());
  assert(!rhs.path_.empty());
  return path_.front() != rhs.path_.front();
}

Module::iterator::iterator() {
  path_.push_front(nullptr);
}

Module::iterator::iterator(Module* m) {
  path_.push_front(nullptr);
  path_.push_front(m);
}

Module::Module(const ModuleDeclaration* psrc, Runtime* rt, DataPlane* dp, Isolate* isolate, Compiler* compiler) {
  rt_ = rt;
  dp_ = dp;
  isolate_ = isolate;
  compiler_ = compiler;

  src_ = nullptr;
  engine_ = new Engine();
  version_ = 0;

  psrc_ = psrc;
  parent_ = nullptr;
}

Module::~Module() {
  for (auto* c : children_) {
    delete c;
  }
  if (src_ != nullptr) {
    delete src_;
  }
  delete engine_;
}

void Module::restart(std::istream& is) {
  // Read save file
  size_t n = 0;
  is >> n;
  unordered_map<MId, pair<Input*, State*>> save;
  for (size_t i = 0; i < n; ++i) {
    string ignore;
    MId id;
    auto input = new Input();
    auto state = new State();

    is >> ignore >> id;
    is >> ignore;
    input->read(is, 16);
    is >> ignore;
    state->read(is, 16);

    save[id] = make_pair(input, state);
  }

  // Update module hierarchy
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    const auto* p = (*i)->psrc_->get_parent();
    assert(p != nullptr);
    assert(p->is(Node::Tag::module_instantiation));
    const auto id = isolate_->isolate(static_cast<const ModuleInstantiation*>(p));
    const auto itr = save.find(id);

    if (itr == save.end()) {
      continue;
    }

    stringstream ss;
    const auto fid = Resolve().get_readable_full_id(static_cast<const ModuleInstantiation*>(p)->get_iid());
    ss << "Updated state for " << fid;
    rt_->info(ss.str());

    (*i)->engine_->set_input(itr->second.first);
    (*i)->engine_->set_state(itr->second.second);
  }

  // Delete contents of save file
  for (auto& s : save) {
    delete s.second.first;
    delete s.second.second;
  }
}

void Module::rebuild() {
  // This method should only be called in a state where all modules are in sync
  // with the user's program. However(!) we do still need to regenerate source.
  // Recall that compilation takes over ownership of a module's source code,
  // and in many cases, deletes or modifies it.
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    const auto ignore = (*i)->psrc_->size_items();
    (*i)->src_ = (*i)->regenerate_ir_source(ignore);
    const auto* iid = static_cast<const ModuleInstantiation*>((*i)->psrc_->get_parent())->get_iid();
    compiler_->compile_and_replace(rt_, (*i)->engine_, (*i)->version_, (*i)->src_, iid);
    (*i)->src_ = nullptr;
    if (compiler_->error()) {
      return;
    }
  }
}

void Module::save(ostream& os) {
  // TODO(eschkufz) Can this really be the only way we have of counting the
  // number of modules in the hierarchy? Yikes, this is lame.
  size_t n = 0;
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    ++n;
  }
  os << n << endl;

  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    const auto* p = (*i)->psrc_->get_parent();
    assert(p != nullptr);
    assert(p->is(Node::Tag::module_instantiation));

    os << "MODULE:" << endl;
    os << isolate_->isolate(static_cast<const ModuleInstantiation*>(p)) << endl;
    
    os << "INPUT:" << endl;
    auto* input = (*i)->engine_->get_input();
    input->write(os, 16);
    delete input;

    os << "STATE:" << endl;
    auto* state = (*i)->engine_->get_state();
    state->write(os, 16);
    delete state;
  }
}

void Module::synchronize(size_t n) {
  // Examine new code and instantiate new modules below the root 
  Instantiator inst(this);
  const auto idx = psrc_->size_items() - n;
  for (auto i = psrc_->begin_items()+idx, ie = psrc_->end_items(); i != ie; ++i) {
    (*i)->accept(&inst);
  }
  // Recompile everything 
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    const auto ignore = (*i == this) ? (psrc_->size_items() - n) : 0;
    (*i)->src_ = (*i)->regenerate_ir_source(ignore);
    const auto* iid = static_cast<const ModuleInstantiation*>((*i)->psrc_->get_parent())->get_iid();
    compiler_->compile_and_replace(rt_, (*i)->engine_, (*i)->version_, (*i)->src_, iid);
    (*i)->src_ = nullptr;
    if (compiler_->error()) {
      return;
    }
  }
  // Synchronize subscriptions with the dataplane. Note that we do this *after*
  // recompilation.  This guarantees that the variable names used by
  // Isolate::isolate() are deterministic.
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    for (auto* r : ModuleInfo((*i)->psrc_).reads()) {
      const auto gid = isolate_->isolate(r);
      dp_->register_id(gid);
      dp_->register_writer((*i)->engine_, gid);
    }
    for (auto* w : ModuleInfo((*i)->psrc_).writes()) {
      const auto gid = isolate_->isolate(w);
      dp_->register_id(gid);
      dp_->register_reader((*i)->engine_, gid);
    }
  }
}

Module::iterator Module::begin() {
  return iterator(this); 
}

Module::iterator Module::end() {
  return iterator();
}

Engine* Module::engine() {
  return engine_;
}

Module::Instantiator::Instantiator(Module* ptr) {
  ptr_ = ptr;
  instances_.push_back(ptr_);
}

void Module::Instantiator::visit(const CaseGenerateConstruct* cgc) {
  if (Elaborate().is_elaborated(cgc)) {
    Elaborate().get_elaboration(cgc)->accept(this);
  }
}

void Module::Instantiator::visit(const IfGenerateConstruct* igc) {
  if (Elaborate().is_elaborated(igc)) {
    Elaborate().get_elaboration(igc)->accept(this);
  }
}

void Module::Instantiator::visit(const LoopGenerateConstruct* lgc) {
  if (Elaborate().is_elaborated(lgc)) {
    for (auto* b : Elaborate().get_elaboration(lgc)) {
      b->accept(this);
    }
  }
}

void Module::Instantiator::visit(const ModuleInstantiation* mi) {
  // Inline Case: Descend past here
  if (Inline().is_inlined(mi)) {
    return Inline().get_source(mi)->accept(this);  
  }

  // Look up the checker associated with this instantiation
  auto itr = ModuleInfo(ptr_->psrc_).children().find(mi->get_iid());
  assert(itr != ModuleInfo(ptr_->psrc_).children().end());

  // Create a new node
  auto* child = new Module(itr->second, ptr_);
  ptr_->children_.push_back(child);

  // Continue down through this module
  ptr_ = child;
  instances_.push_back(ptr_);
  ptr_->psrc_->accept(this);
  ptr_ = child->parent_;
}

ModuleDeclaration* Module::regenerate_ir_source(size_t ignore) {
  auto* md = isolate_->isolate(psrc_, ignore);
  const auto* std = md->get_attrs()->get<String>("__std");
  const auto is_logic = (std != nullptr) && (std->get_readable_val() == "logic");
  if (is_logic) {
    ModuleInfo(md).invalidate();
    LoopUnroll().run(md);
    DeAlias().run(md);
    ConstantProp().run(md);
    DeadCodeEliminate().run(md);
    TermPrinter(cout) << md << "\n";
  }
  return md;
}

Module::Module(const ModuleDeclaration* psrc, Module* parent) :
  Module(psrc, parent->rt_, parent->dp_, parent->isolate_, parent->compiler_) {
  parent_ = parent;
}

} // namespace cascade
