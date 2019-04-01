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

#include <iomanip>
#include <iostream>
#include <sstream>
#include "lib/cascade.h"
#include "src/runtime/runtime.h"
#include "src/target/common/remote_runtime.h"
#include "src/target/compiler.h"
#include "src/target/core/de10/de10_compiler.h"
#include "src/target/core/proxy/proxy_compiler.h"
#include "src/target/core/sw/sw_compiler.h"
#include "src/target/interface/local/local_compiler.h"
#include "src/ui/combinator/many_view.h"
#include "src/ui/combinator/maybe_view.h"
#include "src/ui/log/log_view.h"
#include "src/ui/term/term_controller.h"
#include "src/ui/term/term_view.h"
#include "src/ui/web/web_ui.h"

using namespace std;

namespace cascade {

Cascade::Cascade() {
  view_ = nullptr;
  runtime_ = nullptr;
  controller_ = nullptr;
  remote_runtime_ = nullptr;
  logfile_ = nullptr;
  profiler_ = nullptr;

  set_include_path(".");

  term_ui_ = false;
  web_ui_ = false;
  user_view_ = nullptr;
  log_ = false;

  set_web_ui_port(11111);
  set_web_ui_buffer(1024);
  set_web_ui_debug(false);

  enable_profile(0);
  enable_info(false);
  enable_warning(true);
  enable_error(true);

  enable_inlining(true);
  set_open_loop_target(1);

  set_quartus_host("localhost");
  set_quartus_port(9900);

  set_slave_mode(false);
  set_slave_port(8800);
  set_slave_path("./cascade_sock");
}

Cascade::~Cascade() {
  stop_now();
}

Cascade& Cascade::set_include_path(const string& path) {
  include_path_ = path;
  return *this;
}

Cascade& Cascade::attach_term_ui() {
  term_ui_ = true;
  web_ui_ = false;
  if (user_view_ != nullptr) {
    delete user_view_;
    user_view_ = nullptr;
  }
  return *this;
}

Cascade& Cascade::attach_web_ui() {
  term_ui_ = false;
  web_ui_ = true;
  if (user_view_ != nullptr) {
    delete user_view_;
    user_view_ = nullptr;
  }
  return *this;
}

Cascade& Cascade::set_web_ui_port(size_t port) {
  web_ui_port_ = port;
  return *this;
}

Cascade& Cascade::set_web_ui_buffer(size_t buffer) {
  web_ui_buffer_ = buffer;
  return *this;
}

Cascade& Cascade::set_web_ui_debug(bool debug) {
  web_ui_debug_ = debug;
  return *this;
}

Cascade& Cascade::attach_view(View* v) {
  term_ui_ = false;
  web_ui_ = false;
  if (user_view_ != nullptr) {
    delete user_view_;
  }
  user_view_ = v;
  return *this;
}

Cascade& Cascade::attach_logfile() {
  log_ = true;
  return *this;
}

Cascade& Cascade::enable_profile(size_t n) {
  enable_profile_ = n;
  return *this;
}

Cascade& Cascade::enable_info(bool enable) {
  enable_info_ = enable;
  return *this;
}

Cascade& Cascade::enable_warning(bool enable) {
  enable_warning_ = enable;
  return *this;
}

Cascade& Cascade::enable_error(bool enable) {
  enable_error_ = enable;
  return *this;
}

Cascade& Cascade::enable_inlining(bool enable) {
  enable_inlining_ = enable;
  return *this;
}

Cascade& Cascade::set_open_loop_target(size_t n) {
  open_loop_target_ = n;
  return *this;
}

Cascade& Cascade::set_quartus_host(const string& host) {
  quartus_host_ = host;
  return *this;
}

Cascade& Cascade::set_quartus_port(size_t port) {
  quartus_port_ = port;
  return *this;
}

Cascade& Cascade::set_slave_mode(bool slave) {
  slave_ = slave;
  return *this;
}

Cascade& Cascade::set_slave_port(size_t port) {
  slave_port_ = port;
  return *this;
}

Cascade& Cascade::set_slave_path(const string& path) {
  slave_path_ = path;
  return *this;
}

Cascade& Cascade::run() {
  // Setup Compiler State
  auto* dc = new De10Compiler();
    dc->set_host(quartus_host_);
    dc->set_port(quartus_port_);
  auto* pc = new ProxyCompiler();
  auto* sc = new SwCompiler();
    sc->set_include_dirs(include_path_);
  auto* c = new Compiler();
    c->set_de10_compiler(dc);
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);

