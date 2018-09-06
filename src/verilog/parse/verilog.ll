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
#define YY_USER_ACTION parser->loc_.columns(yyleng);

std::string strip_num(const char* c, size_t n);
std::string strip_path(const char* c);
%}

%%

%{
parser->loc_.step();
%}

[ \t]+     parser->text_ += yytext; parser->loc_.step();
[\n]       parser->text_ += yytext; parser->loc_.lines(1); parser->loc_.step();

"//"[^\n]*                  parser->text_ += yytext; parser->loc_.step();
"/*"([^*]|(\*+[^*/]))*\*+\/ parser->text_ += yytext; for (size_t i = 0; i < yyleng; ++i) { if (yytext[i] == '\n') { parser->loc_.lines(1); } } parser->loc_.step();

"&&"      parser->text_ += yytext; return yyParser::make_AAMP(parser->loc_);
"&"       parser->text_ += yytext; return yyParser::make_AMP(parser->loc_);
"@"       parser->text_ += yytext; return yyParser::make_AT(parser->loc_);
"!"       parser->text_ += yytext; return yyParser::make_BANG(parser->loc_);
"!=="     parser->text_ += yytext; return yyParser::make_BEEQ(parser->loc_);
"!="      parser->text_ += yytext; return yyParser::make_BEQ(parser->loc_);
"^"       parser->text_ += yytext; return yyParser::make_CARAT(parser->loc_);
"}"       parser->text_ += yytext; return yyParser::make_CCURLY(parser->loc_);
":"       parser->text_ += yytext; return yyParser::make_COLON(parser->loc_);
","       parser->text_ += yytext; return yyParser::make_COMMA(parser->loc_);
")"       parser->text_ += yytext; return yyParser::make_CPAREN(parser->loc_);
"]"       parser->text_ += yytext; return yyParser::make_CSQUARE(parser->loc_);
"*)"      parser->text_ += yytext; return yyParser::make_CTIMES(parser->loc_);
"/"       parser->text_ += yytext; return yyParser::make_DIV(parser->loc_);
"."       parser->text_ += yytext; return yyParser::make_DOT(parser->loc_);
"==="     parser->text_ += yytext; return yyParser::make_EEEQ(parser->loc_);
"=="      parser->text_ += yytext; return yyParser::make_EEQ(parser->loc_);
"="       parser->text_ += yytext; return yyParser::make_EQ(parser->loc_);
">="      parser->text_ += yytext; return yyParser::make_GEQ(parser->loc_);
">>>"     parser->text_ += yytext; return yyParser::make_GGGT(parser->loc_);
">>"      parser->text_ += yytext; return yyParser::make_GGT(parser->loc_);
">"       parser->text_ += yytext; return yyParser::make_GT(parser->loc_);
"<="      parser->text_ += yytext; return yyParser::make_LEQ(parser->loc_);
"<<<"     parser->text_ += yytext; return yyParser::make_LLLT(parser->loc_);
"<<"      parser->text_ += yytext; return yyParser::make_LLT(parser->loc_);
"<"       parser->text_ += yytext; return yyParser::make_LT(parser->loc_);
"-:"      parser->text_ += yytext; return yyParser::make_MCOLON(parser->loc_);
"-"       parser->text_ += yytext; return yyParser::make_MINUS(parser->loc_);
"%"       parser->text_ += yytext; return yyParser::make_MOD(parser->loc_);
"{"       parser->text_ += yytext; return yyParser::make_OCURLY(parser->loc_);
"("       parser->text_ += yytext; return yyParser::make_OPAREN(parser->loc_);
"["       parser->text_ += yytext; return yyParser::make_OSQUARE(parser->loc_);
"(*"      parser->text_ += yytext; return yyParser::make_OTIMES(parser->loc_);
"+:"      parser->text_ += yytext; return yyParser::make_PCOLON(parser->loc_);
"|"       parser->text_ += yytext; return yyParser::make_PIPE(parser->loc_);
"||"      parser->text_ += yytext; return yyParser::make_PPIPE(parser->loc_);
"+"       parser->text_ += yytext; return yyParser::make_PLUS(parser->loc_);
"#"       parser->text_ += yytext; return yyParser::make_POUND(parser->loc_);
"?"       parser->text_ += yytext; return yyParser::make_QMARK(parser->loc_);
";"[ \t]* parser->text_ += yytext; return yyParser::make_SCOLON(parser->loc_);
"(*)"     parser->text_ += yytext; return yyParser::make_STAR(parser->loc_);
"~&"      parser->text_ += yytext; return yyParser::make_TAMP(parser->loc_);
"~^"      parser->text_ += yytext; return yyParser::make_TCARAT(parser->loc_);
"^~"      parser->text_ += yytext; return yyParser::make_TCARAT(parser->loc_);
"~"       parser->text_ += yytext; return yyParser::make_TILDE(parser->loc_);
"*"       parser->text_ += yytext; return yyParser::make_TIMES(parser->loc_);
"**"      parser->text_ += yytext; return yyParser::make_TTIMES(parser->loc_);
"~|"      parser->text_ += yytext; return yyParser::make_TPIPE(parser->loc_);

