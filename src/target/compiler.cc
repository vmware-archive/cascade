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

#include "src/target/compiler.h"

#include <cassert>
#include "src/runtime/data_plane.h"
#include "src/runtime/runtime.h"
#include "src/target/core/de10/de10_compiler.h"
#include "src/target/core/proxy/proxy_compiler.h"
#include "src/target/core/sw/sw_compiler.h"
#include "src/target/engine.h"
#include "src/target/interface/local/local_compiler.h"
#include "src/target/interface/remote/remote_compiler.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

Compiler::Compiler() {
  de10_compiler_ = nullptr;
  proxy_compiler_ = nullptr;
  sw_compiler_ = nullptr;

  local_compiler_ = nullptr;
  remote_compiler_ = nullptr;

  // We only really need two jit threads, one for the current background job,
  // and a second to preempt it if necessary before it finishes. But since
  // there's startup and shutdown costs associated with threads, let's double
  // provision.
  pool_.stop_now();
  pool_.set_num_threads(4);
  pool_.run();
}

Compiler::~Compiler() {
  if (de10_compiler_ != nullptr) {
    de10_compiler_->abort();
  }
  if (proxy_compiler_ != nullptr) {
    proxy_compiler_->abort();
  }
  if (sw_compiler_ != nullptr) {
    sw_compiler_->abort();
  }
  if (local_compiler_ != nullptr) {
    local_compiler_->abort();
  }
  if (remote_compiler_ != nullptr) {
    remote_compiler_->abort();
  }

  pool_.stop_now();

  if (de10_compiler_ != nullptr) {
    delete de10_compiler_;
  }
  if (proxy_compiler_ != nullptr) {
    delete proxy_compiler_;
  }
  if (sw_compiler_ != nullptr) {
    delete sw_compiler_;
  }
  if (local_compiler_ != nullptr) {
    delete local_compiler_;
  }
  if (remote_compiler_ != nullptr) {
    delete remote_compiler_;
  }
}

Compiler& Compiler::set_de10_compiler(De10Compiler* c) {
  assert(de10_compiler_ == nullptr);
  assert(c != nullptr);
  de10_compiler_ = c;
  de10_compiler_->set_compiler(this);
  return *this;
}

Compiler& Compiler::set_proxy_compiler(ProxyCompiler* c) {
  assert(proxy_compiler_ == nullptr);
  assert(c != nullptr);
  proxy_compiler_ = c;
  proxy_compiler_->set_compiler(this);
  return *this;
}

Compiler& Compiler::set_sw_compiler(SwCompiler* c) {
  assert(sw_compiler_ == nullptr);
  assert(c != nullptr);
  sw_compiler_ = c;
  sw_compiler_->set_compiler(this);
  return *this;
}

Compiler& Compiler::set_local_compiler(LocalCompiler* c) {
  assert(local_compiler_ == nullptr);
  assert(c != nullptr);
  local_compiler_ = c;
  local_compiler_->set_compiler(this);
  return *this;
}

Compiler& Compiler::set_remote_compiler(RemoteCompiler* c) {
  assert(remote_compiler_ == nullptr);
  assert(c != nullptr);
  remote_compiler_ = c;
  remote_compiler_->set_compiler(this);
  return *this;
}

