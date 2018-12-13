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

#include "src/target/core_compiler.h"

#include <sstream>
#include "src/target/compiler.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

CoreCompiler::CoreCompiler() {
  set_compiler(nullptr);
}

CoreCompiler& CoreCompiler::set_compiler(Compiler* c) {
  compiler_ = c;
  return *this;
}

Core* CoreCompiler::compile(Interface* interface, ModuleDeclaration* md) {
  const auto std = md->get_attrs()->get<String>("__std");
  if (std->eq("clock")) {
    return compile_clock(interface, md); 
  } else if (std->eq("fifo")) {
    return compile_fifo(interface, md);
  } else if (std->eq("gpio")) {
    return compile_gpio(interface, md);
  } else if (std->eq("led")) {
    return compile_led(interface, md);
  } else if (std->eq("memory")) {
    return compile_memory(interface, md);
  } else if (std->eq("pad")) {
    return compile_pad(interface, md);
  } else if (std->eq("reset")) {
    return compile_reset(interface, md);
  } else {
    return compile_logic(interface, md);
  }
}

Clock* CoreCompiler::compile_clock(Interface* interface, ModuleDeclaration* md) {
  (void) interface;
  delete md;
  error("No compiler support available for modules of type clock");
  return nullptr;
}

Fifo* CoreCompiler::compile_fifo(Interface* interface, ModuleDeclaration* md) {
  (void) interface;
  delete md;
  error("No compiler support available for modules of type fifo");
  return nullptr;
}

Gpio* CoreCompiler::compile_gpio(Interface* interface, ModuleDeclaration* md) {
  (void) interface;
  delete md;
  error("No compiler support available for modules of type gpio");
  return nullptr;
}

Led* CoreCompiler::compile_led(Interface* interface, ModuleDeclaration* md) {
  (void) interface;
  delete md;
  error("No compiler support available for modules of type led");
  return nullptr;
}

Memory* CoreCompiler::compile_memory(Interface* interface, ModuleDeclaration* md) {
  (void) interface;
  delete md;
  error("No compiler support available for modules of type memory");
  return nullptr;
}

Pad* CoreCompiler::compile_pad(Interface* interface, ModuleDeclaration* md) {
  (void) interface;
  delete md;
  error("No compiler support available for modules of type pad");
  return nullptr;
}

Reset* CoreCompiler::compile_reset(Interface* interface, ModuleDeclaration* md) {
  (void) interface;
  delete md;
  error("No compiler support available for modules of type reset");
  return nullptr;
}

Logic* CoreCompiler::compile_logic(Interface* interface, ModuleDeclaration* md) {
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
