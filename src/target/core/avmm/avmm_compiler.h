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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_AVMM_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_AVMM_AVMM_COMPILER_H

#include <condition_variable>
#include <map>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>
#include "target/compiler.h"
#include "target/core_compiler.h"
#include "target/core/avmm/avmm_logic.h"
#include "target/core/avmm/avmm_rewrite.h"
#include "target/core/avmm/program_boxer.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/ast/ast.h"

#include <fstream>
#include "common/system.h"
#include "cascade/cascade.h"
#include "target/core/avmm/syncbuf.h"

namespace cascade::avmm {

class AvmmCompiler : public CoreCompiler {
  public:
    AvmmCompiler();
    ~AvmmCompiler() override = default;

    void release(size_t slot);
    void stop_compile(Engine::Id id) override;

  private:
    // Compilation States:
    enum class State : uint8_t {
      FREE = 0,
      COMPILING,
      WAITING,
      STOPPED,
      CURRENT
    };
    // Slot Information:
    struct Slot {
      Engine::Id id;
      State state;
      std::string text;
    };

    // State:
    Cascade* caslib_;
    syncbuf requests_;
    syncbuf responses_;

    // Program Management:
    std::mutex lock_;
    std::condition_variable cv_;
    std::vector<Slot> slots_;

    // Compiler Interface:
    AvmmLogic* compile_logic(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;

    // Compilation Helpers:
    bool id_in_use(Engine::Id id) const;
    int get_free_slot() const;
    void compile();
    void reprogram();
    void kill_all();
};

inline AvmmCompiler::AvmmCompiler() : CoreCompiler() {
  slots_.resize(4, {0, State::FREE, ""});
  
  caslib_ = nullptr;
}

inline void AvmmCompiler::release(size_t slot) {
  // Return this slot to the pool if necessary. This method should only be
  // invoked on successfully compiled cores, which means we don't have to worry
  // about transfering compilation ownership or invoking a killall.
  std::lock_guard<std::mutex> lg(lock_);
  assert(slots_[slot].state == State::CURRENT);
  slots_[slot].state = State::FREE;
  cv_.notify_all();
}

inline void AvmmCompiler::stop_compile(Engine::Id id) {
  // Nothing to do if this id isn't in use
  std::lock_guard<std::mutex> lg(lock_);
  if (!id_in_use(id)) {
    return;
  }

  // Free any slot with this id which is in the compiling or waiting state.
  auto need_new_owner = false;
  for (auto& s : slots_) {
    if (s.id == id) {
      switch (s.state) {
        case State::COMPILING:
          need_new_owner = true;
          s.state = State::STOPPED;
          // fallthrough
        case State::WAITING:
          s.state = State::STOPPED;
          break;
        default:
          break;
      } 
    }
  }
  // If we need a new compilation lead, find a waiting slot and promote it.
  // Note that there might not be any more waiting slots. That's fine.
  if (need_new_owner) {
    for (auto& s : slots_) {
      if (s.state == State::WAITING) {
        s.state = State::COMPILING;
        break;
      }
    }
  }

  cv_.notify_all();
}

inline AvmmLogic* AvmmCompiler::compile_logic(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  std::unique_lock<std::mutex> lg(lock_);
  ModuleInfo info(md);

  // Check for unsupported language features
  auto unsupported = false;
  if (info.uses_mixed_triggers()) {
    get_compiler()->error("The Avmm backend does not currently support code with mixed triggers!");
    unsupported = true;
  } else if (!info.implied_latches().empty()) {
    get_compiler()->error("The Avmm backend does not currently support the use of implied latches!");
    unsupported = true;
  }
  if (unsupported) {
    delete md;
    return nullptr;
  }

  // Find a new slot and generate code for this module. If either step fails,
  // return nullptr. Otherwise, advance the sequence counter and compile.
  const auto slot = get_free_slot();
  if (slot == -1) {
    get_compiler()->error("No remaining slots available on avmm fabric");
    delete md;
    return nullptr;
  }
  
  // Create a new core with address identity based on module id
  auto* de = new AvmmLogic(interface, md, requests_, responses_);

  // Register inputs, state, and outputs. Invoke these methods
  // lexicographically to ensure a deterministic variable table ordering. The
  // final invocation of index_tasks is lexicographic by construction, as it's
  // based on a recursive descent of the AST.
  std::map<VId, const Identifier*> is;
  for (auto* i : info.inputs()) {
    is.insert(std::make_pair(to_vid(i), i));
  }
  for (const auto& i : is) {
    de->set_input(i.second, i.first);
  }
  std::map<VId, const Identifier*> ss;
  for (auto* s : info.stateful()) {
    ss.insert(std::make_pair(to_vid(s), s));
  }
  for (const auto& s : ss) {
    de->set_state(s.second, s.first);
  }
  std::map<VId, const Identifier*> os;
  for (auto* o : info.outputs()) {
    os.insert(std::make_pair(to_vid(o), o));
  }
  for (const auto& o : os) {
    de->set_output(o.second, o.first);
  }
  de->index_tasks();

  // Check table and index sizes. If this program uses too much state, we won't
  // be able to uniquely name its elements using our current addressing scheme.
  if (de->get_table().size() >= 0x1000) {
    get_compiler()->error("Unable to compile a module with more than 4096 entries in variable table");
    delete de;
    return nullptr;
  }

  // Downgrade any compilation slots to waiting slots, and stop any slots that are
  // working on this id.
  for (auto& s : slots_) {
    if (s.state == State::COMPILING) {
      s.state = State::WAITING;
    }
    if ((s.id == id) && (s.state == State::WAITING)) {
      s.state = State::STOPPED;
    }
  }

  slots_[slot].id = id;
  slots_[slot].state = State::COMPILING;
  slots_[slot].text = AvmmRewrite().run(md, de, slot);

  while (true) {
    switch (slots_[slot].state) {
      case State::COMPILING: {
        compile();
        reprogram();
        break;
      }
      case State::WAITING:
        cv_.wait(lg);
        break;
      case State::STOPPED:
        slots_[slot].state = State::FREE;
        delete de;
        return nullptr;
      case State::CURRENT:
        de->set_callback([this, slot]{release(slot);});
        return de;
      default:
        // Control should never reach here
        assert(false);
        break;
    }
  }
}

inline bool AvmmCompiler::id_in_use(Engine::Id id) const {
  for (const auto& s : slots_) {
    if ((s.id == id) && (s.state != State::FREE)) {
      return true;
    }
  }
  return false;
}

inline int AvmmCompiler::get_free_slot() const {
  for (size_t i = 0, ie = slots_.size(); i < ie; ++i) {
    if (slots_[i].state == State::FREE) {
      return i;
    }
  }
  return -1;
}

inline void AvmmCompiler::compile() {
  ProgramBoxer pb;
  for (size_t i = 0, ie = slots_.size(); i < ie; ++i) {
    if (slots_[i].state != State::FREE) {
      pb.push(i, slots_[i].text);
    }
  }
  const auto text = pb.get();
  std::ofstream ofs(System::src_root() + "/src/target/core/avmm/fpga/program_logic.v", std::ofstream::out);
  ofs << text << std::endl;
  ofs.flush();
  ofs.close();
  
  caslib_ = new Cascade();
  int ifd = caslib_->open(&requests_);
  int ofd = caslib_->open(&responses_);
  caslib_->run();
  (*caslib_) << "`include \"data/march/minimal.v\"\n";
  (*caslib_) << "integer ifd = " << ifd << ";\n";
  (*caslib_) << "integer ofd = " << ofd << ";\n";
  (*caslib_) << "`include \"src/target/core/avmm/fpga/avmm_wrapper.v\"\n";
  (*caslib_) << std::endl;
}

inline void AvmmCompiler::reprogram() {
  for (auto& s : slots_) {
    if ((s.state == State::COMPILING) || (s.state == State::WAITING)) {
      s.state = State::CURRENT;
    }     
  }
  cv_.notify_all();
}

inline void AvmmCompiler::kill_all() {
  if (caslib_ != nullptr) {
    delete caslib_;
  }
}

} // namespace cascade::avmm

#endif