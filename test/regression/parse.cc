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

TEST(parse, pass_and) {
  run_parse("data/test/regression/parse/pass/and.v", false);
}
TEST(parse, pass_aoo) {
  run_parse("data/test/regression/parse/pass/aoo.v", false);
}
TEST(parse, pass_assign_1) {
  run_parse("data/test/regression/parse/pass/assign_1.v", false);
}
TEST(parse, pass_assign_statement) {
  run_parse("data/test/regression/parse/pass/assign_statement.v", false);
}
TEST(parse, pass_attributes) {
  run_parse("data/test/regression/parse/pass/attributes.v", false);
}
TEST(parse, pass_case_statement) {
  run_parse("data/test/regression/parse/pass/case_statement.v", false);
}
TEST(parse, pass_conditional_statement) {
  run_parse("data/test/regression/parse/pass/conditional_statement.v", false);
}
TEST(parse, pass_construct) {
  run_parse("data/test/regression/parse/pass/construct.v", false);
}
TEST(parse, pass_continuous_assign) {
  run_parse("data/test/regression/parse/pass/continuous_assign.v", false);
}
TEST(parse, pass_declaration) {
  run_parse("data/test/regression/parse/pass/declaration.v", false);
}
TEST(parse, pass_issue_6) {
  run_parse("data/test/regression/parse/pass/issue_6.v", false);
}
TEST(parse, pass_issue_224) {
  run_parse("data/test/regression/parse/pass/issue_224.v", false);
}
TEST(parse, pass_loop_statement) {
  run_parse("data/test/regression/parse/pass/loop_statement.v", false);
}
TEST(parse, pass_module_declaration_1) {
  run_parse("data/test/regression/parse/pass/module_declaration_1.v", false);
}
TEST(parse, pass_module_declaration_2) {
  run_parse("data/test/regression/parse/pass/module_declaration_2.v", false);
}
TEST(parse, pass_module_declaration_3) {
  run_parse("data/test/regression/parse/pass/module_declaration_3.v", false);
}
TEST(parse, pass_module_declaration_4) {
  run_parse("data/test/regression/parse/pass/module_declaration_4.v", false);
}
TEST(parse, pass_module_declaration_5) {
  run_parse("data/test/regression/parse/pass/module_declaration_5.v", false);
}
TEST(parse, pass_module_declaration_6) {
  run_parse("data/test/regression/parse/pass/module_declaration_6.v", false);
}
TEST(parse, pass_module_declaration_7) {
  run_parse("data/test/regression/parse/pass/module_declaration_7.v", false);
}
TEST(parse, pass_module_declaration_8) {
  run_parse("data/test/regression/parse/pass/module_declaration_8.v", false);
}
TEST(parse, pass_port_declaration) {
  run_parse("data/test/regression/parse/pass/port_declaration.v", false);
}
TEST(parse, pass_port_list) {
  run_parse("data/test/regression/parse/pass/port_list.v", false);
}
TEST(parse, pass_system_task) {
  run_parse("data/test/regression/parse/pass/system_task.v", false);
}

TEST(parse, fail_assign_1) {
  run_parse("data/test/regression/parse/fail/assign_1.v", true);
}
TEST(parse, fail_missing_endmodule) {
  run_parse("data/test/regression/parse/fail/missing_endmodule.v", true);
}
TEST(parse, fail_module_declaration_1) {
  run_parse("data/test/regression/parse/fail/module_declaration_1.v", true);
}
TEST(parse, fail_module_declaration_2) {
  run_parse("data/test/regression/parse/fail/module_declaration_2.v", true);
}
