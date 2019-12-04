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
#include <string>
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
  ADD "add"
  ADDI "addi"
  AND "and"
  ANDI "andi"
  BEQ "beq"
  HALT "halt"
  J "j"
  LUI "lui"
  LW "lw"
  NOP "nop"
  NOR "nor"
  OR "or"
  ORI "ori"
  SLL "sll"
  SLT "slt"
  SLTI "slti"
  SRA "sra"
  SRAV "srav"
  SRL "srl"
  SRLV "srlv"
  SUB "sub"
  SW "sw"
  XOR "xor"
  XORI "xori"
  
  COLON ":"
  COMMA ","
  CPAREN ")"
  OPAREN "("

  END 0 "end of file"
;
%token <uint32_t> REG "reg"
%token <std::string> LABEL "label"
%token <uint32_t> IMM "number"

%printer { yyoutput << $$; } <*>;

%%

%start code;

code 
  : %empty 
  | code inst 
  ;

inst
  : LABEL COLON {
    if (!driver.bind($1)) {
      error(driver.loc, "Duplicate label definition!");
      YYERROR;
    }
  }
  | ADD REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $4, $6, $2, 0x0, 0x20);
  }
  | ADDI REG COMMA REG COMMA IMM {
    driver.emit_itype(0x8, $4, $2, $6); 
  }
  | AND REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $4, $6, $2, 0x0, 0x24);
  }
  | ANDI REG COMMA REG COMMA IMM {
    driver.emit_itype(0xc, $4, $2, $6); 
  }
  | BEQ REG COMMA REG COMMA LABEL {
    driver.emit_branch(0x4, $2, $4, $6);
  }
  | HALT {
    driver.emit_break(0x10000);
  }
  | J LABEL {
    driver.emit_jtype(0x2, $2);
  }
  | LUI REG COMMA IMM {
    driver.emit_itype(0xf, 0, $2, $4);
  } 
  | LW REG COMMA IMM OPAREN REG CPAREN {
    driver.emit_itype(0x23, $6, $2, $4);
  }
  | NOP {
    driver.emit_nop();
  }
  | NOR REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $4, $6, $2, 0x0, 0x27);
  } 
  | OR REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $4, $6, $2, 0x0, 0x25);
  }
  | ORI REG COMMA REG COMMA IMM {
    driver.emit_itype(0xd, $4, $2, $6); 
  }
  | SLL REG COMMA REG COMMA IMM {
    driver.emit_rtype(0x0, 0x0, $4, $2, $6, 0x0);
  }
  | SLT REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $4, $6, $2, 0x0, 0x2a);
  }
  | SLTI REG COMMA REG COMMA IMM {
    driver.emit_itype(0xa, $4, $2, $6); 
  }
  | SRA REG COMMA REG COMMA IMM {
    driver.emit_rtype(0x0, 0x0, $4, $2, $6, 0x3);
  }
  | SRAV REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $6, $4, $2, 0x0, 0x7);
  }
  | SRL REG COMMA REG COMMA IMM {
    driver.emit_rtype(0x0, 0x0, $4, $2, $6, 0x2);
  }
  | SRLV REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $6, $4, $2, 0x0, 0x6);
  }
  | SUB REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $4, $6, $2, 0x0, 0x22);
  }
  | SW REG COMMA IMM OPAREN REG CPAREN {
    driver.emit_itype(0x2b, $6, $2, $4);
  }
  | XOR REG COMMA REG COMMA REG {
    driver.emit_rtype(0x0, $4, $6, $2, 0x0, 0x26);
  }
  | XORI REG COMMA REG COMMA IMM {
    driver.emit_itype(0xe, $4, $2, $6); 
  }

%%

void ns::yyParser::error(const location_type& l, const std::string& m) {
  driver.set_why(l, m);
}
