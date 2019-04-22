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

#include "src/target/core/de10/pass/finish_mangle.h"

#include "src/target/core/de10/pass/text_mangle.h"
#include "src/verilog/ast/ast.h"

namespace cascade {

FinishMangle::FinishMangle(TextMangle* tm) : Rewriter() {
  tm_ = tm;
}

Statement* FinishMangle::rewrite(NonblockingAssign* na) {
  auto itr = tm_->find(na);
  return (itr != tm_->end()) ? itr->second : na;
}

Statement* FinishMangle::rewrite(DisplayStatement* ds) {
  auto itr = tm_->find(ds);
  return (itr != tm_->end()) ? itr->second : ds;
}

Statement* FinishMangle::rewrite(ErrorStatement* es) {
  auto itr = tm_->find(es);
  return (itr != tm_->end()) ? itr->second : es;
}

Statement* FinishMangle::rewrite(FinishStatement* fs) {
  auto itr = tm_->find(fs);
  return (itr != tm_->end()) ? itr->second : fs;
}

Statement* FinishMangle::rewrite(GetStatement* gs) {
  auto itr = tm_->find(gs);
  return (itr != tm_->end()) ? itr->second : gs;
}

Statement* FinishMangle::rewrite(InfoStatement* is) {
  auto itr = tm_->find(is);
  return (itr != tm_->end()) ? itr->second : is;
}

Statement* FinishMangle::rewrite(PutStatement* ps) {
  auto itr = tm_->find(ps);
  return (itr != tm_->end()) ? itr->second : ps;
}

Statement* FinishMangle::rewrite(RestartStatement* rs) {
  auto itr = tm_->find(rs);
  return (itr != tm_->end()) ? itr->second : rs;
}

Statement* FinishMangle::rewrite(RetargetStatement* rs) {
  auto itr = tm_->find(rs);
  return (itr != tm_->end()) ? itr->second : rs;
}

Statement* FinishMangle::rewrite(SaveStatement* ss) {
  auto itr = tm_->find(ss);
  return (itr != tm_->end()) ? itr->second : ss;
}

Statement* FinishMangle::rewrite(SeekStatement* ss) {
  auto itr = tm_->find(ss);
  return (itr != tm_->end()) ? itr->second : ss;
}

Statement* FinishMangle::rewrite(WarningStatement* ws) {
  auto itr = tm_->find(ws);
  return (itr != tm_->end()) ? itr->second : ws;
}

Statement* FinishMangle::rewrite(WriteStatement* ws) {
  auto itr = tm_->find(ws);
  return (itr != tm_->end()) ? itr->second : ws;
}

} // namespace cascade

