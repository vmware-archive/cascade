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

#include "cascade/cascade.h"
#include "runtime/runtime.h"
#include "target/compiler.h"
#include "target/core/de10/de10_compiler.h"
#include "target/core/proxy/proxy_compiler.h"
#include "target/core/sw/sw_compiler.h"
#include "target/interface/local/local_compiler.h"

using namespace std;

namespace cascade {

Cascade::Cascade() {
  runtime_ = new Runtime();
  set_enable_inlining(true);
  set_open_loop_target(1);
  set_quartus_server("localhost", 9900);
}

Cascade::~Cascade() {
  stop_now();
  delete runtime_;
}

Cascade& Cascade::set_include_dirs(const string& path) {
  runtime_->set_include_dirs(path);
  return *this;
}

Cascade& Cascade::set_enable_inlining(bool enable) {
  runtime_->disable_inlining(!enable);
  return *this;
}

Cascade& Cascade::set_open_loop_target(size_t n) {
  runtime_->set_open_loop_target(n);
  return *this;
}

Cascade& Cascade::set_quartus_server(const string& host, size_t port) {
  auto* dc = new De10Compiler();
    dc->set_host(host);
    dc->set_port(port);
  auto* lc = new LocalCompiler();
    lc->set_runtime(runtime_);
  auto* pc = new ProxyCompiler();
  auto* sc = new SwCompiler();
//    sc->set_include_dirs(include_path_);
// TODO(eschkufz) fix this!!!
  auto* c = new Compiler();
    c->set_de10_compiler(dc);
    c->set_local_compiler(lc);
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);
  runtime_->set_compiler(c);
  return *this;
}

Cascade& Cascade::set_stdin(streambuf* sb) {
  delete runtime_->rdbuf(0, sb);
  return *this;
}

Cascade& Cascade::set_stdout(streambuf* sb) {
  delete runtime_->rdbuf(1, sb);
  return *this;
}

Cascade& Cascade::set_stderr(streambuf* sb) {
  delete runtime_->rdbuf(2, sb);
  return *this;
}

Cascade& Cascade::set_stdwarn(streambuf* sb) {
  delete runtime_->rdbuf(3, sb);
  return *this;
}

Cascade& Cascade::set_stdinfo(streambuf* sb) {
  delete runtime_->rdbuf(4, sb);
  return *this;
}

Cascade& Cascade::set_stdlog(streambuf* sb) {
  delete runtime_->rdbuf(5, sb);
  return *this;
}

Cascade& Cascade::run() {
  runtime_->run();
  return *this;
}

Cascade& Cascade::request_stop() {
  runtime_->request_stop();
  return *this;
}

Cascade& Cascade::wait_for_stop() {
  runtime_->wait_for_stop();
  return *this;
}

Cascade& Cascade::stop_now() {
  runtime_->stop_now();
  return *this;
}

evalstream Cascade::eval() {
  return evalstream(runtime_);
}

} // namespace cascade
