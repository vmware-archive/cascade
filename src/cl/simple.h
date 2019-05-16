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

#ifndef CASCADE_SRC_CL_SIMPLE_H
#define CASCADE_SRC_CL_SIMPLE_H

#include <iomanip>
#include "args.h"
#include "flag_arg.h"

namespace cascade::cl {

class Simple {
  public:
    static void read(int argc, char** argv, std::ostream& out = std::cout, std::ostream& err = std::cerr) {
      Group::create("Help and Command Line Options");
      auto& help = FlagArg::create("--help")
        .alias("-h")
        .description("Print command line information and quit");
      StrArg<std::string>::create("--config")
        .usage("<path>")
        .initial("...")
        .description("Import command line arguments from a config file");

      read_args(argc, argv, err);
      if (help) {
        write_help(out);
        exit(0);
      }
      for (auto i = Args::arg_begin(), ie = Args::arg_end(); i != ie; ++i) {
        if (error(*i)) {
          err << "Error (" << *((*i)->alias_begin()) << "): "; 
          write_error(err, *i);
          err << std::endl;
          exit(1);
        }
      }
    }

  private:
    static void read_args(int argc, char** argv, std::ostream& err) {
      std::vector<std::string> args;
      std::stringstream ss;
      for (int i = 0; i < argc; ++i) {
        ss << std::quoted(argv[i]) << std::endl;
      }
      get_args(ss, args, err);
      std::vector<char*> cps;
      for (const auto& a : args) {
        cps.push_back((char*)a.c_str());
      }
      Args::read(cps.size(), cps.data());
    }
    static void get_args(std::istream& is, std::vector<std::string>& args, std::ostream& err) {
      while (!is.eof()) {
        const auto arg = get_arg(is);
        if (arg == "--config") {
          const auto path = get_arg(is);
          std::ifstream ifs(path);
          if (!ifs.is_open()) {
            err << "Error: Unable to open config file \"" << path << "\"!" << std::endl;
            exit(1);
          }
          get_args(ifs, args, err);
        } else {
          args.push_back(arg);
        }
      }
      args.pop_back();
    }
    static std::string get_arg(std::istream& is) {
      while (isspace(is.peek()) || is.peek() == '#') {
        for (; isspace(is.peek()); is.get());
        if (is.peek() == '#') {
          while (is.get() != '\n');
        }
      }
      std::string s = "";
      is >> std::quoted(s);
      return s;
    }

    static bool error(const Arg* a) {
      return a->error() || a->duplicated() || (a->required() && !a->provided());
    }
    static void write_error(std::ostream& os, Arg* a) {
      if (a->error()) {
        os << "Unable to parse argument!";
      } else if (a->duplicated()) {
        os << "Argument appears more than once!";
      } else {
        os << "Required value not provided!";
      } 
    }

    static void write_help(std::ostream& os) {
      std::vector<Group*> gs(Args::group_begin(), Args::group_end());
      std::sort(gs.begin(), gs.end(), [](Group* g1, Group* g2) {return g1->name() <= g2->name();});

      for (auto i = gs.begin(), ie = gs.end(); i != ie; ++i) {
        os << std::endl << (*i)->name() << ":" << std::endl;

        std::vector<Arg*> as((*i)->arg_begin(), (*i)->arg_end());
        std::sort(as.begin(), as.end(), [](Arg* a1, Arg* a2) {return *(a1->alias_begin()) <= *(a2->alias_begin());});

        for (auto j = as.begin(), je = as.end(); j != je; ++j) {
          os << " ";
          for (auto k = (*j)->alias_begin(), ke = (*j)->alias_end(); k != ke; ++k) {
            os << *k << " ";
          }
          os << ((*j)->usage() == "" ? "" : (*j)->usage()) << std::endl;
          os << "    Desc:     " << ((*j)->description() == "" ? "<none>" : (*j)->description()) << std::endl;
          os << "    Required: " << ((*j)->required() ? "yes" : "no") << std::endl;
          if (error(*j)) {
            os << "    Error:    ";
            write_error(os, *j);
            os << std::endl;
          } else {
            os << "    Value:    ";
            (*j)->write(os);
            os << std::endl;
          }
        }
      }
      os << std::endl;
    }
};

} // namespace cascade::cl

#endif
