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

#include "src/target/core/sw/sw_compiler.h"

#include "src/base/stream/incstream.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

SwCompiler::SwCompiler() : CoreCompiler() { 
  set_include_dirs("");
  set_led(nullptr, nullptr);
  set_pad(nullptr, nullptr);
  set_reset(nullptr, nullptr);
}

SwCompiler& SwCompiler::set_include_dirs(const std::string& s) {
  include_dirs_ = s;
  return *this;
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

void SwCompiler::abort() {
  // Does nothing.
}

SwClock* SwCompiler::compile_clock(Interface* interface, ModuleDeclaration* md) {
  if (!check_io(md, 0, 1)) {
    error("Unable to compile a software clock with more than one output");
    delete md;
    return nullptr;
  }

  const auto* out = *ModuleInfo(md).outputs().begin();
  const auto id = to_vid(out);
  delete md;

  return new SwClock(interface, id);
}

SwFifo* SwCompiler::compile_fifo(Interface* interface, ModuleDeclaration* md) {
  ModuleInfo info(md);

  // Create a memory with the appropriate size and width parameters
  size_t depth = 0;
  size_t byte_size = 0;
  for (auto* l : info.locals()) {
    if (l->get_parent()->is(Node::Tag::localparam_declaration)) {
      auto* ld = static_cast<const LocalparamDeclaration*>(l->get_parent());
      const auto* id = ld->get_attrs()->get<Identifier>("__id");
      if (id == nullptr) {
        continue;
      } else if (id->back_ids()->eq("DEPTH")) {
        depth = Evaluate().get_value(ld->get_val()).to_int();
      } else if (id->back_ids()->eq("BYTE_SIZE")) {
        byte_size = Evaluate().get_value(ld->get_val()).to_int();
      }
    }
  }
  if (depth == 0 || byte_size == 0) {
    error("Unable to compile a software fifo with depth or byte size equal to zero");
    delete md;
    return nullptr;
  }
  auto* fifo = new SwFifo(interface, depth, byte_size);

  // Set backup file if __file annotation was provided
  auto* file = md->get_attrs()->get<String>("__file");
  auto* count = md->get_attrs()->get<Number>("__count");
  if (file != nullptr) {
    const auto c = (count == nullptr) ? 1 : count->get_val().to_int();
    const auto f = incstream(include_dirs_).find(file->get_readable_val());
    if (f == "") {
      error("Unable to compile a software fifo with an unresolvable file annotation");
      delete md;
      return nullptr;
    }
    fifo->set_file(f, c);
  } 

  // Set up input output connections
  for (auto* l : info.locals()) {
    assert(l->get_parent()->is_subclass_of(Node::Tag::declaration));
    auto* d = static_cast<const Declaration*>(l->get_parent());
    if (d->get_parent()->is(Node::Tag::port_declaration)) {
      auto* pd = static_cast<const PortDeclaration*>(d->get_parent());
      const auto* id = pd->get_attrs()->get<Identifier>("__id");
      if (id->back_ids()->eq("clock")) {
        fifo->set_clock(to_vid(l));
      } else if (id->back_ids()->eq("rreq")) {
        fifo->set_rreq(to_vid(l));
      } else if (id->back_ids()->eq("rdata")) {
        fifo->set_rdata(to_vid(l));
      } else if (id->back_ids()->eq("wreq")) {
        fifo->set_wreq(to_vid(l));
      } else if (id->back_ids()->eq("wdata")) {
        fifo->set_wdata(to_vid(l));
      } else if (id->back_ids()->eq("empty")) {
        fifo->set_empty(to_vid(l));
      } else if (id->back_ids()->eq("full")) {
        fifo->set_full(to_vid(l));
      }
    }
  }

  delete md;
  return fifo;
}

SwLed* SwCompiler::compile_led(Interface* interface, ModuleDeclaration* md) {
  if (led_ == nullptr) {
    error("Unable to compile a software led without a reference to a software fpga");
    delete md;
    return nullptr;
  }
  if (!check_io(md, 8, 8)) {
    error("Unable to compile a software led with more than 8 outputs");
    delete md;
    return nullptr;
  }
  
  if (!ModuleInfo(md).inputs().empty()) {
    const auto* in = *ModuleInfo(md).inputs().begin();
    const auto id = to_vid(in);
    const auto w = Evaluate().get_width(in);
    delete md;
    return new SwLed(interface, id, w, led_, led_lock_);
  } else {
    delete md;
    return new SwLed(interface, nullid(), 0, led_, led_lock_);
  }
}

SwLogic* SwCompiler::compile_logic(Interface* interface, ModuleDeclaration* md) {
  ModuleInfo info(md);
  auto* c = new SwLogic(interface, md);
  for (auto* i : info.inputs()) {
    c->set_read(i, to_vid(i));
  }
  for (auto* o : info.outputs()) {
    c->set_write(o, to_vid(o));
  }
  for (auto* s : info.stateful()) { 
    c->set_state(s, to_vid(s));
  }
  for (auto* s : info.streams()) {
    c->set_stream(s, to_vid(s));
  }
  return c;
} 

SwMemory* SwCompiler::compile_memory(Interface* interface, ModuleDeclaration* md) {
  ModuleInfo info(md);

  // Create a memory with the appropriate size and width parameters
  size_t addr_size = 0;
  size_t byte_size = 0;
  for (auto* l : info.locals()) {
    if (l->get_parent()->is(Node::Tag::localparam_declaration)) {
      auto* ld = static_cast<const LocalparamDeclaration*>(l->get_parent());
      const auto* id = ld->get_attrs()->get<Identifier>("__id");
      if (id == nullptr) {
        continue;
      } else if (id->back_ids()->eq("ADDR_SIZE")) {
        addr_size = Evaluate().get_value(ld->get_val()).to_int();
      } else if (id->back_ids()->eq("BYTE_SIZE")) {
        byte_size = Evaluate().get_value(ld->get_val()).to_int();
      }
    }
  }
  if (addr_size == 0 || byte_size == 0) {
    error("Unable to compile a software memory with address or data width equal to zero");
    delete md;
    return nullptr;
  }
  auto* mem = new SwMemory(interface, addr_size, byte_size);

  // Set backup file if __file annotation was provided
  auto* file = md->get_attrs()->get<String>("__file");
  if (file != nullptr) {
    const auto f = incstream(include_dirs_).find(file->get_readable_val());
    mem->set_file((f == "") ? file->get_readable_val() : f); 
  }

  // Set up input output connections
  for (auto* l : info.locals()) {
    assert(l->get_parent()->is_subclass_of(Node::Tag::declaration));
    auto* d = static_cast<const Declaration*>(l->get_parent());
    if (d->get_parent()->is(Node::Tag::port_declaration)) {
      auto* pd = static_cast<const PortDeclaration*>(d->get_parent());
      const auto* id = pd->get_attrs()->get<Identifier>("__id");
      if (id->back_ids()->eq("clock")) {
        mem->set_clock(to_vid(l));
      } else if (id->back_ids()->eq("wen")) {
        mem->set_wen(to_vid(l));
      } else if (id->back_ids()->eq("raddr1")) {
        mem->set_raddr1(to_vid(l));
      } else if (id->back_ids()->eq("rdata1")) {
        mem->set_rdata1(to_vid(l));
      } else if (id->back_ids()->eq("raddr2")) {
        mem->set_raddr2(to_vid(l));
      } else if (id->back_ids()->eq("rdata2")) {
        mem->set_rdata2(to_vid(l));
      } else if (id->back_ids()->eq("waddr")) {
        mem->set_waddr(to_vid(l));
      } else if (id->back_ids()->eq("wdata")) {
        mem->set_wdata(to_vid(l));
      }
    }
  }

  delete md;
  return mem;
}

SwPad* SwCompiler::compile_pad(Interface* interface, ModuleDeclaration* md) {
  if (pad_ == nullptr) {
    error("Unable to compile a software pad without a reference to a software fpga");
    delete md;
    return nullptr;
  }
  if (pad_ == nullptr || !check_io(md, 0, 4)) {
    error("Unable to compile a software pad with more than four inputs");
    delete md;
    return nullptr;
  }

  const auto* out = *ModuleInfo(md).outputs().begin();
  const auto id = to_vid(out);
  const auto w = Evaluate().get_width(out);
  delete md;
  
  return new SwPad(interface, id, w, pad_, pad_lock_);
}

SwReset* SwCompiler::compile_reset(Interface* interface, ModuleDeclaration* md) {
  if (pad_ == nullptr) {
    error("Unable to compile a software reset without a reference to a software fpga");
    delete md;
    return nullptr;
  }
  if (pad_ == nullptr || !check_io(md, 0, 1)) {
    error("Unable to compile a software reset with more than one input");
    delete md;
    return nullptr;
  }

  const auto* out = *ModuleInfo(md).outputs().begin();
  const auto id = to_vid(out);
  delete md;

  return new SwReset(interface, id, reset_, reset_lock_);
}

} // namespace cascade
