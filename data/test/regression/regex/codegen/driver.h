#ifndef DRIVER_H
#define DRIVER_H

#include <iostream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "nfa.h"
#include "regex.tab.hh"

namespace ns {

struct Driver {
  public:
    void run(std::istream& is, bool tl, bool tp) {
      lexer.switch_streams(&is);
      lexer.set_debug(tl);
      loc.initialize();

      yyParser parser(*this);
      parser.set_debug_level(tp);

      error = parser.parse() != 0;
    }

    void set_why(const location& l, const std::string& m) {
      std::ostringstream oss;
      oss << l << ": " << m;
      why = oss.str();
    }

    yyLexer lexer;
    location loc;

    Nfa* res;
    bool error;
    std::string why;
};

} // namespace ns

#endif 
