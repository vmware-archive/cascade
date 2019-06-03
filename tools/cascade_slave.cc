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
#include <signal.h>
#include "cascade/cascade_slave.h"
#include "cl/cl.h"

using namespace cascade;
using namespace cascade::cl;
using namespace std;

namespace {

__attribute__((unused)) auto& g1 = Group::create("Cascade Slave Options");
auto& slave_port = StrArg<size_t>::create("--slave_port")
  .usage("<int>")
  .description("Port to listen for slave connections on")
  .initial(8800);
auto& slave_path = StrArg<string>::create("--slave_path")
  .usage("<path/to/socket>")
  .description("Path to listen for slave_connections on")
  .initial("/tmp/fpga_socket");

__attribute__((unused)) auto& g2 = Group::create("Quartus Server Options");
auto& quartus_host = StrArg<string>::create("--quartus_host")
  .usage("<host>")
  .description("Location of quartus server")
  .initial("localhost");
auto& quartus_port = StrArg<uint32_t>::create("--quartus_port")
  .usage("<port>")
  .description("Location of quartus server")
  .initial(9900);

CascadeSlave slave_;

void int_handler(int sig) {
  (void) sig;
  ::slave_.request_stop();
}

void segv_handler(int sig) {
  (void) sig;
  cerr << "\033[31mCASCADE SLAVE SHUTDOWN UNEXPECTEDLY\033[00m" << endl;
  cerr << "\033[31mPlease rerun with --enable_log and forward log file to developers\033[00m" << endl;
  exit(1);
}

} // namespace

int main(int argc, char** argv) {
  // Parse command line
  Simple::read(argc, argv);

  // Attach signal handlers
  { struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = ::segv_handler;
    sigaction(SIGSEGV, &action, nullptr);
  }
  { struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = ::int_handler;
    sigaction(SIGINT, &action, nullptr);
  }

  slave_.set_listeners(::slave_path.value(), ::slave_port.value());
  slave_.set_quartus_server(::quartus_host.value(), ::quartus_port.value());
  slave_.run();
  slave_.wait_for_stop();
  cout << "Goodbye!" << endl;

  return 0;
}
