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

#ifndef CASCADE_SRC_VERILOG_AST_PORT_DECLARATION_H
#define CASCADE_SRC_VERILOG_AST_PORT_DECLARATION_H

#include "src/verilog/ast/types/attributes.h"
#include "src/verilog/ast/types/declaration.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/module_item.h"

namespace cascade {

class PortDeclaration : public ModuleItem {
  public:
    // Supporting Concepts:
    enum Type : uint8_t {
      INOUT = 0,
      INPUT,
      OUTPUT
    };

    // Constructors:
    PortDeclaration(Attributes* attrs__, Type type__, Declaration* decl__);
    ~PortDeclaration() override;

    // Node Interface:
    NODE(PortDeclaration)
    inline PortDeclaration* clone() const override;

    // Get/Set:
    PTR_GET_SET(PortDeclaration, Attributes, attrs)
    VAL_GET_SET(PortDeclaration, Type, type)
    PTR_GET_SET(PortDeclaration, Declaration, decl)

  private:
    PTR_ATTR(Attributes, attrs);
    VAL_ATTR(Type, type);
    PTR_ATTR(Declaration, decl);
};

inline PortDeclaration::PortDeclaration(Attributes* attrs__, Type type__, Declaration* decl__) : ModuleItem() {
  PTR_SETUP(attrs);
  VAL_SETUP(type);
  PTR_SETUP(decl);
  parent_ = nullptr;
}

inline PortDeclaration::~PortDeclaration() {
  PTR_TEARDOWN(attrs);
  VAL_TEARDOWN(type);
  PTR_TEARDOWN(decl);
}

inline PortDeclaration* PortDeclaration::clone() const {
  return new PortDeclaration(attrs_->clone(), type_, decl_->clone());
}

} // namespace cascade 

#endif
