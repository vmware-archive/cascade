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
#include "cl/cl.h"
#include "target/core/avmm/de10/quartus_server.h"

using namespace cascade;
using namespace cascade::cl;
using namespace std;

namespace {

__attribute__((unused)) auto& g = Group::create("Quartus Server Options");
auto& cache = StrArg<string>::create("--cache")
  .usage("<path/to/cache>")
  .description("Path to directory to use as compilation cache")
  .initial("/tmp/quartus_cache");
auto& path = StrArg<string>::create("--path")
  .usage("<path/to/quarus>")
  .description("Path to quartus installation directory")
  .initial("~/intelFPGA_lite/17.1/quartus");
auto& tunnel_command = StrArg<string>::create("--tunnel-command")
  .usage("<command to reach remote-host>")
  .description("Optional tunnel command to reach the host where quartus installation directory is located")
  .initial("");
auto& port = StrArg<uint32_t>::create("--port")
  .usage("<int>")
  .description("Port to run quartus server on")
  .initial(9900);

avmm::QuartusServer* qs = nullptr;

void handler(int sig) {
  (void) sig;
  qs->request_stop();
}

} // namespace

int main(int argc, char** argv) {
  // Parse command line:
  Simple::read(argc, argv);

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = ::handler;
  sigaction(SIGINT, &action, nullptr);

  ::qs = new avmm::QuartusServer();
  ::qs->set_cache_path(::cache.value());
  ::qs->set_quartus_path(::path.value());
  ::qs->set_quartus_tunnel_command(::tunnel_command.value());
  ::qs->set_port(::port.value());

  if (::qs->error()) {
    cout << "Unable to locate core quartus components!" << endl;
  } else {
    ::qs->run();
    ::qs->wait_for_stop();
  }
  delete ::qs;

  cout << "Goodbye!" << endl;
  return 0;
}
