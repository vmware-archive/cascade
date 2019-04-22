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

#include "src/target/core/de10/pass/rewrite_text.h"

#include "src/target/core/de10/de10_logic.h"
#include "src/target/core/de10/pass/text_mangle.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/ast/ast.h"

namespace cascade {

RewriteText::RewriteText(const ModuleDeclaration* md, const De10Logic* de, TextMangle* tm) : Builder() { 
  md_ = md;
  de_ = de;
  tm_ = tm;
}

Attributes* RewriteText::build(const Attributes* as) {
  (void) as;
  return new Attributes();
}

ModuleItem* RewriteText::build(const RegDeclaration* rd) {
  return ModuleInfo(md_).is_stateful(rd->get_id()) ? nullptr : rd->clone();
}

ModuleItem* RewriteText::build(const PortDeclaration* pd) {
  ModuleInfo info(md_);
  if (info.is_stateful(pd->get_decl()->get_id()) || info.is_input(pd->get_decl()->get_id())) {
    return nullptr;
  } else {
    return pd->get_decl()->clone();
  }
}

Expression* RewriteText::build(const EofExpression* ee) {
  // This is a bit confusing: the de10 compiler has created an entry in the
  // variable table for the argument to this expression (like we do with
  // arguments to display statements). Prior to transfering control to the fpga
  // we'll place the result of this eof check into this location in hardware.
  const auto itr = de_->table_find(ee->get_arg());
  assert(itr != de_->table_end());
  return new Identifier(new Id("__var"), new Number(Bits(32, itr->second.index())));
}

Statement* RewriteText::build(const NonblockingAssign* na) {
  auto* res = Builder::build(na);
  tm_->replace(na, res);
  return res;
}

Statement* RewriteText::build(const DisplayStatement* ds) {
  auto* res = Builder::build(ds);
  tm_->replace(ds, res);
  return res;
}

Statement* RewriteText::build(const ErrorStatement* es) {
  auto* res = Builder::build(es);
  tm_->replace(es, res);
  return res;
}

Statement* RewriteText::build(const FinishStatement* fs) {
  auto* res = Builder::build(fs);
  tm_->replace(fs, res);
  return res;
}

Statement* RewriteText::build(const GetStatement* gs) {
  auto* res = Builder::build(gs);
  tm_->replace(gs, res);
  return res;
}

Statement* RewriteText::build(const InfoStatement* is) {
  auto* res = Builder::build(is);
  tm_->replace(is, res);
  return res;
}

Statement* RewriteText::build(const PutStatement* ps) {
  auto* res = Builder::build(ps);
  tm_->replace(ps, res);
  return res;
}

Statement* RewriteText::build(const RestartStatement* rs) {
  auto* res = Builder::build(rs);
  tm_->replace(rs, res);
  return res;
}

Statement* RewriteText::build(const RetargetStatement* rs) {
  auto* res = Builder::build(rs);
  tm_->replace(rs, res);
  return res;
}

Statement* RewriteText::build(const SaveStatement* ss) {
  auto* res = Builder::build(ss);
  tm_->replace(ss, res);
  return res;
}

Statement* RewriteText::build(const SeekStatement* ss) {
  auto* res = Builder::build(ss);
  tm_->replace(ss, res);
  return res;
}

Statement* RewriteText::build(const WarningStatement* ws) {
  auto* res = Builder::build(ws);
  tm_->replace(ws, res);
  return res;
}

Statement* RewriteText::build(const WriteStatement* ws) {
  auto* res = Builder::build(ws);
  tm_->replace(ws, res);
  return res;
}

} // namespace cascade
