#ifndef DRIVER_H
#define DRIVER_H

#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "mips32.tab.hh"
#include "lexer.h"

namespace ns {

struct Driver {
  public:
    void run(std::istream& is, bool tl, bool tp) {
      lexer.switch_streams(&is);
      lexer.set_debug(tl);
      loc.initialize();

      yyParser parser(*this);
      parser.set_debug_level(tp);

      pc = 0;
      labels.clear();
      abs_refs.clear();
      refs.clear();
      hex.clear();
      error = parser.parse() != 0;

      for (const auto& r : refs) {
        if (error) {
          return;
        }
        resolve_ref(r.first, r.second, false);
      }
      for (const auto& r : abs_refs) {
        if (error) {
          return;
        }
        resolve_ref(r.first, r.second, true);
      }
    }

    bool bind(const std::string& label) {
      if (labels.find(label) != labels.end()) {
        return false;
      }
      labels[label] = pc;
      return true;
    }

    void emit_branch(uint32_t op, uint32_t rs, uint32_t rt, const std::string& label) {
      refs[pc] = label;
      emit_itype(op, rs, rt, 0x0);
    }

    void emit_break(uint32_t code) {
      const auto inst = (code << 6) | 0xd;
      hex.push_back(inst);
      pc += 4;
    }

    void emit_itype(uint32_t op, uint32_t rs, uint32_t rt, uint32_t imm) {
      const auto inst = (op << 26) | (rs << 21) | (rt << 16) | imm;
      hex.push_back(inst);
      pc += 4;
    }

    void emit_jtype(uint32_t op, const std::string& label) {
      const auto inst = (op << 26) | 0x0;
      abs_refs[pc] = label;
      hex.push_back(inst);
      pc += 4;
    }

    void emit_nop() {
      hex.push_back(0);
      pc += 4;
    }

    void emit_rtype(uint32_t op, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt, uint32_t func) {
      const auto inst = (op << 26) | (rs << 21) | (rt << 16) | (rd << 11) | (shamt << 6) | func;
      hex.push_back(inst);
      pc += 4;
    }

    void set_why(const location& l, const std::string& m) {
      std::ostringstream oss;
      oss << l << ": " << m;
      why = oss.str();
    }

    void resolve_ref(uint32_t pc, const std::string& target, bool jump) {
      auto ptr = (uint16_t*)(&hex[pc/4]);
      auto itr = labels.find(target);
      if (itr == labels.end()) {
        error = true;
        set_why(loc, "Reference to undefined label");
      } else if (jump) {
        *ptr = itr->second/4;
      } else {
        *ptr = (itr->second - (pc + 4))/4;
      }
    }

    yyLexer lexer;
    location loc;

    bool error;
    std::string why;

    uint32_t pc;
    std::unordered_map<std::string, uint32_t> labels;
    std::unordered_map<uint32_t, std::string> abs_refs;
    std::unordered_map<uint32_t, std::string> refs;
    std::vector<uint32_t> hex;
};

} // namespace ns

#endif 
