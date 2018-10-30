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

#include "gtest/gtest.h"
#include "src/target/common/remote_runtime.h"
#include "src/target/compiler.h"
#include "src/target/core/sw/sw_compiler.h"
#include "test/harness.h"

using namespace cascade;

TEST(remote, hello_1) {
  RemoteRuntime rr;
  auto c = new Compiler();
    c->set_sw_compiler(new SwCompiler());
  rr.set_compiler(c);
  rr.run();
  run_code("minimal_remote", "data/test/simple/hello_1.v", "Hello World");
  rr.stop_now();
}
TEST(remote, pipeline_1) {
  RemoteRuntime rr;
  auto c = new Compiler();
    c->set_sw_compiler(new SwCompiler());
  rr.set_compiler(c);
  rr.run();
  run_code("minimal_remote", "data/test/simple/pipeline_1.v", "0123456789");
  rr.stop_now();
}
TEST(remote, pipeline_2) {
  RemoteRuntime rr;
  auto c = new Compiler();
    c->set_sw_compiler(new SwCompiler());
  rr.set_compiler(c);
  rr.run();
  run_code("minimal_remote", "data/test/simple/pipeline_2.v", "0123456789");
  rr.stop_now();
}
TEST(remote, bitcoin) {
  RemoteRuntime rr;
  auto c = new Compiler();
    c->set_sw_compiler(new SwCompiler());
  rr.set_compiler(c);
  rr.run();
  run_bitcoin("minimal_remote", "data/test/bitcoin/bitcoin_9.v", "f 93");
  rr.stop_now();
}
TEST(remote, bubble) {
  RemoteRuntime rr;
  auto c = new Compiler();
    c->set_sw_compiler(new SwCompiler());
  rr.set_compiler(c);
  rr.run();
  run_mips("minimal_remote", "data/test/mips32/src/bubble.s", "1");
  rr.stop_now();
}
TEST(remote, regex) {
  RemoteRuntime rr;
  auto c = new Compiler();
    c->set_sw_compiler(new SwCompiler());
  rr.set_compiler(c);
  rr.run();
  run_regex("minimal_remote", "(Achilles)|(THE END)", "data/test/regex/data/iliad.txt", "424");
  rr.stop_now();
}
