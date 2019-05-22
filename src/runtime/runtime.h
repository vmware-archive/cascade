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

#include <ctime>
#include <functional>
#include <iosfwd>
#include <mutex>
#include <string>
#include <vector>
#include "base/bits/bits.h"
#include "base/thread/asynchronous.h"
#include "base/log/log.h"
#include "runtime/ids.h"
#include "verilog/ast/ast_fwd.h"

namespace cascade {

class Compiler;
class DataPlane;
class Isolate;
class Log;
class Module;
class Parser;
class Program;

class Runtime : public Asynchronous {
  public:
    // Typedefs:
    typedef std::function<void()> Interrupt;

    // Constructors:
    explicit Runtime();
    ~Runtime() override;

    // Configuration Interface:
    Runtime& set_compiler(Compiler* c);
    Runtime& set_include_dirs(const std::string& s);
    Runtime& set_open_loop_target(size_t olt);
    Runtime& disable_inlining(bool di);

    // Controller Interface:
    // 
    // Invokes eval_stream() on this string in the gap between this and the
    // next timestep.  Returns immediately. Code which is successfully eval'ed
    // will begin execution at the beginning of the following timestep.
    void eval(const std::string& s);
    // Invokes eval_stream() in the gap between this and the next timestep.
    // Returns immediately. Code which is successfully eval'ed will begin
    // execution at the beginning of the following timestep.
    void eval(std::istream& is, bool is_term);

    // System Task Interface:
    //
    // Print a newline-teriminated string between this and the next timestep.
    void display(const std::string& s);
    // Print a string between this and the next timestep. 
    void write(const std::string& s);
    // Shutdown the runtime and print statistics if arg is non-zero between
    // this and the next timestep. 
    void finish(uint32_t arg);
    // Prints an error message between this and the next timestep. 
    void error(const std::string& s);
    // Prints a warning message between this and the next timestep. 
    void warning(const std::string& s);
    // Prints an info message between this and the next timestep. 
    void info(const std::string& s);
    // Loads the current state of the simulation from a file
    void restart(const std::string& path);
    // Schedules a recompilation of the current program to a new march target in
    // between this and the next timestep. Returns immediately.
    void retarget(const std::string& s);
    // Saves the current state of the simulation to a file
    void save(const std::string& path);

    // Program-Logic Interface:
    //
    // Schedules an interrupt on the interrupt queue. Returns false if the
    // runtime is no longer active, meaning there is no hope of this interrupt
    // ever being serviced.
    bool schedule_interrupt(Interrupt int_);
    // Writes a value to the dataplane. Invoking this method to insert
    // arbitrary values may be useful for simulating noisy circuits. However in
    // general, the use of this method is probably best left to modules which
    // are going about their normal execution.
    void write(VId id, const Bits* bits);
    // Writes a value to the dataplane. Invoking this method to insert
    // arbitrary values may be useful for simulating noisy circuits. However in
    // general, the use of this method is probably best left to modules which
    // are going about their normal execution.
    void write(VId id, bool b);

    // Stream I/O Interface:
    //
    // In contrast to the system task and program logic interfaces, these
    // methods are scheduled immediately, regardless of the execution state of
    // the runtime. From a design perspective, this interface is a thin wrapper
    // around streambufs. Attempting to perform an operation on an unrecognized
    // or closed stream id will result in undefined behavior.
    // 
    // Creates an entry in the stream table by calling new filebuf(path, in|out).
    SId fopen(const std::string& path);
    // Returns an entry in the stream table
    std::streambuf* rdbuf(SId id) const;
    // Replaces an entry in the stream table and returns its previous value
    std::streambuf* rdbuf(SId id, std::streambuf* sb);
    // Streambuf operators: The boolean argument to pubseekoff/pos is used to
    // select between read/write (true/false) pointers. pubseekoff assumes
    // std::cur as its locator.
    int32_t in_avail(SId id);
    uint32_t pubseekoff(SId id, int32_t n, bool r);
    uint32_t pubseekpos(SId id, int32_t n, bool r);
    int32_t pubsync(SId id);
    int32_t sbumpc(SId id);
    int32_t sgetc(SId id);
    uint32_t sgetn(SId id, char* c, uint32_t n);
    int32_t sputc(SId id, char c);
    uint32_t sputn(SId id, const char* c, uint32_t n);

    // Profiling Interface:
    //
    // Returns the logical simulation time. This method is thread-safe and can
    // be invoked at any time.
    uint64_t time() const;
    // Returns the current rate at which the runtime executes the simulation loop.
    // This method is thread-safe and can be invoked at any time.
    std::string current_frequency() const;
    // Returns the overall rate at which the runtime executed the simulation loop.
    // This method is thread-safe and can be invoked at any time.
    std::string overall_frequency() const;

  private:
    static constexpr uint32_t stdin_ = 0x8000'0000;
    static constexpr uint32_t stdout_ = 0x8000'0001;
    static constexpr uint32_t stderr_ = 0x8000'0002;
    static constexpr uint32_t stdwarn_ = 0x8000'0003;
    static constexpr uint32_t stdinfo_ = 0x8000'0004;
    static constexpr uint32_t stdlog_ = 0x8000'0005;

    // Major Components:
    Log* log_;
    Parser* parser_;
    DataPlane* dp_;
    Isolate* isolate_;
    Compiler* compiler_;

    // Program State:
    Program* program_;
    Module* root_;

    // Configuration State:
    bool disable_inlining_;
    bool enable_open_loop_;
    size_t open_loop_itrs_;
    size_t open_loop_target_;

    // Interrupt Queue:
    size_t item_evals_;
    std::vector<Interrupt> ints_;
    std::recursive_mutex int_lock_;

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
    bool eval_stream(std::istream& is, bool is_term);
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
    // Drains the interrupt queue
    void drain_interrupts();
    // Invokes done_simulation on every module, completing the simulation
    void done_simulation();

    // Runs in open loop until timeout or a system task is triggered
    void open_loop_scheduler();
    // Runs a single iteration of the reference scheduling algoirthm
    void reference_scheduler();

    // Logging Helpers
    //
    // Dumps parse errors to the console
    void log_parse_errors();
    // Dumps typechecking warnings to the console
    void log_checker_warns();
    // Dumps typechecking errors to the console
    void log_checker_errors();
    // Dumps compilation errors to the console
    void log_compiler_errors();
    // Dumps ctrl-d intercept
    void log_ctrl_d();

    // Time Keeping Helpers:
    //
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
