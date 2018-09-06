// Copyright 2017-2018 VMware, Inc.
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
#include "src/target/core/de10/quartus_client.h"
#include "src/target/interface/local/local_compiler.h"
#include "src/ui/combinator/maybe_view.h"
#include "src/ui/stream/stream_controller.h"
#include "src/ui/term/term_controller.h"
#include "src/ui/term/term_view.h"
#include "src/ui/web/web_ui.h"

using namespace cl;
using namespace cascade;
using namespace std;

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

__attribute__((unused)) auto& g3 = Group::create("DE10 Compiler Options");
auto& batch_window = StrArg<int>::create("--batch_window")
  .usage("<int>")
  .description("Number of seconds to wait before checking for compilation requests")
  .initial(3);
auto& quartus_server_host = StrArg<string>::create("--quartus_server_host")
  .usage("<host>")
  .description("Location of quartus server")
  .initial("localhost");
auto& quartus_server_port = StrArg<uint32_t>::create("--quartus_server_port")
  .usage("<port>")
  .description("Location of quartus server")
  .initial(9900);

__attribute__((unused)) auto& g4 = Group::create("Profiling Options");
auto& profile_interval = StrArg<int>::create("--profile_interval")
  .usage("<n>")
  .description("Number of milliseconds to wait between profiling events; setting n to zero disables profiling")
  .initial(0);
auto& batch = FlagArg::create("--batch")
  .description("Deactivate UI; use -e to provide program input");

__attribute__((unused)) auto& g5 = Group::create("Optimization Options");
auto& disable_inlining = FlagArg::create("--disable_inlining")
  .description("Disable inlining for user logic; note that this may disable other optimizations as well");
auto& open_loop_target = StrArg<size_t>::create("--open_loop_target")
  .usage("<n>")
  .description("Maximum number of seconds to run in open loop for before transferring control back to runtime")
  .initial(1);

class Profiler : public Asynchronous {
  public:
    Profiler(Runtime* rt) : Asynchronous() { 
      rt_ = rt;
    }
    ~Profiler() override = default;
  private:
    void run_logic() override {
      while (!stop_requested()) {
        clog << "TIME = " << time(nullptr) << endl;
        clog << "FREQ = " << rt_->current_frequency() << endl;
        sleep_for(profile_interval.value());        
      }
    }
    Runtime* rt_;
};

// Cascade Components:
QuartusClient* qc = nullptr;
View* view = nullptr;
Runtime* runtime = nullptr;
Controller* controller = nullptr;
// Profiling Components:
Profiler* profiler = nullptr;

void segv_handler(int sig) {
  (void) sig;
  runtime->crash(cerr);
  exit(1);
}

int main(int argc, char** argv) {
  // Parse command line
  Simple::read(argc, argv);

  // Attach signal handlers
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = segv_handler;
  sigaction(SIGSEGV, &action, nullptr);

  // Setup Global MVC State
  if (ui.value() == "web") {
    view = new MaybeView();
    runtime = new Runtime(view);
    auto ui = new WebUi(runtime);
    ui->set_port(web_ui_port.value());
    ui->set_buffer(web_ui_buffer.value());
    ui->set_debug(web_ui_debug.value());
    controller = ui;
    dynamic_cast<MaybeView*>(view)->attach(dynamic_cast<View*>(ui));
  } else {
    view = new TermView();
    runtime = new Runtime(view);
    controller = new TermController(runtime);
  }
  // Setup Profiler
  profiler = new Profiler(runtime);
  if (profile_interval.value() > 0) {
    profiler->run();
  }

  // Setup Compiler State
  qc = new QuartusClient();
  qc->set_batch_window(batch_window.value());
  qc->set_host(quartus_server_host.value());
  qc->set_port(quartus_server_port.value());
  qc->run();

  auto lc = new LocalCompiler();
  lc->set_runtime(runtime);
  auto dc = new De10Compiler();
  dc->set_quartus_client(qc);
  auto c = new Compiler();
  c->set_runtime(runtime);
  c->set_num_jit_threads(64);
  c->set_local_compiler(lc);
  c->set_de10_compiler(dc);
  runtime->set_compiler(c);
  runtime->set_include_dirs(inc_dirs.value() + ":" + System::src_root());
  runtime->disable_inlining(disable_inlining.value());
  runtime->set_open_loop_target(open_loop_target.value());

  // Start the runtime 
  runtime->run();

  // Parse march configuration
  incstream mis(System::src_root());
  if (mis.open("data/march/" + march.value() + ".v")) {
    StreamController(runtime, mis).run_to_completion();
  } else {
    view->error("Unrecognized march option " + march.value() + "!");
  }
  // Parse file inputs
  incstream fis(inc_dirs.value());
  if (input_path.value() != "") {
    if (fis.open(input_path)) {
      StreamController(runtime, fis).run_to_completion();
    } else {
      view->error("Unable to open input file " + input_path.value() + "!");
    }
  }
  // Switch over to a live console (unless the --batch flag has been provided)
  if (!batch.value()) {
    controller->run();
  }

  // Wait for the runtime to stop and then shutdown remaining threads
  runtime->wait_for_stop();
  qc->stop_now();
  controller->stop_now();
  profiler->stop_now();

  // Tear down global state
  delete qc;
  delete profiler;
  if (ui.value() == "web") {
    delete runtime;
    delete controller;
  } else {
    delete view;
    delete runtime;
    delete controller;
  }

  cout << "Goodbye!" << endl;
  return 0;
}
