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
#include "target/compiler.h"
#include "target/compiler/rpc.h"
#include "target/core_compiler.h"
#include "target/compiler/proxy_core.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"

namespace cascade {

class ProxyCompiler : public CoreCompiler {
  public:
    typedef uint32_t Id;

    ProxyCompiler();
    ~ProxyCompiler() override;

  private:
    struct ConnInfo {
      Id pid;
      sockstream* async_sock;
      sockstream* sync_sock;
    };
    std::unordered_map<std::string, ConnInfo> conns_;
    bool running_;

    // Core Compiler Interface:
    Clock* compile_clock(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    Custom* compile_custom(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    Gpio* compile_gpio(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    Led* compile_led(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    Pad* compile_pad(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    Reset* compile_reset(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    Logic* compile_logic(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;

    // Generic compilation method. In terms of implementation, proxy cores are
    // mostly all the same. This method is here simply for the sake of
    // type-correctness.
    template <typename T>
    ProxyCore<T>* generic_compile(Engine::Id id, ModuleDeclaration* md, Interface* interface);

    void async_loop(sockstream* sock);
    void stop_compile(Engine::Id id) override;
    void stop_async() override;

    bool open(const std::string& loc);
    bool close(const ConnInfo& ci);

    sockstream* get_sock(const std::string& loc);
    sockstream* get_tcp_sock(const std::string& loc);
    sockstream* get_unix_sock(const std::string& loc);
};

template <typename T>
inline ProxyCore<T>* ProxyCompiler::generic_compile(Engine::Id id, ModuleDeclaration* md, Interface* interface) {
  // Open a connection to this location if necessary.
  const auto& loc = md->get_attrs()->get<String>("__loc")->get_readable_val();
  if (!open(loc)) {
    get_compiler()->error("Unable to establish connection with remote compiler");
    delete md;
    return nullptr;
  }
  const auto& conn = conns_[loc];

  // Change __loc to "remote" and send a compile request via a temp socket.  
  md->get_attrs()->set_or_replace("__loc", new String("remote"));
  auto* sock = get_sock(loc);
  if (sock == nullptr) {
    get_compiler()->error("Unable to establish connection with remote compiler");
    delete md;
    return nullptr;
  }

  // Send a blocking compile request
  Rpc(Rpc::Type::COMPILE, conn.pid, id, 0).serialize(*sock);
  TextPrinter(*sock) << md << "\n";
  delete md;
  sock->flush();

  // If successful, this response will contain the index for this engine in the
  // remote compiler's engine table
  Rpc res;
  res.deserialize(*sock);
  delete sock;
  if (res.type_ == Rpc::Type::FAIL) {
    get_compiler()->error("An unhandled error occured during compilation in the remote compiler");
    return nullptr;
  }
  return new ProxyCore<T>(interface, conn.pid, id, res.n_, conn.sync_sock);
}

} // namespace cascade

#endif
