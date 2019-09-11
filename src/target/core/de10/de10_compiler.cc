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

#include "target/core/de10/de10_compiler.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <map>
#include <unistd.h>
#include "common/sockstream.h"
#include "target/compiler.h"
#include "target/core/de10/de10_rewrite.h"
#include "target/core/de10/program_boxer.h"
#include "target/core/de10/quartus_server.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/ast/ast.h"

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

  slots_.resize(4, {0, State::FREE, ""});

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

void De10Compiler::release(size_t slot) {
  // Return this slot to the pool if necessary. This method should only be
  // invoked on successfully compiled cores, which means we don't have to worry
  // about transfering compilation ownership or invoking a killall.
  lock_guard<mutex> lg(lock_);
  assert(slots_[slot].state = State::CURRENT);
  slots_[slot].state = State::FREE;
  cv_.notify_all();
}

void De10Compiler::stop_compile(Engine::Id id) {
  // Nothing to do if this id isn't in use
  lock_guard<mutex> lg(lock_);
  if (!id_in_use(id)) {
    return;
  }

  // Free any slot with this id which is in the compiling or waiting state.
  auto need_new_owner = false;
  for (auto& s : slots_) {
    if (s.id == id) {
      switch (s.state) {
        case State::COMPILING:
          need_new_owner = true;
          s.state = State::STOPPED;
          // fallthrough
        case State::WAITING:
          s.state = State::STOPPED;
          break;
        default:
          break;
      } 
    }
  }
  // If we need a new compilation lead, find a waiting slot and promote it.
  // Note that there might not be any more waiting slots. That's fine.
  if (need_new_owner) {
    for (auto& s : slots_) {
      if (s.state == State::WAITING) {
        s.state = State::COMPILING;
        break;
      }
    }
  }

  sockstream sock(host_.c_str(), port_);
  assert(!sock.error());
  kill_all(&sock);
  cv_.notify_all();
}

De10Gpio* De10Compiler::compile_gpio(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  (void) id;

  if (virtual_base_ == MAP_FAILED) {
    get_compiler()->error("De10 gpio compilation failed due to inability to memory map device");
    delete md;
    return nullptr;
  }
  if (!check_io(md, 8, 8)) {
    get_compiler()->error("Unable to compile a de10 gpio with more than 8 outputs");
    delete md;
    return nullptr;
  }

  volatile uint8_t* led_addr = virtual_base_+((ALT_LWFPGALVS_OFST + GPIO_PIO_BASE) & HW_REGS_MASK);
  if (!ModuleInfo(md).inputs().empty()) {
    const auto* in = *ModuleInfo(md).inputs().begin();
    const auto iid = to_vid(in);
    delete md;
    return new De10Gpio(interface, iid, led_addr);
  } else {
    delete md;
    return new De10Gpio(interface, nullid(), led_addr);
  }
}

De10Led* De10Compiler::compile_led(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  (void) id;

  if (virtual_base_ == MAP_FAILED) {
    get_compiler()->error("De10 led compilation failed due to inability to memory map device");
    delete md;
    return nullptr;
  }
  if (!check_io(md, 8, 8)) {
    get_compiler()->error("Unable to compile a de10 led with more than 8 outputs");
    delete md;
    return nullptr;
  }

  volatile uint8_t* led_addr = virtual_base_+((ALT_LWFPGALVS_OFST + LED_PIO_BASE) & HW_REGS_MASK);
  if (!ModuleInfo(md).inputs().empty()) {
    const auto* in = *ModuleInfo(md).inputs().begin();
    const auto iid = to_vid(in);
    delete md;
    return new De10Led(interface, iid, led_addr);
  } else {
    delete md;
    return new De10Led(interface, nullid(), led_addr);
  }
}

