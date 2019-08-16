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

#include "runtime/module.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include "runtime/data_plane.h"
#include "runtime/isolate.h"
#include "runtime/runtime.h"
#include "target/compiler.h"
#include "target/engine.h"
#include "target/state.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"
#include "verilog/program/elaborate.h"
#include "verilog/program/inline.h"
#include "verilog/transform/block_flatten.h"
#include "verilog/transform/constant_prop.h"
#include "verilog/transform/control_merge.h"
#include "verilog/transform/de_alias.h"
#include "verilog/transform/delete_initial.h"
#include "verilog/transform/dead_code_eliminate.h"
#include "verilog/transform/event_expand.h"
#include "verilog/transform/index_normalize.h"
#include "verilog/transform/loop_unroll.h"

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

Module::Module(const ModuleDeclaration* psrc, Runtime* rt, Module* parent) {
  rt_ = rt;

  psrc_ = psrc;
  parent_ = parent;

  engine_ = rt_->get_compiler()->compile_stub(rt_->get_next_id(), psrc);
  version_ = 0;
}

Module::~Module() {
  for (auto* c : children_) {
    delete c;
  }
  delete engine_;
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

size_t Module::size() const {
  size_t res = 0;
  for (auto i = iterator(const_cast<Module*>(this)), ie = const_cast<Module*>(this)->end(); i != ie; ++i) {
    ++res;
  }
  return res;
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
    (*i)->compile_and_replace(ignore);
    if (rt_->get_compiler()->error()) {
      return;
    }
  }
  // Synchronize subscriptions with the dataplane. Note that we do this *after*
  // recompilation.  This guarantees that the variable names used by
  // Isolate::isolate() are deterministic.
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    for (auto* r : ModuleInfo((*i)->psrc_).reads()) {
      const auto gid = rt_->get_isolate()->isolate(r);
      rt_->get_data_plane()->register_id(gid);
      rt_->get_data_plane()->register_writer((*i)->engine_, gid);
    }
    for (auto* w : ModuleInfo((*i)->psrc_).writes()) {
      const auto gid = rt_->get_isolate()->isolate(w);
      rt_->get_data_plane()->register_id(gid);
      rt_->get_data_plane()->register_reader((*i)->engine_, gid);
    }
  }
}

void Module::rebuild() {
  // This method should only be called in a state where all modules are in sync
  // with the user's program. However(!) we do still need to regenerate source.
  // Recall that compilation takes over ownership of a module's source code.
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    const auto ignore = (*i)->psrc_->size_items();
    (*i)->compile_and_replace(ignore);
    if (rt_->get_compiler()->error()) {
      return;
    }
  }
}

void Module::save(ostream& os) {
  os << size() << endl;

  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    const auto* p = (*i)->psrc_->get_parent();
    assert(p != nullptr);
    assert(p->is(Node::Tag::module_instantiation));

    os << "MODULE:" << endl;
    os << rt_->get_isolate()->isolate(static_cast<const ModuleInstantiation*>(p)) << endl;
    
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
    const auto id = rt_->get_isolate()->isolate(static_cast<const ModuleInstantiation*>(p));
    const auto itr = save.find(id);

    if (itr == save.end()) {
      continue;
    }

    ostream os(rt_->rdbuf(Runtime::stdinfo_));
    const auto fid = Resolve().get_readable_full_id(static_cast<const ModuleInstantiation*>(p)->get_iid());
    os << "Updated state for " << fid << endl;

    (*i)->engine_->set_input(itr->second.first);
    (*i)->engine_->set_state(itr->second.second);
  }

  // Delete contents of save file
  for (auto& s : save) {
    delete s.second.first;
    delete s.second.second;
  }
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
  auto* child = new Module(itr->second, ptr_->rt_, ptr_);
  ptr_->children_.push_back(child);

  // Continue down through this module
  ptr_ = child;
  instances_.push_back(ptr_);
  ptr_->psrc_->accept(this);
  ptr_ = child->parent_;
}



