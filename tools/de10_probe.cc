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

#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include "ext/cl/include/cl.h"
#include "src/target/core/de10/quartus_server.h"

using namespace cascade;
using namespace cl;
using namespace std;

constexpr auto HW_REGS_BASE = 0xfc000000u;
constexpr auto HW_REGS_SPAN = 0x04000000u;
constexpr auto HW_REGS_MASK = HW_REGS_SPAN - 1;
constexpr auto ALT_LWFPGALVS_OFST = 0xff200000u;
constexpr auto LED_PIO_BASE = 0x00003000u;
constexpr auto PAD_PIO_BASE = 0x00004000u;
constexpr auto GPIO_PIO_BASE = 0x00005000u;
constexpr auto LOG_PIO_BASE = 0x00040000u;

#define DE10_READ(x)    (*((volatile uint32_t*)(x)))
#define DE10_WRITE(x,y) (*((volatile uint32_t*)(x)) = (y))
#define MANGLE(addr, idx) ((volatile uint8_t*)(((idx) << 2) + (size_t)addr))

__attribute__((unused)) auto& g1 = Group::create("Configuration Options");
auto& mid = StrArg<QuartusServer::Id>::create("--mid")
  .usage("<MId>")
  .description("Module ID")
  .initial(0);
auto& read_cmd = StrArg<string>::create("-r")
  .usage("<VId>")
  .description("Variable ID")
  .initial("-1");
auto& write_cmd = StrArg<string>::create("-w")
  .usage("<VId>:<Val>")
  .description("Variable ID and 32-bit value to write")
  .initial("-1:0");

int done(int code, int fd, volatile uint8_t* vbase) {
  if (fd != -1) {
    close(fd);
  }
  if ((vbase != nullptr) && (vbase != MAP_FAILED)) {
    munmap(reinterpret_cast<void*>(const_cast<uint8_t*>(vbase)), HW_REGS_SPAN);
  }
  return code;
}

int main(int argc, char** argv) {
  Simple::read(argc, argv);

  int fd = -1;
  volatile uint8_t* vbase= nullptr;

  fd = open("/dev/mem", (O_RDWR | O_SYNC));
  if (fd == -1) {
    return done(1, fd, vbase);
  } 
  vbase = reinterpret_cast<volatile uint8_t*>(mmap(nullptr, HW_REGS_SPAN, (PROT_READ|PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE));
  if (vbase == MAP_FAILED) {
    return done(2, fd, vbase);
  }
  volatile uint8_t* addr = vbase+((ALT_LWFPGALVS_OFST + LOG_PIO_BASE) & HW_REGS_MASK) + (mid.value() << 14);

  stringstream ss1(read_cmd.value());
  int r;
  ss1 >> r;
  
  if (r >= 0) {
    cout << "VID[" << r << "] = " << DE10_READ(MANGLE(addr, r)) << endl;
    return done(0, fd, vbase);
  }

  stringstream ss2(write_cmd.value());
  string s;
  getline(ss2, s, ':');

  stringstream ss3(s);
  int w;
  ss3 >> w;
  uint32_t val;
  ss2 >> val;

  if (w >= 0) {
    cout << "VID[" << w << "] = " << val << endl;
    return done(0, fd, vbase);
  }

  cout << "No commands provided" << endl;
  return done(0, fd, vbase);
}
