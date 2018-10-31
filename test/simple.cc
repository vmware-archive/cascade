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
#include "test/harness.h"

using namespace cascade;

TEST(simple, arithmetic_divide) {
  run_code("minimal","data/test/simple/arithmetic_divide.v", "2"); 
}
TEST(simple, arithmetic_minus) {
  run_code("minimal","data/test/simple/arithmetic_minus.v", "0"); 
}
TEST(simple, arithmetic_mod) {
  run_code("minimal","data/test/simple/arithmetic_mod.v", "3"); 
}
TEST(simple, arithmetic_multiply) {
  run_code("minimal","data/test/simple/arithmetic_multiply.v", "56"); 
}
TEST(simple, arithmetic_plus) {
  run_code("minimal","data/test/simple/arithmetic_plus.v", "12"); 
}
TEST(simple, arithmetic_pow) {
  run_code("minimal","data/test/simple/arithmetic_pow.v", "16"); 
}
TEST(simple, array_1) {
  run_code("minimal","data/test/simple/array_1.v", "0123");
}
TEST(simple, assign_1) {
  run_code("minimal","data/test/simple/assign_1.v", "1");
}
TEST(simple, assign_2) {
  run_code("minimal","data/test/simple/assign_2.v", "1");
}
TEST(simple, assign_3) {
  run_code("minimal","data/test/simple/assign_3.v", "1");
}
TEST(simple, assign_4) {
  run_code("minimal","data/test/simple/assign_4.v", "1");
}
TEST(simple, assign_5) {
  run_code("minimal","data/test/simple/assign_5.v", "1");
}
TEST(simple, assign_6) {
  run_code("minimal","data/test/simple/assign_6.v", "1");
}
TEST(simple, assign_7) {
  run_code("minimal","data/test/simple/assign_7.v", "170");
}
TEST(simple, bitwise_and) {
  run_code("minimal","data/test/simple/bitwise_and.v", "1");
}
TEST(simple, bitwise_or) {
  run_code("minimal","data/test/simple/bitwise_or.v", "7");
}
TEST(simple, bitwise_sll) {
  run_code("minimal","data/test/simple/bitwise_sll.v", "6");
}
TEST(simple, bitwise_slr) {
  run_code("minimal","data/test/simple/bitwise_slr.v", "1");
}
TEST(simple, bitwise_xnor) {
  run_code("minimal","data/test/simple/bitwise_xnor.v", "9");
}
TEST(simple, bitwise_not) {
  run_code("minimal","data/test/simple/bitwise_not.v", "12");
}
TEST(simple, bitwise_xor) {
  run_code("minimal","data/test/simple/bitwise_xor.v", "6");
}
TEST(simple, case_1) {
  run_code("minimal","data/test/simple/case_1.v", "yes");
}
TEST(simple, case_2) {
  run_code("minimal","data/test/simple/case_2.v", "yes");
}
TEST(simple, case_3) {
  run_code("minimal","data/test/simple/case_3.v", "123");
}
TEST(simple, concat_1) {
  run_code("minimal","data/test/simple/concat_1.v", "170");
}
TEST(simple, concat_2) {
  run_code("minimal","data/test/simple/concat_2.v", "170");
}
TEST(simple, concat_3) {
  run_code("minimal","data/test/simple/concat_3.v", "ffffffffffffffff");
}
TEST(simple, cond_1) {
  run_code("minimal","data/test/simple/cond_1.v", "123");
}
TEST(simple, fifo_1) {
  run_code("minimal","data/test/simple/fifo_1.v", "1000000001100200300410");
}
TEST(simple, fifo_2) {
  run_code("minimal","data/test/simple/fifo_2.v", "1001110");
}
TEST(simple, fifo_3) {
  run_code("minimal","data/test/simple/fifo_3.v", "1001101201301410");
}
TEST(simple, fifo_4) {
  run_code("minimal","data/test/simple/fifo_4.v", "45");
}
TEST(simple, fifo_5) {
  run_code("minimal","data/test/simple/fifo_5.v", "90");
}
TEST(simple, finish_1) {
  run_code("minimal","data/test/simple/finish_1.v", "Hello World");
}
//TEST(simple, for_1) {
//  run_code("minimal","data/test/simple/for_1.v", "333");
//}
TEST(simple, generate_1) {
  run_code("minimal","data/test/simple/generate_1.v", "01234567");
}
TEST(simple, generate_2) {
  run_code("minimal","data/test/simple/generate_2.v", "32");
}
TEST(simple, generate_3) {
  run_code("minimal","data/test/simple/generate_3.v", "1357");
}
TEST(simple, hello_1) {
  run_code("minimal","data/test/simple/hello_1.v", "Hello World");
}
TEST(simple, hello_2) {
  run_code("minimal","data/test/simple/hello_2.v", "Hello World");
}
TEST(simple, hello_3) {
  run_code("minimal","data/test/simple/hello_3.v", "Hello World");
}
TEST(simple, inst_1) {
  run_code("minimal","data/test/simple/inst_1.v", "");
}
TEST(simple, inst_2) {
  run_code("minimal","data/test/simple/inst_2.v", "2");
}
TEST(simple, inst_3) {
  run_code("minimal","data/test/simple/inst_3.v", "1");
}
TEST(simple, issue_228) {
  run_code("minimal","data/test/simple/issue_228.v", "");
}
TEST(simple, logical_and) {
  run_code("minimal","data/test/simple/logical_and.v", "011");
}
TEST(simple, logical_eq) {
  run_code("minimal","data/test/simple/logical_eq.v", "01");
}
TEST(simple, logical_gt) {
  run_code("minimal","data/test/simple/logical_gt.v", "10");
}
TEST(simple, logical_gte) {
  run_code("minimal","data/test/simple/logical_gte.v", "10");
}
TEST(simple, logical_lt) {
  run_code("minimal","data/test/simple/logical_lt.v", "10");
}
TEST(simple, logical_lte) {
  run_code("minimal","data/test/simple/logical_lte.v", "10");
}
TEST(simple, logical_ne) {
  run_code("minimal","data/test/simple/logical_ne.v", "10");
}
TEST(simple, logical_not) {
  run_code("minimal","data/test/simple/logical_not.v", "01");
}
TEST(simple, logical_or) {
  run_code("minimal","data/test/simple/logical_or.v", "110");
}
TEST(simple, mem_1) {
  run_code("minimal","data/test/simple/mem_1.v", "0011223344556677");
}
TEST(simple, mem_2) {
  run_code("minimal","data/test/simple/mem_2.v", "01234567");
}
TEST(simple, nested_1) {
  run_code("minimal","data/test/simple/nested_1.v", "8");
}
TEST(simple, nonblock_1) {
  run_code("minimal","data/test/simple/nonblock_1.v", "01");
}
TEST(simple, nonblock_2) {
  run_code("minimal","data/test/simple/nonblock_2.v", "3");
}
TEST(simple, nonblock_3) {
  run_code("minimal","data/test/simple/nonblock_3.v", "0 1 2 4 8 ");
}
TEST(simple, pipeline_1) {
  run_code("minimal","data/test/simple/pipeline_1.v", "0123456789");
}
TEST(simple, pipeline_2) {
  run_code("minimal","data/test/simple/pipeline_2.v", "0123456789");
}
TEST(simple, precedence) {
  run_code("minimal","data/test/simple/precedence.v", "7");
}
TEST(simple, range_1) {
  run_code("minimal","data/test/simple/range_1.v", "7");
}
TEST(simple, range_2) {
  run_code("minimal","data/test/simple/range_2.v", "7");
}
TEST(simple, range_3) {
  run_code("minimal","data/test/simple/range_3.v", "7");
}
TEST(simple, reduce_and) {
  run_code("minimal","data/test/simple/reduce_and.v", "10");
}
TEST(simple, reduce_nand) {
  run_code("minimal","data/test/simple/reduce_nand.v", "01");
}
TEST(simple, reduce_or) {
  run_code("minimal","data/test/simple/reduce_or.v", "10");
}
TEST(simple, reduce_nor) {
  run_code("minimal","data/test/simple/reduce_nor.v", "01");
}
TEST(simple, reduce_xor) {
  run_code("minimal","data/test/simple/reduce_xor.v", "10");
}
TEST(simple, reduce_xnor) {
  run_code("minimal","data/test/simple/reduce_xnor.v", "01");
}
//TEST(simple, repeat_1) {
//  run_code("minimal","data/test/simple/repeat_1.v", "122");
//}
//TEST(simple, repeat_2) {
//  run_code("minimal","data/test/simple/repeat_2.v", "666666");
//}
TEST(simple, sign_1) {
  run_code("minimal","data/test/simple/sign_1.v", "-41431655761-416553221841143165576165532-41");
}
TEST(simple, sign_2) {
  run_code("minimal","data/test/simple/sign_2.v", "000");
}
//TEST(simple, wait_1) {
//  run_code("minimal","data/test/simple/wait_1.v", "Hello World");
//}
//TEST(simple, while_1) {
//  run_code("minimal","data/test/simple/while_1.v", "333");
//}
