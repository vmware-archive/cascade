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

#include "target/core/avmm/de10/de10_compiler.h"

#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include "common/sockstream.h"
#include "common/system.h"
#include "target/core/avmm/de10/quartus_server.h"

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

De10Compiler::De10Compiler() : AvmmCompiler<2,12,uint16_t,uint32_t>() { 
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

De10Logic* De10Compiler::build(Interface* interface, ModuleDeclaration* md, size_t slot) {
  volatile uint8_t* addr = virtual_base_+((ALT_LWFPGALVS_OFST + LOG_PIO_BASE) & HW_REGS_MASK) + (slot << 14);
  return new De10Logic(interface, md, slot, addr);
}

bool De10Compiler::compile(const string& text, mutex& lock) {
  sockstream sock(host_.c_str(), port_);
  assert(!sock.error());

  compile(&sock, text);
  lock.unlock();
  const auto res = block_on_compile(&sock);
  lock.lock();

  if (res) {
    reprogram(&sock);
    return true;
  } else {
    return false;
  }
}

void De10Compiler::stop_compile() {
  sockstream sock(host_.c_str(), port_);
  assert(!sock.error());

  sock.put(static_cast<uint8_t>(QuartusServer::Rpc::KILL_ALL));
  sock.flush();
  sock.get();
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

void De10Compiler::compile(sockstream* sock, const string& text) {
  // Send a compile request. We'll block here until there are no more compile threads
  // running.
  sock->put(static_cast<uint8_t>(QuartusServer::Rpc::COMPILE));
  sock->flush();
  sock->get();

  // Send code to the quartus server, we won't hear back from this socket until
  // compilation is finished or it was aborted.
  sock->write(text.c_str(), text.length());
  sock->put('\0');
  sock->flush();
}

bool De10Compiler::block_on_compile(sockstream* sock) {
  return (static_cast<QuartusServer::Rpc>(sock->get()) == QuartusServer::Rpc::OKAY);
}

void De10Compiler::reprogram(sockstream* sock) {
  get_compiler()->schedule_state_safe_interrupt([this, sock]{
    // Send REPROGRAM request
    sock->put(static_cast<uint8_t>(QuartusServer::Rpc::REPROGRAM));
    sock->flush();

    // We'll receive a 32-bit value indicating number of bytes and then the contents
    // of the rbf file that was generated by the quartus server
    uint32_t len = 0;
    sock->read(reinterpret_cast<char*>(&len), sizeof(len));
    vector<char> rbf;
    rbf.resize(len);
    sock->read(reinterpret_cast<char*>(rbf.data()), len);

    // Form a path to the temporary location we'll be storing the rbf file
    // and the de10 config fool
    const auto rbf_path = System::src_root() + "/src/target/core/avmm/de10/device/DE10_NANO_SoC_GHRD.rbf";
    const auto de10_config = System::src_root() + "/build/tools/de10_config";

    // Copy the rbf file to this location
    ofstream ofs(rbf_path);
    ofs.write(rbf.data(), len);
    ofs.close();

    // Run the reprogram tool
    System::execute(de10_config + " program " + rbf_path);

    // Send a byte to acknowledge that we're done
    sock->put(0);
    sock->flush();
  });
}

} // namespace cascade
