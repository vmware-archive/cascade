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

#include "src/target/core/de10/de10_compiler.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <map>
#include <unistd.h>
#include "src/base/stream/sockstream.h"
#include "src/target/core/de10/module_boxer.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

constexpr auto HW_REGS_BASE = 0xfc000000u;
constexpr auto HW_REGS_SPAN = 0x04000000u;
constexpr auto HW_REGS_MASK = HW_REGS_SPAN - 1;
constexpr auto ALT_LWFPGALVS_OFST = 0xff200000u;
constexpr auto LED_PIO_BASE = 0x00003000u;
constexpr auto PAD_PIO_BASE = 0x00004000u;
constexpr auto GPIO_PIO_BASE = 0x00005000u;
constexpr auto LOG_PIO_BASE = 0x00040000u;

De10Compiler::De10Compiler() : CoreCompiler() { 
  fd_ = open("/dev/mem", (O_RDWR | O_SYNC));
  if (fd_ == -1) {
    virtual_base_ = reinterpret_cast<volatile uint8_t*>(MAP_FAILED);
  } else {
    virtual_base_ = reinterpret_cast<volatile uint8_t*>(mmap(nullptr, HW_REGS_SPAN, (PROT_READ|PROT_WRITE), MAP_SHARED, fd_, HW_REGS_BASE));
  }

  set_host("localhost"); 
  set_port(9900);
}

De10Compiler::~De10Compiler() {
  // Close the descriptor, and unmap the memory associated with the fpga
  if (fd_ != -1) {
    close(fd_);
  }
  if (virtual_base_ != MAP_FAILED) {
    munmap(reinterpret_cast<void*>(const_cast<uint8_t*>(virtual_base_)), HW_REGS_SPAN);
  }
}

De10Compiler& De10Compiler::set_host(const string& host) {
  host_ = host;
  return *this;
}

De10Compiler& De10Compiler::set_port(uint32_t port) {
  port_ = port;
  return *this;
}

void De10Compiler::cleanup(QuartusServer::Id id) {
  sockstream sock(host_.c_str(), port_);;
  sock.put(static_cast<uint8_t>(QuartusServer::Rpc::RETURN_SLOT));
  sock.put(static_cast<uint8_t>(id));
  sock.flush();
}

void De10Compiler::abort() {
  sockstream sock(host_.c_str(), port_);;
  sock.put(static_cast<uint8_t>(QuartusServer::Rpc::ABORT));
  sock.flush();
}

De10Gpio* De10Compiler::compile_gpio(Interface* interface, ModuleDeclaration* md) {
  if (virtual_base_ == MAP_FAILED) {
    error("De10 gpio compilation failed due to inability to memory map device");
    delete md;
    return nullptr;
  }
  if (!check_io(md, 8, 8)) {
    error("Unable to compile a de10 gpio with more than 8 outputs");
    delete md;
    return nullptr;
  }

  volatile uint8_t* led_addr = virtual_base_+((ALT_LWFPGALVS_OFST + GPIO_PIO_BASE) & HW_REGS_MASK);
  if (!ModuleInfo(md).inputs().empty()) {
    const auto* in = *ModuleInfo(md).inputs().begin();
    const auto id = to_vid(in);
    delete md;
    return new De10Gpio(interface, id, led_addr);
  } else {
    delete md;
    return new De10Gpio(interface, nullid(), led_addr);
  }
}

De10Led* De10Compiler::compile_led(Interface* interface, ModuleDeclaration* md) {
  if (virtual_base_ == MAP_FAILED) {
    error("De10 led compilation failed due to inability to memory map device");
    delete md;
    return nullptr;
  }
  if (!check_io(md, 8, 8)) {
    error("Unable to compile a de10 led with more than 8 outputs");
    delete md;
    return nullptr;
  }

  volatile uint8_t* led_addr = virtual_base_+((ALT_LWFPGALVS_OFST + LED_PIO_BASE) & HW_REGS_MASK);
  if (!ModuleInfo(md).inputs().empty()) {
    const auto* in = *ModuleInfo(md).inputs().begin();
    const auto id = to_vid(in);
    delete md;
    return new De10Led(interface, id, led_addr);
  } else {
    delete md;
    return new De10Led(interface, nullid(), led_addr);
  }
}

