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

#include "src/verilog/analyze/indices.h"

#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

size_t HashId::operator()(const Expression* e) const {
  return (e == nullptr) ? 0 : Evaluate().get_value(e).to_int();
}

size_t HashId::operator()(const Id* id) const {
  return id->get_sid() + this->operator()(id->get_isel());
}

size_t HashId::operator()(const Identifier* id) const {
  size_t res = 0;
  for (auto i = id->begin_ids(), ie = id->end_ids(); i != ie; ++i) {
    res += this->operator()(*i);
  }
  return res;
}

bool EqId::operator()(const Expression* e1, const Expression* e2) const {
  if ((e1 == nullptr) && (e2 == nullptr)) {
    return true;
  } else if ((e1 != nullptr) && (e2 != nullptr)) {
    return Evaluate().get_value(e1).to_int() == Evaluate().get_value(e2).to_int();
  } else {
    return false;
  }
}

bool EqId::operator()(const Id* id1, const Id* id2) const {
  return (id1->get_sid() == id2->get_sid()) && this->operator()(id1->get_isel(), id2->get_isel());
}

bool EqId::operator()(const Identifier* id1, const Identifier* id2) const {
  if (id1->size_ids() != id2->size_ids()) {
    return false;
  }
  for (auto i = id1->begin_ids(), j = id2->begin_ids(), ie = id1->end_ids(); i != ie; ++i, ++j) {
    if (!this->operator()(*i, *j)) {
      return false;
    }
  }
  return true;
}

} // namespace cascade
