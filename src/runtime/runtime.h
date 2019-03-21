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
#include "src/base/bits/bits.h"
#include "src/base/thread/asynchronous.h"
#include "src/base/log/log.h"
#include "src/runtime/ids.h"
#include "src/verilog/ast/ast_fwd.h"

namespace cascade {

class Compiler;
class DataPlane;
class Isolate;
class Log;
class Module;
class Parser;
class Program;
class View;

// At the top level, FPGA-JIT is organized according to the MVC design pattern.
// The user interacts with the program through the controller, observes the
// results of those interactions through the view, and all major state is
// stored in the runtime (ie the model). The runtime is one of two major
// threads of control, the other being the controller.

class Runtime : public Asynchronous {
  public:
    // Typedefs:
    typedef std::function<void()> Interrupt;

    // Constructors:
    explicit Runtime(View* view);
    ~Runtime() override;

    // Configuration Interface:
    Runtime& set_compiler(Compiler* c);
    Runtime& set_include_dirs(const std::string& s);
    Runtime& set_open_loop_target(size_t olt);
    Runtime& disable_inlining(bool di);
    Runtime& enable_info(bool ei);
    Runtime& disable_warning(bool dw);
    Runtime& disable_error(bool de);

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
    // Invokes eval_node() in the gap between this and the next timestep on
    // each of the nodes in an iterator range. Code which is successfully
    // eval'ed will begin execution at the beginning of the following timestep.
    // Nodes which follow a failed eval are deleted.
    template <typename InputItr>
    void eval(InputItr begin, InputItr end);

    // System Task Interface:
    //
    // Print a newline-teriminated string to the view between this and the next
    // timestep. Returns immediately.
    void display(const std::string& s);
    // Print a string to the view between this and the next timestep. Returns
    // immediately.
    void write(const std::string& s);
    // Shutdown the runtime and print statistics if arg is non-zero between
    // this and the next timestep. Returns immediately.
    void finish(int arg);
    // Prints an error message between this and the next timestep. Returns
    // immediately.
    void error(const std::string& s);
    // Prints a warning message between this and the next timestep. Returns
    // immediately.
    void warning(const std::string& s);
    // Prints an info message between this and the next timestep. Returns
    // immediately.
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
    // Schedules an intterupt on the interrupt queue
    void schedule_interrupt(Interrupt int_);
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
    // the simulation. From a design point of view, this interface is a thin
    // wrapper around streambufs.
    // 
    // Establishes an entry in the stream table and returns a unique id
    // associated with that entry. If no file named path exists, it is created
    // in the current working directory. The stream is opened in read/append
    // mode and the read/write pointers are set to the 0'th byte in the file.
    SId fopen(const std::string& path);
    // Removes an entry from the stream table. Does nothing if there is no
    // stream associated with id.
    void close(SId id);
    // Updates the read/write pointer (r == true/false) in the stream
    // associated with id. Does nothing if there is no stream associated with
    // id.  Seeks past the beginning or end of a stream are clamped.
    void seekoff(SId id, int n, bool r);
    // Copies up to m bytes beginning from the read pointer in the stream
    // associated with id and advances the pointer by m bytes. m may be less
    // than n if the end of file is encountered first.
    size_t sgetn(SId id, char* c, size_t n);
    // Copies n characters from c starting from the write pointer in the stream
    // associated with id. Does nothing if there is no stream associated with
    // id.
    void sputn(SId id, const char* c, size_t n);
    // Returns an estimate of the number of characters remanining between the
    // read pointer and the end of the stream. Postiive result: at least this
    // many characters. -1: end of file. 0: No information available.
    size_t in_avail(SId id);

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
    // MVC State:
    View* view_;
    
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
    std::string include_dirs_;
    bool disable_inlining_;
    bool enable_open_loop_;
    size_t open_loop_itrs_;
    size_t open_loop_target_;
    bool enable_info_;
    bool disable_warning_;
    bool disable_error_;

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
    // Invokes eval_stream() on the file pointed to by an include statement.
    bool eval_include(String* s);
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
inline void Runtime::eval(InputItr begin, InputItr end) {
  schedule_interrupt(Interrupt([this, begin, end]{
    log_->clear();
    eval_nodes(begin, end);
  }));
}

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
