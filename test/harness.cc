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

#include "test/harness.h"

#include <fstream>
#include <sstream>
#include "gtest/gtest.h"
#include "src/base/system/system.h"
#include "src/runtime/runtime.h"
#include "src/target/compiler.h"
#include "src/target/core/proxy/proxy_compiler.h"
#include "src/target/core/sw/sw_compiler.h"
#include "src/target/interface/local/local_compiler.h"
#include "src/ui/view.h"
#include "src/ui/stream/stream_controller.h"
#include "src/verilog/parse/parser.h"

using namespace cascade;
using namespace std;

namespace cascade {

PView::PView(ostream& os) : View(), os_(os) { }

void PView::print(size_t t, const string& s) {
  (void) t;
  os_ << s;
}

EView::EView() : View() {
  error_ = false;
}

bool EView::error() const {
  return error_;
}

void EView::error(size_t t, const string& s) {
  (void) t;
  (void) s;
  error_ = true;
} 

void run_parse(const string& path, bool expected) {
  ifstream ifs(path);
  ASSERT_TRUE(ifs.is_open());

  Parser p;
  Log log;
  p.parse(ifs, &log);
  EXPECT_EQ(log.error(), expected);
}

void run_typecheck(const string& march, const string& path, bool expected) {
  EView view;
  Runtime runtime(&view);
  auto pc = new ProxyCompiler();
  auto sc = new SwCompiler();
  auto lc = new LocalCompiler();
    lc->set_runtime(&runtime);
  auto c = new Compiler();
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);
    c->set_local_compiler(lc);
  runtime.set_compiler(c);
  runtime.run();

  ifstream mf("data/march/" + march + ".v");
  ASSERT_TRUE(mf.is_open());
  StreamController(&runtime, mf).run_to_completion();

  ifstream ifs(path);
  ASSERT_TRUE(ifs.is_open());
  StreamController(&runtime, ifs).run_to_completion();

  stringstream ss("initial $finish;");
  if (expected) {
    StreamController(&runtime, ss).run_to_completion();
  }

  runtime.wait_for_stop();
  EXPECT_EQ(view.error(), expected);
}

void run_code(const string& march, const string& path, const string& expected) {
  stringstream ss;
  PView view(ss);
  Runtime runtime(&view);
  auto pc = new ProxyCompiler();
  auto sc = new SwCompiler();
  auto lc = new LocalCompiler();
    lc->set_runtime(&runtime);
  auto c = new Compiler();
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);
    c->set_local_compiler(lc);
  runtime.set_compiler(c);
  runtime.run();

  ifstream mf("data/march/" + march + ".v");
  ASSERT_TRUE(mf.is_open());
  StreamController(&runtime, mf).run_to_completion();

  ifstream ifs(path);
  ASSERT_TRUE(ifs.is_open());
  StreamController(&runtime, ifs).run_to_completion();

  runtime.wait_for_stop();
  EXPECT_EQ(ss.str(), expected);
}

void run_bitcoin(const string& march, const string& path, const string& expected) {
  run_code(march, path, expected);
}

void run_mips(const string& march, const string& path, const string& expected) {
  System().execute("data/test/mips32/asm/asm " + path + " > data/test/mips32/sc/imem.mem");

  stringstream ss;
  PView view(ss);
  Runtime runtime(&view);
  auto pc = new ProxyCompiler();
  auto sc = new SwCompiler();
  auto lc = new LocalCompiler();
    lc->set_runtime(&runtime);
  auto c = new Compiler();
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);
    c->set_local_compiler(lc);
  runtime.set_include_dirs("data/test/mips32/sc/");
  runtime.set_compiler(c);
  runtime.run();

  ifstream mf("data/march/" + march + ".v");
  ASSERT_TRUE(mf.is_open());
  StreamController(&runtime, mf).run_to_completion();

  ifstream ifs("data/test/mips32/sc/cpu.v");
  ASSERT_TRUE(ifs.is_open());
  StreamController(&runtime, ifs).run_to_completion();

  runtime.wait_for_stop();
  EXPECT_EQ(ss.str(), expected);
}

void run_regex(const string& march, const string& regex, const string& input, const string& expected) {
  System().execute("echo \"" + regex + "\" | data/test/regex/codegen/codegen > data/test/regex/codegen/regex.v");
  System().execute("cat " + input + " | data/test/regex/data/encode > data/test/regex/data/fifo.data");

  stringstream ss;
  PView view(ss);
  Runtime runtime(&view);
  auto pc = new ProxyCompiler();
  auto sc = new SwCompiler();
  auto lc = new LocalCompiler();
    lc->set_runtime(&runtime);
  auto c = new Compiler();
    c->set_proxy_compiler(pc);
    c->set_sw_compiler(sc);
    c->set_local_compiler(lc);
  runtime.set_compiler(c);
  runtime.run();

  ifstream mf("data/march/" + march + ".v");
  ASSERT_TRUE(mf.is_open());
  StreamController(&runtime, mf).run_to_completion();

  ifstream ifs("data/test/regex/codegen/regex.v");
  ASSERT_TRUE(ifs.is_open());
  StreamController(&runtime, ifs).run_to_completion();

  runtime.wait_for_stop();
  EXPECT_EQ(ss.str(), expected);
}

} // namespace cascade
