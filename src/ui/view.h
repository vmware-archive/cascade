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

    // Display an error message
    virtual void error(const std::string& s);
    // Display a normal message
    virtual void print(const std::string& s);
    // Display a warning message
    virtual void warn(const std::string& s);

    // A module declaration was successfully eval'ed.
    virtual void eval_decl(const Program* p, const ModuleDeclaration* md);
    // A module item was successfully eval'ed.
    virtual void eval_item(const Program* p, const ModuleDeclaration* md);
};

inline void View::error(const std::string& s) {
  (void) s;
}

inline void View::print(const std::string& s) {
  (void) s;
}

inline void View::warn(const std::string& s) {
  (void) s;
}

inline void View::eval_decl(const Program* p, const ModuleDeclaration* md) {
  (void) md;
  (void) p;
}

inline void View::eval_item(const Program* p, const ModuleDeclaration* md) {
  (void) md;
  (void) p;
}

} // namespace cascade

#endif
