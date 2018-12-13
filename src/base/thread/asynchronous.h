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

#ifndef CASCADE_SRC_BASE_THREAD_ASYNCHRONOUS_H
#define CASCADE_SRC_BASE_THREAD_ASYNCHRONOUS_H

#include <chrono>
#include <thread>

namespace cascade {

// This class is meant to represent the basic concept of an asynchronous
// object: one that can be started and stopped independently of the main
// program thread.

class Asynchronous {
  public:
    // Initializes an asynchronous object in the stopped state.
    Asynchronous();
    // Invokes stop_now().
    virtual ~Asynchronous();

    // Invokes stop_now() and creates a new thread which invokes run_logic().
    // Returns immediately.  
    void run();
    // Requests the thread running run_logic() halt execution in a reasonable
    // amount of time. Returns immediately.
    void request_stop();
    // Blocks until the thread running run_logic() has completed execution and
    // then invokes done_logic().
    void wait_for_stop();

    // Convenience method; invokes run(), request_stop(), and wait_for_stop();
    void run_to_completion();
    // Convenience method; invokves request_stop(), and wait_for_stop().
    void stop_now();
      
  protected:
    // An arbitrary compute task. This method must halt execution in a
    // reasonable amount of time whenever stop_requested() returns true.
    virtual void run_logic();
    // Invoked when run_logic() has halted execution. This method must run
    // to completion in a reasonable amount of time and be okay with being
    // invoked several times in a row.
    virtual void stop_logic();

    // Convenience Method: Puts this thread to sleep for n milliseconds
    void sleep_for(size_t n);

    // Returns true whenever request_stop() is invoked.
    bool stop_requested() const;

  private:
    std::thread thread_;
    bool stop_requested_;
};

inline Asynchronous::Asynchronous() : thread_() {
  stop_requested_ = true;
}

inline Asynchronous::~Asynchronous() {
  stop_now();
}

inline void Asynchronous::run() {
  stop_now();
  stop_requested_ = false;
  thread_ = std::thread([this]{run_logic();});
}

inline void Asynchronous::request_stop() {
  stop_requested_ = true;
}

inline void Asynchronous::wait_for_stop() {
  if (thread_.joinable()) {
    thread_.join();
  }
  stop_logic();
}

inline void Asynchronous::run_to_completion() {
  run();
  request_stop();
  wait_for_stop();
}

inline void Asynchronous::stop_now() {
  request_stop();
  wait_for_stop();
}

inline void Asynchronous::run_logic() {
  // Does nothing.
}

inline void Asynchronous::stop_logic() {
  // Does nothing.
}

inline void Asynchronous::sleep_for(size_t n) {
  std::this_thread::sleep_for(std::chrono::milliseconds(n));
}

inline bool Asynchronous::stop_requested() const {
  return stop_requested_;
}

} // namespace cascade

#endif
