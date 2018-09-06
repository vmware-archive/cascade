%{ 
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <string>
#include "mips32.tab.hh"
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

blank [ \t]
label [$a-zA-Z][$a-zA-Z_0-9]*
num   [0-9a-fA-FxX]+
lcom  #[^\n]*\n

%{
#define YY_DECL yyParser::symbol_type yyLexer::yylex(Driver& driver)
#define YY_USER_ACTION driver.loc.columns(yyleng);
%}

%%

%{
driver.loc.step();
%}

{blank}+ driver.loc.step();
[\n]+    driver.loc.lines(yyleng); driver.loc.step();
{lcom}   driver.loc.lines(yyleng); driver.loc.step();

"$0"     return yyParser::make_REG(0, driver.loc);
"$r0"    return yyParser::make_REG(0, driver.loc);
"$zero"  return yyParser::make_REG(0, driver.loc);
"$1"     return yyParser::make_REG(1, driver.loc);
"$at"    return yyParser::make_REG(1, driver.loc);
"$2"     return yyParser::make_REG(2, driver.loc);
"$v0"    return yyParser::make_REG(2, driver.loc);
"$3"     return yyParser::make_REG(3, driver.loc);
"$v1"    return yyParser::make_REG(3, driver.loc);
"$4"     return yyParser::make_REG(4, driver.loc);
"$a0"    return yyParser::make_REG(4, driver.loc);
"$5"     return yyParser::make_REG(5, driver.loc);
"$a1"    return yyParser::make_REG(5, driver.loc);
"$6"     return yyParser::make_REG(6, driver.loc);
"$a2"    return yyParser::make_REG(6, driver.loc);
"$7"     return yyParser::make_REG(7, driver.loc);
"$a3"    return yyParser::make_REG(7, driver.loc);
"$8"     return yyParser::make_REG(8, driver.loc);
"$t0"    return yyParser::make_REG(8, driver.loc);
"$9"     return yyParser::make_REG(9, driver.loc);
"$t1"    return yyParser::make_REG(9, driver.loc);
"$10"    return yyParser::make_REG(10, driver.loc);
"$t2"    return yyParser::make_REG(10, driver.loc);
"$11"    return yyParser::make_REG(11, driver.loc);
"$t3"    return yyParser::make_REG(11, driver.loc);
"$12"    return yyParser::make_REG(12, driver.loc);
"$t4"    return yyParser::make_REG(12, driver.loc);
"$13"    return yyParser::make_REG(13, driver.loc);
"$t5"    return yyParser::make_REG(13, driver.loc);
"$14"    return yyParser::make_REG(14, driver.loc);
"$t6"    return yyParser::make_REG(14, driver.loc);
"$15"    return yyParser::make_REG(15, driver.loc);
"$t7"    return yyParser::make_REG(15, driver.loc);
"$16"    return yyParser::make_REG(16, driver.loc);
"$s0"    return yyParser::make_REG(16, driver.loc);
"$17"    return yyParser::make_REG(17, driver.loc);
"$s1"    return yyParser::make_REG(17, driver.loc);
"$18"    return yyParser::make_REG(18, driver.loc);
"$s2"    return yyParser::make_REG(18, driver.loc);
"$19"    return yyParser::make_REG(19, driver.loc);
"$s3"    return yyParser::make_REG(19, driver.loc);
"$20"    return yyParser::make_REG(20, driver.loc);
"$s4"    return yyParser::make_REG(20, driver.loc);
"$21"    return yyParser::make_REG(21, driver.loc);
"$s5"    return yyParser::make_REG(21, driver.loc);
"$22"    return yyParser::make_REG(22, driver.loc);
"$s6"    return yyParser::make_REG(22, driver.loc);
"$23"    return yyParser::make_REG(23, driver.loc);
"$s7"    return yyParser::make_REG(23, driver.loc);
"$24"    return yyParser::make_REG(24, driver.loc);
"$t8"    return yyParser::make_REG(24, driver.loc);
"$25"    return yyParser::make_REG(25, driver.loc);
"$t9"    return yyParser::make_REG(25, driver.loc);
"$26"    return yyParser::make_REG(26, driver.loc);
"$k0"    return yyParser::make_REG(26, driver.loc);
"$27"    return yyParser::make_REG(27, driver.loc);
"$k1"    return yyParser::make_REG(27, driver.loc);
"$28"    return yyParser::make_REG(28, driver.loc);
"$gp"    return yyParser::make_REG(28, driver.loc);
"$29"    return yyParser::make_REG(29, driver.loc);
"$sp"    return yyParser::make_REG(29, driver.loc);
"$30"    return yyParser::make_REG(30, driver.loc);
"$fp"    return yyParser::make_REG(30, driver.loc);
"$31"    return yyParser::make_REG(31, driver.loc);
"$ra"    return yyParser::make_REG(31, driver.loc);

"add"  return yyParser::make_ADD(driver.loc);
"addi" return yyParser::make_ADDI(driver.loc);
"and"  return yyParser::make_AND(driver.loc);
"andi" return yyParser::make_ANDI(driver.loc);
"beq"  return yyParser::make_BEQ(driver.loc);
"halt" return yyParser::make_HALT(driver.loc);
"j"    return yyParser::make_J(driver.loc);
"lui"  return yyParser::make_LUI(driver.loc);
"lw"   return yyParser::make_LW(driver.loc);
"nop"  return yyParser::make_NOP(driver.loc);
"nor"  return yyParser::make_NOR(driver.loc);
"or"   return yyParser::make_OR(driver.loc);
"ori"  return yyParser::make_ORI(driver.loc);
"sll"  return yyParser::make_SLL(driver.loc);
"slt"  return yyParser::make_SLT(driver.loc);
"slti" return yyParser::make_SLTI(driver.loc);
"sra"  return yyParser::make_SRA(driver.loc);
"srav" return yyParser::make_SRAV(driver.loc);
"srl"  return yyParser::make_SRL(driver.loc);
"srlv" return yyParser::make_SRLV(driver.loc);
"sub"  return yyParser::make_SUB(driver.loc);
"sw"   return yyParser::make_SW(driver.loc);
"xor"  return yyParser::make_XOR(driver.loc);
"xori" return yyParser::make_XORI(driver.loc);

":"      return yyParser::make_COLON(driver.loc);
","      return yyParser::make_COMMA(driver.loc);
")"      return yyParser::make_CPAREN(driver.loc);
"("      return yyParser::make_OPAREN(driver.loc);

{label} return yyParser::make_LABEL(yytext, driver.loc);

{num} {
  errno = 0;
  uint32_t n = strtoul(yytext, NULL, 0);
  if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
    driver.set_why(driver.loc, "integer is out of range");
  }
  return yyParser::make_IMM(n, driver.loc);
}

.       driver.set_why(driver.loc, "invalid character");
<<EOF>> return yyParser::make_END(driver.loc);

%%

int yyFlexLexer::yylex() {
  return 0;
}
