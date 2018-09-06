%{ 
#include "regex.tab.hh"
#include "driver.h"
#include "lexer.h"

using namespace ns;
%}

%option c++ 
%option debug 
%option interactive 
%option noinput
%option nounput 
%option noyywrap 

token [0-9a-zA-Z .^$] 

%{
#define YY_DECL yyParser::symbol_type yyLexer::yylex(Driver& driver)
#define YY_USER_ACTION driver.loc.columns(yyleng);
%}

%%

%{
driver.loc.step();
%}

")"     return yyParser::make_CPAREN(driver.loc);
"("     return yyParser::make_OPAREN(driver.loc);
"|"     return yyParser::make_PIPE(driver.loc);
"+"     return yyParser::make_PLUS(driver.loc);
"?"     return yyParser::make_QMARK(driver.loc);
"*"     return yyParser::make_STAR(driver.loc);

{token} return yyParser::make_TOKEN(yytext[0], driver.loc);

.       driver.set_why(driver.loc, "invalid character");
"\n"    return yyParser::make_END(driver.loc);
<<EOF>> return yyParser::make_END(driver.loc);

%%

int yyFlexLexer::yylex() {
  return 0;
}
