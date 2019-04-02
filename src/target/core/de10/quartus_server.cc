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

#include "src/target/core/de10/quartus_server.h"

#include <fstream>
#include <sstream>
#include "src/base/stream/sockstream.h"
#include "src/base/system/system.h"

using namespace std;

namespace cascade {

QuartusServer::QuartusServer() : Asynchronous(), worker_(this) { 
  set_cache("/tmp/quartus_cache/");
  set_path("");
  set_port(9900);
  set_usb("");
  sock_ = nullptr;
}

QuartusServer& QuartusServer::set_cache(const string& path) {
  cache_path_ = path;
  return *this;
}

QuartusServer& QuartusServer::set_path(const string& path) {
  quartus_path_ = path;
  return *this;
}

QuartusServer& QuartusServer::set_port(uint32_t port) {
  port_ = port;
  return *this;
}

QuartusServer& QuartusServer::set_usb(const string& usb) {
  usb_ = usb;
  return *this;
}

bool QuartusServer::check() const {
  // Return false if we can't locate any of the necessary quartus components
  if (System::execute("ls " + quartus_path_ + "/sopc_builder/bin/qsys-generate > /dev/null") != 0) {
    return false;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_map > /dev/null") != 0) {
    return false;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_fit > /dev/null") != 0) {
    return false;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_asm > /dev/null") != 0) {
    return false;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_pgm > /dev/null") != 0) {
    return false;
  }
  return true;
}

QuartusServer::Worker::Worker(QuartusServer* qs) { 
  qs_ = qs;
}

void QuartusServer::Worker::run_logic() {
  string text;
  getline(*qs_->sock_, text, '\0');

  // A message of length 1 signals that no compilation is necessary.
  if (text.length() == 1) {
    qs_->sock_->put(0);
    qs_->sock_->flush();
    return;
  }

  // Check whether this program is in the cache
  auto itr = qs_->cache_.find(text);

  // If it isn't, we need to compile it
  if (itr == qs_->cache_.end()) {
    ofstream ofs1(System::src_root() + "/src/target/core/de10/fpga/ip/program_logic.v");
    ofs1 << text << endl;
    ofs1.flush();
    ofs1.close();

    if (stop_requested() || System::execute(qs_->quartus_path_ + "/sopc_builder/bin/qsys-generate " + System::src_root() + "/src/target/core/de10/fpga/soc_system.qsys --synthesis=VERILOG") != 0) {
      qs_->sock_->put(0);
      qs_->sock_->flush();
      return;
    } 
    if (stop_requested() || System::execute(qs_->quartus_path_ + "/bin/quartus_map " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
      qs_->sock_->put(0);
      qs_->sock_->flush();
      return;
    } 
    if (stop_requested() || System::execute(qs_->quartus_path_ + "/bin/quartus_fit " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
      qs_->sock_->put(0);
      qs_->sock_->flush();
      return;
    } 
    if (stop_requested() || System::execute(qs_->quartus_path_ + "/bin/quartus_asm " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
      qs_->sock_->put(0);
      qs_->sock_->flush();
      return;
    }

    stringstream ss;
    ss << "bitstream_" << qs_->cache_.size() << ".sof";
    const auto file = ss.str();
    System::execute("cp " + System::src_root() + "/src/target/core/de10/fpga/output_files/DE10_NANO_SoC_GHRD.sof " + qs_->cache_path_ + "/" + file);

    itr = qs_->cache_.insert(make_pair(text, file)).first;

    ofstream ofs2(qs_->cache_path_ + "/index.txt", ios::app);
    ofs2 << text << '\0' << file << '\0';
    ofs2.flush();
    ofs2.close();
  } 

  // Now that it's definitely in the cache, use this bitstream to program the device
  if (System::execute(qs_->quartus_path_ + "/bin/quartus_pgm -c \"DE-SoC " + qs_->usb_ + "\" --mode JTAG -o \"P;" + qs_->cache_path_ + "/" + itr->second + "@2\"") != 0) {
    qs_->sock_->put(0);
  } else {
    qs_->sock_->put(1);
  }
  qs_->sock_->flush();
}

void QuartusServer::init_cache() {
  // Create the cache if it doesn't already exist
  System::execute("mkdir -p " + cache_path_);
  System::execute("touch " + cache_path_ + "/index.txt");

  // Read cache into memory
  ifstream ifs(cache_path_ + "/index.txt");
  while (true) {
    string text;
    getline(ifs, text, '\0');
    if (ifs.eof()) {
      break;
    } 

    string path;
    getline(ifs, path, '\0');
    cache_.insert(make_pair(text, path));
  } 
}

void QuartusServer::run_logic() {
  init_cache();

  sockserver server(port_, 8);
  if (server.error()) {
    return;
  }

  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(server.descriptor(), &master_set);

  fd_set read_set;
  FD_ZERO(&read_set);

  struct timeval timeout = {0, 1000};

  while (!stop_requested()) {
    read_set = master_set;
    select(server.descriptor()+1, &read_set, nullptr, nullptr, &timeout);
    if (!FD_ISSET(server.descriptor(), &read_set)) {
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
    if (sock_ != nullptr) {
      delete sock_;
    }
    sock_ = server.accept();
    worker_.run();
  }
}

void QuartusServer::stop_logic() {
  cache_.clear();
}

} // namespace cascade
