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
#include "runtime/data_plane.h"
#include "runtime/runtime.h"
#include "target/engine.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"
#include "verilog/transform/delete_initial.h"

using namespace std;

namespace cascade {

Compiler::Compiler() {
  // We only really need two jit threads, one for the current background job,
  // and a second to preempt it if necessary before it finishes. We
  // double-provision for the sake of performance, as there is shutdown cost
  // associated with aborting compilations.
  pool_.set_num_threads(4);
  pool_.run();
}

Compiler::~Compiler() {
  for (auto& cc : core_compilers_) {
    cc.second->abort();
  }
  for (auto& ic : interface_compilers_) {
    ic.second->abort();
  }
  pool_.stop_now();
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

Engine* Compiler::compile(ModuleDeclaration* md) {
  if (StubCheck().check(md)) {
    delete md;
    return new Engine();
  }

  const auto* loc = md->get_attrs()->get<String>("__loc");
  const auto* target = md->get_attrs()->get<String>("__target");

  auto* ic = (loc->get_readable_val() == "remote") ? get_interface_compiler("remote") : get_interface_compiler("local");
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

  auto* cc = get_core_compiler(target->get_readable_val());
  if (cc == nullptr) {
    error("Unable to locate the required core compiler");
    delete md;
    return nullptr;
  }
  auto* c = cc->compile(i, md);
  if (c == nullptr) {
    delete i;
    return nullptr;
  }

  return new Engine(i, c, ic, cc);
}

void Compiler::compile_and_replace(Runtime* rt, Engine* e, size_t& version, ModuleDeclaration* md, const Identifier* id) {
  // Bump the sequence number for this engine
  ++version;
  const auto this_version = version;

  // Record human readable name for this module
  const auto fid = Resolve().get_readable_full_id(id);

  // Lookup annotations 
  const auto* std = md->get_attrs()->get<String>("__std");
  const auto* t = md->get_attrs()->get<String>("__target");
  const auto* l = md->get_attrs()->get<String>("__loc");

  // Check: Is this a stub, an std module, was jit compilation requested?  
  const auto tsep = t->get_readable_val().find_first_of(';');
  const auto lsep = l->get_readable_val().find_first_of(';');
  const auto jit = std->eq("logic") && !StubCheck().check(md) && ((tsep != string::npos) || (lsep != string::npos));

  // If we're jit compiling, we'll need a second copy of the source and we'll
  // need to adjust the annotations. Otherwise just allocate a dummy module.
  // Having a non-null module as an invariant simplifies the code path below.
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

  // Fast Path. Compile and replace the original engine.  If an error occurred,
  // then simply preserve the original message. If compilation was aborted
  // without explanation, that's an error that requires explanation.
  stringstream ss;
  TextPrinter(ss) << "fast-pass recompilation of " << fid << " with attributes " << md->get_attrs();
  auto* e_fast = compile(md);
  if (e_fast == nullptr) {
    if (!error()) {
      error("An unhandled error occurred during module compilation");
    }
  } else {
    e->replace_with(e_fast);
    if (e->is_stub()) {
      ostream(rt->rdbuf(Runtime::stdinfo_)) << "Deferring " << ss.str() << endl;
    } else {
      ostream(rt->rdbuf(Runtime::stdinfo_)) << "Finished " << ss.str() << endl;
    }
  }

  // Slow Path: Schedule a thread to compile in the background and swap in the
  // results in a safe runtime window when it's done. Note that we schedule an
  // interrupt regardless. This is to trigger an interaction with the runtime
  // even if only just for the sake of catching an error. 
  if (jit && (e_fast != nullptr)) {
    pool_.insert(new ThreadPool::Job([this, rt, e, md2, fid, this_version, &version]{
      stringstream ss;
      TextPrinter(ss) << "slow-pass recompilation of " << fid << " with attributes " << md2->get_attrs();
      const auto str = ss.str();

      DeleteInitial().run(md2);
      auto* e_slow = compile(md2);
      // If compilation came back before the runtime is shutdown, we can swap
      // it in place of the fast-pass compilation. Otherwise we delete it.
      rt->schedule_interrupt([e, e_slow, rt, str, this_version, &version]{
        if ((this_version < version) || (e_slow == nullptr)) {
          ostream(rt->rdbuf(Runtime::stdinfo_)) << "Aborted " << str << endl;
        } else {
          e->replace_with(e_slow);
          ostream(rt->rdbuf(Runtime::stdinfo_)) << "Finished " << str << endl;
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
