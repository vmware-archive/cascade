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

TEST(type_check, pass_array_1) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/array_1.v", false);
}
TEST(type_check, pass_array_2) {
// TODO: UNCOMMENT THIS WHEN WE ACTIVATE SUPPORT FOR INSTANTIATION ARRAYS
//  run_typecheck("minimal", "data/test/regression/type_check/pass/array_2.v", false);
}
TEST(type_check, pass_array_3) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/array_3.v", false);
}
TEST(type_check, pass_array_4) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/array_4.v", false);
}
TEST(type_check, pass_array_5) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/array_5.v", false);
}
TEST(type_check, pass_declaration_1) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/declaration_1.v", false);
}
TEST(type_check, pass_generate_1) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/generate_1.v", false);
}
TEST(type_check, pass_generate_2) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/generate_2.v", false);
}
TEST(type_check, pass_generate_3) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/generate_3.v", false);
}
TEST(type_check, pass_generate_4) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/generate_4.v", false);
}
TEST(type_check, pass_instantiation_1) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/instantiation_1.v", false);
}
TEST(type_check, pass_instantiation_2) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/instantiation_2.v", false);
}
TEST(type_check, pass_instantiation_3) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/instantiation_3.v", false);
}
TEST(type_check, pass_instantiation_4) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/instantiation_4.v", false);
}
TEST(type_check, pass_issue_4) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/issue_4.v", false);
}
TEST(type_check, pass_issue_14) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/issue_14.v", false);
}
TEST(type_check, pass_issue_23a) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/issue_23a.v", false);
}
TEST(type_check, pass_issue_23b) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/issue_23b.v", false);
}
TEST(type_check, pass_issue_23c) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/issue_23c.v", false);
}
TEST(type_check, pass_resolution_1) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_1.v", false);
}
TEST(type_check, pass_resolution_2) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_2.v", false);
}
TEST(type_check, pass_resolution_3) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_3.v", false);
}
TEST(type_check, pass_resolution_4) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_4.v", false);
}
TEST(type_check, pass_resolution_5) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_5.v", false);
}
TEST(type_check, pass_resolution_6) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_6.v", false);
}
TEST(type_check, pass_resolution_7) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_7.v", false);
}
TEST(type_check, pass_resolution_8) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_8.v", false);
}
TEST(type_check, pass_resolution_9) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_9.v", false);
}
TEST(type_check, pass_resolution_10) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_10.v", false);
}
TEST(type_check, pass_resolution_11) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_11.v", false);
}
TEST(type_check, pass_resolution_12) {
  run_typecheck("minimal", "data/test/regression/type_check/pass/resolution_12.v", false);
}

TEST(type_check, fail_array_1) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_1.v", true);
}
TEST(type_check, fail_array_2) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_2.v", true);
}
TEST(type_check, fail_array_3) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_3.v", true);
}
TEST(type_check, fail_array_4) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_4.v", true);
}
TEST(type_check, fail_array_5) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_5.v", true);
}
TEST(type_check, fail_array_6) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_6.v", true);
}
TEST(type_check, fail_array_7) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_7.v", true);
}
TEST(type_check, fail_array_8) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_8.v", true);
}
TEST(type_check, fail_array_9) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_9.v", true);
}
TEST(type_check, fail_array_10) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/array_10.v", true);
}
TEST(type_check, fail_assign_1) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/assign_1.v", true);
}
TEST(type_check, fail_assign_2) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/assign_2.v", true);
}
TEST(type_check, fail_assign_3) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/assign_3.v", true);
}
TEST(type_check, fail_declaration_1) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/declaration_1.v", true);
}
TEST(type_check, fail_declaration_2) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/declaration_2.v", true);
}
TEST(type_check, fail_declaration_3) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/declaration_3.v", true);
}
TEST(type_check, fail_declaration_4) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/declaration_4.v", true);
}
TEST(type_check, fail_declaration_5) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/declaration_5.v", true);
}
TEST(type_check, fail_declaration_6) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/declaration_6.v", true);
}
TEST(type_check, fail_declaration_7) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/declaration_7.v", true);
}
TEST(type_check, fail_generate_1) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/generate_1.v", true);
}
TEST(type_check, fail_generate_2) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/generate_2.v", true);
}
TEST(type_check, fail_generate_3) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/generate_3.v", true);
}
TEST(type_check, fail_generate_4) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/generate_4.v", true);
}
TEST(type_check, fail_hierarchical_1) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/hierarchical_1.v", true);
}
TEST(type_check, fail_instantiation_1) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/instantiation_1.v", true);
}
TEST(type_check, fail_instantiation_2) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/instantiation_2.v", true);
}
TEST(type_check, fail_instantiation_3) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/instantiation_3.v", true);
}
TEST(type_check, fail_instantiation_4) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/instantiation_4.v", true);
}
TEST(type_check, fail_instantiation_5) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/instantiation_5.v", true);
}
TEST(type_check, fail_instantiation_6) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/instantiation_6.v", true);
}
TEST(type_check, fail_instantiation_7) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/instantiation_7.v", true);
}
TEST(type_check, fail_issue_13a) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_13a.v", true);
}
TEST(type_check, fail_issue_13b) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_13b.v", true);
}
TEST(type_check, fail_issue_23a) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_23a.v", true);
}
TEST(type_check, fail_issue_23b) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_23b.v", true);
}
TEST(type_check, fail_issue_23c) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_23c.v", true);
}
TEST(type_check, fail_issue_30c) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_30c.v", true);
}
TEST(type_check, fail_issue_30d) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_30d.v", true);
}
TEST(type_check, fail_issue_30e) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_30e.v", true);
}
TEST(type_check, fail_issue_226) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_226.v", true);
}
TEST(type_check, fail_issue_239a) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_239a.v", true);
}
TEST(type_check, fail_issue_239b) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_239b.v", true);
}
TEST(type_check, fail_issue_239c) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_239c.v", true);
}
TEST(type_check, fail_issue_239d) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/issue_239d.v", true);
}
TEST(type_check, fail_parameter_1) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/parameter_1.v", true);
}
TEST(type_check, fail_parameter_2) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/parameter_2.v", true);
}
TEST(type_check, fail_resolution_1) {
  run_typecheck("minimal", "data/test/regression/type_check/fail/resolution_1.v", true);
}
