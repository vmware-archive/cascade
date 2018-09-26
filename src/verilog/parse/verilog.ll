%{ 
#include <cctype>
#include <string>
#include "src/verilog/parse/verilog.tab.hh"
#include "src/verilog/parse/lexer.h"
#include "src/verilog/parse/parser.h"

using namespace cascade;
%}

%option c++ 
%option debug 
%option interactive 
%option noinput
%option nounput 
%option noyywrap 

%{
#define YY_DECL yyParser::symbol_type yyLexer::yylex(Parser* parser)
#define YY_USER_ACTION parser->loc().columns(yyleng);

std::pair<bool, std::string> strip_num(const char* c, size_t n);
std::string strip_path(const char* c);
%}

%%

[ \t]+     parser->loc().columns(yyleng); parser->loc().step();
[\n]       parser->loc().lines(1); parser->loc().step();

"//"[^\n]*                  parser->loc().columns(yyleng); parser->loc().step();
"/*"([^*]|(\*+[^*/]))*\*+\/ for (size_t i = 0; i < yyleng; ++i) { if (yytext[i] == '\n') { parser->loc().lines(1); } else { parser->loc().columns(1); } } parser->loc().step();

"&&"      return yyParser::make_AAMP(parser->loc());
"&"       return yyParser::make_AMP(parser->loc());
"@"       return yyParser::make_AT(parser->loc());
"!"       return yyParser::make_BANG(parser->loc());
"!=="     return yyParser::make_BEEQ(parser->loc());
"!="      return yyParser::make_BEQ(parser->loc());
"^"       return yyParser::make_CARAT(parser->loc());
"}"       return yyParser::make_CCURLY(parser->loc());
":"       return yyParser::make_COLON(parser->loc());
","       return yyParser::make_COMMA(parser->loc());
")"       return yyParser::make_CPAREN(parser->loc());
"]"       return yyParser::make_CSQUARE(parser->loc());
"*)"      return yyParser::make_CTIMES(parser->loc());
"/"       return yyParser::make_DIV(parser->loc());
"."       return yyParser::make_DOT(parser->loc());
"==="     return yyParser::make_EEEQ(parser->loc());
"=="      return yyParser::make_EEQ(parser->loc());
"="       return yyParser::make_EQ(parser->loc());
">="      return yyParser::make_GEQ(parser->loc());
">>>"     return yyParser::make_GGGT(parser->loc());
">>"      return yyParser::make_GGT(parser->loc());
">"       return yyParser::make_GT(parser->loc());
"<="      return yyParser::make_LEQ(parser->loc());
"<<<"     return yyParser::make_LLLT(parser->loc());
"<<"      return yyParser::make_LLT(parser->loc());
"<"       return yyParser::make_LT(parser->loc());
"-:"      return yyParser::make_MCOLON(parser->loc());
"-"       return yyParser::make_MINUS(parser->loc());
"%"       return yyParser::make_MOD(parser->loc());
"{"       return yyParser::make_OCURLY(parser->loc());
"("       return yyParser::make_OPAREN(parser->loc());
"["       return yyParser::make_OSQUARE(parser->loc());
"(*"      return yyParser::make_OTIMES(parser->loc());
"+:"      return yyParser::make_PCOLON(parser->loc());
"|"       return yyParser::make_PIPE(parser->loc());
"||"      return yyParser::make_PPIPE(parser->loc());
"+"       return yyParser::make_PLUS(parser->loc());
"#"       return yyParser::make_POUND(parser->loc());
"?"       return yyParser::make_QMARK(parser->loc());
";"       return yyParser::make_SCOLON(parser->loc());
"(*)"     return yyParser::make_STAR(parser->loc());
"~&"      return yyParser::make_TAMP(parser->loc());
"~^"      return yyParser::make_TCARAT(parser->loc());
"^~"      return yyParser::make_TCARAT(parser->loc());
"~"       return yyParser::make_TILDE(parser->loc());
"*"       return yyParser::make_TIMES(parser->loc());
"**"      return yyParser::make_TTIMES(parser->loc());
"~|"      return yyParser::make_TPIPE(parser->loc());

