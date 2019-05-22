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

#include "cascade/cascade_slave.h"
#include "target/common/remote_runtime.h"
#include "target/compiler.h"
#include "target/core/de10/de10_compiler.h"
#include "target/core/proxy/proxy_compiler.h"
#include "target/core/sw/sw_compiler.h"

using namespace std;

namespace cascade {

CascadeSlave::CascadeSlave() {
  remote_runtime_ = new RemoteRuntime();
  set_listeners("./cascade_sock", 8800);
  set_quartus_server("localhost", 9900);
}

CascadeSlave::~CascadeSlave() {
  stop_now();
  delete remote_runtime_;
}

CascadeSlave& CascadeSlave::set_listeners(const string& path, size_t port) {
  remote_runtime_->set_path(path);
  remote_runtime_->set_port(port);
  return *this;
}

CascadeSlave& CascadeSlave::set_quartus_server(const string& host, size_t port) {
  auto* dc = new De10Compiler();
    dc->set_host(host);
    dc->set_port(port);
  auto* pc = new ProxyCompiler();
  auto* sc = new SwCompiler();
  auto* c = new Compiler();
    c->set_de10_compiler(dc);
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);
  remote_runtime_->set_compiler(c);
  return *this;
}

CascadeSlave& CascadeSlave::run() {
  remote_runtime_->run();
  return *this;
}

CascadeSlave& CascadeSlave::request_stop() {
  remote_runtime_->request_stop();
  return *this;
}

CascadeSlave& CascadeSlave::wait_for_stop() {
  remote_runtime_->wait_for_stop();
  return *this;
}

CascadeSlave& CascadeSlave::stop_now() {
  remote_runtime_->stop_now();
  return *this;
}

} // namespace cascade
