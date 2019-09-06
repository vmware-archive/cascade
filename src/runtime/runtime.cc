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

#include "runtime/runtime.h"

#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include "common/incstream.h"
#include "common/indstream.h"
#include "common/system.h"
#include "runtime/data_plane.h"
#include "runtime/isolate.h"
#include "runtime/module.h"
#include "runtime/nullbuf.h"
#include "target/compiler/local_compiler.h"
#include "target/engine.h"
#include "verilog/parse/parser.h"
#include "verilog/print/text/text_printer.h"
#include "verilog/program/inline.h"
#include "verilog/program/program.h"

using namespace std;

namespace cascade {

Runtime::Runtime() : Thread() {
  pool_.set_num_threads(4);
  pool_.run();

  log_ = new Log();
  parser_ = new Parser(log_);
  compiler_ = new LocalCompiler(this);
  dp_ = new DataPlane();
  isolate_ = new Isolate();

  program_ = new Program();
  root_ = nullptr;
  next_id_ = 0;

  enable_open_loop_ = false;
  open_loop_itrs_ = 2;
  open_loop_target_ = 1;
  disable_inlining_ = false;

  finished_ = false;
  item_evals_ = 0;

  schedule_all_ = false;
  clock_ = nullptr;
  inlined_logic_ = nullptr;

  begin_time_ = ::time(nullptr);
  last_time_ = ::time(nullptr);
  logical_time_ = 0;

  // Allocate standard streams 
  for (size_t i = 0; i < 6; ++i) {
    rdbuf(i, nullptr);
  }
}

Runtime::~Runtime() {
  // INVARIANT: The runtime should always be in a state where finish was called
  // before teardown. This guarantees that outstanding async threads won't
  // attempt to schedule interrupts that will never be serviced.

  stop_now();
  if (!finished_) {
    finish(0);
    run();
    stop_now();
  }

  // INVARIANT: At this point we know that the interrupt queue is empty and no
  // new asyncrhonous threads have been started (the only place this happens is
  // inside interrupts).

  compiler_->stop_compile();
  compiler_->stop_async();
  pool_.stop_now();

  // INVARIANT: All outstanding asynchronous threads have finished executing,
  // and any interrupts scheduled by those threads have either fizzled or had
  // their alternate callbacks executed. 
  
  // INVARIANT: There is now a single active thread of control in the runtime
  // and it is safe to tear down state.

  delete program_;
  if (root_ != nullptr) {
    delete root_;
  }

  delete log_;
  delete parser_;
  delete compiler_;
  delete dp_;
  delete isolate_;

  for (auto* s : streambufs_) {
    delete s;
  }
}

Runtime& Runtime::set_include_dirs(const string& s) {
  parser_->set_include_dirs(s);
  return *this;
}

Runtime& Runtime::set_open_loop_target(size_t olt) {
  open_loop_target_ = olt;
  return *this;
}

Runtime& Runtime::set_disable_inlining(bool di) {
  disable_inlining_ = di;
  return *this;
}

Runtime& Runtime::set_profile_interval(size_t n) {
  profile_interval_ = n;
  last_check_ = ::time(nullptr);
  return *this;
}

DataPlane* Runtime::get_data_plane() {
  return dp_;
}

Compiler* Runtime::get_compiler() {
  return compiler_;
}

Isolate* Runtime::get_isolate() {
  return isolate_;
}

Engine::Id Runtime::get_next_id() {
  return next_id_++;
}

pair<bool, bool> Runtime::eval(istream& is) {
  log_->clear();
  const auto eof = parser_->parse(is);
  auto err = log_->error();

  if (err) {
    log_parse_errors();
  } else {
    schedule_blocking_interrupt([this, &is, &err]{
      err = !eval_nodes(parser_->begin(), parser_->end());
    });
  }
  return make_pair(eof, err);
}

pair<bool, bool> Runtime::eval_all(istream& is) {
  auto eof = false;
  auto err = false;
  schedule_blocking_interrupt([this, &is, &eof, &err]{
    while (!eof && !err) {
      log_->clear();
      eof = parser_->parse(is);
      err = log_->error();
      if (err) {
        log_parse_errors();
      } else {
        err = !eval_nodes(parser_->begin(), parser_->end());
      }
    }
  });
  return make_pair(eof, err);
}

bool Runtime::schedule_interrupt(Interrupt int_) {
  lock_guard<recursive_mutex> lg(int_lock_);
  if (finished_) {
    return false;
  }
  ints_.push_back([this, int_]{
    if (!finished_) {
      int_();
    }    
  });
  return true;
}

bool Runtime::schedule_interrupt(Interrupt int_, Interrupt alt) {
  lock_guard<recursive_mutex> lg(int_lock_);
  if (finished_) {
    alt();
    return false;
  }
  ints_.push_back([this, int_, alt]{
    if (!finished_) {
      int_();
    } else {
      alt();  
    }
  });
  return true;
}

void Runtime::schedule_blocking_interrupt(Interrupt int_) {
  unique_lock<mutex> lg(block_lock_);
  if (schedule_interrupt(int_)) {
    block_cv_.wait(lg);
  }
}

void Runtime::schedule_blocking_interrupt(Interrupt int_, Interrupt alt) {
  unique_lock<mutex> lg(block_lock_);
  if (schedule_interrupt(int_, alt)) {
    block_cv_.wait(lg);
  }
}

void Runtime::schedule_state_safe_interrupt(Interrupt int__) {
  schedule_blocking_interrupt(
    [this, int__]{
      // As with retarget(), invoking this method in a state where item_evals_ >
      // 0 can be problematic. This condition guarantees safety.
      if (item_evals_ > 0) {
        return schedule_state_safe_interrupt(int__);
      }
      stringstream ss;
      root_->save(ss);
      int__();
      root_->restart(ss);
    },
    int__
  );
}

void Runtime::schedule_asynchronous(Asynchronous async) {
  pool_.insert(async); 
}

bool Runtime::is_finished() const {
  return finished_;
}

void Runtime::finish(uint32_t arg) {
  if (arg > 0) {
    ostream(rdbuf(stdout_)) 
      << "Simulation Time: " << logical_time_ << "\n"
      << "Wall Clock Time: " << (::time(nullptr) - begin_time_) << "s" << "\n"
      << "Clock Frequency: " << overall_frequency() << endl;
  } 
  request_stop();
  finished_ = true;
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
      ostream(rdbuf(stderr_)) << "Unable to open save file '" << path << "'\"!" << endl;
      finish(0);
      return;
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
      ostream(rdbuf(stderr_)) << "Unrecognized march option '" << s << "'!" << endl;
      finish(0);
      return;
    }

