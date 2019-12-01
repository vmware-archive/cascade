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

TEST(bitcoin, bitcoin_1) {
  run_code("regression/minimal","share/test/regression/bitcoin/bitcoin_1.v", "44420006\nccca888e\n999abbb8\ndddefffc\n");
}
TEST(bitcoin, bitcoin_2) {
  run_code("regression/minimal","share/test/regression/bitcoin/bitcoin_2.v", "ac62db15\n428c35fb\n5eb9e502\n29ce9275\n");
}
TEST(bitcoin, bitcoin_3) {
  run_code("regression/minimal","share/test/regression/bitcoin/bitcoin_3.v", "5abb9899\n3effdcdd\n00801191\n22a233b3\n");
}
TEST(bitcoin, bitcoin_4) {
  run_code("regression/minimal","share/test/regression/bitcoin/bitcoin_4.v", "3e90be27\n943e50c9\nca20d79b\n9f57a0ec\n");
}
TEST(bitcoin, bitcoin_5) {
  run_code("regression/minimal","share/test/regression/bitcoin/bitcoin_5.v", "99b95531\n01654565\ncdedcda9\nfefc7630\n");
}
TEST(bitcoin, bitcoin_6) {
  run_code("regression/minimal","share/test/regression/bitcoin/bitcoin_6.v", "111399df\n01474547\nfcdcfc98\nfefc7630\n");
}
TEST(bitcoin, bitcoin_7) {
  run_code("regression/minimal","share/test/regression/bitcoin/bitcoin_7.v", 
    "bffb8e7e13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf13579bdf 13579bdf13579bdf13579bdf0d18e67013579bdf13579bdf13579bdf515ae676\n"
    "d7cb434202468ace02468ace02468ace02468ace02468ace02468ace02468ace02468ace02468ace02468ace02468ace02468ace02468ace02468ace02468ace 02468ace02468ace02468ace4decec0102468ace02468ace02468ace1ab7748f\n"
    "c85a5e5cfedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98fedcba98 fedcba98fedcba98fedcba98590989fafedcba98fedcba98fedcba98f2a445b2\n"
    "aea238bf765432107654321076543210765432107654321076543210765432107654321076543210765432107654321076543210765432107654321076543210 76543210765432107654321079738cc576543210765432107654321057528cc1\n"
  );
}
TEST(bitcoin, bitcoin_8) {
  run_code("regression/minimal","share/test/regression/bitcoin/bitcoin_8.v",
    "c14c2700d3d3a04cb44cf5fb4c63b9596927165926ee25dcae5e4a85a5487431\n"
    "61c541a57d4923f315954ac98a12f7a7b3601686996861f84c5fe80c327a1885\n"
    "9699250944f14d01c4bba0700a2576f738cdb044b7208935a696fd931370187a\n"
    "45c43ebfcdabdef4bb717ad33e1e1ea2b95ab32743aecaa9864a57390522724b\n"
  );
}
TEST(bitcoin, run_4) {
  run_code("regression/minimal","share/test/benchmark/bitcoin/run_4.v", "0000000f 00000093\n");
}
