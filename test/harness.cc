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

#include "harness.h"

#include "cascade/cascade.h"
#include "cl/cl.h"
#include "common/system.h"
#include "gtest/gtest.h"

using namespace cascade;
using namespace cascade::cl;
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

void run_parse(const string& path, bool expected) {
  run_typecheck("regression/minimal", path, expected);
}

void run_typecheck(const string& march, const string& path, bool expected) {
  Cascade c;
  c.set_include_dirs(System::src_root());
  c.run();

  c << "`include \"share/march/" << march << ".v\"\n" 
    << "`include \"" << path << "\"" << endl;

  c.stop_now();
  EXPECT_EQ(c.bad(), expected);
}

void run_code(const string& march, const string& path, const string& expected) {
  auto* sb = new stringbuf();

  Cascade c;
  c.set_include_dirs(System::src_root());
  c.set_stdout(sb);
  c.run();

  c << "`include \"share/march/" << march << ".v\"\n"
    << "`include \"" << path << "\"" << endl;

  c.stop_now();
  ASSERT_FALSE(c.bad());

  c.run();
  c.wait_for_stop();
  EXPECT_EQ(sb->str(), expected);
}

void run_concurrent(const string& march, const string& path, const string& expected) {
  std::thread t1(run_code, march, path, expected);
  std::thread t2(run_code, march, path, expected);
  t1.join();
  t2.join();
}

void run_benchmark(const string& path, const string& expected) {
  auto* sb = new stringbuf();

  Cascade c;
  c.set_include_dirs(System::src_root());
  c.set_stdout(sb);
  c.set_quartus_server(::quartus_host.value(), ::quartus_port.value());
  c.run();

  c << "`include \"share/march/" << ::march.value() << ".v\"\n" 
    << "`include \"" << path << "\"" << endl;

  c.stop_now();
  ASSERT_FALSE(c.bad());

  c.run();
  c.wait_for_stop();
  EXPECT_EQ(sb->str(), expected);
}

} // namespace cascade
