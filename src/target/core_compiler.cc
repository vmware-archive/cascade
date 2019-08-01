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

#include "target/core_compiler.h"

#include <sstream>
#include "target/compiler.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/ast/ast.h"

using namespace std;

namespace cascade {

CoreCompiler::CoreCompiler() {
  shutdown_ = false;
  set_compiler(nullptr);
}

CoreCompiler& CoreCompiler::set_compiler(Compiler* c) {
  compiler_ = c;
  return *this;
}

Core* CoreCompiler::compile(const Uuid& uuid, size_t version, ModuleDeclaration* md, Interface* interface) {
  { lock_guard<mutex> lg(lock_);
    // Return immediately if we're in the shutdown state
    if (shutdown_) {
      delete md;
      return nullptr; 
    }
    // Also return immediately if this is a compile request for an old version
    const auto itr = compilations_.find(uuid);
    if ((itr != compilations_.end()) && (version < itr->second)) {
      delete md;
      return nullptr;
    }
    // Record this as the most current version that we've seen for this uuid
    compilations_[uuid] = version;
  }

  const auto* std = md->get_attrs()->get<String>("__std");
  if (std->eq("clock")) {
    return compile_clock(uuid, md, interface);
  } else if (std->eq("gpio")) {
    return compile_gpio(uuid, md, interface);
  } else if (std->eq("led")) {
    return compile_led(uuid, md, interface);
  } else if (std->eq("logic")) {
    return compile_logic(uuid, md, interface);
  } else if (std->eq("pad")) {
    return compile_pad(uuid, md, interface);
  } else if (std->eq("reset")) {
    return compile_reset(uuid, md, interface);
  } else {
    return compile_custom(uuid, md, interface);
  }
}

void CoreCompiler::shutdown() {
  lock_guard<mutex> lg(lock_); 
  shutdown_ = true;
  for (const auto& c : compilations_) {
    abort(c.first);
  }
}   

Clock* CoreCompiler::compile_clock(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  (void) uuid;
  (void) interface;
  delete md;
  error("No compiler support available for modules of type clock");
  return nullptr;
}

Custom* CoreCompiler::compile_custom(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  (void) uuid;
  (void) interface;

  const auto* std = md->get_attrs()->get<String>("__std");
  assert(std != nullptr);
  error("No compiler support available for custom modules of type " + std->get_readable_val());

  delete md;
  return nullptr;
}

Gpio* CoreCompiler::compile_gpio(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  (void) uuid;
  (void) interface;
  delete md;
  error("No compiler support available for modules of type gpio");
  return nullptr;
}

Led* CoreCompiler::compile_led(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  (void) uuid;
  (void) interface;
  delete md;
  error("No compiler support available for modules of type led");
  return nullptr;
}

Pad* CoreCompiler::compile_pad(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  (void) uuid;
  (void) interface;
  delete md;
  error("No compiler support available for modules of type pad");
  return nullptr;
}

Reset* CoreCompiler::compile_reset(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  (void) uuid;
  (void) interface;
  delete md;
  error("No compiler support available for modules of type reset");
  return nullptr;
}

Logic* CoreCompiler::compile_logic(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  (void) uuid;
  (void) interface;
  delete md;
  error("No compiler support available for modules of type logic");
  return nullptr;
} 

void CoreCompiler::error(const string& s) {
  if (compiler_ != nullptr) {
    compiler_->error(s);
  }
}

MId CoreCompiler::to_mid(const Identifier* id) const {
  return to_vid(id);
}

VId CoreCompiler::to_vid(const Identifier* id) const {
  VId res;
  stringstream ss(id->front_ids()->get_readable_sid().substr(3));
  ss >> res;
  return res;
}

bool CoreCompiler::check_io(const ModuleDeclaration* md, size_t is, size_t os) const {
  if (is == 0) {
    if (ModuleInfo(md).inputs().size() != 0) {
      return false;
    }
  } else {
    if ((ModuleInfo(md).inputs().size() == 1) && (Evaluate().get_width(*ModuleInfo(md).inputs().begin()) > is)) {
      return false;
    } else if (ModuleInfo(md).inputs().size() > 1) {
      return false;
    }
  }
  if (os == 0) {
    if (ModuleInfo(md).outputs().size() != 0) {
      return false;
    }
  } else {
    if ((ModuleInfo(md).outputs().size() == 1) && (Evaluate().get_width(*ModuleInfo(md).outputs().begin()) > os)) {
      return false;
    } else if (ModuleInfo(md).outputs().size() > 1) {
      return false;
    }
  }
  return true;
}

} // namespace cascade
