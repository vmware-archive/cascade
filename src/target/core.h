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

#ifndef CASCADE_SRC_TARGET_CORE_H
#define CASCADE_SRC_TARGET_CORE_H

#include "src/base/bits/bits.h"
#include "src/runtime/ids.h"

namespace cascade {

// This class encapsulates the target-specific implementation of module logic.

class Interface;
class Input;
class State;

class Core {
  public:
    explicit Core(Interface* interface);
    virtual ~Core() = default;

    // This method must return the values of all stateful elements contained in
    // this module. It is called at most once before this core is torn down.
    virtual State* get_state() = 0;
    // This method must update the value of any stateful elements contained in
    // this module. It is called exactly once before resync().
    virtual void set_state(const State* s) = 0;
    // This method must return the values of all inputs connected to this
    // module. It is called at most once before this core is torn down.
    virtual Input* get_input() = 0;
    // This method must update the value of an input connected to this module.
    // It is called exactly once before resync().
    virtual void set_input(const Input* i) = 0;
    // Target-specific implementations may override this method to perform
    // last minute initialization prior to beginning execution.
    virtual void resync();

    // Overriding this method to return true will cause the runtime to call the
    // done_step() method at the end of each logical time step. The default
    // implementation returns false.
    virtual bool overrides_done_step() const;
    // Target-specific implementations may override this method to perform
    // arbitary logic at the end of each logical time step. The default
    // implementation does nothing.
    virtual void done_step();
    // Overriding this method to return true will cause the runtime to call the
    // done_simulation() method at the end of the simulation. The default
    // implementation returns false.
    virtual bool overrides_done_simulation() const;
    // Target-specific implementations may override this method to perform
    // arbitrary logic at the end of the simulation. The default implementation
    // does nothing.
    virtual void done_simulation();

    // This method is invoked whenever new values are presented on this
    // module's input ports. It is required to perform whatever internal logic
    // is necessary such that evaluate_logic() and update_logic() behave
    // correctly.
    virtual void read(VId id, const Bits* b) = 0;
    // This method must update all logic and then inform the runtime of any
    // changes to this module's output ports or the evaluation of any system
    // tasks by invoking the appropriate methods on the Interface obtained by a
    // call to interface().
    virtual void evaluate() = 0;
    // This method must return true iff there are pending update events
    // following the invocation of evaluate_logic() or update_logic().
    virtual bool there_are_updates() const = 0;
    // This method must update all stateful logic and then inform the runtime
    // of any changes to this module's output ports or the evaluation of any
    // system tasks by invoking the appropriate methods on the Interface
    // obtained by a call to interface().
    virtual void update() = 0;
    // This method must return true if the previous call to either evaluate()
    // or update() resulted in at least one system task. If you don't want to
    // think too hard about this. It's safe (though definitely less performant)
    // to always return true.
    virtual bool there_were_tasks() const = 0;

    // Target-specific implementations may override this method if there is a
    // performance-specific advantage to doing so. This method must perform an
    // update() if there_are_updates() would report true, and then return true.
    // Otherwise, it must return false. 
    virtual bool conditional_update();
    // Target-specific implementations may override this method if there is a
    // performance-specific advantage to doing so. This method is only called
    // in a state where the entire program has been inlined into this core such
    // that the only input clk, is the runtime's clock, it has value val, and
    // there are no outputs. This method must run for up to itr iterations, or
    // until a system task is generated before returning control. On return it
    // must report the number of iterations that it ran for. 
    virtual size_t open_loop(VId clk, bool val, size_t itr);

    // Light-weight RTTI:
    virtual bool is_clock() const;
    virtual bool is_logic() const;
    virtual bool is_stub() const;

  protected:
    Interface* interface();

  private:
    Interface* interface_;
};

// These classes represent the various cascade standard library types.
// Conceptually, they are functionally identical to cores. However
// semantically, they may represent substantially different behaviors, or have
// default implementations where a generic core may not.

class Clock : public Core { 
  public:
    using Core::Core;
    bool there_were_tasks() const override;
    bool is_clock() const override;
};

class Fifo : public Core { 
  public:
    using Core::Core;
    bool there_were_tasks() const override;
};

class Gpio : public Core { 
  public:
    using Core::Core;
    bool there_were_tasks() const override;
};

class Led : public Core { 
  public:
    using Core::Core;
    bool there_were_tasks() const override;
};

class Logic : public Core { 
  public:
    using Core::Core;
    bool is_logic() const override;
};

class Memory : public Core { 
  public:
    using Core::Core;
    bool there_were_tasks() const override;
};

class Pad : public Core { 
  public:
    using Core::Core;
    bool there_were_tasks() const override;
};

class Reset : public Core { 
  public:
    using Core::Core;
    bool there_were_tasks() const override;
};

inline Core::Core(Interface* interface) {
  interface_ = interface;
}

inline void Core::resync() {
  // Does nothing.
}

inline bool Core::overrides_done_step() const {
  return false;
}

inline void Core::done_step() {
  // Does nothing.
}

inline bool Core::overrides_done_simulation() const {
  return false;
}

inline void Core::done_simulation() {
  // Does nothing.
}

inline bool Core::conditional_update() {
  if (there_are_updates()) {
    update();
    return true;
  }
  return false;
}

inline size_t Core::open_loop(VId clk, bool val, size_t itr) {
  Bits bits(1, val);
  size_t res = 0;
  for (auto tasks = false; (res < itr) && !tasks; ++res) {
    // The clock is scheduled first. It changes its value on update and sends
    // the new value to this core. 
    bits.flip(0);
    read(clk, &bits); 

    // Drain evaluations and updates for this core.
    for (auto done = false; !done; ) {
      evaluate();
      tasks = tasks || there_were_tasks();
      done = !conditional_update();
      tasks = tasks || there_were_tasks();
    }
    done_step();
  }
  return res;  
}

inline bool Core::is_clock() const {
  return false;
}

inline bool Core::is_logic() const {
  return false;
}

inline bool Core::is_stub() const {
  return false;
}

inline Interface* Core::interface() {
  return interface_;
}

inline bool Clock::there_were_tasks() const {
  return false;
}

inline bool Clock::is_clock() const {
  return true;
}

inline bool Fifo::there_were_tasks() const {
  return false;
}

inline bool Gpio::there_were_tasks() const {
  return false;
}

inline bool Led::there_were_tasks() const {
  return false;
}

inline bool Logic::is_logic() const {
  return true;
}

inline bool Memory::there_were_tasks() const {
  return false;
}

inline bool Pad::there_were_tasks() const {
  return false;
}

inline bool Reset::there_were_tasks() const {
  return false;
}

} // namespace cascade

#endif
