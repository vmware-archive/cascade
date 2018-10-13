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

#include "src/target/core/de10/de10_compiler.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "src/target/core/de10/boxer.h"
#include "src/target/core/de10/quartus_client.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

constexpr auto HW_REGS_BASE = 0xfc000000;
constexpr auto HW_REGS_SPAN = 0x04000000;
constexpr auto HW_REGS_MASK = HW_REGS_SPAN - 1;
constexpr auto ALT_LWFPGALVS_OFST = 0xff200000;
constexpr auto LED_PIO_BASE = 0x00003000;
constexpr auto PAD_PIO_BASE = 0x00004000;
constexpr auto GPIO_PIO_BASE = 0x00005000;
constexpr auto LOG_PIO_BASE = 0x00040000;

De10Compiler::De10Compiler() : CoreCompiler() { 
  fd_ = open("/dev/mem", (O_RDWR | O_SYNC));
  if (fd_ == -1) {
    virtual_base_ = (volatile uint32_t*)MAP_FAILED;
    return;
  }
  virtual_base_ = (volatile uint32_t*)mmap(NULL, HW_REGS_SPAN, (PROT_READ|PROT_WRITE), MAP_SHARED, fd_, HW_REGS_BASE);
  if (virtual_base_ == MAP_FAILED) {
    return;
  }
  set_quartus_client(nullptr);
}

De10Compiler::~De10Compiler() {
  if (fd_ != -1) {
    close(fd_);
  }
  if (virtual_base_ != MAP_FAILED) {
    munmap((void*)virtual_base_, HW_REGS_SPAN);
  }
}

De10Compiler& De10Compiler::set_quartus_client(QuartusClient* qc) {
  qc_ = qc;
  return *this;
}

void De10Compiler::teardown(Core* c) {
  // Does nothing. For a given run of cascade, which corresponds to the
  // lifetime of this object, the code that we care about will only grow.
  (void) c;
}

De10Gpio* De10Compiler::compile_gpio(Interface* interface, ModuleDeclaration* md) {
  if (virtual_base_ == MAP_FAILED || !check_io(md, 8, 8)) {
    delete md;
    return nullptr;
  }

  auto led_addr = (volatile uint32_t*)(virtual_base_+((ALT_LWFPGALVS_OFST + GPIO_PIO_BASE) & HW_REGS_MASK));
  if (!ModuleInfo(md).inputs().empty()) {
    const auto in = *ModuleInfo(md).inputs().begin();
    const auto id = to_vid(in);
    delete md;
    return new De10Gpio(interface, id, led_addr);
  } else {
    delete md;
    return new De10Gpio(interface, nullid(), led_addr);
  }
}

De10Led* De10Compiler::compile_led(Interface* interface, ModuleDeclaration* md) {
  if (virtual_base_ == MAP_FAILED || !check_io(md, 8, 8)) {
    delete md;
    return nullptr;
  }

  auto led_addr = (volatile uint32_t*)(virtual_base_+((ALT_LWFPGALVS_OFST + LED_PIO_BASE) & HW_REGS_MASK));
  if (!ModuleInfo(md).inputs().empty()) {
    const auto in = *ModuleInfo(md).inputs().begin();
    const auto id = to_vid(in);
    delete md;
    return new De10Led(interface, id, led_addr);
  } else {
    delete md;
    return new De10Led(interface, nullid(), led_addr);
  }
}

De10Pad* De10Compiler::compile_pad(Interface* interface, ModuleDeclaration* md) {
  if (virtual_base_ == MAP_FAILED || !check_io(md, 0, 4)) {
    delete md;
    return nullptr;
  }

  auto pad_addr = (volatile uint32_t*)(virtual_base_+((ALT_LWFPGALVS_OFST + PAD_PIO_BASE) & HW_REGS_MASK));
  const auto out = *ModuleInfo(md).outputs().begin();
  const auto id = to_vid(out);
  const auto w = Evaluate().get_width(out);
  delete md;

  return new De10Pad(interface, id, w, pad_addr);
}

De10Logic* De10Compiler::compile_logic(Interface* interface, ModuleDeclaration* md) {
  ModuleInfo info(md);

  // Create a new core with address identity based on module id
  auto addr = (volatile uint32_t*)(virtual_base_+((ALT_LWFPGALVS_OFST + LOG_PIO_BASE) & HW_REGS_MASK) + 4*to_mid(md->get_id()));
  auto de = new De10Logic(interface, md, addr);

  // Register inputs, state, and outputs
  for (auto i : info.inputs()) {
    de->set_input(i, to_vid(i));
  }
  for (auto s : info.stateful()) {
    de->set_state(s, to_vid(s));
  }
  for (auto o : info.outputs()) {
    de->set_output(o, to_vid(o));
  }

  // Blocking request to quartus client to update fpga.
  const auto mid = to_mid(md->get_id());
  if (!qc_->refresh(mid, Boxer().box(mid, md, de))) {
    delete de;
    return nullptr;
  }
  return de;
}

} // namespace cascade
