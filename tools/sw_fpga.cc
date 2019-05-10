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

#include <iostream>
#include <mutex>
#include <ncurses.h>
#include <string>
#include "cl.h"
#include "base/bits/bits.h"
#include "target/common/remote_runtime.h"
#include "target/compiler.h"
#include "target/core/sw/sw_compiler.h"

using namespace cl;
using namespace cascade;
using namespace std;

namespace {
        
__attribute__((unused)) auto& g = Group::create("Software Target Options");
auto& port = StrArg<size_t>::create("--port")
  .usage("<int>")
  .description("Port to listen for connections on")
  .initial(8800);
auto& path = StrArg<string>::create("--path")
  .usage("<path/to/socket>")
  .description("Path to listen for connections on")
  .initial("/tmp/fpga_socket");

void toggle_pad(Bits& pad, mutex& pad_lock, char c) {
  lock_guard<mutex> lg(pad_lock);
  pad.flip(c-'1');
}

void toggle_rst(Bits& rst, mutex& rst_lock) {
  lock_guard<mutex> lg(rst_lock);
  rst.flip(0);
}

void draw_pad(Bits& pad, mutex& pad_lock, int row, int col) {
  lock_guard<mutex> lg(pad_lock);
  for (int i = 0; i < 4; ++i) {
    const auto p = pad.get(i) ? '_' : '-';
    attron(COLOR_PAIR(2));
    mvprintw((row/2)+1, (col/2)+(2*i)-4, "%c ", p);
    attroff(COLOR_PAIR(2));
  }
}

void draw_rst(Bits& rst, mutex& rst_lock, int row, int col) {
  lock_guard<mutex> lg(rst_lock);
  const auto r = rst.to_bool() ? '_' : '-';
  attron(COLOR_PAIR(1));
  mvprintw((row/2)+1, (col/2)+6, "%c ", r);
  attroff(COLOR_PAIR(1));
}

void draw_led(Bits& led, mutex& led_lock, int row, int col) {
  lock_guard<mutex> lg(led_lock);
  for (int i = 0; i < 8; ++i) {
    const auto l = led.get(i);
    const auto cl = l ? 1 : 2;
    const auto ch = l ? '*' : '.';
    attron(COLOR_PAIR(cl));
    mvprintw((row/2)-1, (col/2)+(2*i)-8, "%c ", ch);
    attroff(COLOR_PAIR(cl));
  }
}

} // namespace 

int main(int argc, char** argv) {
  // Parse command line:
  Simple::read(argc, argv);

  initscr();      
  timeout(1000/40);
  noecho();     
  curs_set(0);
  raw();
  start_color();

  init_color(COLOR_WHITE, 700, 700, 700);
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);

  Bits led(8, 0);
  Bits pad(4, 0);
  Bits rst(1, 0);

  mutex led_lock;
  mutex pad_lock;
  mutex rst_lock;

  auto* sc = new SwCompiler();
    sc->set_led(&led, &led_lock);
    sc->set_pad(&pad, &pad_lock);
    sc->set_reset(&rst, &rst_lock);
  auto* c = new Compiler();
    c->set_sw_compiler(sc);

  auto* runtime = new RemoteRuntime();
  runtime->set_compiler(c);
  runtime->set_path(::path.value());
  runtime->set_port(::port.value());
  runtime->run();

  for (auto running = true; running; ) {
    auto input = getch();
    switch (input) {
      case '1':
      case '2':
      case '3':
      case '4':
        toggle_pad(pad, pad_lock, input);
        break;
      case 'r':
        toggle_rst(rst, rst_lock);
        break;
      case 'q':
        running = false;
        runtime->stop_now();
        break;
      default:
        break;
    }

    int row; 
    int col;
    getmaxyx(stdscr, row, col);
    draw_pad(pad, pad_lock, row, col);
    draw_rst(rst, rst_lock, row, col);
    draw_led(led, led_lock, row, col);
    refresh();      
  }
  endwin();

  delete runtime;

  cout << "Goodbye!" << endl;
  return 0;
}
