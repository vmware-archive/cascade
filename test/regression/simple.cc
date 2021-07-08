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

TEST(simple, arithmetic_divide) {
  run_code("regression/minimal","share/cascade/test/regression/simple/arithmetic_divide.v", "2"); 
}
TEST(simple, arithmetic_minus) {
  run_code("regression/minimal","share/cascade/test/regression/simple/arithmetic_minus.v", "0"); 
}
TEST(simple, arithmetic_mod) {
  run_code("regression/minimal","share/cascade/test/regression/simple/arithmetic_mod.v", "3"); 
}
TEST(simple, arithmetic_multiply) {
  run_code("regression/minimal","share/cascade/test/regression/simple/arithmetic_multiply.v", "56"); 
}
TEST(simple, arithmetic_plus) {
  run_code("regression/minimal","share/cascade/test/regression/simple/arithmetic_plus.v", "12"); 
}
TEST(simple, arithmetic_pow) {
  run_code("regression/minimal","share/cascade/test/regression/simple/arithmetic_pow.v", "16"); 
}
TEST(simple, array_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/array_1.v", "0123");
}
TEST(simple, array_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/array_2.v", "0255");
}
TEST(simple, array_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/array_3.v", "10");
}
TEST(simple, array_4) {
  run_code("regression/minimal","share/cascade/test/regression/simple/array_4.v", "2550");
}
TEST(simple, assign_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/assign_1.v", "1");
}
TEST(simple, assign_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/assign_2.v", "1");
}
TEST(simple, assign_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/assign_3.v", "1");
}
TEST(simple, assign_4) {
  run_code("regression/minimal","share/cascade/test/regression/simple/assign_4.v", "1");
}
TEST(simple, assign_5) {
  run_code("regression/minimal","share/cascade/test/regression/simple/assign_5.v", "1");
}
TEST(simple, assign_6) {
  run_code("regression/minimal","share/cascade/test/regression/simple/assign_6.v", "1");
}
TEST(simple, assign_7) {
  run_code("regression/minimal","share/cascade/test/regression/simple/assign_7.v", "170");
}
TEST(simple, bitwise_and) {
  run_code("regression/minimal","share/cascade/test/regression/simple/bitwise_and.v", "1");
}
TEST(simple, bitwise_or) {
  run_code("regression/minimal","share/cascade/test/regression/simple/bitwise_or.v", "7");
}
TEST(simple, bitwise_sll) {
  run_code("regression/minimal","share/cascade/test/regression/simple/bitwise_sll.v", "6");
}
TEST(simple, bitwise_slr) {
  run_code("regression/minimal","share/cascade/test/regression/simple/bitwise_slr.v", "1");
}
TEST(simple, bitwise_xnor) {
  run_code("regression/minimal","share/cascade/test/regression/simple/bitwise_xnor.v", "9");
}
TEST(simple, bitwise_not) {
  run_code("regression/minimal","share/cascade/test/regression/simple/bitwise_not.v", "12");
}
TEST(simple, bitwise_xor) {
  run_code("regression/minimal","share/cascade/test/regression/simple/bitwise_xor.v", "6");
}
TEST(simple, case_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/case_1.v", "yes");
}
TEST(simple, case_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/case_2.v", "yes");
}
TEST(simple, case_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/case_3.v", "123");
}
TEST(simple, concat_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/concat_1.v", "170");
}
TEST(simple, concat_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/concat_2.v", "170");
}
TEST(simple, concat_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/concat_3.v", "ffffffffffffffff");
}
TEST(simple, cond_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/cond_1.v", "123");
}
TEST(simple, declaration_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/declaration_1.v", "8");
}
TEST(simple, define_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/define_1.v", "22");
}
TEST(simple, fifo_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/fifo_1.v", "1000000001100200300410");
}
TEST(simple, finish_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/finish_1.v", "Hello World");
}
TEST(simple, for_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/for_1.v", "333");
}
TEST(simple, for_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/for_2.v", "012458");
}
TEST(simple, generate_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/generate_1.v", "01234567");
}
TEST(simple, generate_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/generate_2.v", "32");
}
TEST(simple, generate_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/generate_3.v", "1357");
}
TEST(simple, generate_4) {
  run_code("regression/minimal","share/cascade/test/regression/simple/generate_4.v", "10");
}
TEST(simple, hello_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/hello_1.v", "Hello World");
}
TEST(simple, hello_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/hello_2.v", "Hello World");
}
TEST(simple, hello_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/hello_3.v", "Hello World");
}
TEST(simple, hex) {
  run_code("regression/minimal","share/cascade/test/regression/simple/hex.v", "dead\ndead\n");
}
TEST(simple, ifdef_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/ifdef_1.v", "1234567");
}
TEST(simple, include_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/include_1.v", "once");
}
TEST(simple, inst_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/inst_1.v", "");
}
TEST(simple, inst_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/inst_2.v", "2");
}
TEST(simple, inst_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/inst_3.v", "1");
}
TEST(simple, io_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/io_1.v", "1234512345");
}
TEST(simple, io_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/io_2.v", "ffff -1 c Hello 5.55");
}
TEST(simple, io_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/io_3.v", "97.00aa97-97");
}
TEST(simple, io_4) {
  run_code("regression/minimal","share/cascade/test/regression/simple/io_4.v", "32 65535 -1");
}
TEST(simple, issue_20a) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_20a.v", "");
}
TEST(simple, issue_41a) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_41a.v", "-1");
}
TEST(simple, issue_41b) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_41b.v", "-4");
}
TEST(simple, issue_47a) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_47a.v", "00254");
}
TEST(simple, issue_47b) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_47b.v", "00254");
}
TEST(simple, issue_47c) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_47c.v", "000");
}
TEST(simple, issue_47d) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_47d.v", "000");
}
TEST(simple, issue_54a) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_54a.v", "1");
}
TEST(simple, issue_54b) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_54b.v", "2");
}
TEST(simple, issue_54c) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_54c.v", "00000000");
}
TEST(simple, issue_81a) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_81a.v", "0123");
}
TEST(simple, issue_81b) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_81b.v", "0123");
}
TEST(simple, issue_152) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_152.v", "7bcb43d769f764 >> 86 = 00000000000000\n");
}
TEST(simple, issue_195) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_195.v", "012012");
}
TEST(simple, issue_228) {
  run_code("regression/minimal","share/cascade/test/regression/simple/issue_228.v", "");
}
TEST(simple, logical_and) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_and.v", "011");
}
TEST(simple, logical_eq) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_eq.v", "01");
}
TEST(simple, logical_gt) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_gt.v", "10");
}
TEST(simple, logical_gte) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_gte.v", "10");
}
TEST(simple, logical_lt) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_lt.v", "10");
}
TEST(simple, logical_lte) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_lte.v", "10");
}
TEST(simple, logical_ne) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_ne.v", "10");
}
TEST(simple, logical_not) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_not.v", "01");
}
TEST(simple, logical_or) {
  run_code("regression/minimal","share/cascade/test/regression/simple/logical_or.v", "110");
}
TEST(simple, mem_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/mem_1.v", "0011223344556677");
}
TEST(simple, mem_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/mem_2.v", "0001020304050607");
}
TEST(simple, nested_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/nested_1.v", "8");
}
TEST(simple, nonblock_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/nonblock_1.v", "01");
}
TEST(simple, nonblock_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/nonblock_2.v", "3");
}
TEST(simple, nonblock_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/nonblock_3.v", "0 1 2 4 8 ");
}
TEST(simple, pipeline_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/pipeline_1.v", "0123456789");
}
TEST(simple, pipeline_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/pipeline_2.v", "0123456789");
}
TEST(simple, precedence) {
  run_code("regression/minimal","share/cascade/test/regression/simple/precedence.v", "7");
}
TEST(simple, range_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/range_1.v", "7");
}
TEST(simple, range_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/range_2.v", "7");
}
TEST(simple, range_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/range_3.v", "7");
}
TEST(simple, real_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/real_1.v", "269488144269488144");
}
TEST(simple, real_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/real_2.v", "11111111111");
}
TEST(simple, reduce_and) {
  run_code("regression/minimal","share/cascade/test/regression/simple/reduce_and.v", "10");
}
TEST(simple, reduce_nand) {
  run_code("regression/minimal","share/cascade/test/regression/simple/reduce_nand.v", "01");
}
TEST(simple, reduce_or) {
  run_code("regression/minimal","share/cascade/test/regression/simple/reduce_or.v", "10");
}
TEST(simple, reduce_nor) {
  run_code("regression/minimal","share/cascade/test/regression/simple/reduce_nor.v", "01");
}
TEST(simple, reduce_xor) {
  run_code("regression/minimal","share/cascade/test/regression/simple/reduce_xor.v", "10");
}
TEST(simple, reduce_xnor) {
  run_code("regression/minimal","share/cascade/test/regression/simple/reduce_xnor.v", "01");
}
TEST(simple, repeat_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/repeat_1.v", "122");
}
TEST(simple, repeat_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/repeat_2.v", "666666");
}
TEST(simple, repeat_3) {
  run_code("regression/minimal","share/cascade/test/regression/simple/repeat_3.v", "999999999");
}
TEST(simple, seq_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/seq_1.v", "12");
}
TEST(simple, sign_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/sign_1.v", "-41431655761-416553221841143165576165532-41");
}
TEST(simple, sign_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/sign_2.v", "0000000000");
}
TEST(simple, string) {
  run_code("regression/minimal","share/cascade/test/regression/simple/string.v", "   Hello world is stored as 00000048656c6c6f20776f726c64\nHello world!!! is stored as 48656c6c6f20776f726c64212121\n");
}
TEST(simple, while_1) {
  run_code("regression/minimal","share/cascade/test/regression/simple/while_1.v", "333");
}
TEST(simple, while_2) {
  run_code("regression/minimal","share/cascade/test/regression/simple/while_2.v", "012345");
}
