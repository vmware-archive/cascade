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

namespace cascade {

ProxyCompiler::ProxyCompiler() : CoreCompiler() { }

ProxyCompiler::~ProxyCompiler() {
  for (auto& s : socks_) {
    auto* sock = s.second.second;
    Rpc(Rpc::Type::TEARDOWN_CONNECTION, s.second.first).serialize(*sock);
    sock->flush();
    Rpc rpc;
    rpc.deserialize(*sock);
    assert(rpc.type_ == Rpc::Type::OKAY);
    delete sock;
  }
}

void ProxyCompiler::abort(const Uuid& uuid) {
  for (auto& s : socks_) {
    auto* sock = s.second.second;
    Rpc(Rpc::Type::ABORT, 0).serialize(*sock);
    uuid.serialize(*sock);
    sock->flush();
    Rpc rpc;
    rpc.deserialize(*sock);
    assert(rpc.type_ == Rpc::Type::OKAY);
  }
}

Clock* ProxyCompiler::compile_clock(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Clock>(uuid, md, interface);
}

Custom* ProxyCompiler::compile_custom(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Custom>(uuid, md, interface);
}

Gpio* ProxyCompiler::compile_gpio(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Gpio>(uuid, md, interface);
}

Led* ProxyCompiler::compile_led(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Led>(uuid, md, interface);
}

Logic* ProxyCompiler::compile_logic(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Logic>(uuid, md, interface);
}

Pad* ProxyCompiler::compile_pad(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Pad>(uuid, md, interface);
}

Reset* ProxyCompiler::compile_reset(const Uuid& uuid, ModuleDeclaration* md, Interface* interface) {
  return generic_compile<Reset>(uuid, md, interface);
}

pair<Rpc::Id, sockstream*> ProxyCompiler::get_persistent_sock(const string& loc) {
  // Check whether we already have a persistent socket for this location.
  const auto itr = socks_.find(loc);
  if (itr != socks_.end()) {
    return itr->second;
  }
  // If not, create a new one, and if creation fails, return nullptr.
  auto* sock = get_temp_sock(loc);
  if (sock == nullptr) {
    return make_pair(0, nullptr); 
  }

  // We have a working socket, so now let's register it.
  Rpc(Rpc::Type::REGISTER_CONNECTION, 0).serialize(*sock);
  sock->flush();
  Rpc rpc;
  rpc.deserialize(*sock);
  assert(rpc.type_ == Rpc::Type::OKAY);

  // The inbound id attached to this reply is the persistent connection id for
  // this socket. Record this and the socket in the socket table.
  const auto res = make_pair(rpc.id_, sock);
  socks_.insert(make_pair(loc, res));
  return res;
}

sockstream* ProxyCompiler::get_temp_sock(const string& loc) {
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

} // namespace cascade
