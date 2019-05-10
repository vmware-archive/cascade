%{ 
#include <cctype>
#include <string>
#include "base/stream/incstream.h"
#include "verilog/parse/verilog.tab.hh"
#include "verilog/parse/lexer.h"
#include "verilog/parse/parser.h"

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
#define YY_USER_ACTION parser->get_loc().columns(yyleng);

#undef YY_BUF_SIZE
#define YY_BUF_SIZE 1024*1024

std::pair<bool, std::string> strip_num(const char* c, size_t n);
%}

SPACE       ([ \t])
WHITESPACE  ([ \t\n])
NEWLINE     ([\n])
SL_COMMENT  ("//"[^\n]*)
ML_COMMENT  ("/*"([^*]|(\*+[^*/]))*\*+\/)
QUOTED_STR  (\"[^"\n]*\")
IDENTIFIER  ([a-zA-Z_][a-zA-Z0-9_]*)
DECIMAL     ([0-9][0-9_]*)
BINARY      ([01][01_]*)
OCTAL       ([0-7][0-7_]*)
HEX         ([0-9a-fA-F][0-9a-fA-F_]*) 
DEFINE_TEXT ((([^\\\n]*\\\n)*)[^\\\n]*\n)
IF_TEXT     ([^`]*)

%x DEFINE_ARGS
%x DEFINE_BODY

%x IFDEF_IF
%x IFDEF_TRUE
%x IFDEF_FALSE
%x IFDEF_DONE

%x MACRO_ARGS

%%

{SPACE}+    YY_REC; parser->get_loc().columns(yyleng); parser->get_loc().step();
{NEWLINE}   YY_REC; parser->get_loc().lines(1); parser->get_loc().step();

"`include"{SPACE}+{QUOTED_STR} {
  YY_REC;

  std::string s(yytext);
  const auto begin = s.find_first_of('"');
  const auto end = s.find_last_of('"');
  const auto path = s.substr(begin+1, end-begin-1);

  incstream is(parser->include_dirs_);
  if (!is.open(path)) {
    parser->log_->error("Unable to locate file " + path);
    return yyParser::make_UNPARSEABLE(parser->get_loc());
  }
  std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
  content += "`__end_include";
  for (auto i = content.rbegin(), ie = content.rend(); i != ie; ++i) {
    unput(*i);
  }

  if (parser->depth() == 15) {
    parser->log_->error("Exceeded maximum nesting depth (15) for include statements. Do you have a circular include?");
    return yyParser::make_UNPARSEABLE(parser->get_loc());
  }
  parser->push(path);
}
"`__end_include" parser->pop();

"`define"{SPACE}+{IDENTIFIER} {
  YY_REC; 
  parser->name_ = yytext;
  parser->name_ = parser->name_.substr(parser->name_.find_first_not_of(" \n\t", 7));
  BEGIN(DEFINE_BODY);
}
"`define"{SPACE}+{IDENTIFIER}"(" {
  YY_REC; 
  parser->name_ = yytext;
  parser->name_ = parser->name_.substr(parser->name_.find_first_not_of(" \n\t", 7));
  parser->name_.pop_back();
  BEGIN(DEFINE_ARGS);
}
<DEFINE_ARGS>{SPACE}*{IDENTIFIER}{SPACE}*"," {
  YY_REC;
  std::string arg = yytext;
  arg.pop_back();
  arg = arg.substr(arg.find_first_not_of(" \n\t"));
  arg = arg.substr(0, arg.find_first_of(" \n\t")-1);
  parser->macros_[parser->name_].first.push_back(arg);
}
<DEFINE_ARGS>{SPACE}*{IDENTIFIER}{SPACE}*")" {
  YY_REC;
  std::string arg = yytext;
  arg.pop_back();
  arg = arg.substr(arg.find_first_not_of(" \n\t"));
  arg = arg.substr(0, arg.find_first_of(" \n\t)")-1);
  parser->macros_[parser->name_].first.push_back(arg);
  BEGIN(DEFINE_BODY);
}
<DEFINE_BODY>{DEFINE_TEXT} {
  YY_REC;
  BEGIN(0);
  std::string text = yytext;
  for (auto& c : text) {
    if ((c == '\\') || (c == '\n')) {
      c = ' ';
    }
  }
  text.pop_back();
  parser->macros_[parser->name_].second = text;
}
"`undef"{SPACE}+{IDENTIFIER} {
  YY_REC;
  parser->name_ = yytext;
  parser->name_ = parser->name_.substr(parser->name_.find_first_not_of(" \n\t", 6));
  if (parser->is_defined(parser->name_)) {
    parser->undefine(parser->name_);
  }
} 

"`ifdef" {
  YY_REC;
  parser->polarity_ = true;
  ++parser->nesting_;
  BEGIN(IFDEF_IF);
}
"`ifndef" {
  YY_REC;
  parser->polarity_ = false;
  ++parser->nesting_;
  BEGIN(IFDEF_IF);
}
<IFDEF_IF>{SPACE}+{IDENTIFIER} {
  YY_REC;
  
  std::string name = yytext;
  name = name.substr(name.find_first_not_of(" \n\t"));
  const auto isdef = parser->is_defined(name);

  if ((isdef && parser->polarity_) || (!isdef && !parser->polarity_)) {
    BEGIN(IFDEF_TRUE);
  } else {
    BEGIN(IFDEF_FALSE);
  }
  parser->polarity_ = true;
}
<IFDEF_TRUE>{IF_TEXT} {
  YY_REC;
  yymore();
}
<IFDEF_TRUE>"`ifdef" {
  YY_REC;
  yymore();
  ++parser->nesting_;
}
<IFDEF_TRUE>"`ifndef" {
  YY_REC;
  yymore();
  ++parser->nesting_;
}
<IFDEF_TRUE>"`else" {
  YY_REC;
  if (parser->nesting_ == 1) {
    parser->text_ = yytext;
    parser->text_.resize(parser->text_.length()-5);
    BEGIN(IFDEF_DONE);
  } else {
    yymore();
  }
}
<IFDEF_TRUE>"`elsif" {
  YY_REC;
  if (parser->nesting_ == 1) {
    parser->text_ = yytext;
    parser->text_.resize(parser->text_.length()-6);
    BEGIN(IFDEF_DONE);
  } else {
    yymore();
  }
}
<IFDEF_TRUE>"`endif" {
  YY_REC;
  --parser->nesting_;
  if (parser->nesting_ == 0) {
    parser->text_ = yytext;
    parser->text_.resize(parser->text_.length()-6);
    for (auto i = parser->text_.rbegin(), ie = parser->text_.rend(); i != ie; ++i) {
      unput(*i);
    }
    BEGIN(0);
  } else {
    yymore();
  }
}
<IFDEF_TRUE>"`"{IDENTIFIER} {
  YY_REC;
  yymore();
}
<IFDEF_FALSE>{IF_TEXT} {
  YY_REC;
  yymore();
}
<IFDEF_FALSE>"`ifdef" {
  YY_REC;
  yymore();
  ++parser->nesting_;
}
<IFDEF_FALSE>"`ifndef" {
  YY_REC;
  yymore();
  ++parser->nesting_;
}
<IFDEF_FALSE>"`else" {
  YY_REC;
  if (parser->nesting_ == 1) {
    BEGIN(IFDEF_TRUE);
  } else {
    yymore();
  }
}
<IFDEF_FALSE>"`elsif" {
  YY_REC;
  if (parser->nesting_ == 1) {
    BEGIN(IFDEF_IF);
  } else {
    yymore();
  }
}
<IFDEF_FALSE>"`endif" {
  YY_REC;
  --parser->nesting_;
  if (parser->nesting_ == 0) {
    BEGIN(0);
  } else {
    yymore();
  }
}
<IFDEF_FALSE>"`"{IDENTIFIER} {
  YY_REC;
  yymore();
}
<IFDEF_DONE>{IF_TEXT} {
  YY_REC;
  yymore();
}
<IFDEF_DONE>"`ifdef" {
  YY_REC;
  yymore();
  ++parser->nesting_;
}
<IFDEF_DONE>"`ifndef" {
  YY_REC;
  yymore();
  ++parser->nesting_;
}
<IFDEF_DONE>"`else" {
  YY_REC;
  yymore();
}
<IFDEF_DONE>"`elsif" {
  YY_REC;
  yymore();
}
<IFDEF_DONE>"`endif" {
  YY_REC;
  --parser->nesting_;
  if (parser->nesting_ == 0) {
    for (auto i = parser->text_.rbegin(), ie = parser->text_.rend(); i != ie; ++i) {
      unput(*i);
    }
    BEGIN(0);
  } else {
    yymore();
  }
}
<IFDEF_DONE>"`"{IDENTIFIER} {
  YY_REC;
  yymore();
}

"`"{IDENTIFIER} {
  YY_REC;
  parser->name_ = yytext;
  parser->name_ = parser->name_.substr(1);
  parser->args_.clear();

  if (!parser->is_defined(parser->name_)) {
    parser->log_->error("Reference to unrecognized macro " + parser->name_);
    return yyParser::make_UNPARSEABLE(parser->get_loc());
  } else if (parser->arity(parser->name_) > 0) {
    BEGIN(MACRO_ARGS);
  } else if (parser->arity(parser->name_) != parser->args_.size()) {
    parser->log_->error("Usage error for macro named " + parser->name_);
    return yyParser::make_UNPARSEABLE(parser->get_loc());
  } else {
    const auto text = parser->replace(parser->name_, parser->args_);
    for (auto i = text.rbegin(), ie = text.rend(); i != ie; ++i) {
      unput(*i);
    }
    BEGIN(0);
  }
}
<MACRO_ARGS>{SPACE}*"(" {
  YY_REC;
  ++parser->nesting_;
  if (parser->nesting_ > 1) {
    yymore();
  }
}
<MACRO_ARGS>{SPACE}*")" {
  YY_REC;
  --parser->nesting_;
  if (parser->nesting_ > 0) {
    yymore();
  } else {
    std::string arg = yytext;
    arg.pop_back();
    parser->args_.push_back(arg);

    if (parser->arity(parser->name_) != parser->args_.size()) {
      parser->log_->error("Usage error for macro named " + parser->name_);
      return yyParser::make_UNPARSEABLE(parser->get_loc());
    } else {
      const auto text = parser->replace(parser->name_, parser->args_);
      for (auto i = text.rbegin(), ie = text.rend(); i != ie; ++i) {
        unput(*i);
      }
      BEGIN(0);
    }
  }
}
<MACRO_ARGS>{SPACE}*"{" {
  YY_REC;
  ++parser->nesting_;
  yymore();
}
<MACRO_ARGS>{SPACE}*"}" {
  YY_REC;
  --parser->nesting_;
  yymore();
}
<MACRO_ARGS>{SPACE}*[^(){},]* {
  YY_REC;
  yymore();
}
<MACRO_ARGS>"," {
  YY_REC;
  if (parser->nesting_ == 1) {
    std::string arg = yytext;
    arg.pop_back();
    parser->args_.push_back(arg);
  } else {
    yymore();
  }
}

{SL_COMMENT} {
  YY_REC; 
  parser->get_loc().columns(yyleng); 
  parser->get_loc().step();
}
{ML_COMMENT} {
  YY_REC; 
  for (size_t i = 0; i < yyleng; ++i) { 
    if (yytext[i] == '\n') { 
      parser->get_loc().lines(1); 
    } else { 
      parser->get_loc().columns(1); 
    } 
  } 
  parser->get_loc().step();
}

"&&"      YY_REC; return yyParser::make_AAMP(parser->get_loc());
"&"       YY_REC; return yyParser::make_AMP(parser->get_loc());
"@"       YY_REC; return yyParser::make_AT(parser->get_loc());
"!"       YY_REC; return yyParser::make_BANG(parser->get_loc());
"!=="     YY_REC; return yyParser::make_BEEQ(parser->get_loc());
"!="      YY_REC; return yyParser::make_BEQ(parser->get_loc());
"^"       YY_REC; return yyParser::make_CARAT(parser->get_loc());
"}"       YY_REC; return yyParser::make_CCURLY(parser->get_loc());
":"       YY_REC; return yyParser::make_COLON(parser->get_loc());
","       YY_REC; return yyParser::make_COMMA(parser->get_loc());
")"       YY_REC; return yyParser::make_CPAREN(parser->get_loc());
"]"       YY_REC; return yyParser::make_CSQUARE(parser->get_loc());
"*)"      YY_REC; return yyParser::make_CTIMES(parser->get_loc());
"/"       YY_REC; return yyParser::make_DIV(parser->get_loc());
"."       YY_REC; return yyParser::make_DOT(parser->get_loc());
"==="     YY_REC; return yyParser::make_EEEQ(parser->get_loc());
"=="      YY_REC; return yyParser::make_EEQ(parser->get_loc());
"="       YY_REC; return yyParser::make_EQ(parser->get_loc());
">="      YY_REC; return yyParser::make_GEQ(parser->get_loc());
">>>"     YY_REC; return yyParser::make_GGGT(parser->get_loc());
">>"      YY_REC; return yyParser::make_GGT(parser->get_loc());
">"       YY_REC; return yyParser::make_GT(parser->get_loc());
"<="      YY_REC; return yyParser::make_LEQ(parser->get_loc());
"<<<"     YY_REC; return yyParser::make_LLLT(parser->get_loc());
"<<"      YY_REC; return yyParser::make_LLT(parser->get_loc());
"<"       YY_REC; return yyParser::make_LT(parser->get_loc());
"-:"      YY_REC; return yyParser::make_MCOLON(parser->get_loc());
"-"       YY_REC; return yyParser::make_MINUS(parser->get_loc());
"%"       YY_REC; return yyParser::make_MOD(parser->get_loc());
"{"       YY_REC; return yyParser::make_OCURLY(parser->get_loc());
"("       YY_REC; return yyParser::make_OPAREN(parser->get_loc());
"["       YY_REC; return yyParser::make_OSQUARE(parser->get_loc());
"(*"      YY_REC; return yyParser::make_OTIMES(parser->get_loc());
"+:"      YY_REC; return yyParser::make_PCOLON(parser->get_loc());
"|"       YY_REC; return yyParser::make_PIPE(parser->get_loc());
"||"      YY_REC; return yyParser::make_PPIPE(parser->get_loc());
"+"       YY_REC; return yyParser::make_PLUS(parser->get_loc());
"#"       YY_REC; return yyParser::make_POUND(parser->get_loc());
"?"       YY_REC; return yyParser::make_QMARK(parser->get_loc());
";"       YY_REC; return yyParser::make_SCOLON(parser->get_loc());
"(*)"     YY_REC; return yyParser::make_STAR(parser->get_loc());
"~&"      YY_REC; return yyParser::make_TAMP(parser->get_loc());
"~^"      YY_REC; return yyParser::make_TCARAT(parser->get_loc());
"^~"      YY_REC; return yyParser::make_TCARAT(parser->get_loc());
"~"       YY_REC; return yyParser::make_TILDE(parser->get_loc());
"*"       YY_REC; return yyParser::make_TIMES(parser->get_loc());
"**"      YY_REC; return yyParser::make_TTIMES(parser->get_loc());
"~|"      YY_REC; return yyParser::make_TPIPE(parser->get_loc());

"always"      YY_REC; return yyParser::make_ALWAYS(parser->get_loc());
"assign"      YY_REC; return yyParser::make_ASSIGN(parser->get_loc());
"begin"       YY_REC; return yyParser::make_BEGIN_(parser->get_loc());
"case"        YY_REC; return yyParser::make_CASE(parser->get_loc());
"casex"       YY_REC; return yyParser::make_CASEX(parser->get_loc());
"casez"       YY_REC; return yyParser::make_CASEZ(parser->get_loc());
"default"     YY_REC; return yyParser::make_DEFAULT(parser->get_loc());
"disable"     YY_REC; return yyParser::make_DISABLE(parser->get_loc());
"else"        YY_REC; return yyParser::make_ELSE(parser->get_loc());
"end"         YY_REC; return yyParser::make_END(parser->get_loc());
"endcase"     YY_REC; return yyParser::make_ENDCASE(parser->get_loc());
"endgenerate" YY_REC; return yyParser::make_ENDGENERATE(parser->get_loc());
"endmodule"   YY_REC; return yyParser::make_ENDMODULE(parser->get_loc());
"for"         YY_REC; return yyParser::make_FOR(parser->get_loc());
"fork"        YY_REC; return yyParser::make_FORK(parser->get_loc());
"forever"     YY_REC; return yyParser::make_FOREVER(parser->get_loc());
"generate"    YY_REC; return yyParser::make_GENERATE(parser->get_loc());
"genvar"      YY_REC; return yyParser::make_GENVAR(parser->get_loc());
"if"          YY_REC; return yyParser::make_IF(parser->get_loc());
"initial"     YY_REC; return yyParser::make_INITIAL_(parser->get_loc());
"inout"       YY_REC; return yyParser::make_INOUT(parser->get_loc());
"input"       YY_REC; return yyParser::make_INPUT(parser->get_loc());
"integer"     YY_REC; return yyParser::make_INTEGER(parser->get_loc());
"join"        YY_REC; return yyParser::make_JOIN(parser->get_loc());
"localparam"  YY_REC; return yyParser::make_LOCALPARAM(parser->get_loc());
"macromodule" YY_REC; return yyParser::make_MACROMODULE(parser->get_loc());
"module"      YY_REC; return yyParser::make_MODULE(parser->get_loc());
"negedge"     YY_REC; return yyParser::make_NEGEDGE(parser->get_loc());
"or"          YY_REC; return yyParser::make_OR(parser->get_loc());
"output"      YY_REC; return yyParser::make_OUTPUT(parser->get_loc());
"parameter"   YY_REC; return yyParser::make_PARAMETER(parser->get_loc());
"posedge"     YY_REC; return yyParser::make_POSEDGE(parser->get_loc());
"reg"         YY_REC; return yyParser::make_REG(parser->get_loc());
"repeat"      YY_REC; return yyParser::make_REPEAT(parser->get_loc());
"signed"      YY_REC; return yyParser::make_SIGNED(parser->get_loc());
"stream"      YY_REC; return yyParser::make_STREAM(parser->get_loc());
"wait"        YY_REC; return yyParser::make_WAIT(parser->get_loc());
"while"       YY_REC; return yyParser::make_WHILE(parser->get_loc());
"wire"        YY_REC; return yyParser::make_WIRE(parser->get_loc());

"$display"  YY_REC; return yyParser::make_SYS_DISPLAY(parser->get_loc());
"$eof"      YY_REC; return yyParser::make_SYS_EOF(parser->get_loc());
"$error"    YY_REC; return yyParser::make_SYS_ERROR(parser->get_loc());
"$fatal"    YY_REC; return yyParser::make_SYS_FATAL(parser->get_loc());
"$finish"   YY_REC; return yyParser::make_SYS_FINISH(parser->get_loc());
"$fopen"    YY_REC; return yyParser::make_SYS_FOPEN(parser->get_loc());
"$get"      YY_REC; return yyParser::make_SYS_GET(parser->get_loc());
"$info"     YY_REC; return yyParser::make_SYS_INFO(parser->get_loc());
"$put"      YY_REC; return yyParser::make_SYS_PUT(parser->get_loc());
"$restart"  YY_REC; return yyParser::make_SYS_RESTART(parser->get_loc());
"$retarget" YY_REC; return yyParser::make_SYS_RETARGET(parser->get_loc());
"$save"     YY_REC; return yyParser::make_SYS_SAVE(parser->get_loc());
"$seek"     YY_REC; return yyParser::make_SYS_SEEK(parser->get_loc());
"$warning"  YY_REC; return yyParser::make_SYS_WARNING(parser->get_loc());
"$write"    YY_REC; return yyParser::make_SYS_WRITE(parser->get_loc());

{DECIMAL}                        YY_REC; return yyParser::make_UNSIGNED_NUM(yytext, parser->get_loc());
'[sS]?[dD]{WHITESPACE}*{DECIMAL} YY_REC; return yyParser::make_DECIMAL_VALUE(strip_num(yytext, yyleng), parser->get_loc());
'[sS]?[bB]{WHITESPACE}*{BINARY}  YY_REC; return yyParser::make_BINARY_VALUE(strip_num(yytext, yyleng), parser->get_loc());
'[sS]?[oO]{WHITESPACE}*{OCTAL}   YY_REC; return yyParser::make_OCTAL_VALUE(strip_num(yytext, yyleng), parser->get_loc());
'[sS]?[hH]{WHITESPACE}*{HEX}     YY_REC; return yyParser::make_HEX_VALUE(strip_num(yytext, yyleng), parser->get_loc());

{IDENTIFIER} YY_REC; return yyParser::make_SIMPLE_ID(yytext, parser->get_loc());
{QUOTED_STR} YY_REC; return yyParser::make_STRING(std::string(yytext+1,yyleng-2), parser->get_loc());

<<EOF>> return yyParser::make_END_OF_FILE(parser->get_loc());

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
    if (!static_cast<bool>(isspace(c[i])) && (c[i] != '_')) {
      s += c[i];
    }
  }

  return std::make_pair(is_signed, s);
}

int yyFlexLexer::yylex() {
  return 0;
}
