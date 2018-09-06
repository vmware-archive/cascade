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

#ifndef CASCADE_TEST_HARNESS_H
#define CASCADE_TEST_HARNESS_H

#include <iostream>
#include <string>
#include "src/ui/view.h"

namespace cascade {

// A stripped-down view; filters everything other than print messages
class PView : public View {
  public:
    PView(std::ostream& os);
    ~PView() override = default;

    void print(const std::string& s) override;

  private:
    std::ostream& os_;
};

// Another stripped-down view; filters everything other than error messages
class EView : public View {
  public:
    EView();
    ~EView() override = default;

    bool error() const;
    void error(const std::string& s) override;

  private:
    bool error_;
};

// Harnesses for major components:
void run_parse(const std::string& path, bool expected);
void run_typecheck(const std::string& march, const std::string& path, bool expected);
void run_code(const std::string& march, const std::string& path, const std::string& expected);

// Benchmark harnesses:
void run_bitcoin(const std::string& march, const std::string& path, const std::string& expected);
void run_mips(const std::string& march, const std::string& path, const std::string& expected);
void run_regex(const std::string& march, const std::string& regex, const std::string& input, const std::string& expected);

} // namespace cascade

#endif
