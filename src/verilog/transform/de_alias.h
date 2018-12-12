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

#ifndef CASCADE_SRC_VERILOG_TRANSFORM_DE_ALIAS_H
#define CASCADE_SRC_VERILOG_TRANSFORM_DE_ALIAS_H

#include <unordered_map>
#include "src/verilog/ast/visitors/rewriter.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class DeAlias : public Rewriter {
  public:
    DeAlias();
    ~DeAlias() override = default;

    void run(ModuleDeclaration* md);

  private:
    class AliasTable : public Visitor {
      public:
        AliasTable(const ModuleDeclaration* md);
        ~AliasTable() override = default;

        bool is_alias(const Identifier* id);
        const Identifier* get(const Identifier* id);

      private:
        const Identifier* resolve(const Identifier* id);
        void visit(const ContinuousAssign* ca) override;
        std::unordered_map<const Identifier*, const Identifier*> assigns_;
        std::unordered_map<const Identifier*, const Identifier*> results_;
    };
    AliasTable* table_;

    bool is_self_assign(const ContinuousAssign* ca);

    Attributes* rewrite(Attributes* as) override;
    Expression* rewrite(Identifier* id) override;
    ModuleDeclaration* rewrite(ModuleDeclaration* md) override;
};

} // namespace cascade

#endif
