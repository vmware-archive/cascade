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

#ifndef CASCADE_SRC_VERILOG_TRANSFORM_INDEX_NORMALIZE_H
#define CASCADE_SRC_VERILOG_TRANSFORM_INDEX_NORMALIZE_H

#include <stddef.h>
#include "verilog/ast/visitors/editor.h"

namespace cascade {

class IndexNormalize {
  public:
    void run(ModuleDeclaration* md);

  private:
    class FixDecls : public Editor {
      public: 
        FixDecls();
        ~FixDecls() override = default;
      private:
        void edit(GenvarDeclaration* gd) override;
        void edit(LocalparamDeclaration* ld) override;
        void edit(NetDeclaration* nd) override;
        void edit(ParameterDeclaration* pd) override;
        void edit(RegDeclaration* rd) override;

        void fix_arity(Identifier* id) const;
        void fix_dim(RangeExpression* re) const;
    };

    class FixUses : public Editor {
      public:
        FixUses();
        ~FixUses() override = default;
      private:
        void edit(Attributes* a) override;
        void edit(Identifier* i) override;
        void edit(ModuleDeclaration* md) override;
        void edit(GenvarDeclaration* gd) override;
        void edit(LocalparamDeclaration* ld) override;
        void edit(NetDeclaration* nd) override;
        void edit(ParameterDeclaration* pd) override;
        void edit(RegDeclaration* rd) override;
        void edit(DebugStatement* ds) override;

        void fix_use(Identifier* id, size_t n, const RangeExpression* re) const;
        void fix_scalar(Identifier* id, size_t n, size_t delta) const;
        void fix_range(Identifier* id, size_t n, size_t delta) const;
     };
};

} // namespace cascade

#endif
