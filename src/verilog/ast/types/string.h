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

#ifndef CASCADE_SRC_VERILOG_AST_TYPES_STRING_H
#define CASCADE_SRC_VERILOG_AST_TYPES_STRING_H

#include <string>
#include "src/base/token/tokenize.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/primary.h"

namespace cascade {

class String : public Primary {
  public:
    // Constructors:
    explicit String(const std::string& val);
    explicit String(Tokenize::Token val);
    ~String() override = default;

    // Node Interface:
    NODE(String)
    String* clone() const override;

    // Get/Set:
    VAL_GET_SET(String, Tokenize::Token, val)
    const std::string& get_readable_val();
    const std::string& get_readable_val() const;
    void set_val(const std::string& val);
    void assign_val(const std::string& val);

    // Comparison Operators:
    bool eq(const std::string& rhs) const;

  private:
    VAL_ATTR(Tokenize::Token, val);
};

inline String::String(const std::string& val) : String(Tokenize().map(val)) { }

inline String::String(Tokenize::Token val) : Primary(Node::Tag::string) {
  val_ = val;
  parent_ = nullptr;
}

inline String* String::clone() const {
  return new String(val_);
}

inline const std::string& String::get_readable_val() {
  return Tokenize().unmap(val_);
}

inline const std::string& String::get_readable_val() const {
  return Tokenize().unmap(val_);
}

inline void String::set_val(const std::string& val) {
  val_ = Tokenize().map(val);
}

inline void String::assign_val(const std::string& val) {
  val_ = Tokenize().map(val);
}

inline bool String::eq(const std::string& rhs) const {
  return val_ == Tokenize().map(rhs);
}

} // namespace cascade 

#endif
