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

#include "src/runtime/module.h"

#include <cassert>
#include <unordered_set>
#include "src/runtime/data_plane.h"
#include "src/runtime/isolate.h"
#include "src/target/compiler.h"
#include "src/target/engine.h"
#include "src/target/state.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/debug/debug_printer.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/inline.h"
#include "src/verilog/transform/constant_prop.h"
#include "src/verilog/transform/de_alias.h"
#include "src/verilog/transform/dead_code_eliminate.h"

using namespace std;

namespace cascade {

Module* Module::iterator::operator*() const {
  return path_.front();
}

Module::iterator& Module::iterator::operator++() {
  if (path_.front() == nullptr) {
    return *this;
  }
  const auto ptr = path_.front();
  path_.pop_front();
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
  newest_evals_ = 0;
  engine_ = new Engine();
  source_out_of_date_ = true;
  engine_out_of_date_ = true;

  psrc_ = psrc;
  parent_ = nullptr;
}

Module::~Module() {
  for (auto c : children_) {
    delete c;
  }
  if (src_ != nullptr) {
    delete src_;
  }
  delete engine_;
}

void Module::synchronize(size_t n) {
  // 1. Invalidate the root source no matter what
  source_out_of_date_ = true;
  // 2. Examine new code and instantiate new modules below the root 
  Instantiator inst(this);
  const auto idx = psrc_->get_items()->size() - n;
  for (auto i = psrc_->get_items()->begin()+idx, ie = psrc_->get_items()->end(); i != ie; ++i) {
    (*i)->accept(&inst);
  }
  // 3. Record new gloabl ids and the modules that they reference.
  std::unordered_set<const ModuleDeclaration*> targets;
  for (auto m : inst.instances_) {
    for (auto r : ModuleInfo(m->psrc_).reads()) {
      targets.insert(Resolve().get_origin(r));
      dp_->register_id(isolate_->isolate(r));
    }
    for (auto w : ModuleInfo(m->psrc_).writes()) {
      targets.insert(Resolve().get_origin(w));
      dp_->register_id(isolate_->isolate(w));
    }
  }
  // 4. Flag modules referenced by global ids with missing subscriptions as
  //    needing new source.
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    if (targets.find((*i)->psrc_) == targets.end()) {
      continue;
    }
    for (auto r : ModuleInfo((*i)->psrc_).reads()) {
      const auto gid = isolate_->isolate(r);
      if (dp_->reader_find((*i)->engine_, gid) == dp_->reader_end(gid)) {
        (*i)->source_out_of_date_ = true;
      }
    }
    for (auto w : ModuleInfo((*i)->psrc_).writes()) {
      const auto gid = isolate_->isolate(w);
      if (dp_->writer_find((*i)->engine_, gid) == dp_->writer_end(gid)) {
        (*i)->source_out_of_date_ = true;
      }
    }
  }
  // 5. Regenerate source where necessary and invalidate engines
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    if (!(*i)->source_out_of_date_) {
      continue;
    }
    (*i)->newest_evals_ = n;
    (*i)->src_ = (*i)->regenerate_ir_source();
    (*i)->source_out_of_date_ = false;
    (*i)->engine_out_of_date_ = true;
  }
  // 6. Recompile everything with an outdated engine.
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    if (!(*i)->engine_out_of_date_) {
      continue;
    }
    compiler_->compile_and_replace(rt_, (*i)->engine_, (*i)->src_);
    (*i)->src_ = nullptr;
    if (compiler_->error()) {
      return;
    }
    (*i)->engine_out_of_date_ = false;
  }
  // 7. Synchronize subscriptions with the buffer
  for (auto i = iterator(this), ie = end(); i != ie; ++i) {
    for (auto r : ModuleInfo((*i)->psrc_).reads()) {
      const auto gid = isolate_->isolate(r);
      dp_->register_reader((*i)->engine_, gid);
    }
    for (auto w : ModuleInfo((*i)->psrc_).writes()) {
      const auto gid = isolate_->isolate(w);
      dp_->register_writer((*i)->engine_, gid);
    }
  }
}

Module::iterator Module::begin() {
  return iterator(this); 
}

Module::iterator Module::end() {
  return iterator();
}

ModuleDeclaration* Module::regenerate_ir_source() {
  const auto size = psrc_->get_items()->size();
  const auto ignore = parent_ == nullptr ? size - newest_evals_ : 0;
  auto md = isolate_->isolate(psrc_, ignore);

  const auto std = md->get_attrs()->get<String>("__std");
  const auto is_logic = (std != nullptr) && (std->get_readable_val() == "logic");
  if (is_logic) {
    DeAlias().run(md);
    ConstantProp().run(md);
    DeadCodeEliminate().run(md);
  }

  return md;
}

Engine* Module::engine() {
  return engine_;
}

Module::Instantiator::Instantiator(Module* ptr) {
  ptr_ = ptr;
  instances_.push_back(ptr_);
}

void Module::Instantiator::visit(const CaseGenerateConstruct* cgc) {
  Elaborate().get_elaboration(cgc)->accept(this);
}

void Module::Instantiator::visit(const IfGenerateConstruct* igc) {
  Elaborate().get_elaboration(igc)->accept(this);
}

void Module::Instantiator::visit(const LoopGenerateConstruct* lgc) {
  Elaborate().get_elaboration(lgc)->accept(this);
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
  auto child = new Module(itr->second, ptr_);
  ptr_->children_.push_back(child);

  // Continue down through this module
  ptr_ = child;
  instances_.push_back(ptr_);
  ptr_->psrc_->accept(this);
  ptr_ = child->parent_;
}

Module::Module(const ModuleDeclaration* psrc, Module* parent) :
  Module(psrc, parent->rt_, parent->dp_, parent->isolate_, parent->compiler_) {
  parent_ = parent;
}

} // namespace cascade
