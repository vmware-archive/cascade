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

#include <cstdio>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include "ext/cl/include/cl.h"
#include "src/base/stream/incstream.h"
#include "src/base/system/system.h"
#include "src/base/thread/asynchronous.h"
#include "src/runtime/runtime.h"
#include "src/target/compiler.h"
#include "src/target/core/de10/de10_compiler.h"
#include "src/target/core/proxy/proxy_compiler.h"
#include "src/target/core/sw/sw_compiler.h"
#include "src/target/interface/local/local_compiler.h"
#include "src/ui/combinator/many_view.h"
#include "src/ui/combinator/maybe_view.h"
#include "src/ui/log/log_view.h"
#include "src/ui/stream/stream_controller.h"
#include "src/ui/term/term_controller.h"
#include "src/ui/term/term_view.h"
#include "src/ui/web/web_ui.h"

using namespace cl;
using namespace cascade;
using namespace std;

namespace {

__attribute__((unused)) auto& g1 = Group::create("Cascade Runtime Options");
auto& input_path = StrArg<string>::create("-e")
  .usage("path/to/file.v")
  .description("Read input from file");
auto& march = StrArg<string>::create("--march")
  .usage("minimal|minimal_jit|sw|de10|de10_jit")
  .description("Target architecture")
  .initial("minimal");
auto& ui = StrArg<string>::create("--ui")
  .usage("term|web")
  .description("UI interface")
  .initial("console");
auto& inc_dirs = StrArg<string>::create("-I")
  .usage("<path1>:<path2>:...:<pathn>")
  .description("Paths to search for files on")
  .initial("");

__attribute__((unused)) auto& g2 = Group::create("Web UI Options"); 
auto& web_ui_port = StrArg<string>::create("--web_ui_port")
  .usage("<int>")
  .description("Port to run web ui on")
  .initial("11111");
auto& web_ui_buffer = StrArg<size_t>::create("--web_ui_buffer")
  .usage("<int>")
  .description("Maximum number of log messages to buffer per refresh")
  .initial(128);
auto& web_ui_debug = FlagArg::create("--web_ui_debug")
  .description("Print debug information to web ui");

__attribute__((unused)) auto& g3 = Group::create("Quartus Options");
auto& quartus_host = StrArg<string>::create("--quartus_host")
  .usage("<host>")
  .description("Location of quartus server")
  .initial("localhost");
auto& quartus_port = StrArg<uint32_t>::create("--quartus_port")
  .usage("<port>")
  .description("Location of quartus server")
  .initial(9900);

__attribute__((unused)) auto& g4 = Group::create("Logging Options");
auto& profile_interval = StrArg<int>::create("--profile_interval")
  .usage("<n>")
  .description("Number of seconds to wait between profiling events; setting n to zero disables profiling")
  .initial(0);
auto& batch = FlagArg::create("--batch")
  .description("Deactivate UI; use -e to provide program input");
auto& enable_logging = FlagArg::create("--enable_logging")
  .description("Turns on UI logging");

__attribute__((unused)) auto& g5 = Group::create("Optimization Options");
auto& open_loop_target = StrArg<size_t>::create("--open_loop_target")
  .usage("<n>")
  .description("Maximum number of seconds to run in open loop for before transferring control back to runtime")
  .initial(1);

__attribute__((unused)) auto& g6 = Group::create("Elaboration Tasks");
auto& enable_info = FlagArg::create("--enable_info")
  .description("Turn on info messages");
auto& disable_warning = FlagArg::create("--disable_warning")
  .description("Turn off warning messages");
auto& disable_error = FlagArg::create("--disable_error")
  .description("Turn off error messages");

class Profiler : public Asynchronous {
  public:
    explicit Profiler(Runtime* rt) : Asynchronous() { 
      rt_ = rt;
    }
    ~Profiler() override = default;
  private:
    void run_logic() override {
      while (!stop_requested()) {
        clog << "TIME = " << time(nullptr) << endl;
        clog << "FREQ = " << rt_->current_frequency() << endl;
        sleep_for(1000*::profile_interval.value());        
      }
    }
    Runtime* rt_;
};

// Cascade Components:
ManyView* view = nullptr;
Runtime* runtime = nullptr;
Controller* controller = nullptr;
// Logging Components:
ofstream* logfile = nullptr;
Profiler* profiler = nullptr;

void int_handler(int sig) {
  (void) sig;
  ::runtime->fatal(0, "User Interrupt:\n  > Caught Ctrl-C.");
}

void segv_handler(int sig) {
  (void) sig;
  ::view->crash();
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

  // Setup Global MVC State
  ::view = new ManyView();
  ::runtime = new Runtime(::view);
  if (::enable_logging) {
    ::logfile = new ofstream("cascade.log", ofstream::app);
    ::view->attach(new LogView(*::logfile));
  }
  if (::ui.value() == "web") {
    auto* mv = new MaybeView();
    auto* wui = new WebUi(::runtime);
    wui->set_port(::web_ui_port.value());
    wui->set_buffer(::web_ui_buffer.value());
    wui->set_debug(::web_ui_debug.value());
    ::controller = wui;
    mv->attach(dynamic_cast<View*>(wui));
    ::view->attach(mv);
  } else {
    ::view->attach(new TermView());
    ::controller = new TermController(::runtime);
  }

  // Setup Compiler State
  auto* dc = new De10Compiler();
    dc->set_host(::quartus_host.value());
    dc->set_port(::quartus_port.value());
  auto* pc = new ProxyCompiler();
  auto* sc = new SwCompiler();
    sc->set_include_dirs(::inc_dirs.value() + ":" + System::src_root());
  auto* lc = new LocalCompiler();
    lc->set_runtime(::runtime);
  auto* c = new Compiler();
    c->set_de10_compiler(dc);
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);
    c->set_local_compiler(lc);

  // Start the runtime
  ::runtime->set_compiler(c);
    ::runtime->set_include_dirs(::inc_dirs.value() + ":" + System::src_root());
    ::runtime->set_open_loop_target(::open_loop_target.value());
    ::runtime->enable_info(::enable_info.value());
    ::runtime->disable_warning(::disable_warning.value());
    ::runtime->disable_error(::disable_error.value());
  ::runtime->run();

  // Parse march configuration
  incstream mis(System::src_root());
  stringstream ss1;
  if (mis.open("data/march/" + march.value() + ".v")) {
    ss1 << "include data/march/" + march.value() + ".v;";
    StreamController(::runtime, ss1).run_to_completion();
  } else {
    ::view->error(0, "Unrecognized march option " + ::march.value() + "!");
  }
  // Translate -e to include statement if it was provided
  stringstream ss2;
  if (::input_path.value() != "") {
    ss2 << "include " << ::input_path.value() << ";";
    StreamController(::runtime, ss2).run_to_completion();
  }

  // Switch over to a live console (unless the --batch flag has been provided)
  // and turn on profiling (if the --profile flag was provided)
  if (!::batch.value()) {
    ::controller->run();
  }
  ::profiler = new Profiler(::runtime);
  if (::profile_interval.value() > 0) {
    ::profiler->run();
  }

  // Wait for the runtime to stop and then shutdown remaining threads
  ::runtime->wait_for_stop();
  ::controller->stop_now();
  ::profiler->stop_now();

  // Tear down global state
  delete ::runtime;
  if (::ui.value() == "web") {
    delete ::controller;
  } else {
    delete ::view;
    delete ::controller;
  }
  delete ::logfile;
  delete ::profiler;

  cout << "Goodbye!" << endl;
  return 0;
}