Engine* Compiler::compile(ModuleDeclaration* md) {
  if (StubCheck().check(md)) {
    delete md;
    return new Engine();
  }

  const auto* loc = md->get_attrs()->get<String>("__loc");
  const auto* target = md->get_attrs()->get<String>("__target");

  InterfaceCompiler* ic = nullptr;
  if (loc != nullptr && loc->eq("remote")) {
    ic = remote_compiler_; 
  } else {
    ic = local_compiler_;
  }

  CoreCompiler* cc = nullptr;
  if (loc != nullptr && !loc->eq("runtime") && !loc->eq("remote")) {
    cc = proxy_compiler_;      
  } else if (target->eq("de10")) {
    cc = de10_compiler_; 
  } else if (target->eq("sw")) {
    cc = sw_compiler_;
  } else {
    error("Unable to compile module with unsupported target type " + target->get_readable_val());
    delete md;
    return nullptr;
  }

  if (ic == nullptr) {
    error("Unable to locate the required interface compiler");
    delete md;
    return nullptr;
  }
  auto* i = ic->compile(md);
  if (i == nullptr) {
    // No need to attach an error message here, ic will already have done so if necessary.
    delete md;
    return nullptr;
  }

  if (cc == nullptr) {
    error("Unable to locate the required core compiler");
    delete md;
    return nullptr;
  }
  auto* c = cc->compile(i, md);
  if (c == nullptr) {
    // No need to attach an error message here, cc will already have done so if necessary.
    delete i;
    return nullptr;
  }

  return new Engine(c, i);
}

void Compiler::compile_and_replace(Runtime* rt, Engine* e, ModuleDeclaration* md) {
  // Lookup annotations 
  const auto* std = md->get_attrs()->get<String>("__std");
  const auto* t = md->get_attrs()->get<String>("__target");
  const auto* l = md->get_attrs()->get<String>("__loc");

  // Check: Is this a stub, an std module, was jit compilation requested?  
  const auto tsep = t->get_readable_val().find_first_of(':');
  const auto lsep = l->get_readable_val().find_first_of(':');
  const auto jit = std->eq("logic") && !StubCheck().check(md) && ((tsep != string::npos) || (lsep != string::npos));

  // If we're jit compiling, we'll need a second copy of the source and we'll
  // need to adjust the annotations.
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
  }

  // Fast Path. Compile and replace the original engine.  If an error occurred,
  // then simply preserve the original message. If compilation was aborted
  // without explanation, that's an error that requires explanation.
  stringstream ss;
  TextPrinter(ss) << "fast-pass recompilation of " <<  md->get_id() << " with attributes " << md->get_attrs();
  auto* e_fast = compile(md);
  if (error()) {
    return;
  } else if (e_fast == nullptr) {
    return error("An unhandled error occurred during module compilation");
  }
  e->replace_with(e_fast);
  if (e->is_stub()) {
    rt->info("Deferring " + ss.str());
  } else {
    rt->info("Finished " + ss.str());
  }

  // Slow Path: Schedule a thread to compile in the background and swap in the
  // results in a safe runtime window when it's done. Note that we schedule an
  // interrupt regardless. This is to trigger an interaction with the runtime
  // even if only just for the sake of catching an error.
  if (jit) {
    pool_.insert(new ThreadPool::Job([this, rt, e, md2]{
      stringstream ss;
      TextPrinter(ss) << "slow-pass recompilation of " <<  md2->get_id() << " with attributes " << md2->get_attrs();
      const auto str = ss.str();

      Masker().mask(md2);
      auto* e_slow = compile(md2);
      rt->schedule_interrupt([this, e, e_slow, rt, str]{
        // Nothing to do if compilation failed or was aborted
        if (error() || (e_slow == nullptr)) {
          rt->info("Aborted " + str);
          return;
        } 
        e->replace_with(e_slow);
      });
      rt->info("Finished " + str);
    }));
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

void Compiler::StubCheck::visit(const DisplayStatement* ds) {
  (void) ds;
  stub_ = false;
}

void Compiler::StubCheck::visit(const FinishStatement* fs) {
  (void) fs;
  stub_ = false;
}

void Compiler::StubCheck::visit(const WriteStatement* ws) {
  (void) ws;
  stub_ = false;
}

void Compiler::Masker::mask(ModuleDeclaration* md) {
  md->accept_items(this);
}

void Compiler::Masker::edit(InitialConstruct* ic) {
  ic->get_attrs()->set_or_replace("__ignore", new String("true"));
}

} // namespace cascade
