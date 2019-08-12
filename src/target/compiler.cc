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

#include "target/compiler.h"

#include <cassert>
#include "target/engine.h"
#include "verilog/analyze/module_info.h"
#include "verilog/ast/ast.h"

using namespace std;

namespace cascade {

Compiler::Compiler() {
  shutdown_ = false;
}

Compiler::~Compiler() {
  for (auto& cc : core_compilers_) {
    delete cc.second;
  }
  for (auto& ic : interface_compilers_) {
    delete ic.second;
  }
}

Compiler& Compiler::set_core_compiler(const string& id, CoreCompiler* c) {
  assert(core_compilers_.find(id) == core_compilers_.end());
  assert(c != nullptr);
  core_compilers_[id] = c;
  return *this;
}

Compiler& Compiler::set_interface_compiler(const string& id, InterfaceCompiler* c) {
  assert(interface_compilers_.find(id) == interface_compilers_.end());
  assert(c != nullptr);
  interface_compilers_[id] = c;
  return *this;
}

CoreCompiler* Compiler::get_core_compiler(const std::string& id) {
  const auto itr = core_compilers_.find(id);
  return (itr == core_compilers_.end()) ? nullptr : itr->second;
}

InterfaceCompiler* Compiler::get_interface_compiler(const std::string& id) {
  const auto itr = interface_compilers_.find(id);
  return (itr == interface_compilers_.end()) ? nullptr : itr->second;
}

Engine* Compiler::compile(const Uuid& uuid, ModuleDeclaration* md) {
  { lock_guard<mutex> lg(lock_);
    if (shutdown_) {
      delete md;
      return nullptr;
    } else {
      uuids_.insert(uuid);
    }
  }

  if (StubCheck().check(md)) {
    delete md;
    return new Engine();
  }

  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  const auto target = md->get_attrs()->get<String>("__target")->get_readable_val();

  auto* ic = (loc == "remote") ? get_interface_compiler("remote") : get_interface_compiler("local");
  if (ic == nullptr) {
    error("Unable to locate the required interface compiler");
    delete md;
    return nullptr;
  }
  auto* i = ic->compile(md);
  if (i == nullptr) {
    delete md;
    return nullptr;
  }

  auto* cc = ((loc != "remote") && (loc != "runtime")) ? get_core_compiler("proxy") : get_core_compiler(target);
  if (cc == nullptr) {
    error("Unable to locate the required core compiler");
    delete md;
    return nullptr;
  }
  auto* c = cc->compile(uuid, md, i);
  if (c == nullptr) {
    delete i;
    return nullptr;
  }

  return new Engine(i, c, ic, cc);
}

void Compiler::abort(const Uuid& uuid) {
  for (auto& cc : core_compilers_) {
    cc.second->abort(uuid);
  }
}

void Compiler::abort_all() {
  { lock_guard<mutex> lg(lock_);
    shutdown_ = true;
  }
  for (const auto& u : uuids_) {
    abort(u);
  }
}

void Compiler::error(const string& s) {
  lock_guard<mutex> lg(lock_);
  what_ = (what_ == "") ? s : what_;
}

bool Compiler::error() {
  lock_guard<mutex> lg(lock_);
  return what_ != "";
}

string Compiler::what() {
  lock_guard<mutex> lg(lock_);
  return what_;
}

bool Compiler::StubCheck::check(const ModuleDeclaration* md) {
  ModuleInfo mi(md);
  if (!mi.inputs().empty() || !mi.outputs().empty()) {
    return false;
  }
  stub_ = true;
  md->accept(this);
  return stub_;
}

void Compiler::StubCheck::visit(const InitialConstruct* ic) {
  (void) ic;
  stub_ = false;
}

void Compiler::StubCheck::visit(const FinishStatement* fs) {
  (void) fs;
  stub_ = false;
}

void Compiler::StubCheck::visit(const RestartStatement* rs) {
  (void) rs;
  stub_ = false;
}

void Compiler::StubCheck::visit(const RetargetStatement* rs) {
  (void) rs;
  stub_ = false;
}

void Compiler::StubCheck::visit(const SaveStatement* ss) {
  (void) ss;
  stub_ = false;
}

} // namespace cascade
