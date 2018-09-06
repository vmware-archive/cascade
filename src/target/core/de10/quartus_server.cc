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

#include "src/target/core/de10/quartus_server.h"

#include <fstream>
#include "src/base/socket/socket.h"
#include "src/base/system/system.h"

using namespace std;

namespace cascade {

QuartusServer::QuartusServer() : Asynchronous(), buf_(256) { 
  path("");
  usb("");
  port(9900);
  reuse_sof(false);
}

QuartusServer& QuartusServer::path(const string& path) {
  path_ = path;
  return *this;
}

QuartusServer& QuartusServer::usb(const string& usb) {
  usb_ = usb;
  return *this;
}

QuartusServer& QuartusServer::port(uint32_t port) {
  port_ = port;
  return *this;
}

QuartusServer& QuartusServer::reuse_sof(bool rs) {
  reuse_sof_ = rs;
  return *this;
}

void QuartusServer::run_logic() {
  Socket sock;
  sock.listen(port_, 8);
  if (sock.error()) {
    return;
  }

  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(sock.descriptor(), &master_set);

  fd_set read_set;
  FD_ZERO(&read_set);

  struct timeval timeout = {0, 1000};

  while (!stop_requested()) {
    read_set = master_set;
    select(sock.descriptor()+1, &read_set, nullptr, nullptr, &timeout);
    if (!FD_ISSET(sock.descriptor(), &read_set)) {
      continue;
    }

    const auto fd = ::accept(sock.descriptor(), nullptr, nullptr);
    Socket client(fd);

    uint32_t size = 0;
    client.recv(size);
    buf_.reserve(size);
    buf_.resize(0);
    client.recv(buf_.data(), size);

    ofstream ofs(System::src_root() + "/src/target/core/de10/fpga/ip/program_logic.v");
    ofs.write(buf_.data(), size);
    ofs << endl;
    ofs.close();

    if (!reuse_sof_) {
      if (System::execute(path_ + "/sopc_builder/bin/qsys-generate " + System::src_root() + "/src/target/core/de10/fpga/soc_system.qsys --synthesis=VERILOG") != 0) {
        cout << "Qsys failed!" << endl;
        client.send(true);
        continue;
      } 
      if (System::execute(path_ + "/bin/quartus_map " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
        cout << "Synthesis failed!" << endl;
        client.send(true);
        continue;
      } 
      if (System::execute(path_ + "/bin/quartus_fit " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
        cout << "Optimize failed!" << endl;
        client.send(true);
        continue;
      } 
      if (System::execute(path_ + "/bin/quartus_asm " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
        cout << "ASM failed!" << endl;
        client.send(true);
        continue;
      } 
    }    
    if (System::execute(path_ + "/bin/quartus_pgm -c \"DE-SoC " + usb_ + "\" --mode JTAG -o \"P;" + System::src_root() + "/src/target/core/de10/fpga/output_files/DE10_NANO_SoC_GHRD.sof@2\"") != 0) {
      cout << "PROGR failed!" << endl;
      client.send(true);
      continue;
    } 
    client.send(false);
  }
}

} // namespace cascade
