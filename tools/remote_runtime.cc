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

#include <cstring>
#include <iostream>
#include <signal.h>
#include <string>
#include "ext/cl/include/cl.h"
#include "src/target/common/remote_runtime.h"
#include "src/target/compiler.h"
#include "src/target/core/de10/de10_compiler.h"
#include "src/target/core/proxy/proxy_compiler.h"
#include "src/target/core/sw/sw_compiler.h"

using namespace cl;
using namespace cascade;
using namespace std;

__attribute__((unused)) auto& g1 = Group::create("Backend Server Options");
auto& port = StrArg<size_t>::create("--port")
  .usage("<int>")
  .description("Port to listen for connections on")
  .initial(8800);
auto& path = StrArg<string>::create("--path")
  .usage("<path/to/socket>")
  .description("Path to listen for connections on")
  .initial("/tmp/fpga_socket");

__attribute__((unused)) auto& g2 = Group::create("DE10 Compiler Options");
auto& batch_window = StrArg<int>::create("--batch_window")
  .usage("<int>")
  .description("Number of seconds to wait before checking for compilation requests")
  .initial(3);
auto& quartus_host = StrArg<string>::create("--quartus_host")
  .usage("<host>")
  .description("Location of quartus server")
  .initial("localhost");
auto& quartus_port = StrArg<uint32_t>::create("--quartus_port")
  .usage("<port>")
  .description("Location of quartus server")
  .initial(9900);

RemoteRuntime* runtime = nullptr;

void handler(int sig) {
  (void) sig;
  runtime->request_stop();
}

int main(int argc, char** argv) {
  // Parse command line:
  Simple::read(argc, argv);

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = handler;
  sigaction(SIGINT, &action, nullptr);

  auto dc = new De10Compiler();
    dc->set_host(quartus_host.value());
    dc->set_port(quartus_port.value());
  auto pc = new ProxyCompiler();
  auto sc = new SwCompiler();
  auto c = new Compiler();
    c->set_de10_compiler(dc);
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);

  runtime = new RemoteRuntime();
    runtime->set_compiler(c);
    runtime->set_path(path.value());
    runtime->set_port(port.value());
    runtime->run();
    runtime->wait_for_stop();
  delete runtime;

  cout << "Goodbye!" << endl;
  return 0;
}
