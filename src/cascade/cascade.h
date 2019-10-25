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

#ifndef CASCADE_SRC_CASCADE_CASCADE_H
#define CASCADE_SRC_CASCADE_CASCADE_H

#include <iostream>
#include <sstream>
#include <string>
#include "common/thread.h"
#include "runtime/ids.h"
#include "runtime/runtime.h"

namespace cascade {

class Cascade : public std::iostream {
  public:
    // Typedefs:
    typedef FId Fd;

    // Constructors:
    //
    // Only simple construciton is allowed. All other methods of construction
    // are explicitly forbidden.
    Cascade();
    Cascade(const Cascade& rhs) = delete;
    Cascade(Cascade&& rhs) = delete;
    Cascade& operator=(const Cascade& rhs) = delete;
    Cascade& operator=(Cascade&& rhs) = delete;
    ~Cascade();

    // Configuration Methods:
    //
    // These methods should only be called prior to the first invocation of
    // run.  Inoking any of these methods afterwards is undefined.
    Cascade& set_include_dirs(const std::string& path);
    Cascade& set_enable_inlining(bool enable);
    Cascade& set_open_loop_target(size_t n);
    Cascade& set_quartus_server(const std::string& host, size_t port);
    Cascade& set_profile_interval(size_t n);
    Cascade& set_stdin(std::streambuf* sb);
    Cascade& set_stdout(std::streambuf* sb);
    Cascade& set_stderr(std::streambuf* sb);
    Cascade& set_stdwarn(std::streambuf* sb);
    Cascade& set_stdinfo(std::streambuf* sb);
    Cascade& set_stdlog(std::streambuf* sb);

    // Stream Manipulation Methods:
    //
    // These methods should not be called while cascade is running
    Fd open(std::streambuf* sb);

    // Concurrency Methods:
    Cascade& run();
    Cascade& request_stop();
    Cascade& wait_for_stop();
    Cascade& stop_now();

    // Execution State:
    bool is_running() const;
    bool is_finished() const;

  private:
    class EvalLoop : public Thread {
      public:
        EvalLoop(Cascade* cascade);
        ~EvalLoop() override = default;
      private:
        Cascade* cascade_;
        void run_logic() override;
        void cin_loop();
        void generic_loop();
    };

    Runtime runtime_;
    EvalLoop eval_;
    std::stringbuf sb_;
    bool is_running_;

    bool is_cin() const;
    void halt_eval();
};

} // namespace cascade

#endif
