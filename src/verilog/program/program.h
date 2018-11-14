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

#ifndef CASCADE_SRC_VERILOG_PROGRAM_PROGRAM_H
#define CASCADE_SRC_VERILOG_PROGRAM_PROGRAM_H

#include <vector>
#include "src/base/undo/undo_map.h"
#include "src/verilog/analyze/indices.h"
#include "src/verilog/ast/visitors/editor.h"

namespace cascade {

class Log;
class Parser;

class Program : public Editor {
  public:
    // Iterators:
    typedef typename ManagedUndoMap<const Identifier*, false, ModuleDeclaration*, true, HashId, EqId>::const_iterator decl_iterator;
    typedef typename ManagedUndoMap<const Identifier*, true, ModuleDeclaration*, false, HashId, EqId>::const_iterator elab_iterator;

    // Constructors:
    // 
    // Creates an empty program with no top-level module. 
    Program();
    // Creates a program by invoking declare_and_instantiate on md. Invoking
    // this constructor with a declaration which contains a type error is
    // undefined.
    Program(ModuleDeclaration* md);
    // Creates a program by declaring md and instantiating mi. Invoking this
    // constructor with declaration or instantiation which contain a type error
    // is undefined.
    Program(ModuleDeclaration* md, ModuleInstantiation* mi);
    // Destructor
    ~Program();

    // Configuration Interface:
    //
    // Determines whether or not to disable typechecking
    Program& typecheck(bool tc);

    // Program Building Interface:
    //
    // Declares a new module. If this is the first declaration, it is assumed
    // to be the root. Thereafter, modules without annotations inherit the
    // annotations of the root declaration. Writes errors and warnings to log.
    // If provided, p is assumed to have generated md, and will be used to look
    // up location information for logging.
    bool declare(ModuleDeclaration* md, Log* log, const Parser* p = nullptr);
    // Convenience method. Invokes() declare and then evaluates an
    // automatically generated instantiation of this module. Writes errors and
    // warnings to log. If provided, p is assumed to have generated md, and
    // will be used to look up locaiton information for logging.
    bool declare_and_instantiate(ModuleDeclaration* md, Log* log, const Parser* p = nullptr);
    // Evaluates a new module item; instantiations may in turn result in
    // further evaluations. If this is the first instantiation, it is assumed
    // to be the root. Thereafter, instantiations without annotations inherit
    // the annotations of the root instantiation. Writes errors and warnings to
    // log. If provided, p is assumed to have generated md, and will be used to
    // look up location information for logging. 
    bool eval(ModuleItem* mi, Log* log, const Parser* p = nullptr);

    // Program Transformation Interface:
    // 
    // Inline all modules with type __std == logic
    void inline_all();
    // Outline all modules
    void outline_all();

    // Top-level Source Interface:
    // 
    // Returns a pointer to the top-level instantiated module
    const ModuleDeclaration* src() const;

    // Declaration Iterators:
    decl_iterator root_decl() const;
    decl_iterator decl_find(const Identifier* id) const;
    decl_iterator decl_begin() const; 
    decl_iterator decl_end() const; 

    // Instantiated Source Iterators:
    elab_iterator root_elab() const;
    elab_iterator elab_find(const Identifier* id) const;
    elab_iterator elab_begin() const;
    elab_iterator elab_end() const;

  private:
    // Source:
    ModuleInstantiation* root_inst_;
    ManagedUndoMap<const Identifier*, false, ModuleDeclaration*, true, HashId, EqId> decls_;
    ManagedUndoMap<const Identifier*, true, ModuleDeclaration*, false, HashId, EqId> elabs_;

    // Root Iterators:
    decl_iterator root_ditr_;
    elab_iterator root_eitr_;  

    // Elaboration State:
    std::vector<ModuleInstantiation*> inst_queue_;
    std::vector<GenerateConstruct*> gen_queue_;

    // Configuration Flags:
    bool checker_off_;
    bool decl_check_;
    bool local_only_;
    bool expand_insts_;
    bool expand_gens_;

    // Elaboration Helpers:
    void elaborate(Node* n, Log* log, const Parser* p);
    void elaborate_item(ModuleItem* mi, Log* log, const Parser* p);

    // Eval Helpers:
    void eval_root(ModuleItem* mi, Log* log, const Parser* p);
    void eval_item(ModuleItem* mi, Log* log, const Parser* p);

    // Code Generation Boundaries:
    void edit(ModuleInstantiation* mi) override;
    void edit(CaseGenerateConstruct* cgc) override;
    void edit(IfGenerateConstruct* igc) override;
    void edit(LoopGenerateConstruct* lgc) override;

    // Inlining Helpers:
    void inline_all(ModuleDeclaration* md);
    void outline_all(ModuleDeclaration* md);
};

} // namespace cascade

#endif
