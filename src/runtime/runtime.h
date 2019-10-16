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

#ifndef CASCADE_SRC_RUNTIME_RUNTIME_H
#define CASCADE_SRC_RUNTIME_RUNTIME_H

#include <condition_variable>
#include <ctime>
#include <functional>
#include <iosfwd>
#include <mutex>
#include <string>
#include <vector>
#include "common/bits.h"
#include "common/log.h"
#include "common/thread.h"
#include "common/thread_pool.h"
#include "runtime/ids.h"
#include "target/engine.h"
#include "verilog/ast/ast_fwd.h"

namespace cascade {

class Compiler;
class DataPlane;
class Isolate;
class Log;
class Module;
class Parser;
class Program;

class Runtime : public Thread {
  public:
    // Constexprs:
    static constexpr FId stdin_ = 0x8000'0000;
    static constexpr FId stdout_ = 0x8000'0001;
    static constexpr FId stderr_ = 0x8000'0002;
    static constexpr FId stdwarn_ = 0x8000'0003;
    static constexpr FId stdinfo_ = 0x8000'0004;
    static constexpr FId stdlog_ = 0x8000'0005;

    // Typedefs:
    typedef std::function<void()> Interrupt;
    typedef ThreadPool::Job Asynchronous;

    // Constructors:
    explicit Runtime();
    ~Runtime() override;

    // Configuration Interface:
    // 
    // These methods should all be invoked prior to starting the runtime
    // thread. Invoking these methods afterwards is undefined.
    Runtime& set_include_dirs(const std::string& s);
    Runtime& set_open_loop_target(size_t olt);
    Runtime& set_disable_inlining(bool di);
    Runtime& set_profile_interval(size_t n);

    // Major Component Accessors and Helpers:
    //
    // These methods may be invoked after starting the runtime thread, but the
    // use of these pointers should be handled with care, as these objects have
    // their own constraints on defined behavior.
    Compiler* get_compiler();
    DataPlane* get_data_plane();
    Isolate* get_isolate();
    Engine::Id get_next_id();

    // Eval Interface:
    //
    // Evaluates the next element from an input stream at the end of the
    // current time step. Blocks until completion. Returns a pair indicating
    // whether the end-of-file was reached and whether an error occurred.
    std::pair<bool, bool> eval(std::istream& is);
    // Identical to eval(), but loops until either an end-of-file was reached
    // or an error occurs.
    std::pair<bool, bool> eval_all(std::istream& is);

    // Scheduling Interface:
    //
    // Schedules an interrupt to run between this and the next time step.
    // Interrupts which would execute after a call to finish() will fizzle.
    // This method is non-blocking and returns false if a call to finish() has
    // already executed, meaning the interrupt is guaranteed to fizzle.  Note
    // that returning true does not guarantee that it won't.
    bool schedule_interrupt(Interrupt int_);
    // Identical to the single argument form, but alt is executed instead of
    // int_ if the call to int_ fizzles.
    bool schedule_interrupt(Interrupt int_, Interrupt alt);
    // Identical to the single argument form, but blocks until the interrupt
    // completes execution or fizzles.
    void schedule_blocking_interrupt(Interrupt int_);
    // Identical to the two argument form, but blocks until the interrupt
    // completes execution or alt is executed in its place.
    void schedule_blocking_interrupt(Interrupt int_, Interrupt alt);
    // Schedules an interrupt either between a call to save and restart if the
    // simulation is currently active, or immediately if a call to finish
    // already took place. This method blocks until completion.
    void schedule_state_safe_interrupt(Interrupt int__);
    // Schedules an asynchronous task. Asynchronous tasks begin execution out
    // of phase with the logic time and may begin and end mid-step. If an
    // asynchronous task invokes any of the schedule_xxx_interrupt methods, it
    // must use the two-argument form.
    void schedule_asynchronous(Asynchronous async);
    // Returns true if the runtime has executed a finish statement.
    bool is_finished() const;

    // System Task Interface:
    //
    // Schedules a $debug() at the end of this step and returns immediately.
    void debug(uint32_t action, const std::string& arg);
    // Executes a $finish() and returns immediately.
    void finish(uint32_t arg);
    // Schedules a $restart() at the end of this step and returns immediately.
    void restart(const std::string& path);
    // Schedules a $retarget() at the end of this step and returns immediately.
    void retarget(const std::string& s);
    // Schedules a $save() at the end of this step and returns immediately.
    void save(const std::string& path);

    // Stream I/O Interface:
    //
    // Returns an entry in the stream table
    std::streambuf* rdbuf(FId id) const;
    // Replaces an entry in the stream table and returns its previous value
    std::streambuf* rdbuf(FId id, std::streambuf* sb);
    // Creates an entry in the stream table 
    FId fopen(const std::string& path, uint8_t mode);
    // Streambuf operators:
    int32_t in_avail(FId id);
    uint32_t pubseekoff(FId id, int32_t off, uint8_t way, uint8_t which);
    uint32_t pubseekpos(FId id, int32_t pos, uint8_t which);
    int32_t pubsync(FId id);
    int32_t sbumpc(FId id);
    int32_t sgetc(FId id);
    uint32_t sgetn(FId id, char* c, uint32_t n);
    int32_t sputc(FId id, char c);
    uint32_t sputn(FId id, const char* c, uint32_t n);

