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

#ifndef CASCADE_SRC_UI_COMBINATOR_MAYBE_VIEW_H
#define CASCADE_SRC_UI_COMBINATOR_MAYBE_VIEW_H

#include "src/ui/view.h"

namespace cascade {

class MaybeView : public View {
  public:
    MaybeView(View* view = nullptr);
    ~MaybeView() override;

    void attach(View* v);

    void error(const std::string& s) override;
    void print(const std::string& s) override;
    void warn(const std::string& s) override;

    void eval_decl(const Program* p, const ModuleDeclaration* md) override;
    void eval_item(const Program* p, const ModuleDeclaration* md) override;

  private:
    View* view_;
};

inline MaybeView::MaybeView(View* view) : View() { 
  view_ = view;
}

inline MaybeView::~MaybeView() {
  if (view_ != nullptr) {
    delete view_;
  }
}

inline void MaybeView::attach(View* v) {
  if (view_ != nullptr) {
    delete view_;
  }
  view_ = v;
}

inline void MaybeView::error(const std::string& s) {
  if (view_ != nullptr) {
    view_->error(s);
  }
}

inline void MaybeView::print(const std::string& s) {
  if (view_ != nullptr) {
    view_->print(s);
  }
}

inline void MaybeView::warn(const std::string& s) {
  if (view_ != nullptr) {
    view_->warn(s);
  }
}

inline void MaybeView::eval_decl(const Program* p, const ModuleDeclaration* md) {
  if (view_ != nullptr) {
    view_->eval_decl(p, md);
  }
}

inline void MaybeView::eval_item(const Program* p, const ModuleDeclaration* md) {
  if (view_ != nullptr) {
    view_->eval_item(p, md);
  }
}

} // namespace cascade

#endif
