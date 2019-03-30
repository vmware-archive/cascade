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

#include "test/harness.h"

#include "ext/cl/include/cl.h"
#include "gtest/gtest.h"
#include "lib/cascade.h"
#include "src/base/system/system.h"
#include "src/ui/view.h"
#include "src/verilog/parse/parser.h"

using namespace cascade;
using namespace cl;
using namespace std;

namespace {

auto& march = StrArg<string>::create("--march")
  .initial("minimal");
auto& quartus_host = StrArg<string>::create("--quartus_host")
  .initial("localhost");
auto& quartus_port = StrArg<uint32_t>::create("--quartus_port")
  .initial(9900);

} // namespace

namespace cascade {

PView::PView(ostream& os) : View(), os_(os) { }

void PView::print(size_t t, const string& s) {
  (void) t;
  os_ << s;
}

EView::EView(ostream& os) : View(), os_(os) {
}

void EView::error(size_t t, const string& s) {
  (void) t;
  os_ << s;
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
  stringstream ss;

  Cascade c;
  c.set_include_path(System::src_root());
  c.attach_view(new EView(ss));
  c.run();

  c.eval("include data/march/" + march + ".v;");
  c.eval("include " + path + ";");
  if (expected) {
    c.eval("initial $finish;");
  }

  c.wait_for_stop();
  EXPECT_EQ((ss.str() != ""), expected);
}

void run_code(const string& march, const string& path, const string& expected) {
  stringstream ss;

  Cascade c;
  c.set_include_path(System::src_root());
  c.attach_view(new PView(ss));
  c.run();

  c.eval("include data/march/" + march + ".v;");
  c.eval("include " + path + ";");

  c.wait_for_stop();
  EXPECT_EQ(ss.str(), expected);
}

void run_remote(const string& path, const string& expected) {
  Cascade slave;
  slave.set_slave_mode(true);
  slave.set_slave_path("/tmp/fpga_socket");
  slave.run();
  run_code("minimal_remote", path, expected);
  slave.stop_now();
}

void run_benchmark(const string& path, const string& expected) {
  stringstream ss;

  Cascade c;
  c.set_include_path(System::src_root());
  c.attach_view(new PView(ss));
  c.set_quartus_host(::quartus_host.value());
  c.set_quartus_port(::quartus_port.value());
  c.run();

  c.eval("include data/march/" + ::march.value() + ".v;");
  c.eval("include " + path + ";");

  c.wait_for_stop();
  EXPECT_EQ(ss.str(), expected);
}

} // namespace cascade