  // Setup Global MVC State if running in Master-Mode
  if (!slave_) {
    view_ = new ManyView();
    runtime_ = new Runtime(view_);
    if (web_ui_) {
      auto* mv = new MaybeView();
      auto* wui = new WebUi(runtime_);
      wui->set_port(web_ui_port_);
      wui->set_buffer(web_ui_buffer_);
      wui->set_debug(web_ui_debug_);
      controller_ = wui;
      mv->attach(dynamic_cast<View*>(wui));
      view_->attach(mv);
    } else if (term_ui_) {
      view_->attach(new TermView());
      controller_ = new TermController(runtime_);
    }
    if (log_) {
      logfile_ = new ofstream("cascade.log", ofstream::app);
      view_->attach(new LogView(*logfile_));
    }
    if (user_view_ != nullptr) {
      view_->attach(user_view_);
    }
    auto* lc = new LocalCompiler();
      lc->set_runtime(runtime_);
      c->set_local_compiler(lc);
  }
  // Otherwise setup slave state
  else {
    remote_runtime_ = new RemoteRuntime();
  }

  // Master mode execution path
  if (!slave_) {
    // Start the runtime
    runtime_->set_compiler(c);
      runtime_->set_include_dirs(include_path_);
      runtime_->set_open_loop_target(open_loop_target_);
      runtime_->disable_inlining(!enable_inlining_);
      runtime_->enable_info(enable_info_);
      runtime_->disable_warning(!enable_warning_);
      runtime_->disable_error(!enable_error_);
    runtime_->run();

    if (controller_ != nullptr) {
      controller_->run();
    }
    if (enable_profile_ > 0) {
      profiler_ = new Profiler(runtime_, enable_profile_);
      profiler_->run();
    }
  }
  // Slave mode execution path
  else {
    remote_runtime_->set_compiler(c);
    remote_runtime_->set_path(slave_path_);
    remote_runtime_->set_port(slave_port_);
    remote_runtime_->run();
  }

  return *this;
}

Cascade& Cascade::request_stop() {
  if (runtime_ != nullptr) {
    runtime_->request_stop();
  } else if (remote_runtime_ != nullptr) {
    remote_runtime_->request_stop();
  }
  return *this;
}

Cascade& Cascade::wait_for_stop() {
  if (runtime_ != nullptr) {
    runtime_->wait_for_stop();
    delete runtime_;
    if (controller_ != nullptr) {
      controller_->stop_now();
      delete controller_;
    }
    if (profiler_ != nullptr) {
      profiler_->stop_now();
      delete profiler_;
    }
    if ((view_ != nullptr) && !web_ui_) {
      delete view_;
    }
    if (logfile_ != nullptr) {
      delete logfile_;
    }
    view_ = nullptr;
    runtime_ = nullptr;
    controller_ = nullptr;
    remote_runtime_ = nullptr;
    logfile_ = nullptr;
    profiler_ = nullptr;
  } else if (remote_runtime_ != nullptr) {
    remote_runtime_->wait_for_stop();
    delete remote_runtime_;
    remote_runtime_ = nullptr;
  }
  return *this;
}

Cascade& Cascade::stop_now() {
  request_stop();
  wait_for_stop();
  return *this;
}

Cascade& Cascade::eval(const string& s) {
  runtime_->eval(s);
  return *this;
}

Cascade& Cascade::finish(size_t arg) {
  runtime_->finish(arg);
  return *this;
}

Cascade& Cascade::error(const string& s) {
  runtime_->error(s);
  return *this;
}

Cascade::Profiler::Profiler(Runtime* rt, size_t interval) : Asynchronous() {
  rt_ = rt;
  interval_ = interval;
}

void Cascade::Profiler::run_logic() {
  while (!stop_requested()) {
    stringstream ss;
    ss << "time / freq = " << setw(10) << time(nullptr) << " / " << setw(7) << rt_->current_frequency();
    rt_->info(ss.str());
    sleep_for(1000*interval_);        
  }
}

} // namespace cascade