"always"      parser->text_ += yytext; return yyParser::make_ALWAYS(parser->loc_);
"assign"      parser->text_ += yytext; return yyParser::make_ASSIGN(parser->loc_);
"begin"       parser->text_ += yytext; return yyParser::make_BEGIN_(parser->loc_);
"case"        parser->text_ += yytext; return yyParser::make_CASE(parser->loc_);
"casex"       parser->text_ += yytext; return yyParser::make_CASEX(parser->loc_);
"casez"       parser->text_ += yytext; return yyParser::make_CASEZ(parser->loc_);
"default"     parser->text_ += yytext; return yyParser::make_DEFAULT(parser->loc_);
"disable"     parser->text_ += yytext; return yyParser::make_DISABLE(parser->loc_);
"else"        parser->text_ += yytext; return yyParser::make_ELSE(parser->loc_);
"end"         parser->text_ += yytext; return yyParser::make_END(parser->loc_);
"endcase"     parser->text_ += yytext; return yyParser::make_ENDCASE(parser->loc_);
"endgenerate" parser->text_ += yytext; return yyParser::make_ENDGENERATE(parser->loc_);
"endmodule"   parser->text_ += yytext; return yyParser::make_ENDMODULE(parser->loc_);
"for"         parser->text_ += yytext; return yyParser::make_FOR(parser->loc_);
"fork"        parser->text_ += yytext; return yyParser::make_FORK(parser->loc_);
"forever"     parser->text_ += yytext; return yyParser::make_FOREVER(parser->loc_);
"generate"    parser->text_ += yytext; return yyParser::make_GENERATE(parser->loc_);
"genvar"      parser->text_ += yytext; return yyParser::make_GENVAR(parser->loc_);
"if"          parser->text_ += yytext; return yyParser::make_IF(parser->loc_);
"initial"     parser->text_ += yytext; return yyParser::make_INITIAL_(parser->loc_);
"inout"       parser->text_ += yytext; return yyParser::make_INOUT(parser->loc_);
"input"       parser->text_ += yytext; return yyParser::make_INPUT(parser->loc_);
"integer"     parser->text_ += yytext; return yyParser::make_INTEGER(parser->loc_);
"join"        parser->text_ += yytext; return yyParser::make_JOIN(parser->loc_);
"localparam"  parser->text_ += yytext; return yyParser::make_LOCALPARAM(parser->loc_);
"macromodule" parser->text_ += yytext; return yyParser::make_MACROMODULE(parser->loc_);
"module"      parser->text_ += yytext; return yyParser::make_MODULE(parser->loc_);
"negedge"     parser->text_ += yytext; return yyParser::make_NEGEDGE(parser->loc_);
"or"          parser->text_ += yytext; return yyParser::make_OR(parser->loc_);
"output"      parser->text_ += yytext; return yyParser::make_OUTPUT(parser->loc_);
"parameter"   parser->text_ += yytext; return yyParser::make_PARAMETER(parser->loc_);
"posedge"     parser->text_ += yytext; return yyParser::make_POSEDGE(parser->loc_);
"reg"         parser->text_ += yytext; return yyParser::make_REG(parser->loc_);
"repeat"      parser->text_ += yytext; return yyParser::make_REPEAT(parser->loc_);
"signed"      parser->text_ += yytext; return yyParser::make_SIGNED(parser->loc_);
"wait"        parser->text_ += yytext; return yyParser::make_WAIT(parser->loc_);
"while"       parser->text_ += yytext; return yyParser::make_WHILE(parser->loc_);
"wire"        parser->text_ += yytext; return yyParser::make_WIRE(parser->loc_);

"$display" parser->text_ += yytext; return yyParser::make_SYS_DISPLAY(parser->loc_);
"$finish"  parser->text_ += yytext; return yyParser::make_SYS_FINISH(parser->loc_);
"$write"   parser->text_ += yytext; return yyParser::make_SYS_WRITE(parser->loc_);

"include"[ \t\n]+[^;]+";" parser->text_ += yytext; return yyParser::make_INCLUDE(strip_path(yytext), parser->loc_);

[0-9][0-9_]*                               parser->text_ += yytext; return yyParser::make_UNSIGNED_NUM(yytext, parser->loc_);
'[sS]?[dD][ \t\n]*[0-9][0-9_]*             parser->text_ += yytext; return yyParser::make_DECIMAL_VALUE(strip_num(yytext, yyleng), parser->loc_);
'[sS]?[bB][ \t\n]*[01][01_]*               parser->text_ += yytext; return yyParser::make_BINARY_VALUE(strip_num(yytext, yyleng), parser->loc_);
'[sS]?[oO][ \t\n]*[0-7][0-7_]*             parser->text_ += yytext; return yyParser::make_OCTAL_VALUE(strip_num(yytext, yyleng), parser->loc_);
'[sS]?[hH][ \t\n]*[0-9a-fA-F][0-9a-fA-F_]* parser->text_ += yytext; return yyParser::make_HEX_VALUE(strip_num(yytext, yyleng), parser->loc_);

[a-zA-Z_][a-zA-Z0-9_$]* parser->text_ += yytext; return yyParser::make_SIMPLE_ID(yytext, parser->loc_);
\"[^"\n]*\"             parser->text_ += yytext; return yyParser::make_STRING(std::string(yytext+1,yyleng-2), parser->loc_);

<<EOF>> parser->text_ += yytext; return yyParser::make_END_OF_FILE(parser->loc_);

%%

std::string strip_num(const char* c, size_t n) {
  size_t i = 1;
  if (c[i] == 's' || c[i] == 'S') {
    ++i;
  }
  ++i;

  std::string s;
  for (; i < n; ++i) {
    if (!isspace(c[i]) && c[i] != '_') {
      s += c[i];
    }
  }

  return s;
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
