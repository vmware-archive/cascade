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

#include "target/core/sw/sw_compiler.h"

#include "target/compiler.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"

using namespace std;

namespace cascade {

SwCompiler::SwCompiler() : CoreCompiler() { 
  set_led(nullptr, nullptr);
  set_pad(nullptr, nullptr);
  set_reset(nullptr, nullptr);
}

SwCompiler& SwCompiler::set_led(Bits* b, mutex* l) {
  led_ = b;
  led_lock_ = l;
  return *this;
}

SwCompiler& SwCompiler::set_pad(Bits* b, mutex* l) {
  pad_ = b;
  pad_lock_ = l;
  return *this;
}

SwCompiler& SwCompiler::set_reset(Bits* b, mutex* l) {
  reset_ = b;
  reset_lock_ = l;
  return *this;
}

void SwCompiler::stop_compile(Engine::Id id) {
  // Does nothing. Compilations all return in a reasonable amount of time.
  (void) id;
}

void SwCompiler::stop_async() {
  // Does nnothing. This class does not produce asynchronous tasks.
}

SwClock* SwCompiler::compile_clock(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  (void) id;

  if (!check_io(md, 0, 1)) {
    get_compiler()->error("Unable to compile a software clock with more than one output");
    delete md;
    return nullptr;
  }

  const auto* out = *ModuleInfo(md).outputs().begin();
  const auto oid = to_vid(out);
  delete md;

  return new SwClock(interface, oid);
}

SwLed* SwCompiler::compile_led(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  (void) id;

  if (led_ == nullptr) {
    get_compiler()->error("Unable to compile a software led without a reference to a software fpga");
    delete md;
    return nullptr;
  }
  if (!check_io(md, 8, 8)) {
    get_compiler()->error("Unable to compile a software led with more than 8 outputs");
    delete md;
    return nullptr;
  }
  
  if (!ModuleInfo(md).inputs().empty()) {
    const auto* in = *ModuleInfo(md).inputs().begin();
    const auto iid = to_vid(in);
    const auto w = Evaluate().get_width(in);
    delete md;
    return new SwLed(interface, iid, w, led_, led_lock_);
  } else {
    delete md;
    return new SwLed(interface, nullid(), 0, led_, led_lock_);
  }
}

SwLogic* SwCompiler::compile_logic(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  (void) id;

  ModuleInfo info(md);
  auto* c = new SwLogic(interface, md);
  for (auto* i : info.inputs()) {
    c->set_input(i, to_vid(i));
  }
  for (auto* s : info.stateful()) { 
    c->set_state(s, to_vid(s));
  }
  for (auto* o : info.outputs()) {
    c->set_output(o, to_vid(o));
  }
  return c;
} 

SwPad* SwCompiler::compile_pad(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  (void) id;

  if (pad_ == nullptr) {
    get_compiler()->error("Unable to compile a software pad without a reference to a software fpga");
    delete md;
    return nullptr;
  }
  if (pad_ == nullptr || !check_io(md, 0, 4)) {
    get_compiler()->error("Unable to compile a software pad with more than four inputs");
    delete md;
    return nullptr;
  }

  const auto* out = *ModuleInfo(md).outputs().begin();
  const auto oid = to_vid(out);
  const auto w = Evaluate().get_width(out);
  delete md;
  
  return new SwPad(interface, oid, w, pad_, pad_lock_);
}

SwReset* SwCompiler::compile_reset(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  (void) id;

  if (pad_ == nullptr) {
    get_compiler()->error("Unable to compile a software reset without a reference to a software fpga");
    delete md;
    return nullptr;
  }
  if (pad_ == nullptr || !check_io(md, 0, 1)) {
    get_compiler()->error("Unable to compile a software reset with more than one input");
    delete md;
    return nullptr;
  }

  const auto* out = *ModuleInfo(md).outputs().begin();
  const auto oid = to_vid(out);
  delete md;

  return new SwReset(interface, oid, reset_, reset_lock_);
}

} // namespace cascade
