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

#include "src/target/core/proxy/proxy_compiler.h"

#include <sstream>
#include "src/base/socket/socket.h"

using namespace std;

namespace cascade {

ProxyCompiler::ProxyCompiler() : CoreCompiler(), buf_(1024) { }

ProxyCompiler::~ProxyCompiler() {
  for (auto& c : conns_) {
    c.second->send_rpc(Rpc(Rpc::Type::CONNECTION_CLOSED, 0));
    delete c.second;
  }
}

void ProxyCompiler::abort() {
  // TODO(eschkufz) Invoke a remote abort
}

Clock* ProxyCompiler::compile_clock(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Clock>(interface, md);
}

Fifo* ProxyCompiler::compile_fifo(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Fifo>(interface, md);
}

Gpio* ProxyCompiler::compile_gpio(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Gpio>(interface, md);
}

Led* ProxyCompiler::compile_led(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Led>(interface, md);
}

Memory* ProxyCompiler::compile_memory(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Memory>(interface, md);
}

Pad* ProxyCompiler::compile_pad(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Pad>(interface, md);
}

Reset* ProxyCompiler::compile_reset(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Reset>(interface, md);
}

Logic* ProxyCompiler::compile_logic(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Logic>(interface, md);
}

Connection* ProxyCompiler::get_conn(const ModuleDeclaration* md) {
  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  const auto itr = conns_.find(loc);
  if (itr != conns_.end()) {
    return itr->second;
  }

  auto* conn = (loc.find(':') != string::npos) ? get_tcp_conn(md) : get_unix_conn(md);
  if (conn->error()) {
    delete conn;
    return nullptr; 
  }

  conns_.insert(make_pair(loc, conn));
  return conn;
}

Connection* ProxyCompiler::get_tcp_conn(const ModuleDeclaration* md) {
  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  stringstream ss(loc);

  string host;
  uint32_t port;
  getline(ss, host, ':');
  ss >> port;

  auto* sock = new Socket();
  sock->connect(host, port);
  return new Connection(sock);
}

Connection* ProxyCompiler::get_unix_conn(const ModuleDeclaration* md) {
  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();

  auto* sock = new Socket();
  sock->connect(loc);
  return new Connection(sock); 
}

} // namespace cascade
