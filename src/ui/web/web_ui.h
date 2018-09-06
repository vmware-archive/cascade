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

#ifndef CASCADE_SRC_UI_WEB_WEB_UI_H
#define CASCADE_SRC_UI_WEB_WEB_UI_H

#include <mutex>
#include <string>
#include <vector>
#include "ext/mongoose/mongoose.h"
#include "src/ui/controller.h"
#include "src/ui/view.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class WebUi : public Controller, public View {
  public:
    WebUi(Runtime* rt);
    ~WebUi() override = default;

    WebUi& set_port(const std::string& port);
    WebUi& set_buffer(size_t buffer);
    WebUi& set_debug(bool debug);

    void error(const std::string& s) override;
    void print(const std::string& s) override;
    void warn(const std::string& s) override;

    void eval_decl(const Program* p, const ModuleDeclaration* md) override;
    void eval_item(const Program* p, const ModuleDeclaration* md) override;

    void send_index(struct mg_connection* nc, struct http_message* msg);
    void send_freq();
    void send_buffer(struct mg_connection* nc);
    void recv_eval(const std::string& s);

  private:
    std::string port_;
    size_t max_buffer_;
    bool debug_;

    // Document Root:
    std::string doc_root_;

    // Message Buffer:
    std::vector<std::string> buffer_;
    std::recursive_mutex lock_;
    size_t overflow_;

    // Mongoose State:
    struct mg_serve_http_opts opts_;

    // Asynchronous Interface:
    void run_logic() override;

    // Asynchronous Interface Helpers:
    void buffer(const std::string& api, const std::string& val, bool quote, bool force);
    void ok(const std::string& s);
};

} // namespace cascade

#endif