De10Logic* De10Compiler::compile_logic(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  unique_lock<mutex> lg(lock_);

  // Find a new slot and generate code for this module. If either step fails,
  // return nullptr. Otherwise, advance the sequence counter and compile.
  const auto slot = get_free_slot();
  if (slot == -1) {
    get_compiler()->error("No remaining slots available on de10 fabric");
    return nullptr;
  }

  // Create a new core with address identity based on module id
  volatile uint8_t* addr = virtual_base_+((ALT_LWFPGALVS_OFST + LOG_PIO_BASE) & HW_REGS_MASK) + (slot << 14);
  auto* de = new De10Logic(interface, md, addr, this);

  // Register inputs, state, and outputs. Invoke these methods
  // lexicographically to ensure a deterministic variable table ordering. The
  // final invocation of index_tasks is lexicographic by construction, as
  // it's based on a recursive descent of the AST.
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
  de->index_tasks();

  // Check table and index sizes. If this program uses too much state, we won't
  // be able to uniquely name its elements using our current addressing scheme.
  if (de->get_table().size() >= 0x1000) {
    get_compiler()->error("Unable to compile a module with more than 4096 entries in variable table");
    delete de;
    return nullptr;
  }

  // Downgrade any compilation slots to waiting slots, and stop any slots that are
  // working on this id.
  for (auto& s : slots_) {
    if (s.state == State::COMPILING) {
      s.state = State::WAITING;
    }
    if ((s.id == id) && (s.state == State::WAITING)) {
      s.state = State::STOPPED;
    }
  }

  slots_[slot].id = id;
  slots_[slot].state = State::COMPILING;
  slots_[slot].text = De10Rewrite().run(md, de, slot);

  while (true) {
    switch (slots_[slot].state) {
      case State::COMPILING: {
        sockstream sock(host_.c_str(), port_);
        if (sock.error()) {
          slots_[slot].state = State::FREE;
          get_compiler()->error("Unable to connect to quartus compilation server");
          delete de;
          return nullptr;
        }
        compile(&sock);
        lg.unlock();
        const auto res = block_on_compile(&sock);
        lg.lock();
        if (res) {
          reprogram(&sock);
        }
        break;
      }
      case State::WAITING:
        cv_.wait(lg);
        break;
      case State::STOPPED:
        slots_[slot].state = State::FREE;
        delete de;
        return nullptr;
      case State::CURRENT:
        de->set_slot(slot);
        return de;
      default:
        // Control should never reach here
        assert(false);
        break;
    }
  }
}

De10Pad* De10Compiler::compile_pad(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  (void) id;

  if (virtual_base_ == MAP_FAILED) {
    get_compiler()->error("De10 pad compilation failed due to inability to memory map device");
    delete md;
    return nullptr;
  }
  if (!check_io(md, 0, 4)) {
    get_compiler()->error("Unable to compile a de10 pad with more than 4 inputs");
    delete md;
    return nullptr;
  }

  volatile uint8_t* pad_addr = virtual_base_+((ALT_LWFPGALVS_OFST + PAD_PIO_BASE) & HW_REGS_MASK);
  const auto* out = *ModuleInfo(md).outputs().begin();
  const auto oid = to_vid(out);
  const auto w = Evaluate().get_width(out);
  delete md;

  return new De10Pad(interface, oid, w, pad_addr);
}

bool De10Compiler::id_in_use(Engine::Id id) const {
  for (const auto& s : slots_) {
    if ((s.id == id) && (s.state != State::FREE)) {
      return true;
    }
  }
  return false;
}

int De10Compiler::get_free_slot() const {
  for (size_t i = 0, ie = slots_.size(); i < ie; ++i) {
    if (slots_[i].state == State::FREE) {
      return i;
    }
  }
  return -1;
}

void De10Compiler::compile(sockstream* sock) {
  // Send a compile request. We'll block here until there are no more compile threads
  // running.
  sock->put(static_cast<uint8_t>(QuartusServer::Rpc::COMPILE));
  sock->flush();
  sock->get();

  // Send code to the quartus server, we won't hear back from this socket until 
  // compilation is finished or it was aborted.
  ProgramBoxer pb;
  for (size_t i = 0, ie = slots_.size(); i < ie; ++i) {
    if (slots_[i].state != State::FREE) {
      pb.push(i, slots_[i].text);
    }
  }
  const auto text = pb.get();
  sock->write(text.c_str(), text.length());
  sock->put('\0');
  sock->flush();
}

bool De10Compiler::block_on_compile(sockstream* sock) {
  return (static_cast<QuartusServer::Rpc>(sock->get()) == QuartusServer::Rpc::OKAY);
}

void De10Compiler::reprogram(sockstream* sock) {
  get_compiler()->schedule_state_safe_interrupt([this, sock]{
    sock->put(static_cast<uint8_t>(QuartusServer::Rpc::REPROGRAM));
    sock->flush();
    sock->get();
    for (auto& s : slots_) {
      if ((s.state == State::COMPILING) || (s.state == State::WAITING)) {
        s.state = State::CURRENT;
      }     
    }
    cv_.notify_all();
  });
}

void De10Compiler::kill_all(sockstream* sock) {
  sock->put(static_cast<uint8_t>(QuartusServer::Rpc::KILL_ALL));
  sock->flush();
  sock->get();
}

} // namespace cascade
