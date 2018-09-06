#ifndef LEXER_H
#define LEXER_H

#ifndef __FLEX_LEXER_H
#include <FlexLexer.h>
#endif
#include <iosfwd>
#include "mips32.tab.hh"

namespace ns {

struct yyLexer : public yyFlexLexer {
  yyLexer() : yyFlexLexer() { }
  ~yyLexer() override = default;
      
  yyParser::symbol_type yylex(Driver& driver); 
};

} // namespace ns

#endif
