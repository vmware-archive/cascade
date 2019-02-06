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

#ifndef CASCADE_SRC_UI_VIEW_H
#define CASCADE_SRC_UI_VIEW_H

#include <string>
#include "src/verilog/ast/ast_fwd.h"

namespace cascade {

// At the top level, FPGA-JIT is organized according to the MVC design pattern.
// The user interacts with the program through the controller, observes the
// results of those interactions through the view, and all major state is
// stored in the runtime (ie the model). The view is a passive object, which is
// manipulated by either or both of the controller and/or runtime.

// There are no requirements on the implementation of a view. It can respond to
// all or none of the methods in its api.

class Program;

class View {
  public:
    virtual ~View() = default;

    // Startup hook
    virtual void startup(size_t t);
    // Shutdown hook
    virtual void shutdown(size_t t);

    // Display a normal message
    virtual void print(size_t t, const std::string& s);
    // Display an info message
    virtual void info(size_t t, const std::string& s);
    // Display a warning message
    virtual void warn(size_t t, const std::string& s);
    // Display an error message
    virtual void error(size_t t, const std::string& s);

    // A string was parsed
    virtual void parse(size_t t, size_t d, const std::string& s);
    // An include path was resolved
    virtual void include(size_t t, const std::string& s);
    // A module declaration was successfully eval'ed.
    virtual void decl(size_t t, const Program* p, const ModuleDeclaration* md);
    // A module item was successfully eval'ed.
    virtual void item(size_t t, const Program* p, const ModuleDeclaration* md);

    // The program crashed --- everything has gone up in flames
    virtual void crash();
};

inline void View::startup(size_t t) {
  (void) t;
}

inline void View::shutdown(size_t t) {
  (void) t;
}

inline void View::print(size_t t, const std::string& s) {
  (void) t;
  (void) s;
}

inline void View::info(size_t t, const std::string& s) {
  (void) t;
  (void) s;
}

inline void View::warn(size_t t, const std::string& s) {
  (void) t;
  (void) s;
}

inline void View::error(size_t t, const std::string& s) {
  (void) t;
  (void) s;
}

inline void View::parse(size_t t, size_t d, const std::string& s) {
  (void) t;
  (void) d;
  (void) s;
}

inline void View::include(size_t t, const std::string& s) {
  (void) t;
  (void) s;
}

inline void View::decl(size_t t, const Program* p, const ModuleDeclaration* md) {
  (void) t;
  (void) p;
  (void) md;
}

inline void View::item(size_t t, const Program* p, const ModuleDeclaration* md) {
  (void) t;
  (void) p;
  (void) md;
}

inline void View::crash() {
  // Does nothing.
}

} // namespace cascade

#endif
