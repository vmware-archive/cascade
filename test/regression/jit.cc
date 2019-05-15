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
#include "harness.h"

using namespace cascade;

TEST(jit, initial) {
  run_code("minimal_jit", "data/test/regression/jit/initial.v", "once");
}
TEST(jit, pipeline_1) {
  run_code("minimal_jit", "data/test/regression/simple/pipeline_1.v", "0123456789");
}
TEST(jit, pipeline_2) {
  run_code("minimal_jit", "data/test/regression/simple/pipeline_2.v", "0123456789");
}
TEST(jit, array) {
  run_code("minimal_jit", "data/test/benchmark/array/run_5.v", "1048577\n");
}
TEST(jit, bitcoin) {
  run_code("minimal_jit", "data/test/benchmark/bitcoin/run_4.v", "f 93\n");
}
TEST(jit, mips32) {
  run_code("minimal_jit", "data/test/benchmark/mips32/run_bubble_128.v", "1");
}
TEST(jit, nw) {
  run_code("minimal", "data/test/benchmark/nw/run_4.v", "-1126");
}
TEST(jit, regex) {
  run_code("minimal_jit", "data/test/benchmark/regex/run_disjunct_1.v", "424");
}
