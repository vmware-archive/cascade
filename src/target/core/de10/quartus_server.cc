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

QuartusServer::QuartusServer() : Asynchronous(), buf_(256), worker_(this) { 
  path("");
  usb("");
  port(9900);
  sock_ = nullptr;
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

bool QuartusServer::check() const {
  // Return false if we can't locate any of the necessary quartus components
  if (System::execute("ls " + path_ + "/sopc_builder/bin/qsys-generate > /dev/null") != 0) {
    return false;
  }
  if (System::execute("ls " + path_ + "/bin/quartus_map > /dev/null") != 0) {
    return false;
  }
  if (System::execute("ls " + path_ + "/bin/quartus_fit > /dev/null") != 0) {
    return false;
  }
  if (System::execute("ls " + path_ + "/bin/quartus_asm > /dev/null") != 0) {
    return false;
  }
  if (System::execute("ls " + path_ + "/bin/quartus_pgm > /dev/null") != 0) {
    return false;
  }
  return true;
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

    // Set workers done signal, kill any existing quartus jobs,
    // and wait for the worker to return control here.
    worker_.request_stop();
    System::execute("killall java > /dev/null 2>&1");
    System::execute("killall quartus_map > /dev/null 2>&1");
    System::execute("killall quartus_fit > /dev/null 2>&1");
    System::execute("killall quartus_asm > /dev/null 2>&1");
    worker_.wait_for_stop();
    
    // Create a new socket for this request and rerun the worker.
    // Note that we're not waiting for finish here. Best case, it 
    // finishes, worst case, we kill it the next time through this
    // loop in response to the next incoming request.
    const auto fd = ::accept(sock.descriptor(), nullptr, nullptr);
    if (sock_ != nullptr) {
      delete sock_;
    }
    sock_ = new Socket(fd);
    worker_.run();
  }
}

QuartusServer::Worker::Worker(QuartusServer* qs) { 
  qs_ = qs;
}

void QuartusServer::Worker::run_logic() {
  uint32_t size = 0;
  qs_->sock_->recv(size);
  qs_->buf_.reserve(size);
  qs_->buf_.resize(0);
  qs_->sock_->recv(qs_->buf_.data(), size);


  ofstream ofs(System::src_root() + "/src/target/core/de10/fpga/ip/program_logic.v");
  ofs.write(qs_->buf_.data(), size);
  ofs << endl;
  ofs.close();

  // A message of length 1 signals that no compilation is necessary.
  if (size == 1) {
    return qs_->sock_->send(true);
  }

  // Compile everything.
  if (stop_requested() || System::execute(qs_->path_ + "/sopc_builder/bin/qsys-generate " + System::src_root() + "/src/target/core/de10/fpga/soc_system.qsys --synthesis=VERILOG") != 0) {
    return qs_->sock_->send(false);
  } 
  if (stop_requested() || System::execute(qs_->path_ + "/bin/quartus_map " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return qs_->sock_->send(false);
  } 
  if (stop_requested() || System::execute(qs_->path_ + "/bin/quartus_fit " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return qs_->sock_->send(false);
  } 
  if (stop_requested() || System::execute(qs_->path_ + "/bin/quartus_asm " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return qs_->sock_->send(false);
  } 
  if (System::execute(qs_->path_ + "/bin/quartus_pgm -c \"DE-SoC " + qs_->usb_ + "\" --mode JTAG -o \"P;" + System::src_root() + "/src/target/core/de10/fpga/output_files/DE10_NANO_SoC_GHRD.sof@2\"") != 0) {
    return qs_->sock_->send(false);
  }
  qs_->sock_->send(true);
}

} // namespace cascade
