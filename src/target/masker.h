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

#ifndef CASCADE_SRC_TARGET_MASKER_H
#define CASCADE_SRC_TARGET_MASKER_H

#include "src/verilog/ast/ast.h"
#include "src/verilog/ast/visitors/editor.h"

namespace cascade {

// TODO: Does this class belong in this directory?

class Masker : public Editor {
  public:
    Masker();
    ~Masker() override = default;

    void mask(ModuleDeclaration* md);
    void mask(ModuleDeclaration* md, size_t n);

  private:
    void edit(InitialConstruct* ic) override;
};

inline Masker::Masker() : Editor() { }

inline void Masker::mask(ModuleDeclaration* md) {
  mask(md, md->get_items()->size());
}

inline void Masker::mask(ModuleDeclaration* md, size_t n) {
  auto itr = md->get_items()->begin();
  for (size_t i = 0; i < n; ++i, ++itr) {
    (*itr)->accept(this);
  }
}

inline void Masker::edit(InitialConstruct* ic) {
  ic->get_attrs()->set_or_replace("__ignore", new String("true"));
}

} // namespace cascade 

#endif
