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

#ifndef CASCADE_SRC_VERILOG_AST_ATTRIBUTES_H
#define CASCADE_SRC_VERILOG_AST_ATTRIBUTES_H

#include <cassert>
#include <string>
#include "src/verilog/analyze/indices.h"
#include "src/verilog/ast/types/attr_spec.h"
#include "src/verilog/ast/types/many.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/node.h"
#include "src/verilog/ast/types/string.h"

namespace cascade {

class Attributes : public Node {
  public:
    // Constructors:
    Attributes(Many<AttrSpec>* as__);
    ~Attributes() override;

    // Node Interface:
    NODE(Attributes, TREE(as))
    // Get/Set
    TREE_GET_SET(as)

    // Lookup Interface:
    void erase(const std::string& s);
    template <typename T>
    const T* get(const std::string& s) const;
    void set_or_replace(const Attributes* rhs);
    void set_or_replace(const std::string& s, Expression* e);

  private:
    TREE_ATTR(Many<AttrSpec>*, as);
};

inline Attributes::Attributes(Many<AttrSpec>* as__) : Node() { 
  parent_ = nullptr;
  TREE_SETUP(as);
}

inline Attributes::~Attributes() {
  TREE_TEARDOWN(as);
}

inline void Attributes::erase(const std::string& s) {
  auto i = as_->begin();
  for (auto ie = as_->end(); i != ie; ++i) {
    if ((*i)->get_lhs()->eq(s)) {
      break;
    }
  }
  if (i != as_->end()) {
    as_->purge(i);
  }
}

template <typename T>
inline const T* Attributes::get(const std::string& s) const {
  for (auto va : *as_) {
    if (va->get_lhs()->eq(s) && va->is_non_null_rhs()) {
      return dynamic_cast<const T*>(va->get_rhs());
    }
  }
  return nullptr;
}

inline void Attributes::set_or_replace(const Attributes* rhs) {
  for (auto rva : *rhs->as_) {
    for (auto va : *as_) {
      if (EqId()(va->get_lhs(), rva->get_lhs())) {
        va->replace_rhs(rva->maybe_clone_rhs());
        break;
      }
    }
    as_->push_back(rva->clone());
  }
}

inline void Attributes::set_or_replace(const std::string& s, Expression* e) {
  AttrSpec* as = nullptr;
  for (auto va : *as_) {
    if (va->get_lhs()->eq(s)) {
      as = va;
      break;
    }
  }
  if (as == nullptr) {
    get_as()->push_back(new AttrSpec(new Identifier(s), e));
  } else {
    as->replace_rhs(e);
  }
}

} // namespace cascade

#endif
