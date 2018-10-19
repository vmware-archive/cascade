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
#define YY_REC parser->last_parse_ += yytext;
#define YY_USER_ACTION parser->loc().columns(yyleng);

std::pair<bool, std::string> strip_num(const char* c, size_t n);
std::string strip_path(const char* c);
%}

%%

[ \t]+     YY_REC; parser->loc().columns(yyleng); parser->loc().step();
[\n]       YY_REC; parser->loc().lines(1); parser->loc().step();

"//"[^\n]*                  YY_REC; parser->loc().columns(yyleng); parser->loc().step();
"/*"([^*]|(\*+[^*/]))*\*+\/ YY_REC; for (size_t i = 0; i < yyleng; ++i) { if (yytext[i] == '\n') { parser->loc().lines(1); } else { parser->loc().columns(1); } } parser->loc().step();

"&&"      YY_REC; return yyParser::make_AAMP(parser->loc());
"&"       YY_REC; return yyParser::make_AMP(parser->loc());
"@"       YY_REC; return yyParser::make_AT(parser->loc());
"!"       YY_REC; return yyParser::make_BANG(parser->loc());
"!=="     YY_REC; return yyParser::make_BEEQ(parser->loc());
"!="      YY_REC; return yyParser::make_BEQ(parser->loc());
"^"       YY_REC; return yyParser::make_CARAT(parser->loc());
"}"       YY_REC; return yyParser::make_CCURLY(parser->loc());
":"       YY_REC; return yyParser::make_COLON(parser->loc());
","       YY_REC; return yyParser::make_COMMA(parser->loc());
")"       YY_REC; return yyParser::make_CPAREN(parser->loc());
"]"       YY_REC; return yyParser::make_CSQUARE(parser->loc());
"*)"      YY_REC; return yyParser::make_CTIMES(parser->loc());
"/"       YY_REC; return yyParser::make_DIV(parser->loc());
"."       YY_REC; return yyParser::make_DOT(parser->loc());
"==="     YY_REC; return yyParser::make_EEEQ(parser->loc());
"=="      YY_REC; return yyParser::make_EEQ(parser->loc());
"="       YY_REC; return yyParser::make_EQ(parser->loc());
">="      YY_REC; return yyParser::make_GEQ(parser->loc());
">>>"     YY_REC; return yyParser::make_GGGT(parser->loc());
">>"      YY_REC; return yyParser::make_GGT(parser->loc());
">"       YY_REC; return yyParser::make_GT(parser->loc());
"<="      YY_REC; return yyParser::make_LEQ(parser->loc());
"<<<"     YY_REC; return yyParser::make_LLLT(parser->loc());
"<<"      YY_REC; return yyParser::make_LLT(parser->loc());
"<"       YY_REC; return yyParser::make_LT(parser->loc());
"-:"      YY_REC; return yyParser::make_MCOLON(parser->loc());
"-"       YY_REC; return yyParser::make_MINUS(parser->loc());
"%"       YY_REC; return yyParser::make_MOD(parser->loc());
"{"       YY_REC; return yyParser::make_OCURLY(parser->loc());
"("       YY_REC; return yyParser::make_OPAREN(parser->loc());
"["       YY_REC; return yyParser::make_OSQUARE(parser->loc());
"(*"      YY_REC; return yyParser::make_OTIMES(parser->loc());
"+:"      YY_REC; return yyParser::make_PCOLON(parser->loc());
"|"       YY_REC; return yyParser::make_PIPE(parser->loc());
"||"      YY_REC; return yyParser::make_PPIPE(parser->loc());
"+"       YY_REC; return yyParser::make_PLUS(parser->loc());
"#"       YY_REC; return yyParser::make_POUND(parser->loc());
"?"       YY_REC; return yyParser::make_QMARK(parser->loc());
";"       YY_REC; return yyParser::make_SCOLON(parser->loc());
"(*)"     YY_REC; return yyParser::make_STAR(parser->loc());
"~&"      YY_REC; return yyParser::make_TAMP(parser->loc());
"~^"      YY_REC; return yyParser::make_TCARAT(parser->loc());
"^~"      YY_REC; return yyParser::make_TCARAT(parser->loc());
"~"       YY_REC; return yyParser::make_TILDE(parser->loc());
"*"       YY_REC; return yyParser::make_TIMES(parser->loc());
"**"      YY_REC; return yyParser::make_TTIMES(parser->loc());
"~|"      YY_REC; return yyParser::make_TPIPE(parser->loc());