De10Pad* De10Compiler::compile_pad(Interface* interface, ModuleDeclaration* md) {
  if (virtual_base_ == MAP_FAILED) {
    error("De10 pad compilation failed due to inability to memory map device");
    delete md;
    return nullptr;
  }
  if (!check_io(md, 0, 4)) {
    error("Unable to compile a de10 pad with more than 4 inputs");
    delete md;
    return nullptr;
  }

  volatile uint8_t* pad_addr = virtual_base_+((ALT_LWFPGALVS_OFST + PAD_PIO_BASE) & HW_REGS_MASK);
  const auto* out = *ModuleInfo(md).outputs().begin();
  const auto id = to_vid(out);
  const auto w = Evaluate().get_width(out);
  delete md;

  return new De10Pad(interface, id, w, pad_addr);
}

De10Logic* De10Compiler::compile_logic(Interface* interface, ModuleDeclaration* md) {
  // Connect to quartus server
  sockstream sock1(host_.c_str(), port_);
  if (sock1.error()) {
    error("Unable to connect to quartus compilation server");
    return nullptr;
  }
  // Send a slot request. If it comes back negative, the server is full.
  sock1.put(static_cast<uint8_t>(QuartusServer::Rpc::REQUEST_SLOT));
  sock1.flush();
  const auto sid = sock1.get();
  if (sid == static_cast<uint8_t>(-1)) {
    error("No remaining slots available on de10 fabric");
    return nullptr;
  }

  // Create a new core with address identity based on module id
  volatile uint8_t* addr = virtual_base_+((ALT_LWFPGALVS_OFST + LOG_PIO_BASE) & HW_REGS_MASK) + (sid << 14);
  auto* de = new De10Logic(interface, sid, md, addr);

  // Register inputs, state, and outputs. Invoke these methods
  // lexicographically to ensure a deterministic variable table ordering.
  ModuleInfo info(md);
  map<VId, const Identifier*> is;
  for (auto* i : info.inputs()) {
    is.insert(make_pair(to_vid(i), i));
  }
  for (const auto& i : is) {
    de->set_input(i.second, i.first);
  }
  map<VId, const Identifier*> ss;
  for (auto* s : info.stateful()) {
    ss.insert(make_pair(to_vid(s), s));
  }
  for (const auto& s : ss) {
    de->set_state(s.second, s.first);
  }
  map<VId, const Identifier*> os;
  for (auto* o : info.outputs()) {
    os.insert(make_pair(to_vid(o), o));
  }
  for (const auto& o : os) {
    de->set_output(o.second, o.first);
  }
  // Check size of variable table
  if (de->open_loop_idx() >= 0x1000) {
    error("Unable to compile module with more than 4096 entries in variable table");
    delete de;
    return nullptr;
  }

  // Blocking call to compile.  At this point, we don't expect compilations to
  // fail.  A non-zero return value indicates that the compilation was aborted. 
  sockstream sock2(host_.c_str(), port_);
  sock2.put(static_cast<uint8_t>(QuartusServer::Rpc::UPDATE_SLOT));
  sock2.put(static_cast<uint8_t>(sid));
  const auto text = ModuleBoxer().box(md, de);
  sock2.write(text.c_str(), text.length());
  sock2.put('\0');
  sock2.flush();
  if (sock2.get() == 0) {
    return de;
  }

  // If the compilation was aborted, return this slot to the server.
  sockstream sock3(host_.c_str(), port_);
  sock3.put(static_cast<uint8_t>(QuartusServer::Rpc::RETURN_SLOT));
  sock3.put(static_cast<uint8_t>(sid));
  sock3.flush();
  delete de;
  return nullptr;
}

} // namespace cascade
