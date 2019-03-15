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
#include "src/base/socket/socket.h"
#include "src/target/common/connection.h"
#include "src/target/common/rpc.h"
#include "src/target/compiler.h"
#include "src/target/engine.h"
#include "src/target/interface/remote/remote_compiler.h"
#include "src/target/interface/remote/remote_interface.h"
#include "src/target/state.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/parse/parser.h"

using namespace std;

namespace cascade {

RemoteRuntime::RemoteRuntime() : Asynchronous(), in_buf_(1024), out_buf_(1024) {
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
  rc->set_buffer(&out_buf_);
  compiler_->set_remote_compiler(rc);

  Socket tl;
  tl.listen(port_, 8);
  Socket ul;
  ul.listen(path_, 8);

  if (tl.error() || ul.error()) {
    return;
  }

  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(tl.descriptor(), &master_set);
  FD_SET(ul.descriptor(), &master_set);

  fd_set read_set;
  FD_ZERO(&read_set);

  struct timeval timeout = {0, 100};
  auto max_fd = max(tl.descriptor(), ul.descriptor());

  vector<Connection*> conns;
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
      if (i == tl.descriptor() || i == ul.descriptor()) {
        const auto fd = ::accept(i, nullptr, nullptr);
        FD_SET(fd, &master_set);
        if (fd > max_fd) {
          max_fd = fd;
          conns.resize(max_fd+1, nullptr);
        }
        conns[fd] = new Connection(new Socket(fd));
        continue;
      }
      // Client
      auto* conn = conns[i];
      Rpc rpc;
      conn->recv_rpc(rpc);
      switch (rpc.type_) {
        case Rpc::Type::COMPILE: {
          if (auto* e = compile(conn)) {
            engines.push_back(e);
            conn->send_rpc(Rpc(Rpc::Type::OKAY, engines.size()-1));
          } else {
            conn->send_rpc(Rpc(Rpc::Type::ERROR, 0));
          }
          break;
        }
        case Rpc::Type::GET_STATE:
          get_state(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::SET_STATE:
          set_state(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::GET_INPUT:
          get_input(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::SET_INPUT:
          set_input(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::FINALIZE:
          finalize(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::OVERRIDES_DONE_STEP:
          overrides_done_step(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::DONE_STEP:
          done_step(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::OVERRIDES_DONE_SIMULATION:
          overrides_done_simulation(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::DONE_SIMULATION:
          done_simulation(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::EVALUATE:
          evaluate(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::THERE_ARE_UPDATES:
          there_are_updates(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::UPDATE:
          update(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::THERE_WERE_TASKS:
          there_were_tasks(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::CONDITIONAL_UPDATE:
          conditional_update(conn, engines[rpc.id_]);
          break;
        case Rpc::Type::OPEN_LOOP:
          open_loop(conn, engines[rpc.id_]);
          break;

        case Rpc::Type::ENGINE_TEARDOWN:
          delete engines[rpc.id_];
          engines[rpc.id_] = nullptr;
          conn->send_ack();
          break;
        case Rpc::Type::CONNECTION_TEARDOWN:
        default:
          delete conn;
          conns[i] = nullptr;
          FD_CLR(i, &master_set);
          break;
      }
    }
  }

  for (auto* c : conns) {
    if (c != nullptr) {
      delete c;
    }
  }
  for (auto* e : engines) {
    delete e;
  }
}

Engine* RemoteRuntime::compile(Connection* conn) {
  conn->recv_str(in_buf_);

  Parser p;
  Log log;
  p.parse(in_buf_, &log);
  assert(p.success());
  assert((*p.begin())->is(Node::Tag::module_declaration));
  auto* md = static_cast<ModuleDeclaration*>(*p.begin());

  in_buf_.clear();
  if (log.error() || md == nullptr) {
    return nullptr;
  }
  return compiler_->compile(md);
}

void RemoteRuntime::get_state(Connection* conn, Engine* e) {
  auto* s = e->get_state();
  s->serialize(out_buf_);
  delete s;

  conn->send_str(out_buf_);
  out_buf_.resize(0);
}

void RemoteRuntime::set_state(Connection* conn, Engine* e) {
  conn->recv_str(in_buf_);
  auto* s = new State();
  s->deserialize(in_buf_);
  e->set_state(s);
  delete s;
  in_buf_.clear();

  conn->send_ack();
}

void RemoteRuntime::get_input(Connection* conn, Engine* e) {
  auto* i = e->get_input();
  i->serialize(out_buf_);
  delete i;

  conn->send_str(out_buf_);
  out_buf_.resize(0);
}

void RemoteRuntime::set_input(Connection* conn, Engine* e) {
  conn->recv_str(in_buf_);
  auto* i = new Input();
  i->deserialize(in_buf_);
  e->set_input(i);
  delete i;
  in_buf_.clear();

  conn->send_ack();
}

void RemoteRuntime::finalize(Connection* conn, Engine* e) {
  e->finalize();
  conn->send_ack();
}

void RemoteRuntime::overrides_done_step(Connection* conn, Engine* e) {
  conn->send_bool(e->overrides_done_step());
}

void RemoteRuntime::done_step(Connection* conn, Engine* e) {
  e->done_step();
  conn->send_ack();
}

void RemoteRuntime::overrides_done_simulation(Connection* conn, Engine* e) {
  conn->send_bool(e->overrides_done_simulation());
}

void RemoteRuntime::done_simulation(Connection* conn, Engine* e) {
  e->done_simulation();
  conn->send_ack();
}

void RemoteRuntime::evaluate(Connection* conn, Engine* e) {
  conn->recv_str(in_buf_);
  Value temp(0, &bits_);
  for (temp.deserialize(in_buf_); !in_buf_.eof(); temp.deserialize(in_buf_)) {
    e->read(temp.id_, temp.val_);
  }
  in_buf_.clear();

  e->evaluate();
  conn->send_str(out_buf_);
  out_buf_.resize(0);
}

void RemoteRuntime::there_are_updates(Connection* conn, Engine* e) {
  conn->send_bool(e->there_are_updates());
}

void RemoteRuntime::update(Connection* conn, Engine* e) {
  conn->recv_str(in_buf_);
  Value temp(0, &bits_);
  for (temp.deserialize(in_buf_); !in_buf_.eof(); temp.deserialize(in_buf_)) {
    e->read(temp.id_, temp.val_);
  }
  in_buf_.clear();

  e->update();
  conn->send_str(out_buf_);
  out_buf_.resize(0);
}

void RemoteRuntime::there_were_tasks(Connection* conn, Engine* e) {
  conn->send_bool(e->there_were_tasks());
}

void RemoteRuntime::conditional_update(Connection* conn, Engine* e) {
  conn->recv_str(in_buf_);
  Value temp(0, &bits_);
  for (temp.deserialize(in_buf_); !in_buf_.eof(); temp.deserialize(in_buf_)) {
    e->read(temp.id_, temp.val_);
  }
  in_buf_.clear();

  conn->send_bool(e->conditional_update());
  conn->send_str(out_buf_);
  out_buf_.resize(0);
}

void RemoteRuntime::open_loop(Connection* conn, Engine* e) {
  uint32_t clk = 0;
  conn->recv_double(clk);
  bool val = false;
  conn->recv_bool(val);
  uint32_t itr = 0;
  conn->recv_double(itr);
  conn->send_double(e->open_loop(clk, val, itr));
  conn->send_str(out_buf_);
  out_buf_.resize(0);
}

} // namespace cascade
