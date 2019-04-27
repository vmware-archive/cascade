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

#include "gtest/gtest.h"
#include "lib/cascade.h"
#include "test/harness.h"

using namespace cascade;

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Create a slave runtime for remote tests. This doesn't actually solve
  // the race condition of running a remote test before the slave runtime
  // enters its accept loop, but if there's anything in between here and
  // the first remote test, it makes it *excepptionally unlikely* that it
  // will ever happen.
  Cascade slave;
  slave.set_slave_mode(true);
  slave.set_slave_path("/tmp/fpga_socket");
  slave.run();

  return RUN_ALL_TESTS();
}

TEST(remote, hello_1) {
  run_code("minimal_remote", "data/test/regression/simple/hello_1.v", "Hello World");
}
TEST(remote, pipeline_1) {
  run_code("minimal_remote", "data/test/regression/simple/pipeline_1.v", "0123456789");
}
TEST(remote, pipeline_2) {
  run_code("minimal_remote", "data/test/regression/simple/pipeline_2.v", "0123456789");
}
TEST(remote, io) {
  run_code("minimal_remote", "data/test/regression/simple/io_1.v", "1234512345");
}
TEST(remote, bitcoin) {
  run_code("minimal_remote", "data/test/benchmark/bitcoin/run_4.v", "f 93\n");
}
TEST(remote, bubble) {
  run_code("minimal_remote", "data/test/benchmark/mips32/run_bubble_128.v", "1");
}
TEST(remote, regex) {
  run_code("minimal_remote", "data/test/benchmark/regex/run_disjunct_1.v", "424");
}
