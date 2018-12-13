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

#ifndef CASCADE_SRC_BASE_THREAD_THREAD_POOL_H
#define CASCADE_SRC_BASE_THREAD_THREAD_POOL_H

#include <cassert>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stack>
#include <thread>
#include <vector>
#include "src/base/thread/asynchronous.h"

namespace cascade {

// This class represents an abstract pool of compute. It is provided so that
// objects can schedule Jobs (ie: methods returning void which can be handled
// asynchronously) and block on their completion.

class ThreadPool : public Asynchronous {
  public:
    // Job Typedef:
    typedef std::function<void()> Job;

    // Constructors:
    ThreadPool();
    ~ThreadPool() override = default;

    // Parameter Interface:
    ThreadPool& set_num_threads(size_t n);

    // Schedule a new job. Ignores jobs scheduled between stop() and start().
    void insert(Job* job);

  protected:
    // Start a new pool of num_threads_ threads.
    void run_logic() override;
    // Blocks until all outstanding jobs have run to completion.
    void stop_logic() override;
  
  private:
    std::mutex lock_;
    std::condition_variable cv_;

    size_t num_threads_;
    std::vector<std::thread> threads_;
    std::stack<Job*> jobs_;

    Job* get();
};

inline ThreadPool::ThreadPool() : Asynchronous() {
  set_num_threads(1);
}

inline ThreadPool& ThreadPool::set_num_threads(size_t n) {
  num_threads_ = n;
  return *this;
}

inline void ThreadPool::insert(Job* job) {
  {
    std::lock_guard<std::mutex> lg(lock_);
    jobs_.push(job);
  }
  cv_.notify_one();
}

inline void ThreadPool::run_logic() {
  for (size_t i = 0; i < num_threads_; ++i) {
    threads_.push_back(std::thread([this]{
      while (true) {
        if (auto job = get()) {
          (*job)();
          delete job;
          continue;
        } 
        return;
      }
    }));
  }  
}

inline void ThreadPool::stop_logic() {
  cv_.notify_all();
  for (auto& t : threads_) {
    t.join(); 
  }
  assert(jobs_.empty());
  threads_.clear();
}

inline ThreadPool::Job* ThreadPool::get() {
  std::unique_lock<std::mutex> ul(lock_);
  while (jobs_.empty() && !stop_requested()) {
    cv_.wait(ul);
  }
  if (!jobs_.empty()) {
    auto res = jobs_.top();
    jobs_.pop();
    return res;
  }
  return nullptr;
}

} // namespace cascade

#endif
