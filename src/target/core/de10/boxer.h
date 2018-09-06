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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_BOXER_H
#define CASCADE_SRC_TARGET_CORE_DE10_BOXER_H

#include <string>
#include "src/runtime/ids.h"
#include "src/verilog/ast/visitors/builder.h"

namespace cascade {

class De10Logic;

class Boxer : public Builder {
  public:
    Boxer();
    ~Boxer() override = default;

    std::string box(MId id, const ModuleDeclaration* md, const De10Logic* de);

  private:
    // Source Management:
    const ModuleDeclaration* md_;
    const De10Logic* de_;

    // System task management:
    size_t task_id_;

    // Builder Interface:
    Attributes* build(const Attributes* as) override; 
    ModuleItem* build(const RegDeclaration* rd) override;
    ModuleItem* build(const PortDeclaration* pd) override;
    Statement* build(const NonblockingAssign* na) override;
    Statement* build(const DisplayStatement* ds) override;
    Statement* build(const FinishStatement* fs) override;
    Statement* build(const WriteStatement* ws) override;

    // Helper Methods:
    Statement* mangle_systask(size_t id, const Many<Expression>* args);
};

} // namespace cascade

#endif
