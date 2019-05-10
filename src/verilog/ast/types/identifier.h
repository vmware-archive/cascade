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

#ifndef CASCADE_SRC_VERILOG_AST_IDENTIFIER_H
#define CASCADE_SRC_VERILOG_AST_IDENTIFIER_H

#include <string>
#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/id.h"
#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/primary.h"
#include "verilog/ast/types/string.h"

namespace cascade {

class Identifier : public Primary {
  public:
    // Constructors:
    Identifier();
    explicit Identifier(const std::string& id__);
    explicit Identifier(Id* id__);
    Identifier(Id* id__, Expression* dim__);
    template <typename DimItr>
    Identifier(Id* id__, DimItr begin_dim__, DimItr end_dim__);
    template <typename IdsItr, typename DimItr>
    Identifier(IdsItr ids_begin__, IdsItr ids_end__, DimItr dim_begin__, DimItr dim_end__);
    ~Identifier() override;

    // Node Interface:
    NODE(Identifier)
    Identifier* clone() const override;

    // Get/Set:
    MANY_GET_SET(Identifier, Id, ids)
    MANY_GET_SET(Identifier, Expression, dim)

    // Comparison Operators:
    bool eq(const std::string& rhs) const;
    bool eq(const String* rhs) const;

  private:
    MANY_ATTR(Id, ids);
    MANY_ATTR(Expression, dim);

    friend class Resolve;
    DECORATION(const Identifier*, resolution);
    friend class Monitor;
    friend class SwLogic;
    DECORATION(Vector<const Node*>, monitor);
};

inline Identifier::Identifier() : Primary(Node::Tag::identifier) { 
  MANY_DEFAULT_SETUP(ids);
  MANY_DEFAULT_SETUP(dim);
  parent_ = nullptr;
  resolution_ = nullptr;
}

inline Identifier::Identifier(const std::string& id__) : Identifier(new Id(id__)) { }

inline Identifier::Identifier(Id* id__) : Identifier() {
  push_back_ids(id__);
}

inline Identifier::Identifier(Id* id__, Expression* dim__) : Identifier() {
  push_back_ids(id__);
  push_back_dim(dim__);
}

template <typename DimItr>
inline Identifier::Identifier(Id* id__, DimItr dim_begin__, DimItr dim_end__) : Identifier(id__) {
  MANY_SETUP(dim);
}

template <typename IdsItr, typename DimItr>
inline Identifier::Identifier(IdsItr ids_begin__, IdsItr ids_end__, DimItr dim_begin__, DimItr dim_end__) : Identifier() {
  MANY_SETUP(ids);
  MANY_SETUP(dim);
}

inline Identifier::~Identifier() {
  MANY_TEARDOWN(ids);
  MANY_TEARDOWN(dim);
}

inline Identifier* Identifier::clone() const {
  auto* res = new Identifier();
  MANY_CLONE(ids);
  MANY_CLONE(dim);
  return res;
}

inline bool Identifier::eq(const std::string& rhs) const {
  return (size_ids() == 1) && front_ids()->eq(rhs);
}

inline bool Identifier::eq(const String* rhs) const {
  return (size_ids() == 1) && front_ids()->eq(rhs);
}

} // namespace cascade 

#endif
