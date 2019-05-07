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

#ifndef CASCADE_SRC_VERILOG_PARSE_PARSER_H
#define CASCADE_SRC_VERILOG_PARSE_PARSER_H

#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include "src/verilog/ast/ast_fwd.h"
#include "src/verilog/ast/visitors/editor.h"
#include "src/verilog/parse/lexer.h"
#include "src/verilog/parse/verilog.tab.hh"

namespace cascade {

class Log;

class Parser : public Editor {
  public:
    // Iterators
    typedef std::vector<Node*>::const_iterator const_iterator;

    // Constructors:
    Parser(Log* log);
    ~Parser() override = default;

    // Configuration Interface: 
    //
    // Sets the search path for include directives
    Parser& set_include_dirs(const std::string& s);
    // Attaches a new stream to the parser and sets the eof flag to false
    Parser& set_stream(std::istream& is);

    // Parser Interface:
    //
    // Parses the next element from the current stream.  Writes errors/warnings to log.
    void parse();
    // Returns true if the last parse ended in an end-of-file
    bool eof() const;
    // Returns the current include nesting depth 
    size_t depth() const;
    // Returns iterators over the results of the previous parse
    const_iterator begin() const;
    const_iterator end() const;
    // Returns the text of the last parse.
    const std::string& get_text() const;
    // Returns the file and line number for a node from the last parse.
    std::pair<std::string, size_t> get_loc(const Node* n) const;

  private:
    friend class yyLexer;
    friend class yyParser;
    yyLexer lexer_;
    
    // Location stack:
    std::stack<std::pair<std::string, location>> stack_;

    // State returned by the previous parse:
    std::string include_dirs_;
    Log* log_;
    std::vector<Node*> res_;
    bool eof_;
    std::string last_parse_;
    std::unordered_map<const Node*, std::pair<std::string, size_t>> locs_;

    // The lookahead symbol from the previous parse:
    yyParser::symbol_type backup_;

    // Visitor Interface:
    //
    // Collectively, these methods are responsible for fixing structural
    // artifacts having to do with parsing. For instance, relacing a port list
    // with a single null element with an empty list.
    void edit(ModuleDeclaration* md) override;
    void edit(ModuleInstantiation* mi) override;

    // Helper methods for tracking location information:
    //
    // Pushes a new file path onto the parser's stack. 
    void push(const std::string& path);
    // Removes the last path from the parser's stack.
    void pop();
    // Returns the current path
    std::string& get_path();
    // Returns the current location
    location& get_loc();
    // Sets location to the same value as for n2
    void set_loc(const Node* n1, const Node* n2);
    // Sets filename to the current path, line to a constant value
    void set_loc(const Node* n, size_t line);
    // Sets location to the current path and line 
    void set_loc(const Node* n);
};

} // namespace cascade 

#endif
