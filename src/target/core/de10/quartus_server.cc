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

#include "target/core/de10/quartus_server.h"

#include <fstream>
#include <sstream>
#include "common/sockserver.h"
#include "common/sockstream.h"
#include "common/system.h"

using namespace std;

namespace cascade {

QuartusServer::QuartusServer() : Thread() { 
  set_cache_path("/tmp/quartus_cache/");
  set_quartus_path("");
  set_port(9900);
  set_usb("");
}

QuartusServer& QuartusServer::set_cache_path(const string& path) {
  cache_path_ = path;
  return *this;
}

QuartusServer& QuartusServer::set_quartus_path(const string& path) {
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

bool QuartusServer::error() const {
  // Return true if we can't locate any of the necessary quartus components
  if (System::execute("ls " + quartus_path_ + "/sopc_builder/bin/qsys-generate > /dev/null") != 0) {
    return true;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_map > /dev/null") != 0) {
    return true;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_fit > /dev/null") != 0) {
    return true;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_asm > /dev/null") != 0) {
    return true;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_pgm > /dev/null") != 0) {
    return true;
  }
  return false;
}

void QuartusServer::run_logic() {
  // Initialize thread pool and comilation cache
  init_pool();
  init_cache();

  // Return immediately if we can't create a sockserver
  sockserver server(port_, 8);
  if (server.error()) {
    pool_.stop_now();
    return;
  }

  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(server.descriptor(), &master_set);

  fd_set read_set;
  FD_ZERO(&read_set);

  struct timeval timeout = {1, 0};

  while (!stop_requested()) {
    read_set = master_set;
    select(server.descriptor()+1, &read_set, nullptr, nullptr, &timeout);
    if (!FD_ISSET(server.descriptor(), &read_set)) {
      continue;
    }

    auto* sock = server.accept();
    const auto rpc = static_cast<QuartusServer::Rpc>(sock->get());
    
    // Easy Case: Kill all quartus tasks in this thread
    if (rpc == Rpc::KILL_ALL) {
      kill_all();
      sock->put(static_cast<uint8_t>(Rpc::OKAY));
      sock->flush();
      delete sock;
    } 
    // Hard Case: Kill all quartus tasks in this thread *and* create a new
    // thread to recompile everything, and then possibly reprogram the device.
    else if (rpc == Rpc::COMPILE) {
      pool_.insert([this, sock]{
        string text = "";
        getline(*sock, text, '\0'); 

        kill_all();
        ofstream ofs(System::src_root() + "/src/target/core/de10/fpga/ip/program_logic.v");
        ofs << text << endl;
        ofs.flush();

        sock->put(static_cast<uint8_t>(Rpc::OKAY));
        sock->flush();
        const auto res = compile(text);
        sock->put(static_cast<uint8_t>(res ? Rpc::OKAY : Rpc::ERROR));
        sock->flush();

        if (res) {
          sock->get();
          reprogram(text);
          sock->put(static_cast<uint8_t>(Rpc::OKAY));
          sock->flush();    
        }
        delete sock;
      });
    }
    // Unrecognized RPC
    else {
      assert(false);
      delete sock;
    }
  }

  // Stop the thread pool
  pool_.stop_now();
}

void QuartusServer::init_pool() {
  pool_.stop_now();
  pool_.set_num_threads(4);
  pool_.run();
}

void QuartusServer::init_cache() {
  // Create the cache if it doesn't already exist
  System::execute("mkdir -p " + cache_path_);
  System::execute("touch " + cache_path_ + "/index.txt");

  // Read cache into memory
  ifstream ifs(cache_path_ + "/index.txt");
  cache_.clear();
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

void QuartusServer::kill_all() {
  // Note that we do not kill quartus_pgm.  It runs quickly and an
  // inconsistently programmed fpga is a nightmware we don't want to consider.
  System::execute("killall java > /dev/null 2>&1");
  System::execute("killall quartus_map > /dev/null 2>&1");
  System::execute("killall quartus_fit > /dev/null 2>&1");
  System::execute("killall quartus_asm > /dev/null 2>&1");
}

bool QuartusServer::compile(const std::string& text) {
  // Nothing to do if this code is already in the cache. Otherwise, 
  { lock_guard<mutex> lg(lock_);
    if (cache_.find(text) != cache_.end()) {
      return true;
    }
  }
  // Okay... so this method should really be atomic, but aside from protecting
  // access to the in-memory cache, the only thing we're really worried about
  // is a race condition on two threads writing out a bitstream at the same
  // time. The only way that could happen is if the second thread came to life
  // and invoked kill-all exactly after this thread finished its called to asm,
  // and then a perversely unfair scheduler gave that thread enough cpu time
  // that it made it all the way through compilation and overwrote the
  // bitstream before this thread came back online. If that ever happens, I'll
  // eat my hat.
  if (System::execute(quartus_path_ + "/sopc_builder/bin/qsys-generate " + System::src_root() + "/src/target/core/de10/fpga/soc_system.qsys --synthesis=VERILOG") != 0) {
    return false;
  } 
  if (System::execute(quartus_path_ + "/bin/quartus_map " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return false;
  } 
  if (System::execute(quartus_path_ + "/bin/quartus_fit " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return false;
  } 
  if (System::execute(quartus_path_ + "/bin/quartus_asm " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return false;
  }
  // Update the cache
  { lock_guard<mutex> lg(lock_);
    stringstream ss;
    ss << "bitstream_" << cache_.size() << ".sof";
    const auto file = ss.str();
    System::execute("cp " + System::src_root() + "/src/target/core/de10/fpga/output_files/DE10_NANO_SoC_GHRD.sof " + cache_path_ + "/" + file);

    ofstream ofs(cache_path_ + "/index.txt", ios::app);
    ofs << text << '\0' << file << '\0';
    ofs.flush();
  }
  return true;
}

void QuartusServer::reprogram(const std::string& text) {
  string path = "";
  { lock_guard<mutex> lg(lock_);
    const auto itr = cache_.find(text);
    assert(itr != cache_.end());
    path = itr->second;
  }
  System::execute(quartus_path_ + "/bin/quartus_pgm -c \"DE-SoC " + usb_ + "\" --mode JTAG -o \"P;" + cache_path_ + "/" + path + "@2\"");
}

} // namespace cascade
