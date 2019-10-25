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

#include <cassert>
#include "cascade/cascade.h"
#include "runtime/runtime.h"
#include "target/compiler.h"
#include "target/compiler/proxy_compiler.h"
#include "target/core/de10/de10_compiler.h"
#include "target/core/sw/sw_compiler.h"

using namespace std;

namespace cascade {

Cascade::Cascade() : eval_(this), iostream(&sb_), sb_() {
  is_running_ = false;

  set_enable_inlining(true);
  set_open_loop_target(1);

  runtime_.get_compiler()->set("de10", new de10::De10Compiler());
  runtime_.get_compiler()->set("proxy", new ProxyCompiler());
  runtime_.get_compiler()->set("sw", new SwCompiler());

  set_quartus_server("localhost", 9900);
}

Cascade::~Cascade() {
  stop_now();
}

Cascade& Cascade::set_include_dirs(const string& path) {
  assert(!is_running_);
  runtime_.set_include_dirs(path);
  return *this;
}

Cascade& Cascade::set_enable_inlining(bool enable) {
  assert(!is_running_);
  runtime_.set_disable_inlining(!enable);
  return *this;
}

Cascade& Cascade::set_open_loop_target(size_t n) {
  assert(!is_running_);
  runtime_.set_open_loop_target(n);
  return *this;
}

Cascade& Cascade::set_quartus_server(const string& host, size_t port) {
  assert(!is_running_);
  auto* dc = runtime_.get_compiler()->get("de10");
  assert(dc != nullptr);
  static_cast<de10::De10Compiler*>(dc)->set_host(host);
  static_cast<de10::De10Compiler*>(dc)->set_port(port);
  return *this;
}

Cascade& Cascade::set_profile_interval(size_t n) {
  assert(!is_running_);
  runtime_.set_profile_interval(n);
  return *this;
}

Cascade& Cascade::set_stdin(streambuf* sb) {
  assert(!is_running_);
  runtime_.rdbuf(0, sb);
  return *this;
}

Cascade& Cascade::set_stdout(streambuf* sb) {
  assert(!is_running_);
  runtime_.rdbuf(1, sb);
  return *this;
}

Cascade& Cascade::set_stderr(streambuf* sb) {
  assert(!is_running_);
  runtime_.rdbuf(2, sb);
  return *this;
}

Cascade& Cascade::set_stdwarn(streambuf* sb) {
  assert(!is_running_);
  runtime_.rdbuf(3, sb);
  return *this;
}

Cascade& Cascade::set_stdinfo(streambuf* sb) {
  assert(!is_running_);
  runtime_.rdbuf(4, sb);
  return *this;
}

Cascade& Cascade::set_stdlog(streambuf* sb) {
  assert(!is_running_);
  runtime_.rdbuf(5, sb);
  return *this;
}

Cascade::Fd Cascade::open(streambuf* sb) {
  assert(!is_running_);
  return runtime_.rdbuf(sb);
}

Cascade& Cascade::run() {
  // Ignore multiple calls to run() and don't restart the runtime if the
  // simulation has ended. Otherwise, start the runtime and the eval loop.
  if (is_running_ || is_finished()) {
    return *this;
  } 
  is_running_ = true;
  runtime_.run();
  eval_.run();
  return *this;
}

Cascade& Cascade::request_stop() {
  // Multiple calls to request_stop are harmless.
  halt_eval();
  runtime_.request_stop();
  return *this;
}

Cascade& Cascade::wait_for_stop() {
  // Multiple calls to wait for stop are harmless.

  // Control can reach here either after a call to request_stop() (in which
  // case the eval loop will have been shutdown) or as a way of blocking until
  // the program finishes execution. In the latter case, the eval thread needs
  // to be stopped. In the former case, a second call to halt_eval() here is
  // harmless.
  runtime_.wait_for_stop();
  halt_eval();
  is_running_ = false;
  return *this;
}

Cascade& Cascade::stop_now() {
  // Calling stop_now() multiple times, regardless of state, should have no
  // effect. This is important, as stop_now() is called by the destructor.

  request_stop();
  wait_for_stop();
  return *this;
}

bool Cascade::is_running() const {
  return is_running();
}

bool Cascade::is_finished() const {
  return runtime_.is_finished();
}

Cascade::EvalLoop::EvalLoop(Cascade* cascade) : Thread() {
  cascade_ = cascade;
}

void Cascade::EvalLoop::run_logic() {
  if (cascade_->is_cin()) {
    cin_loop();
  } else {
    generic_loop();
  }
}

void Cascade::EvalLoop::cin_loop() {
  // If we've replaced cascade's rdbuf with stdin, any further stream switching
  // or restarts via run are undefined. All we need to do here is parse inputs
  // indefinitely until either the runtime shuts down through a call to
  // finish(), cin closed via ctrl-d, or the eval thread is terminated via a
  // call to request_stop().
  while (!cascade_->eof() && !cascade_->runtime_.is_finished()) {
    const auto res = cascade_->runtime_.eval(*cascade_);
    if (res.first) {
      cascade_->setstate(ios::eofbit);
    }
    if (res.second) {
      cascade_->setstate(ios::badbit);
    }
  }

  // If control reaches here via eof, the runtime may still be running, and a
  // call to wait_for_stop will hang forever. Submit a stop_request so that
  // calls to wait_for_stop() will return, and set Cascade's failbit. It will
  // never accept input again.
  if (cascade_->eof()) { 
    cascade_->runtime_.request_stop();
    cascade_->setstate(ios::failbit);
  }
}

void Cascade::EvalLoop::generic_loop() {
  // If we're connected to a standard-compliant rdbuf, we can expect to see
  // eofs when inputs dry up. Calling eval when there's nothing in the stream
  // shouldn't hurt (it'll return immediately), so whenever this happens we can
  // just go to sleep and check back again later. When inputs do appear, we
  // parse them all until another eof appears. Running the loop one last time
  // when a stop is requested guarantees we don't miss any inputs due to a race
  // condition on the check against stop_requested().

  for (auto one_more = true; !cascade_->runtime_.is_finished() && !cascade_->bad() && (!stop_requested() || one_more); one_more = !stop_requested()) {
    const auto res = cascade_->runtime_.eval_all(*cascade_);
    if (res.first) {
      cascade_->setstate(ios::eofbit);
    }
    if (res.second) {
      cascade_->setstate(ios::badbit);
    }
    if (!stop_requested()) {
      wait_for(100);
    }
  }
}

bool Cascade::is_cin() const {
  // TODO(eschkufz): Is there a better way to check this? The user might have
  // moved the rdbuf initially associated with cin (the one we care about
  // due to its blocking behavior) somewhere else.
  return rdbuf() == cin.rdbuf();
}

void Cascade::halt_eval() {
  // This method behaves differently depending on whether we're attached cin
  // (which has non-standard blocking semantics) or a compliant rdbuf. It
  // should be safe to invoke this method multiple times in a row.

  // The ugly case: we're stuck in the cin loop. For lack of any better ideas,
  // terminate the cin loop with extreme prejudice. This is the only way I know
  // of to kick control out of a blocking call to cin.get(). Cascade is placed
  // in the fail state.
  if (is_cin()) {
    if (!eval_.was_terminated()) {
      eval_.terminate();
    }
    setstate(ios::failbit);
  }
  // The clean case: flush the rdbuf and make any remaining data available to
  // the generic eval loop. Control will return from the call to stop_now when
  // all inputs have been processed.
  else {
    flush();
    eval_.request_stop();
    eval_.notify();
    eval_.wait_for_stop();
  }
}

} // namespace cascade
