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

#ifndef CASCADE_SRC_VERILOG_AST_ATTRIBUTES_H
#define CASCADE_SRC_VERILOG_AST_ATTRIBUTES_H

#include <string>
#include "src/verilog/analyze/indices.h"
#include "src/verilog/ast/types/attr_spec.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/node.h"
#include "src/verilog/ast/types/string.h"

namespace cascade {

class Attributes : public Node {
  public:
    // Constructors:
    Attributes();
    explicit Attributes(AttrSpec* as);
    template <typename AttrItr>
    Attributes(AttrItr as_begin__, AttrItr as_end__);
    ~Attributes() override;

    // Node Interface:
    NODE(Attributes)
    Attributes* clone() const override;

    // Get/Set
    MANY_GET_SET(Attributes, AttrSpec, as)

    // Lookup Interface:
    void erase(const std::string& s);
    template <typename T>
    const T* get(const std::string& s) const;
    void set_or_replace(const Attributes* rhs);
    void set_or_replace(const std::string& s, Expression* e);

  private:
    MANY_ATTR(AttrSpec, as);
};

inline Attributes::Attributes() : Node(Node::Tag::attributes) {
  MANY_DEFAULT_SETUP(as);
  parent_ = nullptr;
}

inline Attributes::Attributes(AttrSpec* as) : Attributes() {
  push_back_as(as);
}

template <typename AttrItr>
inline Attributes::Attributes(AttrItr as_begin__, AttrItr as_end__) : Attributes() { 
  MANY_SETUP(as);
}

inline Attributes::~Attributes() {
  MANY_TEARDOWN(as);
}

inline Attributes* Attributes::clone() const {
  auto* res = new Attributes();
  MANY_CLONE(as);
  return res;
}

inline void Attributes::erase(const std::string& s) {
  auto i = begin_as();
  for (auto ie = end_as(); i != ie; ++i) {
    if ((*i)->get_lhs()->eq(s)) {
      break;
    }
  }
  if (i != end_as()) {
    purge_as(i);
  }
}

template <typename T>
inline const T* Attributes::get(const std::string& s) const {
  for (auto i = begin_as(), ie = end_as(); i != ie; ++i) {
    if ((*i)->get_lhs()->eq(s) && (*i)->is_non_null_rhs()) {
      return dynamic_cast<const T*>((*i)->get_rhs());
    }
  }
  return nullptr;
}

inline void Attributes::set_or_replace(const Attributes* rhs) {
  for (auto i = rhs->begin_as(), ie = rhs->end_as(); i != ie; ++i) {
    for (auto j = begin_as(), je = end_as(); j != je; ++j) {
      if (EqId()((*j)->get_lhs(), (*i)->get_lhs())) {
        (*j)->replace_rhs((*i)->clone_rhs());
        break;
      }
    }
    push_back_as((*i)->clone());
  }
}

inline void Attributes::set_or_replace(const std::string& s, Expression* e) {
  AttrSpec* as = nullptr;
  for (auto i = begin_as(), ie = end_as(); i != ie; ++i) {
    if ((*i)->get_lhs()->eq(s)) {
      as = *i;
      break;
    }
  }
  if (as == nullptr) {
    push_back_as(new AttrSpec(new Identifier(s), e));
  } else {
    as->replace_rhs(e);
  }
}

} // namespace cascade

#endif
