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

#include <fstream>
#include <string>
#include "src/base/thread/asynchronous.h"

namespace cascade {

class Controller;
class ManyView;
class RemoteRuntime;
class Runtime;
class View;

class Cascade {
  public:
    // Constructors:
    Cascade();
    Cascade(const Cascade& rhs) = delete;
    Cascade(Cascade&& rhs) = delete;
    Cascade& operator=(const Cascade& rhs) = delete;
    Cascade& operator=(Cascade&& rhs) = delete;
    ~Cascade();

    // Path Resolution Options:
    Cascade& set_include_path(const std::string& path);

    // Term UI Options:
    Cascade& attach_term_ui();

    // Web UI Options:
    Cascade& attach_web_ui();
    Cascade& set_web_ui_port(size_t port);
    Cascade& set_web_ui_buffer(size_t buffer);
    Cascade& set_web_ui_debug(bool debug);

    // Log Dump Options:
    Cascade& attach_logfile();

    // User-Defined View Options:
    Cascade& attach_view(View* v);

    // Logging Options:
    Cascade& enable_profile(size_t n);
    Cascade& enable_info(bool enable);
    Cascade& enable_warning(bool enable);
    Cascade& enable_error(bool enable);

    // Optimization Options:
    Cascade& enable_inlining(bool enable);
    Cascade& set_open_loop_target(size_t n);

    // Quartus Server Options:
    Cascade& set_quartus_host(const std::string& host);
    Cascade& set_quartus_port(size_t port);

    // Slave Runtime Options:
    Cascade& set_slave_mode(bool slave);
    Cascade& set_slave_port(size_t port);
    Cascade& set_slave_path(const std::string& path);

    // Start/Stop Methods:
    Cascade& run();
    Cascade& request_stop();
    Cascade& wait_for_stop();
    Cascade& stop_now();

    // Eval Methods:
    Cascade& eval(const std::string& s);

    // System Task Interface:
    Cascade& finish(size_t arg);
    Cascade& error(const std::string& s);

  private:
    // Profiler Helper Class:
    class Profiler : public Asynchronous {
      public:
        explicit Profiler(Runtime* rt, size_t interval);
        ~Profiler() = default;

      private:
        void run_logic() override;
        Runtime* rt_;
        size_t interval_;
    };
  
    // Master-Mode Components:
    ManyView* view_;
    Runtime* runtime_;
    Controller* controller_;

    // Slave-Mode Components:
    RemoteRuntime* remote_runtime_;

    // Logging Components:
    std::ofstream* logfile_;
    Profiler* profiler_;

    // Configuration Options:
    std::string include_path_;

    bool term_ui_;
    bool web_ui_;
    View* user_view_;
    bool log_;

    size_t web_ui_port_;
    size_t web_ui_buffer_;
    bool web_ui_debug_;

    size_t enable_profile_;
    bool enable_info_;
    bool enable_warning_;
    bool enable_error_;

    bool enable_inlining_;
    bool open_loop_target_;

    std::string quartus_host_;
    size_t quartus_port_;

    bool slave_;
    size_t slave_port_;
    std::string slave_path_;
};

} // namespace cascade
