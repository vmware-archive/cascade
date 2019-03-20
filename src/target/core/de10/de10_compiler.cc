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
#include <unistd.h>
#include "src/base/stream/sockstream.h"
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

  curr_seq_ = 1;
  next_seq_ = 2;
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

void De10Compiler::abort() {
  sockstream sock(host_.c_str(), port_);;

  // Set the current sequence number to zero to indicate it's time to shutdown 
  { lock_guard<mutex> lg(lock_);
    curr_seq_ = 0;
    if (!sock.error()) {
      sock.put(1);
      sock.put('\0');
      sock.flush();
    }
  }
  // Notify any outstanding threads to force their shutdown.
  unique_lock<mutex> ul(lock_);
  cv_.notify_all();
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
  ModuleInfo info(md);

  // Check range on mid 
  if (to_mid(md->get_id()) >= 4) {
    error("Unable to compile a module with internal id greater than 3!");
    delete md;
    return nullptr;
  }

  // Create a new core with address identity based on module id
  volatile uint8_t* addr = virtual_base_+((ALT_LWFPGALVS_OFST + LOG_PIO_BASE) & HW_REGS_MASK) + to_mid(md->get_id());
  const auto mid = to_mid(md->get_id());
  auto* de = new De10Logic(interface, md, addr);

  // Register inputs, state, and outputs
  for (auto* i : info.inputs()) {
    de->set_input(i, to_vid(i));
  }
  for (auto* s : info.stateful()) {
    de->set_state(s, to_vid(s));
  }
  for (auto* o : info.outputs()) {
    de->set_output(o, to_vid(o));
  }

  // Check size of variable table
  if (de->open_loop_idx() >= 0x4000) {
    error("Unable to compile module with more than 0x4000 entries in variable table");
    delete de;
    return nullptr;
  }

  // Connect to quartus server
  sockstream sock(host_.c_str(), port_);
  if (sock.error()) {
    error("Unable to connect to quartus compilation server");
    delete de;
    return nullptr;
  }

  // Critical Section #1: 
  size_t my_seq = 0;
  { lock_guard<mutex> lg(lock_); 
    // Try pushing this module into the repository. If the push fails, this
    // code is out of date and we can abort the compilation now.
    if (!pbox_.push(mid, md, de)) {
      delete de;
      return nullptr;
    }
    // The push succeeded, start a new compilation.
    const auto psrc = pbox_.get();
    sock.write(psrc.c_str(), psrc.length());
    sock.put('\0');
    sock.flush();
    // Record which sequence number this module is waiting on
    my_seq = next_seq_++;
    wait_table_[mid] = my_seq;
  }

  // Block on result
  const auto result = (sock.get() == 1);

  // Critical Section #2
  { unique_lock<mutex> ul(lock_);
    // If the request was successfully, no newer results have come back, and we
    // haven't trapped the special sequence number zero, update the current
    // sequence number, and tell everyone to check their exit conditions.
    if (result && (my_seq > curr_seq_) && (curr_seq_ > 0)) {
      curr_seq_ = my_seq; 
    }
    cv_.notify_all();

    // Exit condition loop: 
    for (; true; cv_.wait(ul)) {
      // The special sequence number forces all threads to abort
      if (curr_seq_ == 0) {
        delete de;
        return nullptr;
      }
      // If this thread's entry in the wait table has been replaced, it can
      // abort as well
      if (my_seq < wait_table_[mid]) {
        delete de;
        return nullptr;
      }
      // Otherwise, if we're waiting on a sequence number less than what's been
      // commited, we can return.
      if (wait_table_[mid] <= curr_seq_) {
        return de;
      } 
    } 
  }
}

} // namespace cascade
