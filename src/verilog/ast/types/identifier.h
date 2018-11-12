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

#ifndef CASCADE_SRC_VERILOG_AST_IDENTIFIER_H
#define CASCADE_SRC_VERILOG_AST_IDENTIFIER_H

#include <cassert>
#include <string>
#include <vector>
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/id.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/many.h"
#include "src/verilog/ast/types/primary.h"
#include "src/verilog/ast/types/string.h"

namespace cascade {

class Identifier : public Primary {
  public:
    // Constructors:
    Identifier(const std::string& id__);
    Identifier(Id* id__, Many<Expression>* dim__);
    Identifier(Many<Id>* ids__, Many<Expression>* dim__);
    ~Identifier() override;

    // Node Interface:
    NODE(Identifier, TREE(ids), TREE(dim))
    // Get/Set:
    TREE_GET_SET(ids)
    TREE_GET_SET(dim)

    // Comparison Operators:
    bool eq(const std::string& rhs) const;
    bool eq(const String* rhs) const;

  private:
    TREE_ATTR(Many<Id>*, ids);
    TREE_ATTR(Many<Expression>*, dim);

    friend class Resolve;
    DECORATION(const Identifier*, resolution);
    DECORATION(std::vector<Expression*>, dependents);

    friend class Monitor;
    friend class SwLogic;
    DECORATION(std::vector<const Node*>, monitor);
};

inline Identifier::Identifier(const std::string& id__) : Identifier(new Id(id__, new Maybe<Expression>()), new Many<Expression>()) { }

inline Identifier::Identifier(Id* id__, Many<Expression>* dim__) : Identifier(new Many<Id>(id__), dim__) { }

inline Identifier::Identifier(Many<Id>* ids__, Many<Expression>* dim__) : Primary() {
  parent_ = nullptr;
  TREE_SETUP(ids);
  TREE_SETUP(dim);
  resolution_ = nullptr;
}

inline Identifier::~Identifier() {
  TREE_TEARDOWN(ids);
  TREE_TEARDOWN(dim);
}

inline bool Identifier::eq(const std::string& rhs) const {
  return ids_->size() == 1 && ids_->front()->eq(rhs);
}

inline bool Identifier::eq(const String* rhs) const {
  return ids_->size() == 1 && ids_->front()->eq(rhs);
}

} // namespace cascade 

#endif