    // Temporarily relocate program_ and root_ so that we can scan the contents
    // of this file using the eval_stream() infrastructure.  Also relocate the
    // parser, as it will skip include guards that it's already seen.
    auto* march = program_;
    program_ = new Program();
    auto* backup_root = root_;
    root_ = nullptr;
    auto* backup_parser = parser_;
    parser_ = new Parser(log_);

    // Read the march file
    eval_stream(ifs);
    assert(!log_->error());

    // Wap everything back into place and restore original state
    std::swap(march, program_);
    std::swap(backup_root, root_);
    std::swap(backup_parser, parser_);
    delete backup_parser;
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
        ostream(rdbuf(stderr_)) << "New target does not support modules with standard type " << std1->get_readable_val() << "!" << endl;
        finish(0);
        return;
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

FId Runtime::fopen(const std::string& path, uint8_t mode) {
  auto* fb = new filebuf();
  auto m = ios_base::in;
  switch (mode) {
    case 1: m = ios_base::out; break;
    case 2: m = ios_base::app; break;
    case 3: m = ios_base::in | ios_base::out; break;
    case 4: m = ios_base::in | ios_base::trunc; break;
    case 5: m = ios_base::in | ios_base::app; break;
    default: break;
  }
  fb->open(path.c_str(), m);
  streambufs_.push_back(fb);

  return (streambufs_.size()-1);;
}

streambuf* Runtime::rdbuf(FId id) const {
  const auto fid = id & 0x7fff'ffff;
  assert(fid < streambufs_.size());
  assert(streambufs_[fid] != nullptr);

  return streambufs_[fid];
}

streambuf* Runtime::rdbuf(FId id, streambuf* sb) {
  const auto fid = id & 0x7fff'ffff;
  while (fid >= streambufs_.size()) {
    streambufs_.push_back(new nullbuf());
  }
  auto* res =  streambufs_[fid];
  if (sb != nullptr) {
    streambufs_[fid] = sb;
  }
  return res;
}

int32_t Runtime::in_avail(FId id) {
  return rdbuf(id)->in_avail();
}

uint32_t Runtime::pubseekoff(FId id, int32_t off, uint8_t way, uint8_t which) {
  auto d = ios_base::cur;
  switch (way) {
    case 1: d = ios_base::beg; break;
    case 2: d = ios_base::end; break;
    default: break;
  }
  auto o = ios_base::openmode();
  switch (which) {
    case 1: o = ios_base::in; break;
    case 2: o = ios_base::out; break;
    case 3: o = ios_base::in | ios_base::out; break;
    default: break;
  }
  return rdbuf(id)->pubseekoff(off, d, o);
}

uint32_t Runtime::pubseekpos(FId id, int32_t pos, uint8_t which) {
  auto o = ios_base::openmode();
  switch (which) {
    case 1: o = ios_base::in; break;
    case 2: o = ios_base::out; break;
    case 3: o = ios_base::in | ios_base::out; break;
    default: break;
  }
  return rdbuf(id)->pubseekpos(pos, o);
}

int32_t Runtime::pubsync(FId id) {
  return rdbuf(id)->pubsync();
}

int32_t Runtime::sbumpc(FId id) {
  return rdbuf(id)->sbumpc();
}

int32_t Runtime::sgetc(FId id) {
  return rdbuf(id)->sgetc();
}

uint32_t Runtime::sgetn(FId id, char* c, uint32_t n) {
  return rdbuf(id)->sgetn(c, n);
}

int32_t Runtime::sputc(FId id, char c) {
  // Squelch puts which take place after a call to finish
  return !finished_ ? rdbuf(id)->sputc(c) : c;
}

uint32_t Runtime::sputn(FId id, const char* c, uint32_t n) {
  // Squelch puts which take place after a call to finish
  return !finished_ ? rdbuf(id)->sputn(c, n) : n;
}

void Runtime::run_logic() {
  if (logical_time_ == 0) {
    log_event("BEGIN");
  }
  if (finished_) {
    return;
  }
  while (!stop_requested() && !finished_) {
    if (enable_open_loop_ && !schedule_all_) {
      open_loop_scheduler();
    } else {
      reference_scheduler();
    }
    log_freq();
  }
  if (finished_) {
    done_simulation();
    log_event("END");
  }
}

void Runtime::eval_stream(istream& is) {
  for (auto res = true; res; ) {
    log_->clear();
    const auto eof = parser_->parse(is);

    // Stop eval'ing as soon as we enounter a parse error, and return false.
    if (log_->error()) {
      log_parse_errors();
      return;
    } 
    // An eof marks end of stream, return the last result, and trigger finish
    // if the eof appeared on the term
    if (eof) {
      return;
    }
    // Eval the code we just parsed; if this is the term, only loop for as
    // long as we're inside of an include statement.
    res = eval_nodes(parser_->begin(), parser_->end());
  }
}

bool Runtime::eval_node(Node* n) {
  log_event("PARSE", n);
  if (n->is(Node::Tag::module_declaration)) {
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

  log_event("DECL_OK");
  return true;
}

bool Runtime::eval_item(ModuleItem* mi) {
  program_->eval(mi, log_, parser_); 
  log_checker_warns();
  if (log_->error()) {
    log_checker_errors();
    return false;
  }

  // If the root has the standard six definitions, we just instantiated it.
  // Otherwise, count this as an item instantiated within the root.
  const auto* src = program_->root_elab()->second;
  if (src->size_items() == 6) {
    root_ = new Module(src, this);
    item_evals_ = 6;
  } else {
    ++item_evals_;
  }

  log_event("ITEM_OK");
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
  // Reconfigure scheduling state 
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

  // Determine whether we can reenter open loop in this state. If we can, make
  // sure to adjust open_loop_itrs_ back to something manageable for software!
  // Otherwise we'll hang in the next call to open_loop.
  enable_open_loop_ = (logic_.size() == 2) && (clock_ != nullptr) && (inlined_logic_ != nullptr);
  open_loop_itrs_ = 2;
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

void Runtime::done_simulation() {
  for (auto* m : logic_) {
    m->engine()->done_simulation();
  }
}

void Runtime::drain_interrupts() {
  lock_guard<recursive_mutex> lg(int_lock_);

  // Fast Path: No interrupts
  if (ints_.empty()) {
    return;
  }
  // Slow Path: 
  // We have at least one interrupt.  which could be an eval event (which will
  // require a code rebuild) or a jit handoff.  Schedule a call at the very end
  // of the interrupt queue to first check whether the compiler is in a sound
  // state (ie, jit handoff hasn't failed) and then to rebuild the codebase.
  schedule_interrupt([this]{
    rebuild();
  });
  for (size_t i = 0; i < ints_.size(); ++i) {
    ints_[i]();
  }
  ints_.clear();
  block_cv_.notify_all();
}

void Runtime::open_loop_scheduler() {
  // Record the current time, go open loop, and then record how long we were
  // gone for.  
  const size_t then = ::time(nullptr);
  const auto id = clock_->engine()->get_clock_id();
  const auto val = clock_->engine()->get_clock_val();
  const auto itrs = inlined_logic_->engine()->open_loop(id, val, open_loop_itrs_);
  const size_t now = ::time(nullptr);

  // If we ran for an odd number of iterations, flip the clock
  if (itrs % 2) {
    clock_->engine()->set_clock_val(!val);
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
  ostream os(rdbuf(stderr_));
  os << "Parse Error:";

  indstream is(os);
  is.tab();
  for (auto e = log_->error_begin(), ee = log_->error_end(); e != ee; ++e) {
    is << "\n> ";
    is.tab();
    is << *e;
    is.untab();
  }
  os << endl;
}

void Runtime::log_checker_warns() {
  if (log_->warn_begin() == log_->warn_end()) {
    return;
  }

  ostream os(rdbuf(stdwarn_));
  os << "Typechecker Warning:";

  indstream is(os);
  is.tab();
  for (auto w = log_->warn_begin(), we = log_->warn_end(); w != we; ++w) {
    is << "\n> ";
    is.tab();
    is << *w;
    is.untab();
  }
  os << endl;
}

void Runtime::log_checker_errors() {
  ostream os(rdbuf(stderr_));
  os << "Typechecker Error:";

  indstream is(os);
  is.tab();
  for (auto e = log_->error_begin(), ee = log_->error_end(); e != ee; ++e) {
    is << "\n> ";
    is.tab();
    is << *e;
    is.untab();
  }
  os << endl;
}

void Runtime::log_compiler_errors() {
  ostream(rdbuf(stderr_)) << "Internal Compiler Error:\n  > " << compiler_->what() << endl;
  finish(0);
}

void Runtime::log_event(const string& type, Node* n) {
  stringstream ss;
  ss << "*** " << type << " @ " << logical_time_;
  if (n != nullptr) {
    TextPrinter(ss) << "\n" << n;  
  }
  auto s = ss.str();

  auto event = [this, s]{
    ostream(rdbuf(stdlog_)) << s << endl;
  };
  schedule_interrupt(event, event);
}

void Runtime::log_freq() {
  if (profile_interval_ == 0) {
    return;
  }
  if ((::time(nullptr) - last_check_) < profile_interval_) {
    return;
  }
  auto event = [this]{
    last_check_ = ::time(nullptr);
    ostream(rdbuf(stdlog_)) << "*** PROF @ " << logical_time_ << "\n" << current_frequency() << endl;
  };
  schedule_interrupt(event, event);
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
