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

#ifndef CASCADE_SRC_VERILOG_TRANSFORM_CONSTANT_PROP_H
#define CASCADE_SRC_VERILOG_TRANSFORM_CONSTANT_PROP_H

#include <unordered_map>
#include <unordered_set>
#include "verilog/ast/visitors/rewriter.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class ConstantProp : public Rewriter {
  public:
    ConstantProp();
    ~ConstantProp() override = default;

    void run(ModuleDeclaration* md);

  private:
    // Wires which have been assigned to static constants
    std::unordered_set<const Identifier*> runtime_constants_;
    std::unordered_map<const Identifier*, const ContinuousAssign*> net_assigns_;

    // Returns true if this identifier is the target of an assign statement
    bool is_assign_target(const Identifier* i) const;

    // Helper Class: Returns true if this expression is a runtime constant.
    class RuntimeConstant : public Visitor {
      public:
        explicit RuntimeConstant(ConstantProp* cp);
        ~RuntimeConstant() override = default;

        bool check(const Expression* e);

      private:
        ConstantProp* cp_;
        bool res_;

        void visit(const Attributes* as) override;
        void visit(const Identifier* i) override;
    };

    // Rewriter Interface
    Expression* rewrite(BinaryExpression* be) override;
    Expression* rewrite(ConditionalExpression* ce) override;
    Expression* rewrite(FopenExpression* fe) override;
    Expression* rewrite(Concatenation* c) override;
    Expression* rewrite(Identifier* i) override;
    Expression* rewrite(MultipleConcatenation* mc) override;
    Expression* rewrite(RangeExpression* re) override;
    Expression* rewrite(UnaryExpression* ue) override;
    // TODO(eschkufz) Add support for constexpr evaluating case statements
    Statement* rewrite(ConditionalStatement* cs) override;
};

} // namespace cascade

#endif
