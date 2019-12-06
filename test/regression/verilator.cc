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
#include "test/harness.h"

using namespace cascade;

TEST(verilator32, array) {
  run_code("regression/verilator32", "share/cascade/test/benchmark/array/run_4.v", "65537\n");
}
TEST(verilator32, bitcoin) {
  run_code("regression/verilator32", "share/cascade/test/benchmark/bitcoin/run_11.v", "00000253 000002d7\n");
}
TEST(verilator32, mips32) {
  run_code("regression/verilator32", "share/cascade/test/benchmark/mips32/run_bubble_32.v", "1");
}
TEST(verilator32, nw) {
  run_code("regression/verilator32", "share/cascade/test/benchmark/nw/run_4.v", "-1126");
}
TEST(verilator32, regex) {
  run_code("regression/verilator32", "share/cascade/test/benchmark/regex/run_disjunct_abridged_1.v", "38");
}

TEST(verilator64, array) {
  run_code("regression/verilator64", "share/cascade/test/benchmark/array/run_4.v", "65537\n");
}
TEST(verilator64, bitcoin) {
  run_code("regression/verilator64", "share/cascade/test/benchmark/bitcoin/run_11.v", "00000253 000002d7\n");
}
TEST(verilator64, mips32) {
  run_code("regression/verilator64", "share/cascade/test/benchmark/mips32/run_bubble_32.v", "1");
}
TEST(verilator64, nw) {
  run_code("regression/verilator64", "share/cascade/test/benchmark/nw/run_4.v", "-1126");
}
TEST(verilator64, regex) {
  run_code("regression/verilator64", "share/cascade/test/benchmark/regex/run_disjunct_abridged_1.v", "38");
}
