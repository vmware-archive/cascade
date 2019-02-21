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

#include "src/runtime/runtime.h"

#include <cassert>
#include <cctype>
#include <iostream>
#include <limits>
#include <sstream>
#include "src/base/stream/incstream.h"
#include "src/base/stream/indstream.h"
#include "src/base/system/system.h"
#include "src/runtime/data_plane.h"
#include "src/runtime/isolate.h"
#include "src/runtime/module.h"
#include "src/target/compiler.h"
#include "src/target/engine.h"
#include "src/ui/view.h"
#include "src/verilog/parse/parser.h"
#include "src/verilog/print/debug/debug_printer.h"
#include "src/verilog/program/inline.h"
#include "src/verilog/program/program.h"

using namespace std;

namespace cascade {

Runtime::Runtime(View* view) : Asynchronous() {
  view_ = view;

  log_ = new Log();
  parser_ = new Parser();
  dp_ = new DataPlane();
  isolate_ = new Isolate();
  compiler_ = new Compiler();

  program_ = new Program();
  root_ = nullptr;

  enable_open_loop_ = false;
  open_loop_itrs_ = 2;
  open_loop_target_ = 1;
  disable_inlining_ = false;
  enable_info_ = false;
  disable_warning_ = false;
  disable_error_ = false;

  item_evals_ = 0;

  schedule_all_ = false;
  clock_ = nullptr;
  inlined_logic_ = nullptr;

  begin_time_ = ::time(nullptr);
  last_time_ = ::time(nullptr);
  logical_time_ = 0;
}

Runtime::~Runtime() {
  delete program_;
  if (root_ != nullptr) {
    delete root_;
  }

  delete log_;
  delete parser_;
  delete dp_;
  delete isolate_;
  delete compiler_;
}

Runtime& Runtime::set_compiler(Compiler* c) {
  delete compiler_;
  compiler_ = c;
  return *this;
}

Runtime& Runtime::set_include_dirs(const string& s) {
  include_dirs_ = s;
  return *this;
}

Runtime& Runtime::set_open_loop_target(size_t olt) {
  open_loop_target_ = olt;
  return *this;
}

Runtime& Runtime::disable_inlining(bool di) {
  disable_inlining_ = di;
  return *this;
}

Runtime& Runtime::enable_info(bool ei) {
  enable_info_ = ei;
  return *this;
}

Runtime& Runtime::disable_warning(bool dw) {
  disable_warning_ = dw;
  return *this;
}

Runtime& Runtime::disable_error(bool de) {
  disable_error_ = de;
  return *this;
}

void Runtime::eval(const string& s) {
  auto* ss = new stringstream(s);
  schedule_interrupt([this, ss]{
    log_->clear();
    eval_stream(*ss, false);
    delete ss;
  });
}

void Runtime::eval(istream& is, bool is_term) {
  schedule_interrupt([this, &is, is_term]{
    log_->clear();
    eval_stream(is, is_term);
  });
}

void Runtime::display(const string& s) {
  schedule_interrupt([this, s]{
    view_->print(logical_time_, s + "\n");
  });
}

void Runtime::write(const string& s) {
  schedule_interrupt([this, s]{
    view_->print(logical_time_, s);
  });
}

void Runtime::finish(int arg) {
  schedule_interrupt([this, arg]{
    if (arg > 0) {
      stringstream ss;
      ss << "Simulation Time: " << time() << endl;
      ss << "Wall Clock Time: " << (::time(nullptr) - begin_time_) << "s" << endl;
      ss << "Clock Frequency: " << overall_frequency() << endl;
      view_->print(logical_time_, ss.str());
    } 
    request_stop();
  });
}

void Runtime::error(const string& s) {
  if (!disable_error_) {
    schedule_interrupt([this, s]{
      view_->error(logical_time_, s);
    });
  }
}

void Runtime::warning(const string& s) {
  if (!disable_warning_) {
    schedule_interrupt([this, s]{
      view_->warn(logical_time_, s);
    });
  }
}

void Runtime::info(const string& s) {
  if (enable_info_) {
    schedule_interrupt([this, s]{
      view_->info(logical_time_, s);
    });
  }
}

void Runtime::fatal(int arg, const string& s) {
  error(s);
  finish(arg);
}

void Runtime::restart(const string& path) {
  schedule_interrupt([this, path]{
    // As with retarget(), invoking this method in a state where item_evals_ >
    // 0 can be problematic. This condition guarantees safety.
    if (item_evals_ > 0) {
      return restart(path);
    }
    ifstream ifs(path);
    if (!ifs.is_open()) {
      return fatal(0, "Unable to open save file '" + path + "'!");
    }
    root_->restart(ifs);
  });
}   

void Runtime::retarget(const string& s) {
  schedule_interrupt([this, s]{
    // An unfortunate corner case: Have some evals been processed by the
    // interrupt queue? Remember that Module::rebuild() can only be invoked in
    // a state where there are no unhandled evals. Fortunately, there's an easy
    // fix: just reinvoke retarget().  This will reschedule us on the far side
    // of Runtime::rebuild() and it'll be safe to invoke Module::rebuild().
    if (item_evals_ > 0) {
      return retarget(s);
    }
    // Give up if we can't open the march file which was requested
    incstream ifs(System::src_root());
    if (!ifs.open("data/march/" + s + ".v")) {
      return fatal(0, "Unrecognized march option '" + s + "'!");
    }

    // Temporarily relocate program_ and root_ so that we can scan the contents
    // of this file using the eval_stream() infrastructure. We'll delete the
    // temporary data-structures when we're done.
    auto* march = program_;
    program_ = new Program();
    auto* backup_root = root_;
    root_ = nullptr;
    eval_stream(ifs, false);
    assert(!log_->error());
    std::swap(march, program_);
    std::swap(backup_root, root_);
    item_evals_ = 0;

    // Replace attribute annotations for every elaborated module (this includes the
    // root, which is where logic inherits its annotations from).
    for (auto i = program_->elab_begin(), ie = program_->elab_end(); i != ie; ++i) {
      auto* std1 = i->second->get_attrs()->get<String>("__std");
      assert(std1 != nullptr);

      auto found = false;
      for (auto j = march->elab_begin(), je = march->elab_end(); j != je; ++j) {
        auto* std2 = j->second->get_attrs()->get<String>("__std");
        assert(std2 != nullptr);
       
        if (std1->get_readable_val() == std2->get_readable_val()) {
          i->second->replace_attrs(j->second->get_attrs()->clone());
          found = true; 
          break;
        }   
      }
      if (!found) {
        delete march;
        delete backup_root;
        return fatal(0, "New target does not support modules with standard type " + std1->get_readable_val() + "!");
      }
    }

    // Delete temporaries and rebuild the program
    delete march;
    delete backup_root;
    root_->rebuild();
  });
}

void Runtime::save(const string& path) {
  schedule_interrupt([this, path]{
    // As with retarget(), invoking this method in a state where item_evals_ >
    // 0 can be problematic. This condition guarantees safety.
    if (item_evals_ > 0) {
      return save(path);
    }
    ofstream ofs(path);
    root_->save(ofs);
  });
}

void Runtime::schedule_interrupt(Interrupt int_) {
  lock_guard<recursive_mutex> lg(int_lock_);
  ints_.push_back(int_);
}

void Runtime::write(VId id, const Bits* bits) {
  dp_->write(id, bits);
}

void Runtime::write(VId id, bool b) {
  dp_->write(id, b);
}

uint64_t Runtime::time() const {
  return logical_time_;
}

string Runtime::current_frequency() const {
  const auto now = ::time(nullptr);
  const auto den = (now == last_time_) ? 1 : (now - last_time_);
  const auto res = (logical_time_ - last_logical_time_) / 2 / den; 

  *const_cast<time_t*>(&last_time_) = now;
  *const_cast<uint64_t*>(&last_logical_time_) = logical_time_;

  return format_freq(res);
}

string Runtime::overall_frequency() const {
  const auto now = ::time(nullptr);
  const auto den = (now == begin_time_) ? 1 : (now - begin_time_);
  return format_freq(logical_time_ / 2 / den); 
}

void Runtime::run_logic() {
  view_->startup(logical_time_);
  while (!stop_requested()) {
    if (enable_open_loop_ && !schedule_all_) {
      open_loop_scheduler();
    } else {
      reference_scheduler();
    }
  }
  done_simulation();
  view_->shutdown(logical_time_);
}

bool Runtime::eval_stream(istream& is, bool is_term) {
  auto res = true;
  while (res) {
    parser_->parse(is, log_); 
    const auto text = parser_->get_text();
    const auto depth = parser_->get_depth();
    schedule_interrupt([this, text, depth]{
      view_->parse(logical_time_, depth, text);
    });

    // Stop eval'ing as soon as we enounter a parse error, and return false.
    if (log_->error()) {
      if (is_term) {
        is.ignore(numeric_limits<streamsize>::max(), '\n');
      }
      log_parse_errors();
      return false;
    } 
    // An eof marks end of stream, return the last result, and trigger finish
    // if the eof appeared on the term
    if (parser_->eof()) {
      if (is_term) {
        log_ctrl_d();
      }
      return res;
    }
    // Eval the code we just parsed; if this is the term, don't loop
    res = eval_nodes(parser_->begin(), parser_->end());
    if (is_term) {
      return res;
    }
  }
  return res;
}

bool Runtime::eval_node(Node* n) {
  if (n->is(Node::Tag::string)) {
    auto* s = static_cast<String*>(n);
    return eval_include(s);
  } else if (n->is(Node::Tag::module_declaration)) {
    auto* md = static_cast<ModuleDeclaration*>(n);
    return eval_decl(md);
  } else if (n->is_subclass_of(Node::Tag::module_item)) {
    auto* mi = static_cast<ModuleItem*>(n);
    return eval_item(mi);
  } else {
    assert(false);
    return false;
  }
}

bool Runtime::eval_include(String* s) {
  const auto& path = s->get_readable_val();
  delete s;

  incstream ifs(include_dirs_);
  if (!ifs.open(path)) {
    error("Unable to resolve include directive >>> include " + path + "; <<<");
    error("  Check your filename or try adding additional include paths using the -I option.");
    if (isspace(path.back())) {
      error("  The filename you provided also has trailing whitespace. Did you mean to do that?");
    }
    return false;
  }
  schedule_interrupt([this, path]{
    view_->include(logical_time_, path);
  });

  parser_->push(path);
  const auto res = eval_stream(ifs, false);
  parser_->pop();

  return res;
}

bool Runtime::eval_decl(ModuleDeclaration* md) {
  program_->declare(md, log_, parser_);
  log_checker_warns();
  if (log_->error()) {
    log_checker_errors();
    return false;
  }
  if (disable_inlining_) {
    md->get_attrs()->set_or_replace("__no_inline", new String("true"));
  }
  schedule_interrupt([this, md]{
    view_->decl(logical_time_, program_, md);
  });
  return true;
}

bool Runtime::eval_item(ModuleItem* mi) {
  program_->eval(mi, log_, parser_); 
  log_checker_warns();
  if (log_->error()) {
    log_checker_errors();
    return false;
  }
  schedule_interrupt([this]{
    view_->item(logical_time_, program_, program_->root_elab()->second);
  });

  // If the root is empty, we just instantiated it. Otherwise, count this as an
  // item instantiated within the root.
  const auto* src = program_->root_elab()->second;
  if (src->empty_items()) {
    root_ = new Module(src, this, dp_, isolate_, compiler_);
  } else {
    ++item_evals_;
  }
  return true;
}

void Runtime::rebuild() {
  // Nothing to do if something went wrong with the compiler after the last
  // time this method was called. Trigger a fatal interrupt that will be
  // handled before the next time step.
  if (compiler_->error()) {
    return log_compiler_errors();
  }
  // Also nothing to do if no items have been eval'ed since the last time this
  // method was called
  if (item_evals_ == 0) {
    return;
  } 

  // Inline as much as we can and compile whatever is new. Reset the item_evals_
  // counter as soon as we're done.
  program_->inline_all();
  root_->synchronize(item_evals_);
  item_evals_ = 0;
  if (compiler_->error()) {
    return log_compiler_errors();
  } 

  // Clear scheduling state
  logic_.clear();
  done_logic_.clear();
  clock_ = nullptr;
  inlined_logic_ = nullptr;
  // Reconfigure scheduling state and determine whether optimizations are possible
  for (auto* m : *root_) {
    if (m->engine()->is_stub()) {
      continue;
    }
    logic_.push_back(m);
    if (m->engine()->is_clock()) {
      clock_ = m;
    }
    if (m->engine()->is_logic()) {
      inlined_logic_ = m;
    }
    if (m->engine()->overrides_done_step()) {
      done_logic_.push_back(m);
    }
  }
  schedule_all_ = true;
  enable_open_loop_ = (logic_.size() == 2) && (clock_ != nullptr) && (inlined_logic_ != nullptr);
}

void Runtime::drain_active() {
  for (auto done = false; !done; ) {
    done = true;
    for (auto* m : logic_) {
      if (schedule_all_ || m->engine()->there_are_reads()) {
        m->engine()->evaluate();
        done = false;
      }
    }
    schedule_all_ = false;
  }
}

bool Runtime::drain_updates() {
  auto performed_update = false;
  for (auto* m : logic_) {
    if (m->engine()->conditional_update()) {
      performed_update = true;
    }
  }
  if (!performed_update) {
    return false;
  }
  auto performed_evaluate = false;
  for (auto* m : logic_) {
    if (m->engine()->conditional_evaluate()) {
      performed_evaluate = true;
    }
  }
  return performed_evaluate;
}

void Runtime::done_step() {
  for (auto* m : done_logic_) {
    m->engine()->done_step();
  }
}

void Runtime::drain_interrupts() {
  // Performance Note:
  // This is an inner loop method, so we shouldn't be grabbing a lock here
  // unless we absolutely have to. This method is only ever called between
  // logical time steps, so there's no reason to worry about an engine
  // scheduling a system task interrupt here. What we do have to worry about
  // are things like asynchronous jit handoffs.
  
  // Fast Path: 
  // Leave immediately if there are no interrupts scheduled. This isn't thread
  // safe, but the only asynchronous events we need to consider here are jit
  // handoffs or evals. Since the only thing we risk is a false negative, and
  // whether we handle the event now or during next timestep doesn't really
  // matter, this is fine. 
  if (ints_.empty()) {
    return;
  }

  // Slow Path: 
  // We have at least one interrupt. System tasks are benign, but what could be
  // here is an eval event (which will require a code rebuild) or a jit handoff
  // (which in addition to the eval event, could trigger a fatal compiler
  // error). Since we're already on the slow path here, schedule a call at the
  // very end of the interrupt queue to first check whether the compiler is in
  // a sound state (ie, jit handoff hasn't failed) and then to rebuild the
  // codebase.
  lock_guard<recursive_mutex> lg(int_lock_);
  schedule_interrupt([this]{
    rebuild();
  });
  for (size_t i = 0; i < ints_.size() && !stop_requested(); ++i) {
    ints_[i]();
  }
  ints_.clear();
}

void Runtime::done_simulation() {
  for (auto* m : logic_) {
    m->engine()->done_simulation();
  }
}

void Runtime::open_loop_scheduler() {
  // Record the current time, go open loop, and then record how long we were gone for
  const size_t then = ::time(nullptr);
  const auto val = clock_->engine()->get_bit(1);
  const auto itrs = inlined_logic_->engine()->open_loop(1, val, open_loop_itrs_);
  const size_t now = ::time(nullptr);

  // If we ran for an odd number of iterations, flip the clock
  if (itrs % 2) {
    clock_->engine()->set_bit(1, !val);
  }
  // Drain the interrupt queue and fix up the logical time
  drain_interrupts();
  logical_time_ += itrs;

  // Update open loop iterations based on our target
  const auto delta = now - then;
  auto next = open_loop_itrs_;
  if ((delta < open_loop_target_) && (open_loop_itrs_ == itrs)) {
    next <<= 1;
  } else if (delta > open_loop_target_) {
    next >>= 1;
  }
  open_loop_itrs_ = (next > 0) ? next : open_loop_itrs_;
}

void Runtime::reference_scheduler() {
  while (schedule_all_ || drain_updates()) {
    drain_active();
  }
  done_step();
  drain_interrupts();
  ++logical_time_;
}

void Runtime::log_parse_errors() {
  stringstream ss;
  ss << "Parse Error:";

  indstream is(ss);
  is.tab();
  for (auto e = log_->error_begin(), ee = log_->error_end(); e != ee; ++e) {
    is << "\n> ";
    is.tab();
    is << *e;
    is.untab();
  }
  error(ss.str());
}

void Runtime::log_checker_warns() {
  if (log_->warn_begin() == log_->warn_end()) {
    return;
  }

  stringstream ss;
  ss << "Typechecker Warning:";

  indstream is(ss);
  is.tab();
  for (auto w = log_->warn_begin(), we = log_->warn_end(); w != we; ++w) {
    is << "\n> ";
    is.tab();
    is << *w;
    is.untab();
  }
  warning(ss.str());
}

void Runtime::log_checker_errors() {
  stringstream ss;
  ss << "Typechecker Error:";

  indstream is(ss);
  is.tab();
  for (auto e = log_->error_begin(), ee = log_->error_end(); e != ee; ++e) {
    is << "\n> ";
    is.tab();
    is << *e;
    is.untab();
  }
  error(ss.str());
}

void Runtime::log_compiler_errors() {
  fatal(0, "Internal Compiler Error:\n  > " + compiler_->what());
}

void Runtime::log_ctrl_d() {
  fatal(0, "User Interrupt:\n  > Caught Ctrl-D.");
}

string Runtime::format_freq(uint64_t f) const {
  stringstream ss;
  ss.precision(2);
  if (f > 1000000) {
    ss << fixed << (f/1000000) << " MHz";
  } else if (f > 1000) {
    ss << fixed << (f/1000) << " KHz";
  } else {
    ss << fixed << f << " Hz";
  }
  return ss.str();
}

} // namespace cascade
