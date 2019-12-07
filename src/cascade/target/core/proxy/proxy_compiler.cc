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

#include "target/core/proxy/proxy_compiler.h"

#include <sstream>

using namespace std;

namespace cascade::proxy {

ProxyCompiler::ProxyCompiler() : CoreCompiler() { 
  pool_.set_num_threads(4);
  pool_.run();
  running_ = true;
}

ProxyCompiler::~ProxyCompiler() {
  for (auto& c : conns_) {
    Rpc(Rpc::Type::CLOSE_CONN, c.second.pid, 0, 0).serialize(*c.second.sync_sock);
    c.second.sync_sock->flush();
    // Don't block on a response here. All we need to do is notify the remote
    // compiler that it's okay to release these sockets. Blocking on a response
    // has the potential to introduce a race condition on the other side.
    delete c.second.async_sock;
    delete c.second.sync_sock;
  }
}

void ProxyCompiler::stop_async() {
  running_ = false;
  pool_.stop_now();
}

Clock* ProxyCompiler::compile_clock(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Clock>(id, md, interface);
}

Custom* ProxyCompiler::compile_custom(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Custom>(id, md, interface);
}

Gpio* ProxyCompiler::compile_gpio(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Gpio>(id, md, interface);
}

Led* ProxyCompiler::compile_led(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Led>(id, md, interface);
}

Logic* ProxyCompiler::compile_logic(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Logic>(id, md, interface);
}

Pad* ProxyCompiler::compile_pad(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Pad>(id, md, interface);
}

Reset* ProxyCompiler::compile_reset(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Reset>(id, md, interface);
}

void ProxyCompiler::async_loop(sockstream* sock) {
  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(sock->descriptor(), &master_set);

  fd_set read_set;
  FD_ZERO(&read_set);

  struct timeval timeout = {1, 0};

  while (running_) {
    read_set = master_set;
    select(sock->descriptor()+1, &read_set, nullptr, nullptr, &timeout);
    if (!FD_ISSET(sock->descriptor(), &read_set)) {
      continue;
    }

    // The only message we expect here is the beginning of a state-safe
    // handshake or a notification that the remote connection was closed.
    Rpc rpc; 
    rpc.deserialize(*sock);
    if (rpc.type_ != Rpc::Type::STATE_SAFE_BEGIN) {
      return;
    }

    // Schedule a state safe interrupt which tells the remote compiler to fire
    // its interrupt handler. Block until the remote compiler replies that it's
    // finished. We don't expect to see a close message here.
    get_compiler()->schedule_state_safe_interrupt([sock]{
      Rpc(Rpc::Type::STATE_SAFE_OKAY).serialize(*sock);
      sock->flush();
      Rpc res;
      res.deserialize(*sock);
      assert(res.type_ == Rpc::Type::STATE_SAFE_FINISH);
    });
  }
}

void ProxyCompiler::stop_compile(Engine::Id id) {
  // TODO(eschkufz) Add some better error handling here. We assume that if a
  // connection was opened, all further communication will succeed.

  for (auto& c : conns_) {
    auto* sock = get_sock(c.first);
    assert(sock != nullptr);
    Rpc(Rpc::Type::STOP_COMPILE, c.second.pid, id, 0).serialize(*sock);
    sock->flush();
    Rpc rpc;
    rpc.deserialize(*sock);
    assert(rpc.type_ == Rpc::Type::OKAY);
    delete sock;
  }
}

bool ProxyCompiler::open(const string& loc) {
  // Nothing to do if we already have a connection to this location
  if (conns_.find(loc) != conns_.end()) {
    return true;
  }

  // Open a new asynchronous socket
  ConnInfo ci;
  Rpc rpc;

  // TODO(eschkufz) Add some better error handling here. We assume that if the
  // first connection attempt succeded, then all subsequent parts of the
  // handshake will succeed as well.

  // Step 1: Open the asynchronous socket and sent a register request.  The
  // reply will contain the id the remote compiler associates with this
  // compiler.
  ci.async_sock = get_sock(loc);
  if (ci.async_sock == nullptr) {
    return false;
  }
  Rpc(Rpc::Type::OPEN_CONN_1, 0, 0, 0).serialize(*ci.async_sock);
  ci.async_sock->flush();
  rpc.deserialize(*ci.async_sock);
  assert(rpc.type_ == Rpc::Type::OKAY);
  ci.pid = rpc.pid_;

  // Step 2: Open the synchronous socket and send a register request. This
  // time around, send the pid so that the new socket can be associated with
  // this connection in the remote compiler.
  ci.sync_sock = get_sock(loc);
  assert(ci.sync_sock != nullptr);
  Rpc(Rpc::Type::OPEN_CONN_2, ci.pid, 0, 0).serialize(*ci.sync_sock);
  ci.sync_sock->flush();
  rpc.deserialize(*ci.sync_sock);
  assert(rpc.type_ == Rpc::Type::OKAY);

  // Step 3: Create a thread to listen for asynchronous messages 
  pool_.insert([this, ci]{async_loop(ci.async_sock);});

  // Step 4: Archive the connection
  conns_[loc] = ci;
  return true;
}

sockstream* ProxyCompiler::get_sock(const string& loc) {
  auto* sock = (loc.find(':') != string::npos) ? get_tcp_sock(loc) : get_unix_sock(loc);
  if (sock->error()) {
    delete sock;
    return nullptr;
  }
  return sock;
}

sockstream* ProxyCompiler::get_tcp_sock(const string& loc) {
  stringstream ss(loc);
  string host;
  uint32_t port;
  getline(ss, host, ':');
  ss >> port;
  return new sockstream(host.c_str(), port);
}

sockstream* ProxyCompiler::get_unix_sock(const string& loc) {
  return new sockstream(loc.c_str()); 
}

} // namespace cascade::proxy
