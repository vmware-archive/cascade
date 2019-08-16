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
#include "target/compiler/stub_core.h"
#include "target/core_compiler.h"
#include "target/engine.h"
#include "verilog/analyze/module_info.h"
#include "verilog/ast/ast.h"

using namespace std;

namespace cascade {

Compiler::~Compiler() {
  for (auto& cc : ccs_) {
    delete cc.second;
  }
}

Compiler& Compiler::set(const string& id, CoreCompiler* c) {
  assert(ccs_.find(id) == ccs_.end());
  assert(c != nullptr);
  ccs_[id] = c;
  return *this;
}

CoreCompiler* Compiler::get(const std::string& id) {
  const auto itr = ccs_.find(id);
  return (itr == ccs_.end()) ? nullptr : itr->second;
}

Engine* Compiler::compile_stub(const ModuleDeclaration* md) {
  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  auto* i = get_interface(loc);
  assert(i != nullptr);
  return new Engine(i, new StubCore(i));
}

Engine* Compiler::compile(ModuleDeclaration* md) {
  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  auto* i = get_interface(loc);
  if (i == nullptr) {
    error("Unable to provide an interface for a module with incompatible __loc annotation");
    delete md; 
    return nullptr;
  }

  if (StubCheck().check(md)) {
    delete md;
    return new Engine(i, new StubCore(i));
  }

  const auto target = md->get_attrs()->get<String>("__target")->get_readable_val();
  auto* cc = ((loc != "remote") && (loc != "local")) ? get("proxy") : get(target);
  if (cc == nullptr) {
    error("Unable to locate the required core compiler");
    delete md;
    return nullptr;
  }
  auto* c = cc->compile(md, i);
  if (c == nullptr) {
    delete i;
    return nullptr;
  }

  return new Engine(i, c);
}

void Compiler::stop_compile() {
  for (auto& cc : ccs_) {
    cc.second->stop_compile();
  }
}

void Compiler::stop_async() {
  for (auto& cc : ccs_) {
    cc.second->stop_async();
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