  private:
    // Thread Pool:
    ThreadPool pool_;

    // Major Components:
    Log* log_;
    Parser* parser_;
    Compiler* compiler_;
    DataPlane* dp_;
    Isolate* isolate_;

    // Program State:
    Program* program_;
    Module* root_;
    Engine::Id next_id_;

    // Configuration State:
    bool disable_inlining_;
    bool enable_open_loop_;
    size_t open_loop_itrs_;
    size_t open_loop_target_;
    size_t profile_interval_;

    // Interrupt Queue:
    bool finished_;
    size_t item_evals_;
    std::vector<Interrupt> ints_;
    std::recursive_mutex int_lock_;
    std::mutex block_lock_;
    std::condition_variable block_cv_;

    // Generic Scheduling State:
    std::vector<Module*> logic_;
    std::vector<Module*> done_logic_;
    bool schedule_all_;

    // Optimized Scheduling State:
    Module* clock_;
    Module* inlined_logic_;

    // Time Keeping:
    time_t begin_time_;
    time_t last_time_;
    time_t last_check_;
    uint64_t last_logical_time_;
    uint64_t logical_time_;

    // Stream Table:
    std::vector<std::streambuf*> streambufs_;

    // Implements the semantics of the Verilog Simulation Reference Model and
    // services interrupts between logical simulation steps.
    void run_logic() override;

    // REPL Helpers:
    //
    // Repeatedly parses and then invokes eval_nodes() on the contents of an
    // istream until either an error occurs or end of file is encountered.
    void eval_stream(std::istream& is);
    // Invokes eval_node() on each of the elements in an iterator range.
    // Deletes elements in the range which follow a failed eval.
    template <typename InputItr>
    bool eval_nodes(InputItr begin, InputItr end);
    // Evals a module declaration, a module item, or an include statement. 
    bool eval_node(Node* n);
    // Evals a module declaration. Well-formed code is saved in the typechecker.
    bool eval_decl(ModuleDeclaration* md);
    // Evals a module item. Well-formed code will execute at the next time step.
    bool eval_item(ModuleItem* mi);

    // Module Hierarchy Helpers:
    // 
    // Instantiates new submodules below the root in the module hierarchy and
    // recompiles engine logic as necessary.
    void rebuild();

    // Verilog Simulation Loop Scheduling Helpers:
    //
    // Drains the active queue
    void drain_active();
    // Drains update events for all modules with updates. Return true if doing
    // so resulted in new active events.
    bool drain_updates();
    // Invokes done_step on every module, completing the logical simulation step
    void done_step();
    // Invokes done_simulation on every module, completing the simulation
    void done_simulation();
    // Drains the interrupt queue
    void drain_interrupts();

    // Runs in open loop until timeout or a system task is triggered
    void open_loop_scheduler();
    // Runs a single iteration of the reference scheduling algoirthm
    void reference_scheduler();

    // Logging Helpers
    //
    // Dumps parse errors to stderr
    void log_parse_errors();
    // Dumps typechecking warnings to stdwarn
    void log_checker_warns();
    // Dumps typechecking errors to stderr
    void log_checker_errors();
    // Dumps compilation errors to stderr
    void log_compiler_errors();
    // Dumps an event notification to stdlog
    void log_event(const std::string& type, Node* n = nullptr);
    // Dumps the current virtual clock frequency to stdlog
    void log_freq();

    // Debug Helpers:
    //
    // Resolves an id in the program. Returns nullptr on failure.
    const Node* resolve(const std::string& arg);
    // Prints a code listing for n.
    void list(const Node* n);
    // Prints the scopes in n. This method is undefined for ids which don't
    // point to scopes.
    void showscopes(const Node* n);
    // Prints the scopes in and below n. This method is undefined for ids which
    // don't point to scopes.
    void recursive_showscopes(const Node* n);
    // Prints info for n if n is a variable. This method is undefined for ids
    // which don't point to variables.
    void showvars(const Identifier* id);
    // Prints info for all of the variables below n. This method is undefined
    // for ids which don't point to scopes.
    void recursive_showvars(const Node* n);

    // Time Keeping Helpers:
    //
    // Returns the current virtual clock frequency.
    std::string current_frequency() const;
    // Returns the overall virtual clock frequency.
    std::string overall_frequency() const;
    // Prints a frequency in either MHz, KHz, or Hz. No GHz. We wish.
    std::string format_freq(uint64_t f) const;
};

template <typename InputItr>
inline bool Runtime::eval_nodes(InputItr begin, InputItr end) {
  auto res = true;
  for (; res && (begin != end); ++begin) {
    res = eval_node(*begin);
  }
  for (; begin != end; ++begin) {
    delete *begin;
  }
  return res;
}

} // namespace cascade

#endif
