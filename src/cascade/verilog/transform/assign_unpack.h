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

#ifndef CASCADE_SRC_VERILOG_TRANSFORM_ASSIGN_UNPACK_H
#define CASCADE_SRC_VERILOG_TRANSFORM_ASSIGN_UNPACK_H

#include <cassert>
#include <vector>
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/navigate.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/ast/visitors/rewriter.h"
#include "verilog/ast/visitors/visitor.h"
#include "verilog/build/ast_builder.h"
#include "verilog/print/print.h"

namespace cascade {

class AssignUnpack : public Rewriter {
  public:
    AssignUnpack();
    ~AssignUnpack() override = default;

    void run(ModuleDeclaration* md);

  private:
    ModuleItem* rewrite(ContinuousAssign* ca) override;
    Statement* rewrite(BlockingAssign* ba) override;
    Statement* rewrite(NonblockingAssign* na) override;

    std::vector<NetDeclaration*> decls_;
    std::vector<ModuleItem*> cas_;
    std::vector<Identifier*> rhs_;
    size_t next_id_;

    template <typename IdItr>
    void unpack(IdItr begin, IdItr end, const Expression* rhs);
};

template <typename IdItr>
inline void AssignUnpack::unpack(IdItr begin, IdItr end, const Expression* rhs) {
  // Create a name for this pack and compute its width
  const auto id = next_id_++;
  size_t ww = 0;
  for (auto i = begin, ie = end; i != ie; ++i) {
    const auto* r = Resolve().get_resolution(*i);
    assert(r != nullptr);
    ww += Evaluate().get_width(r);
  }

  // Create a new pack variable
  NetBuilder nb;
  nb << "wire[" << (ww-1) << ":0] __pack_" << id << ";" << std::endl; 
  decls_.push_back(nb.get());

  ItemBuilder ib;
  ib << "assign __pack_" << id << " = " << rhs << ";" << std::endl;
  cas_.push_back(ib.get());

  // Create new assignments
  for (auto i = begin, ie = end; i != ie; ++i) {
    const auto* r = Resolve().get_resolution(*i);
    assert(r != nullptr);
    const auto w = Evaluate().get_width(r);
    rhs_.push_back(new Identifier(
      decls_.back()->get_id()->front_ids()->clone(),
      new RangeExpression(new Number(Bits(32, ww-w)), RangeExpression::Type::PLUS, new Number(Bits(32, w)))
    ));
    ww -= w;
  }
}

} // namespace cascade

#endif
