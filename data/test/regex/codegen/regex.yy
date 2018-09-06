%skeleton "lalr1.cc"
%require "3.0.4"
%defines
%define api.namespace{ns}
%define parser_class_name{yyParser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires
{
#include "nfa.h"
namespace ns {
class Driver;
} // namespace ns
}

%param {Driver& driver}

%locations

%define parse.trace
%define parse.error verbose

%code
{
#include "driver.h"

#undef yylex
#define yylex driver.lexer.yylex
}

%token
  CPAREN ")"
  OPAREN "("
  PIPE   "|"
  PLUS   "+"
  QMARK  "?"
  STAR   "*"

  END 0 "end of file"
;

%token <char> TOKEN "token"
%type <Nfa*> regex;
%type <Nfa*> regex_S;
%type <Nfa*> group;
%type <Nfa*> disjunct;
%type <Nfa*> plus;
%type <Nfa*> qmark;
%type <Nfa*> star;

%left PIPE 
%left PLUS QMARK STAR

%printer { yyoutput << $$; } <*>;

%%

%start main;

main 
  : regex_S { driver.res = $1; }

regex_S
  : regex { $$ = $1; }
  | regex regex_S { $1->concat($2); $$ = $1; }

regex 
  : TOKEN { $$ = new Nfa($1); }
  | group { $$ = $1; }
  | disjunct { $$ = $1; }
  | plus { $$ = $1; }
  | qmark { $$ = $1; }
  | star { $$ = $1; }
  ;

group
  : OPAREN regex_S CPAREN { $$ = $2; }
  ;
disjunct
  : regex PIPE regex { $1->disjoin($3); $$ = $1; }
  ;
plus 
  : regex PLUS { $1->plus(); $$ = $1; }
  ;
qmark
  : regex QMARK { $1->qmark(); $$ = $1; }
  ;
star
  : regex STAR { $1->star(); $$ = $1; }
  ;

%%

void ns::yyParser::error(const location_type& l, const std::string& m) {
  driver.set_why(l, m);
}