"always"      return yyParser::make_ALWAYS(parser->loc());
"assign"      return yyParser::make_ASSIGN(parser->loc());
"begin"       return yyParser::make_BEGIN_(parser->loc());
"case"        return yyParser::make_CASE(parser->loc());
"casex"       return yyParser::make_CASEX(parser->loc());
"casez"       return yyParser::make_CASEZ(parser->loc());
"default"     return yyParser::make_DEFAULT(parser->loc());
"disable"     return yyParser::make_DISABLE(parser->loc());
"else"        return yyParser::make_ELSE(parser->loc());
"end"         return yyParser::make_END(parser->loc());
"endcase"     return yyParser::make_ENDCASE(parser->loc());
"endgenerate" return yyParser::make_ENDGENERATE(parser->loc());
"endmodule"   return yyParser::make_ENDMODULE(parser->loc());
"for"         return yyParser::make_FOR(parser->loc());
"fork"        return yyParser::make_FORK(parser->loc());
"forever"     return yyParser::make_FOREVER(parser->loc());
"generate"    return yyParser::make_GENERATE(parser->loc());
"genvar"      return yyParser::make_GENVAR(parser->loc());
"if"          return yyParser::make_IF(parser->loc());
"initial"     return yyParser::make_INITIAL_(parser->loc());
"inout"       return yyParser::make_INOUT(parser->loc());
"input"       return yyParser::make_INPUT(parser->loc());
"integer"     return yyParser::make_INTEGER(parser->loc());
"join"        return yyParser::make_JOIN(parser->loc());
"localparam"  return yyParser::make_LOCALPARAM(parser->loc());
"macromodule" return yyParser::make_MACROMODULE(parser->loc());
"module"      return yyParser::make_MODULE(parser->loc());
"negedge"     return yyParser::make_NEGEDGE(parser->loc());
"or"          return yyParser::make_OR(parser->loc());
"output"      return yyParser::make_OUTPUT(parser->loc());
"parameter"   return yyParser::make_PARAMETER(parser->loc());
"posedge"     return yyParser::make_POSEDGE(parser->loc());
"reg"         return yyParser::make_REG(parser->loc());
"repeat"      return yyParser::make_REPEAT(parser->loc());
"signed"      return yyParser::make_SIGNED(parser->loc());
"wait"        return yyParser::make_WAIT(parser->loc());
"while"       return yyParser::make_WHILE(parser->loc());
"wire"        return yyParser::make_WIRE(parser->loc());

"$display" return yyParser::make_SYS_DISPLAY(parser->loc());
"$finish"  return yyParser::make_SYS_FINISH(parser->loc());
"$write"   return yyParser::make_SYS_WRITE(parser->loc());

"include"[ \t\n]+[^;]+";" return yyParser::make_INCLUDE(strip_path(yytext), parser->loc());

[0-9][0-9_]*                               return yyParser::make_UNSIGNED_NUM(yytext, parser->loc());
'[sS]?[dD][ \t\n]*[0-9][0-9_]*             return yyParser::make_DECIMAL_VALUE(strip_num(yytext, yyleng), parser->loc());
'[sS]?[bB][ \t\n]*[01][01_]*               return yyParser::make_BINARY_VALUE(strip_num(yytext, yyleng), parser->loc());
'[sS]?[oO][ \t\n]*[0-7][0-7_]*             return yyParser::make_OCTAL_VALUE(strip_num(yytext, yyleng), parser->loc());
'[sS]?[hH][ \t\n]*[0-9a-fA-F][0-9a-fA-F_]* return yyParser::make_HEX_VALUE(strip_num(yytext, yyleng), parser->loc());

[a-zA-Z_][a-zA-Z0-9_$]* return yyParser::make_SIMPLE_ID(yytext, parser->loc());
\"[^"\n]*\"             return yyParser::make_STRING(std::string(yytext+1,yyleng-2), parser->loc());

<<EOF>> return yyParser::make_END_OF_FILE(parser->loc());

%%

std::pair<bool, std::string> strip_num(const char* c, size_t n) {
  auto is_signed = false;

  size_t i = 1;
  if (c[i] == 's' || c[i] == 'S') {
    is_signed = true;
    ++i;
  }
  ++i;

  std::string s;
  for (; i < n; ++i) {
    if (!isspace(c[i]) && c[i] != '_') {
      s += c[i];
    }
  }

  return std::make_pair(is_signed, s);
}

std::string strip_path(const char* c) {
  size_t i = 7;
  while (isspace(c[i++]));

  std::string s;
  for (--i; c[i] != ';'; ++i) {
    s += c[i];
  }

  return s;
}

int yyFlexLexer::yylex() {
  return 0;
}
