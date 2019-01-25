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

#ifndef CASCADE_SRC_UI_COMBINATOR_MANY_VIEW_H
#define CASCADE_SRC_UI_COMBINATOR_MANY_VIEW_H

#include <vector>
#include "src/ui/view.h"

namespace cascade {

class ManyView : public View {
  public:
    ManyView();
    ~ManyView() override;

    void attach(View* v);

    void startup(size_t t) override;
    void shutdown(size_t t) override;

    void error(size_t t, const std::string& s) override;
    void print(size_t t, const std::string& s) override;
    void warn(size_t t, const std::string& s) override;

    void parse(size_t t, const std::string& s) override;
    void eval_decl(size_t t, const Program* p, const ModuleDeclaration* md) override;
    void eval_item(size_t t, const Program* p, const ModuleDeclaration* md) override;

    void crash() override;

  private:
    std::vector<View*> views_;
};

inline ManyView::ManyView() : View() { }

inline ManyView::~ManyView() {
  for (auto* v : views_) {
    delete v;
  }
}

inline void ManyView::attach(View* v) {
  views_.push_back(v);
}

inline void ManyView::startup(size_t t) {
  for (auto* v : views_) {
    v->startup(t);
  }
}

inline void ManyView::shutdown(size_t t) {
  for (auto* v : views_) {
    v->shutdown(t);
  }
}

inline void ManyView::error(size_t t, const std::string& s) {
  for (auto* v : views_) {
    v->error(t, s);
  }
}

inline void ManyView::print(size_t t, const std::string& s) {
  for (auto* v : views_) {
    v->print(t, s);
  }
}

inline void ManyView::warn(size_t t, const std::string& s) {
  for (auto* v : views_) {
    v->warn(t, s);
  }
}

inline void ManyView::parse(size_t t, const std::string& s) {
  for (auto* v : views_) {
    v->parse(t, s);
  }
}

inline void ManyView::eval_decl(size_t t, const Program* p, const ModuleDeclaration* md) {
  for (auto* v : views_) {
    v->eval_decl(t, p, md);
  }
}

inline void ManyView::eval_item(size_t t, const Program* p, const ModuleDeclaration* md) {
  for (auto* v : views_) {
    v->eval_item(t, p, md);
  }
}

inline void ManyView::crash() {
  for (auto* v : views_) {
    v->crash();
  }
} 

} // namespace cascade

#endif
