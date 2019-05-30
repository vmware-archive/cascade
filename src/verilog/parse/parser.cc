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

#include <regex>
#include "verilog/parse/parser.h"
#include "base/log/log.h"

using namespace std;

namespace cascade {

Parser::Parser(Log* log) : Editor() { 
  buf_ = nullptr;
  include_dirs_ = "";
  log_ = log;
  push("<top>");
  last_parse_ = "";
  nesting_ = 0;
}

Parser& Parser::set_include_dirs(const string& s) {
  include_dirs_ = s;
  return *this;
}

void Parser::parse(istream& is) {
  if (is.rdbuf() != buf_) {
    buf_ = is.rdbuf();
    lexer_.switch_streams(&is);
    eof_ = false;
  }
  yyParser parser(this);
  lexer_.set_debug(false);
  parser.set_debug_level(false);

  res_.clear();
  last_parse_ = "";
  locs_.clear();

  get_loc().step();
  parser.parse();
  for (auto* n : res_) {
    n->accept(this);
  }
}

bool Parser::eof() const {
  return eof_;
}

size_t Parser::depth() const {
  return stack_.size();
}

Parser::const_iterator Parser::begin() const {
  return res_.begin();
}

Parser::const_iterator Parser::end() const {
  return res_.end();
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
  if (md->size_ports() == 1 && md->front_ports()->is_null_imp()) {
    md->purge_to_ports(0);
  }
  md->accept_items(this);
}

void Parser::edit(ModuleInstantiation* mi) {
  // PARSER ARTIFACT: Fix empty param list
  if (mi->size_params() == 1 && mi->front_params()->is_null_imp()) {
    mi->purge_to_params(0);
  }
  // PARSER ARTIFICAT: Fix empty port list
  if (mi->size_ports() == 1 && mi->front_ports()->is_null_imp()) {
    mi->purge_to_ports(0);
  } 
}

void Parser::push(const string& path) {
  stack_.push(make_pair(path, location()));
  stack_.top().second.initialize();
}

void Parser::pop() {
  stack_.pop();
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

void Parser::define(const string& name, const vector<string>& args, const string& text) {
  macros_[name] = make_pair(args, text);
}

void Parser::undefine(const string& name) {
  assert(is_defined(name));
  macros_.erase(name);
}

bool Parser::is_defined(const string& name) const {
  return macros_.find(name) != macros_.end();
}

size_t Parser::arity(const string& name) const {
  assert(is_defined(name));
  return macros_.find(name)->second.first.size(); 
}

string Parser::replace(const string& name, const vector<string>& args) const {
  assert(is_defined(name));
  const auto itr = macros_.find(name);

  const auto& formal_args = itr->second.first;
  auto res = itr->second.second;
  assert(formal_args.size() == args.size());

  for (size_t i = 0, ie = args.size(); i < ie; ++i) {
    regex re("\\b" + formal_args[i] + "\\b");
    res = regex_replace(res, re, args[i]);
  }
  return res.substr(1, res.length()-1);
}

} // namespace cascade
