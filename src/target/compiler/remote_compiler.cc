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

#include "target/compiler/remote_compiler.h"

#include <cassert>
#include <unordered_map>
#include "common/log.h"
#include "common/sockserver.h"
#include "common/sockstream.h"
#include "target/compiler/remote_interface.h"
#include "target/engine.h"
#include "target/state.h"
#include "verilog/ast/ast.h"
#include "verilog/parse/parser.h"

using namespace std;

namespace cascade {

RemoteCompiler::RemoteCompiler() : Compiler(), Thread() { 
  set_path("/tmp/fpga_socket");
  set_port(8800);

  sock_ = nullptr;
}

void RemoteCompiler::schedule_state_safe_interrupt(Runtime::Interrupt int_) {
  // TODO(eschkufz) Implement this!!!
  (void) int_;
}

void RemoteCompiler::schedule_asynchronous(Runtime::Asynchronous async) {
  pool_.insert(async);
}

Interface* RemoteCompiler::get_interface(const std::string& loc) {
  if (loc != "remote") {
    return nullptr;        
  }
  if (sock_ == nullptr) {
    return nullptr;
  }
  return new RemoteInterface(sock_);
}

RemoteCompiler& RemoteCompiler::set_path(const string& p) {
  path_ = p;
  return *this;
}

RemoteCompiler& RemoteCompiler::set_port(uint32_t p) {
  port_ = p;
  return *this;
}

void RemoteCompiler::run_logic() {
  sockserver tl(port_, 8);
  sockserver ul(path_.c_str(), 8);
  if (tl.error() || ul.error()) {
    return;
  }

  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(tl.descriptor(), &master_set);
  FD_SET(ul.descriptor(), &master_set);

  fd_set read_set;
  FD_ZERO(&read_set);

  struct timeval timeout = {0, 1000};
  auto max_fd = max(tl.descriptor(), ul.descriptor());

  pool_.set_num_threads(4);
  pool_.run();

  while (!stop_requested()) {
    read_set = master_set;
    select(max_fd+1, &read_set, nullptr, nullptr, &timeout);
    for (auto i = 0; i <= max_fd; ++i) {
      
      // Not ready; nothing to do
      if (!FD_ISSET(i, &read_set)) {
        continue;
      }

      // Listener logic: New connections are added to the read set
      if ((i == tl.descriptor()) || (i == ul.descriptor())) {
        auto* sock = (i == tl.descriptor()) ? tl.accept() : ul.accept();
        const auto fd = sock->descriptor();
        FD_SET(fd, &master_set);
        if (fd > max_fd) {
          max_fd = fd;
          socks_.resize(max_fd+1, nullptr);
        }
        socks_[fd] = sock;
        continue;
      }

      // Client: Grab the socket associated with this fd and handle the request
      auto* sock = socks_[i];
      do {
        Rpc rpc;
        rpc.deserialize(*sock);
        switch (rpc.type_) {

          // Compiler ABI: 
          case Rpc::Type::COMPILE:
            compile(sock, rpc);
            sock = nullptr;
            socks_[i] = nullptr;
            FD_CLR(i, &master_set);
            break;
          case Rpc::Type::STOP_COMPILE: 
            stop_compile(sock, rpc);
            sock = nullptr;
            socks_[i] = nullptr;
            FD_CLR(i, &master_set);
            break;

          // Core ABI:
          case Rpc::Type::GET_STATE:
            get_state(sock, get_engine(rpc));
            break;
          case Rpc::Type::SET_STATE:
            set_state(sock, get_engine(rpc));
            break;
          case Rpc::Type::GET_INPUT:
            get_input(sock, get_engine(rpc));
            break;
          case Rpc::Type::SET_INPUT:
            set_input(sock, get_engine(rpc));
            break;
          case Rpc::Type::FINALIZE:
            finalize(sock, get_engine(rpc));
            break;
          case Rpc::Type::OVERRIDES_DONE_STEP:
            overrides_done_step(sock, get_engine(rpc));
            break;
          case Rpc::Type::DONE_STEP:
            done_step(sock, get_engine(rpc));
            break;
          case Rpc::Type::OVERRIDES_DONE_SIMULATION:
            overrides_done_simulation(sock, get_engine(rpc));
            break;
          case Rpc::Type::DONE_SIMULATION:
            done_simulation(sock, get_engine(rpc));
            break;
          case Rpc::Type::READ:
            read(sock, get_engine(rpc));
            break;
          case Rpc::Type::EVALUATE:
            evaluate(sock, get_engine(rpc));
            break;
          case Rpc::Type::THERE_ARE_UPDATES:
            there_are_updates(sock, get_engine(rpc));
            break;
          case Rpc::Type::UPDATE:
            update(sock, get_engine(rpc));
            break;
          case Rpc::Type::THERE_WERE_TASKS:
            there_were_tasks(sock, get_engine(rpc));
            break;
          case Rpc::Type::CONDITIONAL_UPDATE:
            conditional_update(sock, get_engine(rpc));
            break;
          case Rpc::Type::OPEN_LOOP:
            open_loop(sock, get_engine(rpc));
            break;

          // Proxy Compiler Codes:
          case Rpc::Type::OPEN_CONN_1: 
            open_conn_1(sock, rpc);
            break;
          case Rpc::Type::OPEN_CONN_2:
            open_conn_2(sock, rpc);
            break;
          case Rpc::Type::CLOSE_CONN: {
            Rpc(Rpc::Type::OKAY).serialize(*sock);
            sock->flush();
            sock = nullptr;
            delete socks_[sock_index_[rpc.pid_].first];
            socks_[sock_index_[rpc.pid_].first] = nullptr;
            FD_CLR(sock_index_[rpc.pid_].first, &master_set);
            delete socks_[sock_index_[rpc.pid_].second];
            socks_[sock_index_[rpc.pid_].second] = nullptr;
            FD_CLR(sock_index_[rpc.pid_].second, &master_set);
            break;
          }

          // Proxy Core Codes:
          case Rpc::Type::TEARDOWN_ENGINE:
            teardown_engine(sock, rpc);
            break;

          // Control reaches here innocuosly when fds are closed remotely
          default:
            break;
        }
      } while ((sock != nullptr) && (sock->rdbuf()->in_avail() > 0));
    }
  }

  // Stop all asynchronous threads. 
  pool_.stop_now();

  // We have exclusive access to the indices. Delete their contents.
  for (auto& es : engines_) {
    for (auto* e : es) {
      if (e != nullptr) {
        delete e;
      }
    }
  }
  engines_.clear();
  for (auto* s : socks_) {
    if (s != nullptr) {
      delete s;
    }
  }
  socks_.clear();
}

void RemoteCompiler::compile(sockstream* sock, const Rpc& rpc) {
  // Read the module declaration in the request
  Log log;
  Parser p(&log);
  p.parse(*sock);
  assert(!log.error());
  assert((*p.begin())->is(Node::Tag::module_declaration));
  auto* md = static_cast<ModuleDeclaration*>(*p.begin());

  // Add a new entry to the engine table if necessary
  auto eid = 0;
  { lock_guard<mutex> lg(lock_);
    if ((rpc.pid_ >= engine_index_.size()) || (rpc.eid_ >= engine_index_[rpc.pid_].size())) {
      engine_index_.resize(rpc.pid_+1);
      engine_index_[rpc.pid_].resize(rpc.eid_+1);
      engine_index_[rpc.pid_][rpc.eid_] = engines_.size();
      engines_.resize(engines_.size()+1);
    }
    eid = engine_index_[rpc.pid_][rpc.eid_];
  }

  // Now create a new thread to compile the code, enter it into the
  // engine table, and close the socket when it's done.
  schedule_asynchronous([this, sock, rpc, md, eid]{
    sock_ = socks_[sock_index_[rpc.pid_].second];
    assert(sock_ != nullptr);
    auto* e = Compiler::compile(eid, md);

    if (e != nullptr) {
      { lock_guard<mutex> lg(lock_);
        engines_[eid].push_back(e);
        Rpc(Rpc::Type::OKAY, rpc.pid_, rpc.eid_, engines_[eid].size()-1).serialize(*sock);
      }
      sock->flush();
    } else {
      Rpc(Rpc::Type::FAIL).serialize(*sock);
      sock->flush();
    }
    delete sock;
  });
}

void RemoteCompiler::stop_compile(sockstream* sock, const Rpc& rpc) {
  auto eid = engine_index_[rpc.pid_][rpc.eid_];
  Compiler::stop_compile(eid);
  Rpc(Rpc::Type::OKAY).serialize(*sock);
  sock->flush();
  delete sock;
}

void RemoteCompiler::get_state(sockstream* sock, Engine* e) {
  auto* s = e->get_state();
  s->serialize(*sock);
  delete s;
  sock->flush();
}

void RemoteCompiler::set_state(sockstream* sock, Engine* e) {
  auto* s = new State();
  s->deserialize(*sock);
  e->set_state(s);
  delete s;
}

void RemoteCompiler::get_input(sockstream* sock, Engine* e) {
  auto* i = e->get_input();
  i->serialize(*sock);
  delete i;
  sock->flush();
}

void RemoteCompiler::set_input(sockstream* sock, Engine* e) {
  auto* i = new Input();
  i->deserialize(*sock);
  e->set_input(i);
  delete i;
}

void RemoteCompiler::finalize(sockstream* sock, Engine* e) {
  e->finalize();
  // This call to finalize will have primed the socket with tasks and writes
  // Appending an OKAY rpc indicates that everything has been sent
  Rpc(Rpc::Type::OKAY).serialize(*sock);
  sock->flush();
}

void RemoteCompiler::overrides_done_step(sockstream* sock, Engine* e) {
  sock->put(e->overrides_done_step() ? 1 : 0);
  sock->flush();
}

void RemoteCompiler::done_step(sockstream* sock, Engine* e) {
  (void) sock;
  e->done_step();
}

void RemoteCompiler::overrides_done_simulation(sockstream* sock, Engine* e) {
  sock->put(e->overrides_done_simulation() ? 1 : 0);
  sock->flush();
}

void RemoteCompiler::done_simulation(sockstream* sock, Engine* e) {
  (void) sock;
  e->done_simulation();
}

void RemoteCompiler::read(sockstream* sock, Engine* e) {
  VId id = 0;
  sock->read(reinterpret_cast<char*>(&id), 4); 
  Bits bits;
  bits.deserialize(*sock);
  e->read(id, &bits);
}

void RemoteCompiler::evaluate(sockstream* sock, Engine* e) {
  e->evaluate();
  // This call to evaluate will have primed the socket with tasks and writes
  // Appending an OKAY rpc, indicates that everything has been sent.
  Rpc(Rpc::Type::OKAY).serialize(*sock);
  sock->flush();
}

void RemoteCompiler::there_are_updates(sockstream* sock, Engine* e) {
  sock->put(e->there_are_updates() ? 1 : 0);
}

void RemoteCompiler::update(sockstream* sock, Engine* e) {
  e->update();
  // This call to update will have primed the socket with tasks and writes
  // Appending an OKAY rpc, indicates that everything has been sent.
  Rpc(Rpc::Type::OKAY).serialize(*sock);
  sock->flush();
}

void RemoteCompiler::there_were_tasks(sockstream* sock, Engine* e) {
  sock->put(e->there_were_tasks() ? 1 : 0);
}

void RemoteCompiler::conditional_update(sockstream* sock, Engine* e) {
  const auto res = e->conditional_update();
  // This call to conditional_update will have primed the socket with tasks and
  // writes Appending an OKAY rpc, indicates that everything has been sent.
  Rpc(Rpc::Type::OKAY).serialize(*sock);
  sock->put(res ? 1 : 0);
  sock->flush();
}

void RemoteCompiler::open_loop(sockstream* sock, Engine* e) {
  uint32_t clk = 0;
  sock->read(reinterpret_cast<char*>(&clk), 4);
  bool val = (sock->get() == 1);
  uint32_t itr = 0;
  sock->read(reinterpret_cast<char*>(&itr), 4);

  const uint32_t res = e->open_loop(clk, val, itr);
  // This call to open_loop  will have primed the socket with tasks and
  // writes Appending an OKAY rpc, indicates that everything has been sent.
  Rpc(Rpc::Type::OKAY).serialize(*sock);
  sock->write(reinterpret_cast<const char*>(&res), 4);
  sock->flush();
}

void RemoteCompiler::open_conn_1(sockstream* sock, const Rpc& rpc) {
  (void) rpc;
  const auto pid = sock_index_.size();
  sock_index_.push_back(make_pair(sock->descriptor(), 0));
  Rpc(Rpc::Type::OKAY, pid, 0, 0).serialize(*sock);
  sock->flush();
}

void RemoteCompiler::open_conn_2(sockstream* sock, const Rpc& rpc) {
  sock_index_[rpc.pid_].second = sock->descriptor();
  Rpc(Rpc::Type::OKAY).serialize(*sock);
  sock->flush();
}

void RemoteCompiler::teardown_engine(sockstream* sock, const Rpc& rpc) {
  { lock_guard<mutex> lg(lock_);
    delete engines_[engine_index_[rpc.pid_][rpc.eid_]][rpc.n_];
    engines_[engine_index_[rpc.pid_][rpc.eid_]][rpc.n_] = nullptr;
  }
  Rpc(Rpc::Type::OKAY).serialize(*sock);
  sock->flush();
} 

Engine* RemoteCompiler::get_engine(const Rpc& rpc) {
  lock_guard<mutex> lg(lock_);
  return engines_[engine_index_[rpc.pid_][rpc.eid_]][rpc.n_];
}

} // namespace cascade