"always"      YY_REC; return yyParser::make_ALWAYS(parser->loc());
"assign"      YY_REC; return yyParser::make_ASSIGN(parser->loc());
"begin"       YY_REC; return yyParser::make_BEGIN_(parser->loc());
"case"        YY_REC; return yyParser::make_CASE(parser->loc());
"casex"       YY_REC; return yyParser::make_CASEX(parser->loc());
"casez"       YY_REC; return yyParser::make_CASEZ(parser->loc());
"default"     YY_REC; return yyParser::make_DEFAULT(parser->loc());
"disable"     YY_REC; return yyParser::make_DISABLE(parser->loc());
"else"        YY_REC; return yyParser::make_ELSE(parser->loc());
"end"         YY_REC; return yyParser::make_END(parser->loc());
"endcase"     YY_REC; return yyParser::make_ENDCASE(parser->loc());
"endgenerate" YY_REC; return yyParser::make_ENDGENERATE(parser->loc());
"endmodule"   YY_REC; return yyParser::make_ENDMODULE(parser->loc());
"for"         YY_REC; return yyParser::make_FOR(parser->loc());
"fork"        YY_REC; return yyParser::make_FORK(parser->loc());
"forever"     YY_REC; return yyParser::make_FOREVER(parser->loc());
"generate"    YY_REC; return yyParser::make_GENERATE(parser->loc());
"genvar"      YY_REC; return yyParser::make_GENVAR(parser->loc());
"if"          YY_REC; return yyParser::make_IF(parser->loc());
"initial"     YY_REC; return yyParser::make_INITIAL_(parser->loc());
"inout"       YY_REC; return yyParser::make_INOUT(parser->loc());
"input"       YY_REC; return yyParser::make_INPUT(parser->loc());
"integer"     YY_REC; return yyParser::make_INTEGER(parser->loc());
"join"        YY_REC; return yyParser::make_JOIN(parser->loc());
"localparam"  YY_REC; return yyParser::make_LOCALPARAM(parser->loc());
"macromodule" YY_REC; return yyParser::make_MACROMODULE(parser->loc());
"module"      YY_REC; return yyParser::make_MODULE(parser->loc());
"negedge"     YY_REC; return yyParser::make_NEGEDGE(parser->loc());
"or"          YY_REC; return yyParser::make_OR(parser->loc());
"output"      YY_REC; return yyParser::make_OUTPUT(parser->loc());
"parameter"   YY_REC; return yyParser::make_PARAMETER(parser->loc());
"posedge"     YY_REC; return yyParser::make_POSEDGE(parser->loc());
"reg"         YY_REC; return yyParser::make_REG(parser->loc());
"repeat"      YY_REC; return yyParser::make_REPEAT(parser->loc());
"signed"      YY_REC; return yyParser::make_SIGNED(parser->loc());
"wait"        YY_REC; return yyParser::make_WAIT(parser->loc());
"while"       YY_REC; return yyParser::make_WHILE(parser->loc());
"wire"        YY_REC; return yyParser::make_WIRE(parser->loc());

"$display" YY_REC; return yyParser::make_SYS_DISPLAY(parser->loc());
"$finish"  YY_REC; return yyParser::make_SYS_FINISH(parser->loc());
"$write"   YY_REC; return yyParser::make_SYS_WRITE(parser->loc());

"include"[ \t\n]+[^;]+";" YY_REC; return yyParser::make_INCLUDE(strip_path(yytext), parser->loc());

[0-9][0-9_]*                               YY_REC; return yyParser::make_UNSIGNED_NUM(yytext, parser->loc());
'[sS]?[dD][ \t\n]*[0-9][0-9_]*             YY_REC; return yyParser::make_DECIMAL_VALUE(strip_num(yytext, yyleng), parser->loc());
'[sS]?[bB][ \t\n]*[01][01_]*               YY_REC; return yyParser::make_BINARY_VALUE(strip_num(yytext, yyleng), parser->loc());
'[sS]?[oO][ \t\n]*[0-7][0-7_]*             YY_REC; return yyParser::make_OCTAL_VALUE(strip_num(yytext, yyleng), parser->loc());
'[sS]?[hH][ \t\n]*[0-9a-fA-F][0-9a-fA-F_]* YY_REC; return yyParser::make_HEX_VALUE(strip_num(yytext, yyleng), parser->loc());

[a-zA-Z_][a-zA-Z0-9_$]* YY_REC; return yyParser::make_SIMPLE_ID(yytext, parser->loc());
\"[^"\n]*\"             YY_REC; return yyParser::make_STRING(std::string(yytext+1,yyleng-2), parser->loc());

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
