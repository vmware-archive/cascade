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

#include <sstream>

using namespace std;

namespace cascade {

Parser::Parser() : Editor(), Loggable() { 
  debug_lexer_ = false;
  debug_parser_ = false;
}

Parser& Parser::debug_lexer(bool debug) {
  debug_lexer_ = debug;
  return *this;
}

Parser& Parser::debug_parser(bool debug) {
  debug_parser_ = debug;
  return *this;
}

pair<Node*, bool> Parser::parse(const string& s) {
  stringstream ss(s);
  return parse(ss);
}

pair<Node*, bool> Parser::parse(istream& is) {
  clear_logs();

  lexer_.switch_streams(&is);
  lexer_.set_debug(debug_lexer_);
  text_ = "";
  loc_.initialize();

  yyParser parser(this);
  parser.set_debug_level(debug_parser_);

  res_ = nullptr;
  eof_ = false;
  parser.parse();
  if (res_ != nullptr) {
    res_->accept(this);
  }

  return make_pair(res_, eof_);
}

void Parser::edit(ModuleDeclaration* md) {
  // PARSER ARTIFACT: Fix empty port list
  if (md->get_ports()->size() == 1 && md->get_ports()->front()->get_imp()->null()) {
    md->get_ports()->purge_to(0);
  }
  md->get_items()->accept(this);
}

void Parser::edit(ModuleInstantiation* mi) {
  // PARSER ARTIFACT: Fix empty param list
  if (mi->get_params()->size() == 1 && mi->get_params()->front()->get_imp()->null()) {
    mi->get_params()->purge_to(0);
  }
  // PARSER ARTIFICAT: Fix empty port list
  if (mi->get_ports()->size() == 1 && mi->get_ports()->front()->get_imp()->null()) {
    mi->get_ports()->purge_to(0);
  } 
}

} // namespace cascade
