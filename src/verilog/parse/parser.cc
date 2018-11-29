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

#include "src/verilog/parse/parser.h"

#include "src/base/log/log.h"

using namespace std;

namespace cascade {

Parser::Parser() : Editor() { 
  debug_lexer_ = false;
  debug_parser_ = false;
  push("<top>");
  last_parse_ = "";
}

Parser& Parser::debug_lexer(bool debug) {
  debug_lexer_ = debug;
  return *this;
}

Parser& Parser::debug_parser(bool debug) {
  debug_parser_ = debug;
  return *this;
}

void Parser::push(const string& path) {
  stack_.push(make_pair(path, location()));
  stack_.top().second.initialize();
}

void Parser::pop() {
  stack_.pop();
}

pair<Node*, bool> Parser::parse(istream& is, Log* log) {
  lexer_.switch_streams(&is);
  lexer_.set_debug(debug_lexer_);

  yyParser parser(this);
  parser.set_debug_level(debug_parser_);

  res_ = nullptr;
  eof_ = false;
  last_parse_ = "";
  locs_.clear();
  log_ = log;

  get_loc().step();
  parser.parse();
  if (res_ != nullptr) {
    res_->accept(this);
  }

  return make_pair(res_, eof_);
}

const std::string& Parser::get_text() const {
  return last_parse_;
}

pair<string, size_t> Parser::get_loc(const Node* n) const {
  const auto itr = locs_.find(n);
  if (itr == locs_.end()) {
    return make_pair("<unknown location --- please contact developers>", 0);
  } else {
    return itr->second;
  }
}

void Parser::edit(ModuleDeclaration* md) {
  // PARSER ARTIFACT: Fix empty port list
  if (md->get_ports()->size() == 1 && md->get_ports()->front()->is_null_imp()) {
    md->get_ports()->purge_to(0);
  }
  md->get_items()->accept(this);
}

void Parser::edit(ModuleInstantiation* mi) {
  // PARSER ARTIFACT: Fix empty param list
  if (mi->get_params()->size() == 1 && mi->get_params()->front()->is_null_imp()) {
    mi->get_params()->purge_to(0);
  }
  // PARSER ARTIFICAT: Fix empty port list
  if (mi->get_ports()->size() == 1 && mi->get_ports()->front()->is_null_imp()) {
    mi->get_ports()->purge_to(0);
  } 
}

string& Parser::get_path() {
  assert(!stack_.empty());
  return stack_.top().first;
}

location& Parser::get_loc() {
  assert(!stack_.empty());
  return stack_.top().second;
}

void Parser::set_loc(const Node* n1, const Node* n2) {
  const auto itr = locs_.find(n2);
  if (itr != locs_.end()) {
    locs_.insert(make_pair(n1, itr->second));
  } else {
    locs_.insert(make_pair(n1, make_pair("<missing location --- please contact developers>", 0)));
  }
}

void Parser::set_loc(const Node* n, size_t line) {
  locs_.insert(make_pair(n, make_pair(get_path(), line))); 
}

void Parser::set_loc(const Node* n) {
  locs_.insert(make_pair(n, make_pair(get_path(), get_loc().begin.line))); 
}

} // namespace cascade
