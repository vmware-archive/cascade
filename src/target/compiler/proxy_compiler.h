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

#ifndef CASCADE_SRC_TARGET_COMPILER_PROXY_COMPILER_H
#define CASCADE_SRC_TARGET_COMPILER_PROXY_COMPILER_H

#include <string>
#include <unordered_map>
#include "common/sockstream.h"
#include "target/compiler/rpc.h"
#include "target/core_compiler.h"
#include "target/compiler/proxy_core.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"

namespace cascade {

class ProxyCompiler : public CoreCompiler {
  public:
    ProxyCompiler();
    ~ProxyCompiler() override;

    void abort() override;

  private:
    std::unordered_map<std::string, std::pair<Rpc::Id, sockstream*>> socks_;

    // Core Compiler Interface:
    Clock* compile_clock(ModuleDeclaration* md, Interface* interface) override;
    Custom* compile_custom(ModuleDeclaration* md, Interface* interface) override;
    Gpio* compile_gpio(ModuleDeclaration* md, Interface* interface) override;
    Led* compile_led(ModuleDeclaration* md, Interface* interface) override;
    Pad* compile_pad(ModuleDeclaration* md, Interface* interface) override;
    Reset* compile_reset(ModuleDeclaration* md, Interface* interface) override;
    Logic* compile_logic(ModuleDeclaration* md, Interface* interface) override;

    // Generic compilation method. In terms of implementation, proxy cores are
    // mostly all the same. This method is here simply for the sake of
    // type-correctness.
    template <typename T>
    ProxyCore<T>* generic_compile(ModuleDeclaration* md, Interface* interface);

    std::pair<Rpc::Id, sockstream*> get_persistent_sock(const std::string& loc);
    sockstream* get_temp_sock(const std::string& loc);
    sockstream* get_tcp_sock(const std::string& loc);
    sockstream* get_unix_sock(const std::string& loc);
};

template <typename T>
inline ProxyCore<T>* ProxyCompiler::generic_compile(ModuleDeclaration* md, Interface* interface) {
  // Look up the loc annotation on this module
  const auto& loc = md->get_attrs()->get<String>("__loc")->get_readable_val();

  // Grab a persistent socket connection to the remote runtime at this address.
  auto psock = get_persistent_sock(loc);
  if (psock.second == nullptr) {
    error("Unable to establish connection with slave runtime");
    delete md;
    return nullptr;
  }

  // Open up a temporary socket connection to the remote runtime. Compilations
  // Use a separate connection so that they don't block ABI requests while 
  // running in the background.
  auto* tsock = get_temp_sock(loc);
  if (tsock == nullptr) {
    error("Unable to establish connection with slave runtime");
    delete md;
    return nullptr;
  }

  // Change __loc attribute to "remote" and send compile rpc. This is a
  // blocking request in this thread, but it won't block the remote runtime.
  md->get_attrs()->set_or_replace("__loc", new String("remote"));
  Rpc(Rpc::Type::COMPILE, psock.first).serialize(*tsock);
  TextPrinter(*tsock) << md << "\n";
  delete md;
  tsock->flush();

  Rpc res;
  res.deserialize(*tsock);
  delete tsock;

  if (res.type_ == Rpc::Type::FAIL) {
    // TODO(eschkufz) Forward error messages from slave runtime to here
    error("An unhandled error occured during compilation in the slave runtime");
    return nullptr;
  }
  return new ProxyCore<T>(interface, res.id_, psock.second);
}

} // namespace cascade

#endif
