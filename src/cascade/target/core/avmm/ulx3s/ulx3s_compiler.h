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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_ULX3S_ULX3S_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_AVMM_ULX3S_ULX3S_COMPILER_H

#include <cassert>
#include <fcntl.h>
#include <fstream>
#include <termios.h>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include "common/system.h"
#include "target/core/avmm/avmm_compiler.h"
#include "target/core/avmm/ulx3s/ulx3s_logic.h"

namespace cascade::avmm {

template <size_t M, size_t V, typename A, typename T>
class Ulx3sCompiler : public AvmmCompiler<M,V,A,T> {
  public:
    Ulx3sCompiler();
    ~Ulx3sCompiler() override;

  private:
    // Device Handle:
    int fd_;

    // Logic Core Handle:
    Ulx3sLogic<V,A,T>* logic_;

    // Compilation Cache:
    std::unordered_map<std::string, std::string> cache_;

    // Avmm Compiler Interface:
    Ulx3sLogic<V,A,T>* build(Interface* interface, ModuleDeclaration* md, size_t slot) override;
    bool compile(const std::string& text, std::mutex& lock) override;
    void stop_compile() override;

    // Helper Methods:
    void init_tmp();
    void init_cache();
};

using Ulx3s32Compiler = Ulx3sCompiler<10,21,uint32_t,uint32_t>;

template <size_t M, size_t V, typename A, typename T>
inline Ulx3sCompiler<M,V,A,T>::Ulx3sCompiler() : AvmmCompiler<M,V,A,T>() {
  init_tmp();
  init_cache();
  fd_ = -1;
}

template <size_t M, size_t V, typename A, typename T>
inline Ulx3sCompiler<M,V,A,T>::~Ulx3sCompiler() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

template <size_t M, size_t V, typename A, typename T>
inline Ulx3sLogic<V,A,T>* Ulx3sCompiler<M,V,A,T>::build(Interface* interface, ModuleDeclaration* md, size_t slot) {
  logic_ = new Ulx3sLogic<V,A,T>(interface, md, slot);
  return logic_;
}

template <size_t M, size_t V, typename A, typename T>
inline bool Ulx3sCompiler<M,V,A,T>::compile(const std::string& text, std::mutex& lock) {
  // Stop any previous compilations
  stop_compile();

  // Compile a cache entry if none previously existed
  if (cache_.find(text) == cache_.end()) {
    char path[] = "/tmp/ulx3s/program_logic_XXXXXX.v";
    const auto fd = mkstemps(path, 2);
    const auto dir = std::string(path).substr(0,31);
    close(fd);

    System::execute("mkdir -p " + dir);
    std::ofstream ofs(path);
    ofs << text << std::endl;
    ofs.close();
    System::execute("cp " + System::src_root() + "/share/cascade/ulx3s/*.v " + dir);
    System::execute("cp " + System::src_root() + "/share/cascade/ulx3s/*.lpf " + dir);
    System::execute("mv " + std::string(path) + " " + dir + "/program_logic.v");

    pid_t pid = 0;
    if constexpr (std::is_same<T, uint32_t>::value) {
      pid = System::no_block_begin_execute("cd " + System::src_root() + "/share/cascade/ulx3s/ && ./build_ulx3s_32.sh " + dir, false);
    } 

    // Compilation can take a potentially long time. Release the lock while we
    // wait for completion.
    lock.unlock();
    const auto res = System::no_block_wait_finish(pid);
    lock.lock();

    // If control has reached here with failure, it means compilation was
    // aborted and we can return false. Otherwise, we have a bitstream which we
    // can enter into the cache.
    if (res != 0) {
      return false;
    }

    std::stringstream ss;
    ss << "bitstream_" << cache_.size() << ".bit";

    std::ofstream ofs2;
    if constexpr (std::is_same<T, uint32_t>::value) {
      System::execute("cp " + dir + "/root32.bit /tmp/ulx3s/cache32/" + ss.str());
      ofs2.open("/tmp/ulx3s/cache32/index.txt", std::ios::app);
      ofs2 << text << '\0' << "/tmp/ulx3s/cache32/" << ss.str() << '\0';
      cache_[text] = "/tmp/ulx3s/cache32/" + ss.str();
    }
  }

  // If control has reached here, we have a lock on the device and a bitstream
  // in the cache.  Schedule a state-safe interrupt to reprogram the device.
  const auto path = cache_[text];
  AvmmCompiler<M,V,A,T>::get_compiler()->schedule_state_safe_interrupt([this, path]{
    // Close the device if necessary
    if (fd_ >= 0) {
      close(fd_);
    }

    // Reprogram the device
    if constexpr (std::is_same<T, uint32_t>::value) {
      System::no_block_execute("ujprog -b 3000000 " + path, false);
    }
  
    // Reopen the device
    fd_ = open("/dev/cu.usbserial-120001", O_RDWR | O_NOCTTY | O_SYNC);
    assert(fd_ >= 0);

    struct termios tty;
    const auto gres = tcgetattr(fd_, &tty);
    assert(gres == 0);

    // Set baud rate for input and output
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // Send/receive 8-bit characters
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;   
    // Disable break processing
    tty.c_iflag &= ~IGNBRK;    
    // No signalling chars, echo, or cannonical processing
    tty.c_lflag = 0;
    // No remapping, no delayes
    tty.c_oflag = 0;                
    // Blocking reads
    tty.c_cc[VMIN] = 1; 
    // Shut off xon/xoff control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); 
    // Ignore modem controls, enable reading
    tty.c_cflag |= (CLOCAL | CREAD);
    // Turn off parity
    tty.c_cflag &= ~(PARENB | PARODD);      
    tty.c_cflag |= 0;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    const auto sres = tcsetattr(fd_, TCSANOW, &tty);
    assert(sres == 0);

    // Udpate the device's file descriptor for all active cores
    // TODO(eschkufz): This only updates fd for the most recently comoiled core
    logic_->set_fd(fd_);
  });

  return true;
}

template <size_t M, size_t V, typename A, typename T>
inline void Ulx3sCompiler<M,V,A,T>::stop_compile() {
  if constexpr (std::is_same<T, uint32_t>::value) {
    System::no_block_execute(R"(pkill -9 -P `ps -ax | grep build_ulx3s_32.sh | awk '{print $1}' | head -n1`)", false);
    System::no_block_execute("killall ulx3s >/dev/null", false);
    System::no_block_execute("killall nextpnr-ecp5 /dev/null", false);
  } 
}

template <size_t M, size_t V, typename A, typename T>
inline void Ulx3sCompiler<M,V,A,T>::init_tmp() {
  System::execute("mkdir -p /tmp/ulx3s");
  System::execute("mkdir -p /tmp/ulx3s/cache32");
  System::execute("touch /tmp/ulx3s/cache32/index.txt");
}

template <size_t M, size_t V, typename A, typename T>
inline void Ulx3sCompiler<M,V,A,T>::init_cache() {
  std::ifstream ifs;
  if constexpr (std::is_same<T, uint32_t>::value) {
    ifs.open("/tmp/ulx3s/cache32/index.txt");
  }
  while (true) {
    std::string text;
    getline(ifs, text, '\0');
    if (ifs.eof()) {
      break;
    } 
    std::string path;
    getline(ifs, path, '\0');
    cache_.insert(make_pair(text, path));
  } 
}

} // namespace cascade::avmm

#endif
