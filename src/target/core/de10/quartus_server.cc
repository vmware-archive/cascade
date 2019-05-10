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
#include "base/stream/sockstream.h"
#include "base/system/system.h"
#include "target/core/de10/program_boxer.h"

using namespace std;

namespace cascade {

QuartusServer::QuartusServer() : Asynchronous() { 
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

void QuartusServer::init_slots() {
  slots_.resize(4);
  for (size_t i = 0, ie = slots_.size(); i < ie; ++i) {
    slots_[i].first = QuartusServer::State::OPEN;
    slots_[i].second = "";
  }
}

void QuartusServer::init_versioning() {
  version_ = 0;
}

void QuartusServer::request_slot(sockstream* sock) {
  killall();
  ++version_;

  lock_guard<mutex> lg(lock_);
  uint8_t res = -1;
  for (size_t i = 0, ie = slots_.size(); i < ie; ++i) {
    if (slots_[i].first == QuartusServer::State::OPEN) {
      slots_[i].first = QuartusServer::State::CURRENT;
      res = i;
      break;
    }
  }
  sock->put(res);
  sock->flush();
  delete sock;
}

void QuartusServer::update_slot(sockstream* sock) {
  const auto i = static_cast<size_t>(sock->get());
  assert(i < slots_.size());
  string text = "";
  getline(*sock, text, '\0');

  killall();
  ++version_;

  unique_lock<mutex> ul(lock_);
  slots_[i].first = QuartusServer::State::WAITING;
  slots_[i].second = text;
  pool_.insert(new ThreadPool::Job([this]{recompile(version_);}));

  while (slots_[i].first == QuartusServer::State::WAITING) {
    cv_.wait(ul);
  }  
  sock->put((slots_[i].first == QuartusServer::State::CURRENT) ? 0 : 1);
  sock->flush();
  delete sock;
}

void QuartusServer::return_slot(sockstream* sock) {
  const auto i = static_cast<size_t>(sock->get());
  assert(i < slots_.size());

  lock_guard<mutex> lg(lock_);
  slots_[i].first = QuartusServer::State::OPEN;
  slots_[i].second = "";
  sock->put(0);
  sock->flush();
  delete sock;
}

void QuartusServer::abort(sockstream* sock) {
  killall();
  ++version_;

  lock_guard<mutex> lg(lock_);
  for (auto& s : slots_) {
    if (s.first == QuartusServer::State::WAITING) {
      s.first = QuartusServer::State::OPEN;
    }
  }
  cv_.notify_all();

  if (sock != nullptr) {
    sock->put(0);
    sock->flush();
    delete sock;
  }
}

void QuartusServer::killall() {
  // Note that we never kill quartus_pgm.  It runs quickly and an
  // inconsistently programmed fpga is a nightmware we don't want to consider.
  System::execute("killall java > /dev/null 2>&1");
  System::execute("killall quartus_map > /dev/null 2>&1");
  System::execute("killall quartus_fit > /dev/null 2>&1");
  System::execute("killall quartus_asm > /dev/null 2>&1");
}

void QuartusServer::recompile(size_t my_version) {
  // This method takes a potentially very long time to run to completion. It's
  // important that it be atomic.  But it's also important that simultaneous
  // invocations of recompile() be able to preempt it. The way we deal with
  // this is to break this method into pieces and at each step along the way
  // check whether we've been superceded by a new compilation. This isn't
  // *technically* race free. But the timescales here are so long that it
  // should work correctly with any reasonably fair scheduler.

  string text = "";
  auto itr = cache_.end();

  { // Step 1: Generate program text and do a cache lookup, prep the code for
    // compilation if the lookup fails
    lock_guard<mutex> lg(lock_);
    if (my_version < version_) {
      return;
    } 

    ProgramBoxer pb;
    for (size_t i = 0, ie = slots_.size(); i < ie; ++i) {
      if (slots_[i].first != QuartusServer::State::OPEN) {
        pb.push(i, slots_[i].second);
      }
    }
    text = pb.get();
    itr = cache_.find(text);
    
    if (itr == cache_.end()) {
      ofstream ofs(System::src_root() + "/src/target/core/de10/fpga/ip/program_logic.v");
      ofs << text << endl;
      ofs.flush();
    }
  } 
  { // Step 2: qsys
    lock_guard<mutex> lg(lock_);
    if (my_version < version_) {
      return;
    }
    if ((itr == cache_.end()) && (System::execute(quartus_path_ + "/sopc_builder/bin/qsys-generate " + System::src_root() + "/src/target/core/de10/fpga/soc_system.qsys --synthesis=VERILOG") != 0)) {
      return;
    } 
  } 
  { // Step 3: map
    lock_guard<mutex> lg(lock_);
    if (my_version < version_) {
      return;
    }
    if ((itr == cache_.end()) && (System::execute(quartus_path_ + "/bin/quartus_map " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0)) {
      return;
    } 
  } 
  { // Step 4: fit
    lock_guard<mutex> lg(lock_);
    if (my_version < version_) {
      return;
    }
    if ((itr == cache_.end()) && (System::execute(quartus_path_ + "/bin/quartus_fit " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0)) {
      return;
    } 
  } 
  { // Step 5: asm
    lock_guard<mutex> lg(lock_);
    if (my_version < version_) {
      return;
    }
    if ((itr == cache_.end()) && (System::execute(quartus_path_ + "/bin/quartus_asm " + System::src_root() + "/src/target/core/de10/fpga/DE10_NANO_SoC_GHRD.qpf") != 0)) {
      return;
    }
  } 
  { // Step 6: Put code into cache, reprogram, update status, and notify all
    lock_guard<mutex> lg(lock_);
    if (my_version < version_) {
      return;
    }
    if (itr == cache_.end()) {
      stringstream ss;
      ss << "bitstream_" << cache_.size() << ".sof";
      const auto file = ss.str();
      System::execute("cp " + System::src_root() + "/src/target/core/de10/fpga/output_files/DE10_NANO_SoC_GHRD.sof " + cache_path_ + "/" + file);

      ofstream ofs(cache_path_ + "/index.txt", ios::app);
      ofs << text << '\0' << file << '\0';
      ofs.flush();

      itr = cache_.insert(make_pair(text, file)).first;
    }
    if (System::execute(quartus_path_ + "/bin/quartus_pgm -c \"DE-SoC " + usb_ + "\" --mode JTAG -o \"P;" + cache_path_ + "/" + itr->second + "@2\"") != 0) {
      return;
    } 
    for (auto& s : slots_) {
      if (s.first == QuartusServer::State::WAITING) {
        s.first = QuartusServer::State::CURRENT;
      }
    }
    cv_.notify_all();
  }
}

void QuartusServer::run_logic() {
  init_pool();
  init_cache();
  init_slots();
  init_versioning();

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

  struct timeval timeout = {0, 1000};

  while (!stop_requested()) {
    read_set = master_set;
    select(server.descriptor()+1, &read_set, nullptr, nullptr, &timeout);
    if (!FD_ISSET(server.descriptor(), &read_set)) {
      continue;
    }

    auto* sock = server.accept();
    const auto rpc = static_cast<QuartusServer::Rpc>(sock->get());
    switch (rpc) {
      case QuartusServer::Rpc::REQUEST_SLOT:
        request_slot(sock);
        break;
      case QuartusServer::Rpc::UPDATE_SLOT:
        pool_.insert(new ThreadPool::Job([this, sock]{update_slot(sock);}));
        break;
      case QuartusServer::Rpc::RETURN_SLOT:
        return_slot(sock);
        break;
      case QuartusServer::Rpc::ABORT:
        abort(sock);
        break;

      case QuartusServer::Rpc::ERROR:
      default:
        request_stop();
        break;
    }
  }

  abort(nullptr);
  pool_.stop_now();
}

} // namespace cascade
