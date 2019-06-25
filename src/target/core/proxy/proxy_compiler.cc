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
    Rpc(Rpc::Type::CONNECTION_TEARDOWN, 0).serialize(*s.second);
    s.second->flush();
    Rpc rpc;
    rpc.deserialize(*s.second);
    assert(rpc.type_ == Rpc::Type::OKAY);
    delete s.second;
  }
}

void ProxyCompiler::abort() {
  // TODO(eschkufz) Invoke a remote abort
}

Clock* ProxyCompiler::compile_clock(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Clock>(interface, md);
}

Custom* ProxyCompiler::compile_custom(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Custom>(interface, md);
}

Gpio* ProxyCompiler::compile_gpio(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Gpio>(interface, md);
}

Led* ProxyCompiler::compile_led(Interface* interface, ModuleDeclaration* md) {
  return generic_compile<Led>(interface, md);
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

sockstream* ProxyCompiler::get_sock(const ModuleDeclaration* md) {
  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  const auto itr = socks_.find(loc);
  if (itr != socks_.end()) {
    return itr->second;
  }

  auto* sock = (loc.find(':') != string::npos) ? get_tcp_sock(md) : get_unix_sock(md);
  if (sock->error()) {
    delete sock;
    return nullptr; 
  }

  socks_.insert(make_pair(loc, sock));
  return sock;
}

sockstream* ProxyCompiler::get_tcp_sock(const ModuleDeclaration* md) {
  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  stringstream ss(loc);

  string host;
  uint32_t port;
  getline(ss, host, ':');
  ss >> port;

  return new sockstream(host.c_str(), port);
}

sockstream* ProxyCompiler::get_unix_sock(const ModuleDeclaration* md) {
  const auto loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  return new sockstream(loc.c_str()); 
}

} // namespace cascade
