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
#include <vector>
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
        explicit AliasTable(const ModuleDeclaration* md);
        ~AliasTable() override;

        // Dealiases a variable or returns nullptr on failure. It is the
        // responsibility of the caller to deallocate any resulting memory.
        Identifier* dealias(const Identifier* id);

      private:
        struct Row {
          const Identifier* target_;
          std::vector<const Expression*> slices_;
          bool done_;
        };

        // Base case for populating the alias table. Record assignments of the
        // form x = y, x = y[i], or x = y[i:j], where both sides of the
        // assignment have identical bit-width. 
        void visit(const ContinuousAssign* ca) override;
        // Inductive case for populating the alias table. Follow identifiers
        // accumulate the slices that accumulate along the way.
        void follow(Row& row);
        // Final step for populating the alias table. Replaces slice chains
        // with a single freshly constructed expression.
        void collapse(Row& row);
        // Returns the slice which is obtained by applying y after x.  So
        // merge([16:7], [6:4]) = x[7+6:7+4]. It is the responsibility of
        // the caller to deallocate this memory.
        Expression* merge(const Expression* x, const Expression* y);

        // Alias Table:
        std::unordered_map<const Identifier*, Row> aliases_;
    };
    AliasTable* table_;

    // Returns true if this is now an assignment of the form x = x.
    bool is_self_assign(const ContinuousAssign* ca);

    // Ignores attributes.
    Attributes* rewrite(Attributes* as) override;
    // Replaces an identifier with a de-aliased value.
    Expression* rewrite(Identifier* id) override;
    // Updates module items, ignores ports
    ModuleDeclaration* rewrite(ModuleDeclaration* md) override;
};

} // namespace cascade

#endif
