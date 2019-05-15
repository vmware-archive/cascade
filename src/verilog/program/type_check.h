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

#ifndef CASCADE_SRC_VERILOG_PROGRAM_TYPE_CHECK_H
#define CASCADE_SRC_VERILOG_PROGRAM_TYPE_CHECK_H

#include <string>
#include "verilog/ast/ast.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class Log;
class Parser;
class Program;

class TypeCheck : public Visitor {
  public:
    // Constructors:
    // 
    // Creates a type checker with references to a program for context, and a
    // log in which to write error messages or warnings. If parser is provided,
    // it is assumed that it was used to generate the code which is resulting
    // in an error or a warning.
    TypeCheck(const Program* program, Log* log, const Parser* parser = nullptr);
    ~TypeCheck() override = default;

    // Configuration Interface:
    void deactivate(bool val);
    void declaration_check(bool val);
    void local_only(bool val);

    // Pre-elaboration Checking Interface:
    void pre_elaboration_check(const ModuleInstantiation* mi);
    void pre_elaboration_check(const CaseGenerateConstruct* cgc);
    void pre_elaboration_check(const IfGenerateConstruct* igc);
    void pre_elaboration_check(const LoopGenerateConstruct* lgc);

    // Post-Elaboration Checking Interface:
    void post_elaboration_check(const Node* n);

  private:
    // Auxiliary References:
    const Program* program_;
    Log* log_;
    const Parser* parser_;

    // Configuration Flags:
    bool deactivated_;
    bool decl_check_;
    bool local_only_;

    // Location Tracking:
    const Node* outermost_loop_;
    const ModuleInstantiation* instantiation_;
    bool net_lval_;

    // Error Tracking:
    bool exists_bad_id_;

    // Logging Helpers:
    void warn(const std::string& s, const Node* n);
    void error(const std::string& s, const Node* n);
    void multiple_def(const Node* n);

    // Visitor Interface:
    void visit(const Event* e) override;
    void visit(const EofExpression* ee) override;
    void visit(const Identifier* id) override;
    void visit(const String* s) override;
    void visit(const GenerateBlock* gb) override;
    void visit(const ModuleDeclaration* md) override;
    void visit(const CaseGenerateConstruct* cgc) override;
    void visit(const IfGenerateConstruct* igc) override;
    void visit(const LoopGenerateConstruct* lgc) override;
    void visit(const InitialConstruct* ic) override;
    void visit(const ContinuousAssign* ca) override;
    void visit(const GenvarDeclaration* id) override;
    void visit(const IntegerDeclaration* id) override;
    void visit(const LocalparamDeclaration* ld) override;
    void visit(const NetDeclaration* nd) override;
    void visit(const ParameterDeclaration* pd) override;
    void visit(const RegDeclaration* rd) override;
    void visit(const ModuleInstantiation* mi) override;
    void visit(const BlockingAssign* ba) override;
    void visit(const NonblockingAssign* na) override;
    void visit(const ParBlock* pb) override;
    void visit(const SeqBlock* sb) override;
    void visit(const CaseStatement* cs) override;
    void visit(const ConditionalStatement* cs) override;
    void visit(const ForStatement* fs) override;
    void visit(const ForeverStatement* fs) override;
    void visit(const RepeatStatement* rs) override;
    void visit(const WhileStatement* ws) override;
    void visit(const DisplayStatement* ds) override;
    void visit(const ErrorStatement* es) override;
    void visit(const GetStatement* gs) override;
    void visit(const InfoStatement* is) override;
    void visit(const PutStatement* ps) override;
    void visit(const RestartStatement* rs) override;
    void visit(const RetargetStatement* rs) override;
    void visit(const SaveStatement* ss) override;
    void visit(const SeekStatement* ss) override;
    void visit(const WarningStatement* ws) override;
    void visit(const WriteStatement* ws) override;
    void visit(const WaitStatement* ws) override;
    void visit(const DelayControl* dc) override;

    // Checks whether a range is little-endian and begins at 0
    void check_width(const RangeExpression* re);
    // Checks whether array dimensions are little-endian and begin at 0
    void check_array(Identifier::const_iterator_dim begin, Identifier::const_iterator_dim end);
    // Checks whether a potentially subscripted identifier is a valid array
    // dereference, returns a pointer to the last unused element in its
    // dimensions so that further operations may check its slice.
    Identifier::const_iterator_dim check_deref(const Identifier* r, const Identifier* i);
    // Instantiation Array Checking Helpers:
    void check_arity(const ModuleInstantiation* mi, const Identifier* port, const Expression* arg);
    // Checks well-formedness of printf-style args
    template <typename ArgsItr>
    void check_printf(size_t n, ArgsItr begin, ArgsItr end);
};

template <typename ArgsItr>
void TypeCheck::check_printf(size_t n, ArgsItr begin, ArgsItr end) {
  // Nothing to do if no args were provided
  if (begin == end) {
    return;
  }
  // If first arg is a string, number of %'s must be one less than number of args
  if ((*begin)->is(Node::Tag::string)) {
    const auto* str = static_cast<const String*>(*begin);
    if (static_cast<size_t>(std::count(str->get_readable_val().begin(), str->get_readable_val().end(), '%')) != (n-1)) {
      error("Incorrect number of printf-style arguments provided", (*begin)->get_parent());
    }
  }
  // If first arg is not a string, it must be the only arg
  else if (n != 1) {
    error("printf-style argument formatting requires a string", (*begin)->get_parent());
  }
}

} // namespace cascade

#endif
