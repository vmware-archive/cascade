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
#include "cl.h"
#include "base/system/system.h"
#include "cascade/cascade.h"

using namespace cl;
using namespace cascade;
using namespace std;

namespace {

__attribute__((unused)) auto& g1 = Group::create("Cascade Runtime Options");
auto& march = StrArg<string>::create("--march")
  .usage("minimal|minimal_jit|sw|de10|de10_jit")
  .description("Target architecture")
  .initial("minimal");
auto& inc_dirs = StrArg<string>::create("-I")
  .usage("<path1>:<path2>:...:<pathn>")
  .description("Paths to search for files on")
  .initial("");
auto& input_path = StrArg<string>::create("-e")
  .usage("path/to/file.v")
  .description("Read input from file");
auto& ui = StrArg<string>::create("--ui")
  .usage("term|batch")
  .description("UI interface")
  .initial("term");

__attribute__((unused)) auto& g3 = Group::create("Quartus Server Options");
auto& quartus_host = StrArg<string>::create("--quartus_host")
  .usage("<host>")
  .description("Location of quartus server")
  .initial("localhost");
auto& quartus_port = StrArg<uint32_t>::create("--quartus_port")
  .usage("<port>")
  .description("Location of quartus server")
  .initial(9900);

__attribute__((unused)) auto& g4 = Group::create("Logging Options");
auto& profile = StrArg<int>::create("--profile")
  .usage("<n>")
  .description("Number of seconds to wait between profiling events; setting n to zero disables profiling; only effective with --enable_info")
  .initial(0);
auto& enable_info = FlagArg::create("--enable_info")
  .description("Turn on info messages");
auto& disable_warning = FlagArg::create("--disable_warning")
  .description("Turn off warning messages");
auto& disable_error = FlagArg::create("--disable_error")
  .description("Turn off error messages");
auto& enable_logging = FlagArg::create("--enable_logging")
  .description("Copies cascade output to log file");

__attribute__((unused)) auto& g5 = Group::create("Optimization Options");
auto& disable_inlining = FlagArg::create("--disable_inlining")
  .description("Prevents cascade from inlining modules");
auto& open_loop_target = StrArg<size_t>::create("--open_loop_target")
  .usage("<n>")
  .description("Maximum number of seconds to run in open loop for before transferring control back to runtime")
  .initial(1);

__attribute__((unused)) auto& g6 = Group::create("Slave Runtime Options");
auto& slave = FlagArg::create("--slave")
  .description("Runs Cascade in slave mode");
auto& slave_port = StrArg<size_t>::create("--slave_port")
  .usage("<int>")
  .description("Port to listen for slave connections on")
  .initial(8800);
auto& slave_path = StrArg<string>::create("--slave_path")
  .usage("<path/to/socket>")
  .description("Path to listen for slave_connections on")
  .initial("/tmp/fpga_socket");

Cascade* cascade_ = nullptr;

void int_handler(int sig) {
  (void) sig;
  if (!::slave.value()) {
    ::cascade_->error("User Interrupt:\n  > Caught Ctrl-C.");
    ::cascade_->finish(0);
  } else {
    ::cascade_->request_stop();
  }
}

void segv_handler(int sig) {
  (void) sig;
  cout << "CASCADE SHUTDOWN UNEXPECTEDLY" << endl;
  cout << "Please rerun with --enable_logging and forward log file to developers" << endl;
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

  cascade_ = new Cascade();

  cascade_->set_include_path(::inc_dirs.value() + ":" + System::src_root());

  if (::ui.value() == "term") {
    cascade_->attach_term_ui();
  } 
  if (::enable_logging.value()) {
    cascade_->attach_logfile();
  }

  cascade_->enable_profile(::profile.value());
  cascade_->enable_info(::enable_info.value());
  cascade_->enable_warning(!::disable_warning.value());
  cascade_->enable_error(!::disable_error.value());

  cascade_->enable_inlining(!::disable_inlining.value());
  cascade_->set_open_loop_target(::open_loop_target.value());

  cascade_->set_quartus_host(::quartus_host.value());
  cascade_->set_quartus_port(::quartus_port.value());

  if (::slave.value()) {
    cascade_->set_slave_mode(::slave.value());
    cascade_->set_slave_port(::slave_port.value());
    cascade_->set_slave_path(::slave_path.value());
  }

  cascade_->run();
  if (!::slave.value()) {
    if (::input_path.value() != "") {
      cascade_->eval("`include \"data/march/" + ::march.value() + ".v\"\n" +
                     "`include \"" + ::input_path.value() + "\"");
    } else {
      cascade_->eval("`include \"data/march/" + ::march.value() + ".v\"");
    }
  }
  cascade_->wait_for_stop();

  cout << "Goodbye!" << endl;
  return 0;
}
