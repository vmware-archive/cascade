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

#ifndef CASCADE_SRC_VERILOG_TRANSFORM_DEAD_CODE_ELIMINATE_H
#define CASCADE_SRC_VERILOG_TRANSFORM_DEAD_CODE_ELIMINATE_H

#include <unordered_set>
#include "verilog/ast/visitors/editor.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class DeadCodeEliminate : public Editor {
  public:
    DeadCodeEliminate();
    ~DeadCodeEliminate() override = default;

    void run(ModuleDeclaration* md);

  private:
    struct Index : Visitor {
      explicit Index(DeadCodeEliminate* dce);
      ~Index() override = default;

      void visit(const Attributes* a) override;
      void visit(const Identifier* i) override;
      void visit(const LocalparamDeclaration* ld) override;
      void visit(const NetDeclaration* nd) override;
      void visit(const ParameterDeclaration* pd) override;
      void visit(const RegDeclaration* rd) override;

      DeadCodeEliminate* dce_;
    };

    void edit(ModuleDeclaration* md) override;
    void edit(ParBlock* pb) override;
    void edit(SeqBlock* sb) override;

    std::unordered_set<const Identifier*> use_;
};

} // namespace cascade

#endif