ModuleDeclaration* Module::regenerate_ir_source(size_t ignore) {
  auto* md = rt_->get_isolate()->isolate(psrc_, ignore);
  const auto* std = md->get_attrs()->get<String>("__std");
  const auto is_logic = (std != nullptr) && (std->get_readable_val() == "logic");
  if (is_logic) {
    ModuleInfo(md).invalidate();
    IndexNormalize().run(md);
    LoopUnroll().run(md);
    DeAlias().run(md);
    ConstantProp().run(md);
    EventExpand().run(md);
    ControlMerge().run(md);
    DeadCodeEliminate().run(md);
    BlockFlatten().run(md);
  }
  return md;
}

void Module::compile_and_replace(size_t ignore) {
  // Generate new code and bump the sequence number for this module
  auto* md = regenerate_ir_source(ignore); 
  ++version_;
  const auto this_version = version_;

  // Record human readable name for this module
  const auto* iid = static_cast<const ModuleInstantiation*>(psrc_->get_parent())->get_iid();
  const auto fid = Resolve().get_readable_full_id(iid);

  // Lookup annotations 
  const auto* std = md->get_attrs()->get<String>("__std");
  const auto* t = md->get_attrs()->get<String>("__target");
  const auto* l = md->get_attrs()->get<String>("__loc");

  // Check: Is this an std module, was jit compilation requested?  
  const auto tsep = t->get_readable_val().find_first_of(';');
  const auto lsep = l->get_readable_val().find_first_of(';');
  const auto jit = std->eq("logic") && ((tsep != string::npos) || (lsep != string::npos));

  // If we're jit compiling, we'll need a second copy of the source need to
  // adjust the annotations. 
  ModuleDeclaration* md2 = nullptr;
  if (jit) {
    md2 = md->clone();
    if (tsep != string::npos) {
      md2->get_attrs()->set_or_replace("__target", new String(t->get_readable_val().substr(tsep+1)));
      md->get_attrs()->set_or_replace("__target", new String(t->get_readable_val().substr(0, tsep)));
    }
    if (lsep != string::npos) {
      md2->get_attrs()->set_or_replace("__loc", new String(l->get_readable_val().substr(lsep+1)));
      md->get_attrs()->set_or_replace("__loc", new String(l->get_readable_val().substr(0, lsep)));
    }
  } else {
    md2 = new ModuleDeclaration(new Attributes(), new Identifier("null"));
  }

  // Fast Path. Compile and replace the original engine.  
  stringstream ss;
  TextPrinter(ss) << "fast-pass recompilation of " << fid << " with attributes " << md->get_attrs();
  auto* e_fast = rt_->get_compiler()->compile(engine_->get_id(), md);
  if (e_fast == nullptr) {
    if (!rt_->get_compiler()->error()) {
      rt_->get_compiler()->error("An unhandled error occurred during module compilation");
    }
  } else {
    engine_->replace_with(e_fast);
    if (engine_->is_stub()) {
      ostream(rt_->rdbuf(Runtime::stdinfo_)) << "Deferring " << ss.str() << endl;
    } else {
      ostream(rt_->rdbuf(Runtime::stdinfo_)) << "Finished " << ss.str() << endl;
    }
  }

  // Slow Path: Schedule a thread to compile in the background and swap in the
  // results in a safe runtime window when it's done. 
  if (jit && (e_fast != nullptr)) {
    rt_->schedule_asynchronous(Runtime::Asynchronous([this, this_version, md2, fid]{
      stringstream ss;
      TextPrinter(ss) << "slow-pass recompilation of " << fid << " with attributes " << md2->get_attrs();
      const auto str = ss.str();

      DeleteInitial().run(md2);
      auto* e_slow = rt_->get_compiler()->compile(engine_->get_id(), md2);

      rt_->schedule_interrupt([this, this_version, e_slow, str]{
        if ((this_version < version_) || (e_slow == nullptr)) {
          ostream(rt_->rdbuf(Runtime::stdinfo_)) << "Aborted " << str << endl;
        } else {
          engine_->replace_with(e_slow);
          ostream(rt_->rdbuf(Runtime::stdinfo_)) << "Finished " << str << endl;
        }
      },
      [e_slow] {
        if (e_slow != nullptr) {
          delete e_slow;
        }
      });
    }));
  } else {
    delete md2;
  }
}

} // namespace cascade
