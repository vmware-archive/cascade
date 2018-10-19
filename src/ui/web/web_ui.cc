// Copyright 2017-2018 VMware, Inc.
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

#include "src/ui/web/web_ui.h"

#include <iostream>
#include <sstream>
#include "src/base/system/system.h"
#include "src/runtime/runtime.h"
#include "src/ui/term/term_view.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/print/debug/debug_printer.h"
#include "src/verilog/print/html/html_printer.h"
#include "src/verilog/print/text/text_printer.h"
#include "src/verilog/program/program.h"

using namespace std;

namespace {

void ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
  auto ui = (cascade::WebUi*) nc->mgr->user_data;
  if (ev == MG_EV_HTTP_REQUEST) {
    ui->send_index(nc, (struct http_message*) ev_data);
    return;
  }
  if (ev != MG_EV_WEBSOCKET_FRAME) {
    return;
  }

  auto wm = (struct websocket_message*) ev_data;
  string msg((char*) wm->data, wm->size);

  const auto sep = msg.find(':');
  const auto api = msg.substr(0, sep);
  const auto val = msg.substr(sep+1);

  if (api == "eval") {
    ui->recv_eval(val);
  } else if (api == "freq") {
    ui->send_freq();
  } else if (api == "pull") {
    ui->send_buffer(nc);
  }
}

} // namespace

namespace cascade {

WebUi::WebUi(Runtime* rt) : Controller(rt), View() { 
  set_port("11111");
  set_buffer(128);
  set_debug(false);

  doc_root_ = System::src_root() + "/src/ui/web/";
  overflow_ = 0;
}

WebUi& WebUi::set_port(const string& port) {
  port_ = port;
  return *this;
}

WebUi& WebUi::set_buffer(size_t buffer) {
  max_buffer_ = buffer;
  return *this; 
} 

WebUi& WebUi::set_debug(bool debug) {
  debug_ = debug;
  return *this;
}

void WebUi::error(size_t t, const string& s) {
  (void) t;
  stringstream ss;
  HtmlPrinter(ss) << Color::RED << s << Color::RESET << "\n";
  buffer("log", ss.str(), true, true);
}

void WebUi::print(size_t t, const string& s) {
  (void) t;
  stringstream ss;
  HtmlPrinter(ss) << s;
  buffer("log", ss.str(), true, false);
}

void WebUi::warn(size_t t, const string& s) {
  (void) t;
  stringstream ss;
  HtmlPrinter(ss) << Color::YELLOW << s << Color::RESET << "\n";
  buffer("log", ss.str(), true, true);
}

void WebUi::eval_decl(size_t t, const Program* p, const ModuleDeclaration* md) {
  (void) t;
  (void) p;
  ok("DECL");

  stringstream ss;
  ss << "{\"text\":\"[decl] ";
  HtmlPrinter(ss) << md->get_id();
  ss << "\",\"value\":\"";
  if (debug_) {
    DebugHtmlPrinter(ss) << md;
  } else {
    HtmlPrinter(ss) << md;
  }
  ss << "\"}";
  buffer("eval", ss.str(), false, true);
}

void WebUi::eval_item(size_t t, const Program* p, const ModuleDeclaration* md) {
  (void) t;
  (void) md;
  ok("ITEM");

  for (auto i = p->elab_begin(), ie = p->elab_end(); i != ie; ++i) {
    stringstream ss;
    ss << "{\"text\":\"[elab] ";
    TextPrinter(ss) << Resolve().get_full_id(ModuleInfo(i->second).id());
    ss << "\",\"value\":\"";
    if (debug_) {
      DebugHtmlPrinter(ss, true) << i->second;
    } else {
      HtmlPrinter(ss) << i->second;
    }
    ss << "\"}";
    buffer("eval", ss.str(), false, true);
  }

  stringstream ss;
  ss << "{\"text\":\"[decl] ";
  HtmlPrinter(ss) << p->root_decl()->second->get_id();
  ss << "\",\"value\":\"";
  if (debug_) {
    DebugHtmlPrinter(ss) << p->root_elab()->second;
  } else {
    HtmlPrinter(ss) << p->root_elab()->second;
  }
  ss << "\"}";
  buffer("eval", ss.str(), false, true);
}

void WebUi::crash() {
  error(0, "CASCADE SHUTDOWN UNEXPECTEDLY --- PLEASE FORWARD LOG FILE TO DEVELOPERS");
}

void WebUi::send_index(struct mg_connection* nc, struct http_message* msg) {
  mg_serve_http(nc, msg, opts_);
}

void WebUi::send_freq() {
  buffer("freq", runtime()->current_frequency(), true, true);
}

void WebUi::send_buffer(struct mg_connection* nc) {
  lock_guard<recursive_mutex> lg(lock_);

  if (overflow_ > 0) {
    stringstream ss;
    ss << "Buffer Overflow:" << endl << overflow_ << " results truncated!" << endl;
    error(0, ss.str());
    overflow_ = 0;
  }

  char addr[32];
  mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
  for (const auto& s : buffer_) {
    for (auto c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
      mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, s.c_str(), s.length());
    }
  }
  buffer_.clear();
}

void WebUi::recv_eval(const string& s) {
  runtime()->eval(s);
}

void WebUi::run_logic() {
  TermView tv;

  struct mg_mgr mgr;
  mg_mgr_init(&mgr, this);

  auto nc = mg_bind(&mgr, port_.c_str(), ev_handler);
  if (nc == NULL) {
    tv.error(0, "Unable to start server on port " + port_ + "!");
    return;
  } 
  mg_set_protocol_http_websocket(nc);

  opts_.document_root = doc_root_.c_str();
  opts_.enable_directory_listing = "no";
  tv.print(0, "Running server out of " + doc_root_ + "\n");
  tv.print(0, "Server started on port " + port_ + "\n");

  while (!stop_requested()) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);
  tv.print(0, "Shutting down server\n");
}

void WebUi::buffer(const string& api, const string& val, bool quote, bool force) {
  lock_guard<recursive_mutex> lg(lock_);

  if (!force && (buffer_.size() > max_buffer_)) {
    ++overflow_;
    return;
  } 

  stringstream ss;
  ss << "{\"api\":\"" + api + "\",\"val\":";
  if (quote) {
    ss << "\"" << val << "\"";
  } else {
    ss << val;
  }
  ss << "}";
  buffer_.push_back(ss.str());
}

void WebUi::ok(const std::string& s) {
  stringstream ss;
  HtmlPrinter(ss) << Color::GREEN << s << " OK\n" << Color::RESET;
  buffer("log", ss.str(), true, true);
}

} // namespace cascade
