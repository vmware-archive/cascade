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

#ifndef CASCADE_SRC_VERILOG_PRINT_DEBUG_DEBUG_PRINTER_H
#define CASCADE_SRC_VERILOG_PRINT_DEBUG_DEBUG_PRINTER_H

#include <cassert>
#include <unordered_map>
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/navigate.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/print/term/term_printer.h"
#include "verilog/print/text/text_printer.h"
#include "verilog/program/elaborate.h"
#include "verilog/program/inline.h"

namespace cascade {

template <typename T>
class DebugPrinter : public T {
  public:
    DebugPrinter(std::ostream& os, bool gen = false, bool inl = false);
    ~DebugPrinter() override = default;

  private:
    const ModuleDeclaration* md_;
    std::unordered_map<const Identifier*, size_t> ids_;
    bool gen_;
    bool inl_;

    void visit(const Identifier* id) override;
    void visit(const ModuleDeclaration* md) override;
    void visit(const IfGenerateConstruct* igc) override;
    void visit(const CaseGenerateConstruct* cgc) override;
    void visit(const LoopGenerateConstruct* lgc) override;
    void visit(const ParameterDeclaration* pd) override;
    void visit(const ModuleInstantiation* mi) override;
    void visit(const PortDeclaration* pd) override;

    void debug_id(ModuleInfo& mi, const Identifier* id);
    void debug_conn(ModuleInfo& mi, const Identifier* id);
    void debug_scope(const Node* n, size_t d);
};

using DebugTermPrinter = DebugPrinter<TermPrinter>;
using DebugTextPrinter = DebugPrinter<TextPrinter>;

template <typename T>
inline DebugPrinter<T>::DebugPrinter(std::ostream& os, bool gen, bool inl) : T(os) { 
  md_ = nullptr;
  gen_ = gen;
  inl_ = inl;
}

template <typename T>
inline void DebugPrinter<T>::visit(const Identifier* id) {
  T::visit(id);
  
  // If we can't resolve this variable (say because it's an attrspec or a
  // module type or something is just plain broken) give up
  auto* r = Resolve().get_resolution(id);
  if (r == nullptr) {
    return;
  }
  // If we aren't printing a module declaration, things like local vs external
  // annotations don't make sense.
  if (md_ == nullptr) {
    return;
  }

  // Otherwise, annotate this identifier as either local, an external
  // reference, or a connection.
  *this << Color::GREY;
  if (ModuleInfo(md_).is_local(r)) {
    *this << "(l";
  } else if (ModuleInfo(md_).is_external(r)) {
    *this << "(e";
  } else if (ModuleInfo(md_).is_child(r)) {
    *this << "(c";
  } else {
    *this << "(?";
  }
  // Print out the unique id associated with the identifier that we
  // obtained through resolution.
  if (ids_.find(r) == ids_.end()) {
    ids_[r] = ids_.size();
  }
  *this << ids_[r] << ")" << Color::RESET;
}

template <typename T>
void DebugPrinter<T>::visit(const ModuleDeclaration* md) {
  // Record md for use in printing identifiers below here.
  md_ = md;
  T::visit(md_);

  ModuleInfo mi(md_);
  *this << Color::GREY << "\n\nLocal Variables:" << Color::RESET;
  if (mi.locals().empty()) {
    *this << Color::GREY << "\n  (none)" << Color::RESET;
  } else {
    for (auto* l : mi.locals()) {
      debug_id(mi, l);
    }
  }
  *this << Color::GREY << "\n\nExternal References:" << Color::RESET;
  if (mi.externals().empty()) {
    *this << Color::GREY << "\n  (none)" << Color::RESET;
  } else {
    for (auto* e : mi.externals()) {
      debug_id(mi, e);
    }
  }
  *this << Color::GREY << "\n\nSubmodules:" << Color::RESET;
  if (mi.children().empty()) {
    *this << Color::GREY << "\n  (none)" << Color::RESET;
  } else {
    for (const auto& c : mi.children()) {
      debug_conn(mi, c.first);
    }
  }
  *this << Color::GREY << "\n\nScope Tree:" << Color::RESET;
  debug_scope(md, 1);

  // Reset md. We're done printing a module declaration.
  md_ = nullptr;
}

template <typename T>
void DebugPrinter<T>::visit(const IfGenerateConstruct* igc) {
  if (!gen_) {
    return T::visit(igc);
  }

  *this << Color::GREY << "// if (...)" << Color::RESET;
  if (Elaborate().is_elaborated(igc)) {
    *this << "\n";
    Elaborate().get_elaboration(igc)->accept(this);
  }
}

template <typename T>
void DebugPrinter<T>::visit(const CaseGenerateConstruct* cgc) {
  if (!gen_) {
    return T::visit(cgc);
  }

  *this << Color::GREY << "// case (...)" << Color::RESET;
  if (Elaborate().is_elaborated(cgc)) {
    *this << "\n";
    Elaborate().get_elaboration(cgc)->accept(this);
  }
}

template <typename T>
void DebugPrinter<T>::visit(const LoopGenerateConstruct* lgc) {
  if (!gen_) {
    return T::visit(lgc);
  }

  *this << Color::GREY << "// for (...)" << Color::RESET;
  if (Elaborate().is_elaborated(lgc)) {
    *this << "\n";
    for (auto* b : Elaborate().get_elaboration(lgc)) {
      b->accept(this);
    }
  }
}

template <typename T>
void DebugPrinter<T>::visit(const ParameterDeclaration* pd) {
  if (md_ == nullptr) {
    return T::visit(pd);
  }

  auto* id = pd->get_id();
  auto& op = ModuleInfo(Resolve().get_origin(id)).ordered_params();

  for (size_t i = 0, ie = op.size(); i < ie; ++i) {
    if (op[i] == id) {
      *this << Color::GREY << "(#" << i << ") " << Color::RESET;
      return T::visit(pd);
    }
  }
  *this << Color::GREY << "(#?) " << Color::RESET;
  T::visit(pd);
}

template <typename T>
void DebugPrinter<T>::visit(const ModuleInstantiation* mi) {
  if (!inl_ || !Inline().is_inlined(mi)) {
    return T::visit(mi);
  } 
  *this << Color::GREY << "// " << mi->get_mid() << " " << mi->get_iid() << "(...)\n" << Color::RESET;
  Inline().get_source(mi)->accept(this); 
}

template <typename T>
void DebugPrinter<T>::visit(const PortDeclaration* pd) {
  if (md_ == nullptr) {
    return T::visit(pd);
  }

  auto* id = pd->get_decl()->get_id();
  auto& op = ModuleInfo(Resolve().get_origin(id)).ordered_ports();

  for (size_t i = 0, ie = op.size(); i < ie; ++i) {
    if (op[i] == id) {
      *this << Color::GREY << "(#" << i << ") " << Color::RESET;
      return T::visit(pd);
    }
  }
  *this << Color::GREY << "(#?) " << Color::RESET;
  T::visit(pd);
}

template <typename T>
void DebugPrinter<T>::debug_id(ModuleInfo& mi, const Identifier* id) {
  *this << "\n  ";
  auto* decl = id->get_parent();
  if (decl->get_parent()->is(Node::Tag::port_declaration)) {
    decl = decl->get_parent();
  }
  decl->accept(this);

  *this << "\n    ";
  const auto* fid = Resolve().get_full_id(id);
  fid->accept(this);
  delete fid;
  if (mi.is_stateful(id)) {
    *this << Color::GREY << " (stateful)" << Color::RESET;
  }
  if (mi.is_input(id)) {
    *this << Color::GREY << " (input)" << Color::RESET;
  }
  if (mi.is_output(id)) {
    *this << Color::GREY << " (output)" << Color::RESET;
  }
  if (mi.is_read(id)) {
    *this << Color::GREY << " (read)" << Color::RESET;
  }
  if (mi.is_write(id)) {
    *this << Color::GREY << " (write)" << Color::RESET;
  }
}

template <typename T>
void DebugPrinter<T>::debug_conn(ModuleInfo& mi, const Identifier* id) {
  // Careful not to descend on inlined module instantiations here
  const auto inl = inl_;
  inl_ = false;

  *this << "\n  ";
  assert(id->get_parent()->is(Node::Tag::module_instantiation));
  const auto* inst = static_cast<const ModuleInstantiation*>(id->get_parent());
  inst->accept(this);

  const auto* md = Elaborate().get_elaboration(inst);
  if (md == nullptr) {
    return;
  }
  ModuleInfo sub_mi(md);
  for (const auto& c : mi.connections().find(id)->second) {
    *this << "\n    ";
    c.first->accept(this);
    *this << (sub_mi.is_input(c.first) ? " <- " : " -> ");
    c.second->accept(this);
  }

  // Restore gen flag
  inl_ = inl;
}

template <typename T>
void DebugPrinter<T>::debug_scope(const Node* n, size_t d) {
  Navigate nav(n);

  *this << "\n";
  for (size_t i = 0; i < d; ++i) {
    *this << "  ";
  }
  if (nav.name() == nullptr) {
    *this << "(decl scope)";
  } else {
    *this << nav.name();
  }

  for (auto n = nav.name_begin(), ne = nav.name_end(); n != ne; ++n) {
    *this << "\n";
    for (size_t i = 0; i < (d+1); ++i) {
      *this << "  "; 
    }
    *this << *n;
  }
  for (auto c = nav.child_begin(), ce = nav.child_end(); c != ce; ++c) {
    if ((*c)->is(Node::Tag::module_declaration)) {
      continue;
    }
    debug_scope(*c, d+1);
  }
}

} // namespace cascade

#endif
