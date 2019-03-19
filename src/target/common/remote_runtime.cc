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

#include "src/target/common/remote_runtime.h"

#include <cassert>
#include <vector>
#include "src/base/log/log.h"
#include "src/base/stream/sockstream.h"
#include "src/target/compiler.h"
#include "src/target/engine.h"
#include "src/target/interface/remote/remote_compiler.h"
#include "src/target/interface/remote/remote_interface.h"
#include "src/target/state.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/parse/parser.h"

using namespace std;

namespace cascade {

RemoteRuntime::RemoteRuntime() : Asynchronous() {
  compiler_ = new Compiler();
  set_path("/tmp/fpga_socket");
  set_port(8800);
}

RemoteRuntime::~RemoteRuntime() {
  delete compiler_;
}

RemoteRuntime& RemoteRuntime::set_compiler(Compiler* c) {
  delete compiler_;
  compiler_ = c;
  return *this;
}

RemoteRuntime& RemoteRuntime::set_path(const string& p) {
  path_ = p;
  return *this;
}

RemoteRuntime& RemoteRuntime::set_port(uint32_t p) {
  port_ = p;
  return *this;
}

void RemoteRuntime::run_logic() {
  auto* rc = new RemoteCompiler();
  compiler_->set_remote_compiler(rc);

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

  vector<sockstream*> socks;
  vector<Engine*> engines;

  while (!stop_requested()) {
    read_set = master_set;
    select(max_fd+1, &read_set, nullptr, nullptr, &timeout);
    for (auto i = 0; i <= max_fd; ++i) {
      // Not ready
      if (!FD_ISSET(i, &read_set)) {
        continue;
      }
      // Listener
      if ((i == tl.descriptor()) || (i == ul.descriptor())) {
        auto* sock = (i == tl.descriptor()) ? tl.accept() : ul.accept();
        const auto fd = sock->descriptor();
        FD_SET(fd, &master_set);
        if (fd > max_fd) {
          max_fd = fd;
          socks.resize(max_fd+1, nullptr);
        }
        socks[fd] = sock;
        continue;
      }
      // Client
      auto* sock = socks[i];
      Rpc rpc;
      rpc.deserialize(*sock);
      switch (rpc.type_) {
        case Rpc::Type::COMPILE: {
          const Rpc::Id id = engines.size();
          rc->set_sock(sock);
          rc->set_id(id);
          if (auto* e = compile(sock)) {
            engines.push_back(e);
            Rpc(Rpc::Type::OKAY, id).serialize(*sock);
            sock->flush();
          } else {
            Rpc(Rpc::Type::FAIL, 0).serialize(*sock);
            sock->flush();
          }
          break;
        }
        case Rpc::Type::GET_STATE:
          get_state(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::SET_STATE:
          set_state(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::GET_INPUT:
          get_input(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::SET_INPUT:
          set_input(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::FINALIZE:
          finalize(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::OVERRIDES_DONE_STEP:
          overrides_done_step(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::DONE_STEP:
          done_step(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::OVERRIDES_DONE_SIMULATION:
          overrides_done_simulation(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::DONE_SIMULATION:
          done_simulation(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::READ:
          read(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::EVALUATE:
          evaluate(sock, rpc.id_, engines[rpc.id_]);
          break;
        case Rpc::Type::THERE_ARE_UPDATES:
          there_are_updates(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::UPDATE:
          update(sock, rpc.id_, engines[rpc.id_]);
          break;
        case Rpc::Type::THERE_WERE_TASKS:
          there_were_tasks(sock, engines[rpc.id_]);
          break;
        case Rpc::Type::CONDITIONAL_UPDATE:
          conditional_update(sock, rpc.id_, engines[rpc.id_]);
          break;
        case Rpc::Type::OPEN_LOOP:
          open_loop(sock, rpc.id_, engines[rpc.id_]);
          break;

        case Rpc::Type::ENGINE_TEARDOWN:
          delete engines[rpc.id_];
          engines[rpc.id_] = nullptr;
          sock->flush();
          break;
        case Rpc::Type::CONNECTION_TEARDOWN:
        default:
          delete sock;
          socks[i] = nullptr;
          FD_CLR(i, &master_set);
          break;
      }
    }
  }

  for (auto* s : socks) {
    if (s != nullptr) {
      delete s;
    }
  }
  for (auto* e : engines) {
    delete e;
  }
}

Engine* RemoteRuntime::compile(sockstream* sock) {
  Parser p;
  Log log;
  p.parse(*sock, &log);
  assert(p.success());
  assert((*p.begin())->is(Node::Tag::module_declaration));

  auto* md = static_cast<ModuleDeclaration*>(*p.begin());
  return (log.error() || (md == nullptr)) ? nullptr : compiler_->compile(md);
}

void RemoteRuntime::get_state(sockstream* sock, Engine* e) {
  auto* s = e->get_state();
  s->serialize(*sock);
  delete s;
  sock->flush();
}

void RemoteRuntime::set_state(sockstream* sock, Engine* e) {
  auto* s = new State();
  s->deserialize(*sock);
  e->set_state(s);
  delete s;
}

void RemoteRuntime::get_input(sockstream* sock, Engine* e) {
  auto* i = e->get_input();
  i->serialize(*sock);
  delete i;
  sock->flush();
}

void RemoteRuntime::set_input(sockstream* sock, Engine* e) {
  auto* i = new Input();
  i->deserialize(*sock);
  e->set_input(i);
  delete i;
}

void RemoteRuntime::finalize(sockstream* sock, Engine* e) {
  (void) sock;
  e->finalize();
}

void RemoteRuntime::overrides_done_step(sockstream* sock, Engine* e) {
  sock->put(e->overrides_done_step() ? 1 : 0);
  sock->flush();
}

void RemoteRuntime::done_step(sockstream* sock, Engine* e) {
  (void) sock;
  e->done_step();
}

void RemoteRuntime::overrides_done_simulation(sockstream* sock, Engine* e) {
  sock->put(e->overrides_done_simulation() ? 1 : 0);
  sock->flush();
}

void RemoteRuntime::done_simulation(sockstream* sock, Engine* e) {
  (void) sock;
  e->done_simulation();
}

void RemoteRuntime::read(sockstream* sock, Engine* e) {
  VId id = 0;
  sock->read(reinterpret_cast<char*>(&id), 4); 
  Bits bits;
  bits.deserialize(*sock);
  e->read(id, &bits);
}

void RemoteRuntime::evaluate(sockstream* sock, Rpc::Id id, Engine* e) {
  e->evaluate();
  // This call to evaluate will have primed the socket with tasks and writes
  // Appending an OKAY rpc, indicates that everything has been sent.
  Rpc(Rpc::Type::OKAY, id).serialize(*sock);
  sock->flush();
}

void RemoteRuntime::there_are_updates(sockstream* sock, Engine* e) {
  sock->put(e->there_are_updates() ? 1 : 0);
}

void RemoteRuntime::update(sockstream* sock, Rpc::Id id, Engine* e) {
  e->update();
  // This call to update will have primed the socket with tasks and writes
  // Appending an OKAY rpc, indicates that everything has been sent.
  Rpc(Rpc::Type::OKAY, id).serialize(*sock);
  sock->flush();
}

void RemoteRuntime::there_were_tasks(sockstream* sock, Engine* e) {
  sock->put(e->there_were_tasks() ? 1 : 0);
}

void RemoteRuntime::conditional_update(sockstream* sock, Rpc::Id id, Engine* e) {
  const auto res = e->conditional_update();
  // This call to conditional_update will have primed the socket with tasks and
  // writes Appending an OKAY rpc, indicates that everything has been sent.
  Rpc(Rpc::Type::OKAY, id).serialize(*sock);
  sock->put(res ? 1 : 0);
  sock->flush();
}

void RemoteRuntime::open_loop(sockstream* sock, Rpc::Id id, Engine* e) {
  uint32_t clk = 0;
  sock->read(reinterpret_cast<char*>(&clk), 4);
  bool val = (sock->get() == 1);
  uint32_t itr = 0;
  sock->read(reinterpret_cast<char*>(&itr), 4);

  const uint32_t res = e->open_loop(clk, val, itr);
  // This call to open_loop  will have primed the socket with tasks and
  // writes Appending an OKAY rpc, indicates that everything has been sent.
  Rpc(Rpc::Type::OKAY, id).serialize(*sock);
  sock->write(reinterpret_cast<const char*>(&res), 4);
  sock->flush();
}

} // namespace cascade
