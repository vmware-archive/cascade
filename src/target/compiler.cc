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

#include "src/target/compiler.h"

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

using namespace std;

namespace cascade {

Compiler::Compiler() {
  de10_compiler_ = new De10Compiler();
  proxy_compiler_ = new ProxyCompiler();
  sw_compiler_ = new SwCompiler();

  local_compiler_ = new LocalCompiler();
  remote_compiler_ = new RemoteCompiler();

  rt_ = nullptr;
  set_num_jit_threads(1);
}

Compiler::~Compiler() {
  pool_.stop_now();

  delete de10_compiler_;
  delete proxy_compiler_;
  delete sw_compiler_;

  delete local_compiler_;
  delete remote_compiler_;
}

Compiler& Compiler::set_runtime(Runtime* rt) {
  rt_ = rt;
  return *this;
}

Compiler& Compiler::set_num_jit_threads(size_t n) {
  pool_.stop_now();
  pool_.set_num_threads(n);
  pool_.run();
  return *this;
}

Compiler& Compiler::set_de10_compiler(De10Compiler* c) {
  delete de10_compiler_;
  de10_compiler_ = c;
  return *this;
}

Compiler& Compiler::set_proxy_compiler(ProxyCompiler* c) {
  delete proxy_compiler_;
  proxy_compiler_ = c;
  return *this;
}

Compiler& Compiler::set_sw_compiler(SwCompiler* c) {
  delete sw_compiler_;
  sw_compiler_ = c;
  return *this;
}

Compiler& Compiler::set_local_compiler(LocalCompiler* c) {
  delete local_compiler_;
  local_compiler_ = c;
  return *this;
}

Compiler& Compiler::set_remote_compiler(RemoteCompiler* c) {
  delete remote_compiler_;
  remote_compiler_ = c;
  return *this;
}

Engine* Compiler::compile(ModuleDeclaration* md) {
  if (StubCheck().check(md)) {
    delete md;
    return new Engine();
  }

  const auto loc = md->get_attrs()->get<String>("__loc");
  const auto target = md->get_attrs()->get<String>("__target");

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
    // TODO: error(unrecognized target)
    delete md;
    return nullptr;
  }

  auto i = ic->compile(md);
  if (i == nullptr) {
    // TODO: error(unable to compile interface)
    delete md;
    return nullptr;
  }

  auto c = cc->compile(i, md);
  if (c == nullptr) {
    // TODO: error(unable to compile core)
    delete i;
    return nullptr;
  }

  return new Engine(cc, c, ic, i);
}

Engine* Compiler::compile_and_replace(Engine* e, ModuleDeclaration* md) {
  // This method requires interaction with the runtime.
  if (rt_ == nullptr) {
    delete md;
    return nullptr;
  }

  // Lookup annotations 
  const auto std = md->get_attrs()->get<String>("__std");
  const auto t2 = md->get_attrs()->get<String>("__target2");
  const auto l2 = md->get_attrs()->get<String>("__loc2");

  // Check: Is this a stub, an std module, was jit compilation requested?  
  const auto jit = std->eq("logic") && !StubCheck().check(md) && (t2 != nullptr || l2 != nullptr);

  // If we're jit compiling, we'll need a second copy of the source and we'll
  // need to adjust the annotations.
  ModuleDeclaration* md2 = nullptr;
  if (jit) {
    md2 = md->clone();
    if (t2 != nullptr) {
      md2->get_attrs()->set_or_replace("__target", t2->clone());
    }
    if (l2 != nullptr) {
      md2->get_attrs()->set_or_replace("__loc", l2->clone());
    }
  }

  // Fast Path. Compile and replace the original engine.
  auto e_fast = compile(md);
  if (e_fast == nullptr) {
    return nullptr;
  }
  e->replace_with(e_fast);
  rt_->schedule_all_active();

  // Slow Path: Schedule a thread to compile in the background and swap in the
  // results in a safe runtime window when it's done.
  if (jit) {
    pool_.insert(new ThreadPool::Job([this, e, md2]{
      Masker().mask(md2);
      auto e_slow = compile(md2);
      if (e_slow == nullptr) {
        rt_->fatal(0, "Unable to materialize logic for second-pass jit compilation");
      } else {
        rt_->schedule_interrupt([e, e_slow]{
          e->replace_with(e_slow);
        });
      }
    }));
  }

  return e;
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
  md->get_items()->accept(this);
}

void Compiler::Masker::edit(InitialConstruct* ic) {
  ic->get_attrs()->set_or_replace("__ignore", new String("true"));
}

} // namespace cascade
