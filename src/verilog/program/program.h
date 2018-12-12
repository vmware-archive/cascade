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

#ifndef CASCADE_SRC_VERILOG_PROGRAM_PROGRAM_H
#define CASCADE_SRC_VERILOG_PROGRAM_PROGRAM_H

#include <vector>
#include "src/base/log/log.h"
#include "src/base/undo/undo_map.h"
#include "src/verilog/analyze/indices.h"
#include "src/verilog/ast/visitors/editor.h"

namespace cascade {

class Program : public Editor {
  public:
    // Iterators:
    typedef typename ManagedUndoMap<const Identifier*, ModuleDeclaration*, HashId, EqId>::const_iterator decl_iterator;
    typedef typename UndoMap<const Identifier*, ModuleDeclaration*, HashId, EqId>::const_iterator elab_iterator;

    // Constructors:
    Program();
    Program(ModuleDeclaration* md);
    Program(ModuleDeclaration* md, ModuleInstantiation* mi);
    ~Program();

    // Configuration Interface:
    Program& typecheck(bool tc);

    // Program Building Interface:
    //
    // Declares a new module. If this is the first declaration, it is assumed
    // to be the root. Thereafter, modules without annotations inherit the
    // annotations of the root declaration.
    void declare(ModuleDeclaration* md);
    // Convenience method. Invokes() declare and then evaluates an
    // automatically generated instantiation of this module.
    void declare_and_instantiate(ModuleDeclaration* md);
    // Evaluates a new module item; instantiations may in turn result in
    // further evaluations. If this is the first instantiation, it is assumed
    // to be the root. Thereafter, instantiations without annotations inherit
    // the annotations of the root instantiation.
    void eval(ModuleItem* mi);

    // Program Transformation Interface:
    void inline_all();
    void outline_all();

    // Top-level Source Interface:
    const ModuleDeclaration* src() const;

    // Log Interface:
    const Log& get_log() const;

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
    ManagedUndoMap<const Identifier*, ModuleDeclaration*, HashId, EqId> decls_;
    UndoMap<const Identifier*, ModuleDeclaration*, HashId, EqId> elabs_;

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

    // Error Log:
    Log log_;

    // Elaboration Helpers:
    void elaborate(Node* n);
    void elaborate_item(ModuleItem* mi);

    // Eval Helpers:
    void eval_root(ModuleItem* mi);
    void eval_item(ModuleItem* mi);

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
