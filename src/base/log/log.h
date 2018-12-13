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

#ifndef CASCADE_SRC_MISC_LOGGABLE_H
#define CASCADE_SRC_MISC_LOGGABLE_H

#include <string>
#include <vector>

namespace cascade {

// This class is used to attach logging facilities to objects with complex
// behavior. It allows the user to record messages, warnings, and errors and
// inspect their values at a later time.

class Log {
  public:
    typedef std::vector<std::string>::const_iterator log_iterator;
    typedef std::vector<std::string>::const_iterator error_iterator;
    typedef std::vector<std::string>::const_iterator warn_iterator;

    void log(const std::string& s);
    void error(const std::string& s);
    void warn(const std::string& s);
    void clear();

    bool log() const;
    log_iterator log_begin() const;
    log_iterator log_end() const;

    bool error() const;
    error_iterator error_begin() const;
    error_iterator error_end() const;

    bool warning() const;
    warn_iterator warn_begin() const;
    warn_iterator warn_end() const;

  private:
    std::vector<std::string> logs_;
    std::vector<std::string> errors_;
    std::vector<std::string> warns_;
};

inline void Log::log(const std::string& s) {
  logs_.push_back(s);
} 

inline void Log::error(const std::string& s) {
  errors_.push_back(s);
} 

inline void Log::warn(const std::string& s) {
  warns_.push_back(s);
}

inline void Log::clear() {
  logs_.clear();
  errors_.clear();
  warns_.clear();
}

inline bool Log::log() const {
  return !logs_.empty();
}

inline Log::log_iterator Log::log_begin() const {
  return logs_.begin();
}

inline Log::log_iterator Log::log_end() const {
  return logs_.end();
}
  
inline bool Log::error() const {
  return !errors_.empty();
}

inline Log::error_iterator Log::error_begin() const {
  return errors_.begin();
}

inline Log::error_iterator Log::error_end() const {
  return errors_.end();
}
  
inline bool Log::warning() const {
  return !warns_.empty();
}

inline Log::warn_iterator Log::warn_begin() const {
  return warns_.begin();
}

inline Log::warn_iterator Log::warn_end() const {
  return warns_.end();
}
 
} // namespace cascade 

#endif
