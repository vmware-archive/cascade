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

#ifndef CASCADE_SRC_TARGET_CORE_PROXY_PROXY_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_PROXY_PROXY_COMPILER_H

#include <string>
#include <unordered_map>
#include "common/sockstream.h"
#include "target/common/rpc.h"
#include "target/core_compiler.h"
#include "target/core/proxy/proxy_core.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"

namespace cascade {

class ProxyCompiler : public CoreCompiler {
  public:
    ProxyCompiler();
    ~ProxyCompiler() override;

    void abort() override;

  private:
    std::unordered_map<std::string, sockstream*> socks_;

    // Core Compiler Interface:
    Clock* compile_clock(Interface* interface, ModuleDeclaration* md) override;
    Custom* compile_custom(Interface* interface, ModuleDeclaration* md) override;
    Gpio* compile_gpio(Interface* interface, ModuleDeclaration* md) override;
    Led* compile_led(Interface* interface, ModuleDeclaration* md) override;
    Pad* compile_pad(Interface* interface, ModuleDeclaration* md) override;
    Reset* compile_reset(Interface* interface, ModuleDeclaration* md) override;
    Logic* compile_logic(Interface* interface, ModuleDeclaration* md) override;

    // Generic compilation method. In terms of implementation, proxy cores are
    // mostly all the same. This method is here simply for the sake of
    // type-correctness.
    template <typename T>
    ProxyCore<T>* generic_compile(Interface* interface, ModuleDeclaration* md);

    sockstream* get_sock(const ModuleDeclaration* md);
    sockstream* get_tcp_sock(const ModuleDeclaration* md);
    sockstream* get_unix_sock(const ModuleDeclaration* md);
};

template <typename T>
inline ProxyCore<T>* ProxyCompiler::generic_compile(Interface* interface, ModuleDeclaration* md) {
  // Create socket connection based on __loc annotation
  auto* sock = get_sock(md);
  if (sock == nullptr) {
    error("Unable to establish connection with slave runtime");
    delete md;
    return nullptr;
  }

  // Change __loc attribute to "remote" and send compile rpc
  md->get_attrs()->set_or_replace("__loc", new String("remote"));
  Rpc(Rpc::Type::COMPILE, 0).serialize(*sock);
  TextPrinter(*sock) << md << "\n";
  delete md;
  sock->flush();

  Rpc res;
  res.deserialize(*sock);
  if (res.type_ == Rpc::Type::FAIL) {
    // TODO(eschkufz) Forward error messages from slave runtime to here
    error("An unhandled error occured during compilation in the slave runtime");
    return nullptr;
  }
  return new ProxyCore<T>(interface, res.id_, sock);
}

} // namespace cascade

#endif
