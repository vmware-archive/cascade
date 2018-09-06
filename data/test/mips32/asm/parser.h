#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "driver.h"

namespace ns {

class Parser {
  public:
    Parser() : tl_(false), tr_(false) { }

    Parser& trace_lexemes(bool tl) {
      tl_ = tl;
      return *this;
    }
    Parser& trace_rules(bool tr) {
      tr_ = tr;
      return *this;
    }

    void parse(std::istream& is, std::ostream& os) {
      driver_.run(is, tl_, tr_);
      if (error()) {
        return;
      }
      for (auto h : driver_.hex) {
        os << std::hex << h << std::endl;
      }
    }
    bool error() const {
      return driver_.error;
    }
    const std::string& why() const {
      return driver_.why;
    }

  private:
    Driver driver_;
    bool tl_;
    bool tr_;
};

} // namespace ns

#endif

