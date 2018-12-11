%skeleton "lalr1.cc"
%require  "3.0.4"
%defines 
%define api.namespace {cascade}
%define parser_class_name {yyParser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires 
{
#include <sstream>
#include <string>
#include <tuple>
#include "src/verilog/ast/ast.h"

namespace cascade {

/* Typedefs, since Bison >3.1.2 uses a macro, and std::pair cannot be used. */
typedef std::pair<bool, std::string> SignedNumber;
typedef std::pair<size_t, NetDeclaration::Type> NetList;
typedef std::pair<Identifier*, Maybe<RangeExpression>*> ModuleIdentifier;
typedef std::pair<size_t, std::string> IdList;

class Parser;

} // namespace cascade
}

%param {Parser* parser}

%locations

%define parse.trace
%define parse.error verbose

%code 
{
#include "src/verilog/parse/parser.h"

#undef yylex
#define yylex parser->lexer_.yylex

namespace {

bool is_null(const cascade::Expression* e) {
  if (auto i = dynamic_cast<const cascade::Identifier*>(e)) {
    return i->eq("__null");
  }
  return false;
}

} // namespace 
}

/* Ctrl-D */
%token END_OF_FILE 0 

/* Operators and Tokens */
%token AAMP    "&&"
%token AMP     "&"
%token AT      "@"
%token BANG    "!"
%token BEEQ    "!=="
%token BEQ     "!="
%token CARAT   "^"
%token CCURLY  "}"
%token COLON   ":"
%token COMMA   ","
%token CPAREN  ")"
%token CSQUARE "]"
%token CTIMES  "*)"
%token DIV     "/"
%token DOT     "."
%token EEEQ    "==="
%token EEQ     "=="
%token EQ      "="
%token GEQ     ">="
%token GGGT    ">>>"
%token GGT     ">>"
%token GT      ">"
%token LEQ     "<="
%token LLLT    "<<<" 
%token LLT     "<<"
%token LT      "<"
%token MCOLON  "-:"
%token MINUS   "-"
%token MOD     "%"
%token OCURLY  "{"
%token OPAREN  "("
%token OSQUARE "["
%token OTIMES  "(*"
%token PCOLON  "+:"
%token PIPE    "|"
%token PPIPE   "||"
%token PLUS    "+"
%token POUND   "#"
%token QMARK   "?"
%token SCOLON  ";"
%token STAR    "(*)"
%token TAMP    "~&"
%token TCARAT  "~^"
%token TILDE   "~"
%token TIMES   "*"
%token TTIMES  "**"
%token TPIPE   "~|"

/* Keywords */
%token ALWAYS      "always"
%token ASSIGN      "assign"
%token BEGIN_      "begin"
%token CASE        "case"
%token CASEX       "casex"
%token CASEZ       "casez"
%token DEFAULT     "default"
%token DISABLE     "disable"
%token ELSE        "else"
%token END         "end"
%token ENDCASE     "endcase"
%token ENDGENERATE "endgenerate"
%token ENDMODULE   "endmodule"
%token FOR         "for"
%token FORK        "fork"
%token FOREVER     "forever"
%token GENERATE    "generate"
%token GENVAR      "genvar"
%token IF          "if"
%token INITIAL_    "initial"
%token INOUT       "inout"
%token INPUT       "input"
%token INTEGER     "integer"
%token JOIN        "join"
%token LOCALPARAM  "localparam"
%token MACROMODULE "marcomodule"
%token MODULE      "module"
%token NEGEDGE     "negedge"
%token OR          "or"
%token OUTPUT      "output"
%token PARAMETER   "parameter"
%token POSEDGE     "posedge"
%token REG         "reg"
%token REPEAT      "repeat"
%token SIGNED      "signed"
%token WAIT        "wait"
%token WHILE       "while"
%token WIRE        "wire"

/* System Task Identifiers */
%token SYS_DISPLAY "$display"
%token SYS_FINISH  "$finish"
%token SYS_WRITE   "$write"

/* Identifiers and Strings */
%token <std::string> INCLUDE
%token <std::string> SIMPLE_ID
%token <std::string> STRING

/* Numbers */
%token <std::string> UNSIGNED_NUM
%token <SignedNumber> DECIMAL_VALUE
%token <SignedNumber> BINARY_VALUE
%token <SignedNumber> OCTAL_VALUE
%token <SignedNumber> HEX_VALUE

/* Operator Precedence */
%right QMARK COLON
%left  PPIPE
%left  AAMP
%left  PIPE 
%left  CARAT TCARAT
%left  AMP 
%left  EEQ EEEQ BEQ BEEQ
%left  GT LT GEQ LEQ
%left  LLT LLLT GGT GGGT
%left  PLUS MINUS
%left  TIMES DIV MOD
%left  TTIMES
%right BANG TILDE 

/* List Operator Precedence */
%left  COMMA OR

/* If Else Precedence */
%right THEN ELSE

/* A.1.1 Library Source Text */
%type <String*> include_statement

/* A.1.2 Verilog Source Text */
%type <ModuleDeclaration*> module_declaration

/* A.1.3 Module Parameters and Ports */
%type <Many<ModuleItem>*> module_parameter_port_list
%type <Many<ArgAssign>*> list_of_ports
%type <Many<ModuleItem>*> list_of_port_declarations
%type <ArgAssign*> port
%type <Identifier*> port_expression
%type <Identifier*> port_reference
%type <Many<ModuleItem>*> port_declaration

/* A.1.4 Module Items */
%type <Many<ModuleItem>*> module_item
%type <Many<ModuleItem>*> module_or_generate_item
%type <Many<ModuleItem>*> module_or_generate_item_declaration
%type <Many<ModuleItem>*> non_port_module_item

/* A.2.1.1 Module Parameter Declarations */
%type <Many<Declaration>*> local_parameter_declaration
%type <Many<Declaration>*> parameter_declaration

/* A.2.1.2 Port Declarations */
%type <Many<ModuleItem>*> inout_declaration
%type <Many<ModuleItem>*> input_declaration
%type <Many<ModuleItem>*> output_declaration

/* A.2.1.3 Type Declarations */
%type <Many<ModuleItem>*> integer_declaration
%type <Many<ModuleItem>*> net_declaration
%type <Many<ModuleItem>*> reg_declaration

/* A.2.2.1 Net and Variable Types */
%type <NetDeclaration::Type> net_type
%type <VariableAssign*> variable_type

/* A.2.2.3 Delays */
%type <DelayControl*> delay3
%type <Expression*> delay_value

/* A.2.3 Declaration Lists */
%type <Many<VariableAssign>*> list_of_net_decl_assignments
%type <Many<Identifier>*> list_of_net_identifiers
%type <Many<VariableAssign>*> list_of_param_assignments
%type <Many<Identifier>*> list_of_port_identifiers
%type <Many<VariableAssign>*> list_of_variable_identifiers
%type <Many<VariableAssign>*> list_of_variable_port_identifiers

/* A.2.4 Declaration Assignments */
%type <VariableAssign*> net_decl_assignment
%type <VariableAssign*> param_assignment

/* A.2.5 Declaration Ranges */
%type <RangeExpression*> dimension
%type <RangeExpression*> range

/* A.2.8 Block Item Declarations */
%type <Many<Declaration>*> block_item_declaration
%type <Many<Identifier>*> list_of_block_variable_identifiers
%type <Identifier*> block_variable_type

/* A.4.1 Module Instantiation */
%type <Many<ModuleItem>*> module_instantiation 
%type <Many<ArgAssign>*> parameter_value_assignment
%type <Many<ArgAssign>*> list_of_parameter_assignments
%type <ArgAssign*> ordered_parameter_assignment
%type <ArgAssign*> named_parameter_assignment
%type <ModuleInstantiation*> module_instance
%type <ModuleIdentifier> name_of_module_instance
%type <Many<ArgAssign>*> list_of_port_connections
%type <ArgAssign*> ordered_port_connection
%type <ArgAssign*> named_port_connection

/* A.4.2 Generate Construct */
%type <GenerateRegion*> generate_region
%type <Many<ModuleItem>*> genvar_declaration
%type <Many<Identifier>*> list_of_genvar_identifiers
%type <LoopGenerateConstruct*> loop_generate_construct
%type <VariableAssign*> genvar_initialization
%type <Expression*> genvar_expression
%type <VariableAssign*> genvar_iteration
%type <Expression*> genvar_primary
%type <ConditionalGenerateConstruct*> conditional_generate_construct
%type <IfGenerateConstruct*> if_generate_construct
%type <CaseGenerateConstruct*> case_generate_construct
%type <CaseGenerateItem*> case_generate_item
%type <GenerateBlock*> generate_block
%type <Maybe<GenerateBlock>*> generate_block_or_null

/* A.6.1 Continuous Assignment Statements */
%type <Many<ModuleItem>*> continuous_assign
%type <Many<VariableAssign>*> list_of_net_assignments
%type <VariableAssign*> net_assignment

/* A.6.2 Procedural Blocks and Assignments */
%type <InitialConstruct*> initial_construct
%type <AlwaysConstruct*> always_construct
%type <BlockingAssign*> blocking_assignment
%type <NonblockingAssign*> nonblocking_assignment
%type <VariableAssign*> variable_assignment

/* A.6.3 Parallel and Sequential Blocks */
%type <ParBlock*> par_block
%type <SeqBlock*> seq_block

/* A.6.4 Statements */
%type <Statement*> statement 
%type <Statement*> statement_or_null

/* A.6.5 Timing Control Statements */
%type <DelayControl*> delay_control
%type <TimingControl*> delay_or_event_control
%type <EventControl*> event_control
%type <Many<Event>*> event_expression
%type <TimingControl*> procedural_timing_control
%type <TimingControlStatement*> procedural_timing_control_statement
%type <WaitStatement*> wait_statement

/* A.6.6 Conditional Statements */
%type <ConditionalStatement*> conditional_statement
/* REMOVED IN FAVOR OF BISON CONVENTION %type<ConditionalStatement*> if_else_if_statement */

/* A.6.7 Case Statements */
%type <CaseStatement*> case_statement
%type <CaseItem*> case_item

/* A.6.8 Looping Statements */
%type <LoopStatement*> loop_statement

/* A.6.9 Task Enable Statements */
%type <SystemTaskEnableStatement*> system_task_enable

/* A.8.1 Concatenations */
%type <Concatenation*> concatenation
%type <MultipleConcatenation*> multiple_concatenation

/* A.8.3 Expressions */
%type <ConditionalExpression*> conditional_expression
%type <Expression*> expression
%type <Expression*> mintypmax_expression
%type <Expression*> range_expression

/* A.8.4 Primaries */
%type <Expression*> primary

/* A.8.5 Expression Left-Side Values */
%type <Identifier*> net_lvalue 
%type <Identifier*> variable_lvalue

/* A.8.6 Operators */
%type <UnaryExpression::Op> unary_operator
/* INLINED DUE TO BISON PRECEDENCE CONVENTIONS %type <BinaryExpression::Op> binary_operator */

/* A.8.7 Numbers */
%type <Number*> number
%type <Number*> decimal_number
%type <Number*> octal_number
%type <Number*> binary_number
%type <Number*> hex_number
%type <size_t> size

/* A.8.8 Strings */
%type <String*> string_

/* A.9.1 Attributes */
%type <Many<AttrSpec>*> attribute_instance 
%type <AttrSpec*> attr_spec          
%type <Identifier*> attr_name

/* A.9.3 Identifiers */
%type <Identifier*> hierarchical_identifier
%type <Identifier*> identifier 

/* Auxiliary Rules */
%type <Many<AttrSpec>*> attr_spec_P
%type <Attributes*> attribute_instance_S 
%type <Many<Declaration>*> block_item_declaration_S
%type <Many<Expression>*> braced_rexp_S
%type <Many<CaseGenerateItem>*> case_generate_item_P
%type <Many<CaseItem>*> case_item_P
%type <Maybe<DelayControl>*> delay3_Q
%type <Maybe<TimingControl>*> delay_or_event_control_Q
%type <Many<Expression>*> dimension_S
%type <Expression*> eq_ce_Q
%type <Many<Expression>*> expression_P
%type <Maybe<Expression>*> expression_Q
%type <Maybe<Identifier>*> generate_block_id_Q
%type <size_t> integer_L
%type <Many<ModuleItem>*> list_of_port_declarations_Q
%type <size_t> localparam_L
%type <Maybe<Expression>*> mintypmax_expression_Q
%type <Many<ModuleInstantiation>*> module_instance_P
%type <Many<ModuleItem>*> module_item_S
%type <size_t> module_keyword_L
%type <Many<ModuleItem>*> module_or_generate_item_S
%type <Many<ArgAssign>*> named_parameter_assignment_P
%type <Many<ArgAssign>*> named_port_connection_P
%type <NetList> net_type_L
%type <NetDeclaration::Type> net_type_Q
%type <Many<ModuleItem>*> non_port_module_item_S
%type <Many<ArgAssign>*> ordered_parameter_assignment_P
%type <Many<ArgAssign>*> ordered_port_connection_P
%type <Many<Declaration>*> parameter_declaration_P
%type <size_t> parameter_L
%type <Many<ArgAssign>*> parameter_value_assignment_Q
%type <Many<ModuleItem>*> port_declaration_P
%type <Many<ArgAssign>*> port_P
%type <Maybe<RangeExpression>*> range_Q
%type <size_t> reg_L
%type <bool> signed_Q
%type <IdList> simple_id_L
%type <Many<Statement>*> statement_S

/* Alternate Rules */
/* These rules deviate from the Verilog Spec, due to LALR(1) parser quirks */
%type <Declaration*> alt_parameter_declaration
%type <PortDeclaration*> alt_port_declaration
%type <PortDeclaration::Type> alt_port_type
%type <bool> alt_net_type

%%

main 
  : restore include_statement { parser->res_ = $2; YYACCEPT; }
  | restore module_declaration { parser->res_ = $2; YYACCEPT; }
  | restore non_port_module_item backup { parser->res_ = $2; YYACCEPT; }
  | restore END_OF_FILE { parser->eof_ = true; parser->res_ = nullptr; YYACCEPT; }
  ;

backup : %empty {
  if (!yyla.empty()) {
    parser->backup_.move(yyla);
  }
}

restore : %empty { 
  if (!parser->backup_.empty()) {
    yyla.move(parser->backup_);
  }
}

/* A.1.1 Library Source Text */
include_statement 
  : INCLUDE { $$ = new String($1); }
  ;

/* A.1.2 Verilog Source Text */
module_declaration
  : attribute_instance_S module_keyword_L identifier module_parameter_port_list
      list_of_ports SCOLON module_item_S
      ENDMODULE {
      auto mis = $4;
      mis->concat($7);
      $$ = new ModuleDeclaration($1,$3,$5,mis);
      $$->set_source(parser->source());
      $$->set_line($2);
    }
  | attribute_instance_S module_keyword_L identifier module_parameter_port_list 
      list_of_port_declarations_Q SCOLON non_port_module_item_S
      ENDMODULE {
      auto ps = new Many<ArgAssign>();
      for (auto p : *$5) {
        auto d = dynamic_cast<PortDeclaration*>(p)->get_decl();
        auto aa = new ArgAssign(
          new Maybe<Identifier>(), 
          new Maybe<Expression>(d->get_id()->clone())
        ); 
        ps->push_back(aa); 
      }
      auto mis = $4;
      mis->concat($5);
      mis->concat($7);
      $$ = new ModuleDeclaration($1,$3,ps,mis);
      $$->set_source(parser->source());
      $$->set_line($2);
    }
  ;
module_keyword
  : MODULE 
  | MACROMODULE 
  ;

/* A.1.3 Module Parameters and Ports */
module_parameter_port_list
  : %empty { $$ = new Many<ModuleItem>(); }
  | POUND OPAREN parameter_declaration_P CPAREN { 
    $$ = new Many<ModuleItem>();
    while (!$3->empty()) {
      $$->push_back($3->remove_front());
    }
    delete $3; 
  }
  ;
list_of_ports
  : OPAREN port_P CPAREN { $$ = $2; }
  ;
list_of_port_declarations 
  : OPAREN port_declaration_P CPAREN { $$ = $2; }
  ;
port
  : %empty { $$ = new ArgAssign(new Maybe<Identifier>(), new Maybe<Expression>()); }
  | port_expression { $$ = new ArgAssign(new Maybe<Identifier>(),new Maybe<Expression>($1)); }
  | DOT identifier OPAREN CPAREN { $$ = new ArgAssign(new Maybe<Identifier>($2),new Maybe<Expression>()); }
  | DOT identifier OPAREN port_expression CPAREN { $$ = new ArgAssign(new Maybe<Identifier>($2),new Maybe<Expression>($4)); }
  ;
port_expression
  : port_reference { $$ = $1; }
  /* TODO '{' port_reference (, port_reference)* '}' */
  ;
port_reference
  : identifier /* braced_cre_Q */ { $$ = $1; }
  ;
port_declaration
  : attribute_instance_S inout_declaration { 
    for (auto pd : *$2) {
      dynamic_cast<PortDeclaration*>(pd)->replace_attrs($1->clone());
    }
    delete $1;
    $$ = $2;
  }
  | attribute_instance_S input_declaration {
    for (auto pd : *$2) {
      dynamic_cast<PortDeclaration*>(pd)->replace_attrs($1->clone());
    }
    delete $1;
    $$ = $2;
  }
  | attribute_instance_S output_declaration {
    for (auto pd : *$2) {
      dynamic_cast<PortDeclaration*>(pd)->replace_attrs($1->clone());
    }
    delete $1;
    $$ = $2;
  }
  ;

/* A.1.4 Module Items */
module_item 
  : port_declaration SCOLON { $$ = $1; }
  | non_port_module_item { $$ = $1; }
  ;
module_or_generate_item
  : /*attribute_instance_S*/ module_or_generate_item_declaration { $$ = $1; }
  | /*attribute_instance_S*/ local_parameter_declaration SCOLON { 
    $$ = new Many<ModuleItem>();
    while (!$1->empty()) {
      $$->push_back($1->remove_front());
    }; 
    delete $1;
  }
  /* TODO | attribute_instance_S parameter_override */
  | /*attribute_instance_S*/ continuous_assign { $$ = $1; }
  /* TODO | attribute_instance_S gate_instantiation */
  /* TODO | attribute_instance_S udp_instantiation */
  | attribute_instance_S module_instantiation { 
    for (auto mi : *$2) {
      dynamic_cast<ModuleInstantiation*>(mi)->replace_attrs($1->clone());
    }
    delete $1;
    $$ = $2;
  }
  | attribute_instance_S initial_construct { $2->replace_attrs($1); $$ = new Many<ModuleItem>($2); }
  | /*attribute_instance_S*/ always_construct { $$ = new Many<ModuleItem>($1); }
  | /*attribute_instance_S*/ loop_generate_construct { $$ = new Many<ModuleItem>($1); }
  | attribute_instance_S conditional_generate_construct { 
    if (auto igc = dynamic_cast<IfGenerateConstruct*>($2)) {
      igc->replace_attrs($1);
    } else {
      delete $1;
    }
    $$ = new Many<ModuleItem>($2); 
  }
  ;
module_or_generate_item_declaration
  : net_declaration { $$ = $1; }
  | reg_declaration { $$ = $1; }
  | integer_declaration { $$ = $1; }
  /* TODO | real_declaration */
  /* TODO | time_declaration */
  /* TODO | realtime_declaration */
  /* TODO | event_declaration  */
  | genvar_declaration { $$ = $1; }
  /* TODO | task_declaration */
  /* TODO | function_declaration */
  ;
non_port_module_item
  : module_or_generate_item { $$ = $1; }
  | generate_region { $$ = new Many<ModuleItem>($1); }
  /* TODO | specify_block */
  | /*attribute_instance_S*/ parameter_declaration SCOLON { 
    $$ = new Many<ModuleItem>();
    while (!$1->empty()) {
      $$->push_back($1->remove_front());
    }
    delete $1; 
  }
  /* TODO | attribute_instance_S specparam_declaration SCOLON */
  ;

/* A.2.1.1 Module Parameter Declarations */
local_parameter_declaration
  : attribute_instance_S localparam_L signed_Q range_Q list_of_param_assignments {
    $$ = new Many<Declaration>();
    while (!$5->empty()) {
      auto va = $5->remove_front();
      auto lpd = new LocalparamDeclaration($1->clone(), $3, $4->clone(), va->get_lhs()->clone(), va->get_rhs()->clone());
      lpd->set_source(parser->source());
      lpd->set_line($2);
      $$->push_back(lpd);
      delete va;
    }
    delete $1;
    delete $4;
    delete $5;
  }
  | attribute_instance_S localparam_L parameter_type list_of_param_assignments {
    $$ = new Many<Declaration>();
    while ($4->empty()) {
      auto va = $4->remove_front();
      auto lpd = new LocalparamDeclaration($1->clone(), false, new Maybe<RangeExpression>(), va->get_lhs()->clone(), va->get_rhs()->clone());
      lpd->set_source(parser->source());
      lpd->set_line($2);
      $$->push_back(lpd);
      delete va;
    }
    delete $1;
    delete $4;
  }
  ;
parameter_declaration
  : attribute_instance_S parameter_L signed_Q range_Q list_of_param_assignments {
    $$ = new Many<Declaration>();
    while (!$5->empty()) {
      auto va = $5->remove_front();
      auto pd = new ParameterDeclaration($1->clone(), $3, $4->clone(), va->get_lhs()->clone(), va->get_rhs()->clone());
      pd->set_source(parser->source());
      pd->set_line($2);
      $$->push_back(pd);
      delete va;
    }
    delete $1;
    delete $4;
    delete $5;
  }
  | attribute_instance_S parameter_L parameter_type list_of_param_assignments {
    $$ = new Many<Declaration>();
    while ($4->empty()) {
      auto va = $4->remove_front();
      auto pd = new ParameterDeclaration($1->clone(), false, new Maybe<RangeExpression>(), va->get_lhs()->clone(), va->get_rhs()->clone());
      pd->set_source(parser->source());
      pd->set_line($2);
      $$->push_back(pd);
      delete va;
    }
    delete $1;
    delete $4;
  }
  ;
parameter_type
  : INTEGER 
  ;
/* A.2.1.2 Port Declarations */
inout_declaration
  : INOUT net_type_Q signed_Q range_Q list_of_port_identifiers {
    $$ = new Many<ModuleItem>();
    while (!$5->empty()) {
      auto t = PortDeclaration::INOUT;
      auto d = new NetDeclaration(new Attributes(new Many<AttrSpec>()), $2, new Maybe<DelayControl>(), $5->remove_front(), $3, $4->clone());
      $$->push_back(new PortDeclaration(new Attributes(new Many<AttrSpec>()), t,d));
    }
    delete $4;
    delete $5;
  }
  ;
input_declaration
  : INPUT net_type_Q signed_Q range_Q list_of_port_identifiers {
    $$ = new Many<ModuleItem>();
    while (!$5->empty()) {
      auto t = PortDeclaration::INPUT;
      auto d = new NetDeclaration(new Attributes(new Many<AttrSpec>()), $2,new Maybe<DelayControl>(), $5->remove_front(), $3, $4->clone());
      $$->push_back(new PortDeclaration(new Attributes(new Many<AttrSpec>()), t,d));
    }
    delete $4;
    delete $5;
  }
  ;
output_declaration
  : OUTPUT net_type_Q signed_Q range_Q list_of_port_identifiers {
    $$ = new Many<ModuleItem>();
    while (!$5->empty()) {
      auto t = PortDeclaration::OUTPUT;
      auto d = new NetDeclaration(new Attributes(new Many<AttrSpec>()), $2,new Maybe<DelayControl>(), $5->remove_front(), $3, $4->clone());
      $$->push_back(new PortDeclaration(new Attributes(new Many<AttrSpec>()), t,d));
    }
    delete $4;
    delete $5;
  }
  | OUTPUT REG signed_Q range_Q list_of_variable_port_identifiers {
    $$ = new Many<ModuleItem>();
    while (!$5->empty()) {
      auto va = $5->remove_front();
      auto t = PortDeclaration::OUTPUT;
      auto d = new RegDeclaration(new Attributes(new Many<AttrSpec>()), va->get_lhs()->clone(), $3, $4->clone(), !is_null(va->get_rhs()) ? new Maybe<Expression>(va->get_rhs()->clone()) : new Maybe<Expression>());
      $$->push_back(new PortDeclaration(new Attributes(new Many<AttrSpec>()), t,d));
      delete va;
    }
    delete $4;
    delete $5;
  }
  /* TODO | OUTPUT output_variable_type list_of_variable_port_identifiers */
  ;

/* A.2.1.3 Type Declarations */
integer_declaration
  : attribute_instance_S integer_L list_of_variable_identifiers SCOLON {
    $$ = new Many<ModuleItem>();
    while (!$3->empty()) {
      auto va = $3->remove_front();
      auto id = new IntegerDeclaration($1->clone(), va->get_lhs()->clone(), !is_null(va->get_rhs()) ? new Maybe<Expression>(va->get_rhs()->clone()) : new Maybe<Expression>());
      id->set_source(parser->source());
      id->set_line($2);
      $$->push_back(id);
      delete va;
    }
    delete $1;
    delete $3;
  }
net_declaration 
  /** TODO: Combining cases with below due to lack of support for vectored|scalared */
  : attribute_instance_S net_type_L /* [vectored|scalared] */ signed_Q range_Q delay3_Q list_of_net_identifiers SCOLON {
    $$ = new Many<ModuleItem>();
    while (!$6->empty()) {
      auto nd = new NetDeclaration($1->clone(), $2.second, $5->clone(), $6->remove_front(), $3, $4->clone());
      nd->set_source(parser->source());
      nd->set_line($2.first);
      $$->push_back(nd);
    }
    delete $1;
    delete $4;
    delete $5;
    delete $6;
  }
  /** TODO: Combining cases with below due to lack of support for vectored|scalared */
  | attribute_instance_S net_type_L /* drive_strength [vectored|scalared] */ signed_Q range_Q delay3_Q list_of_net_decl_assignments SCOLON {
    $$ = new Many<ModuleItem>();
    while (!$6->empty()) {
      auto va = $6->remove_front();
      auto nd = new NetDeclaration($1->clone(), $2.second, $5->clone(), va->get_lhs()->clone(), $3, $4->clone());
      nd->set_source(parser->source());
      nd->set_line($2.first);
      $$->push_back(nd);

      auto ca = new ContinuousAssign(new Maybe<DelayControl>(), va);
      $$->push_back(ca);
    }
    delete $1;
    delete $4;
    delete $5;
    delete $6;
  }
  /* TODO | ... lots of cases */
  ;
reg_declaration
  : attribute_instance_S reg_L signed_Q range_Q list_of_variable_identifiers SCOLON {
    $$ = new Many<ModuleItem>();
    while (!$5->empty()) {
      auto va = $5->remove_front();
      auto rd = new RegDeclaration($1->clone(), va->get_lhs()->clone(), $3, $4->clone(), !is_null(va->get_rhs()) ? new Maybe<Expression>(va->get_rhs()->clone()) : new Maybe<Expression>());
      rd->set_source(parser->source());
      rd->set_line($2);
      $$->push_back(rd);
      delete va;
    }
    delete $1;
    delete $4;
    delete $5;
  }
  ;

/* A.2.2.1 Net and Variable Types */
net_type
  /* TODO : SUPPLY0 */
  /* TODO | SUPPLY1 */
  /* TODO | TRI */
  /* TODO | TRIAND */
  /* TODO | TRIOR */
  /* TODO | TRI0 */ 
  /* TODO | TRI1 */
  /* TODO | UWIRE */
  : WIRE { $$ = NetDeclaration::WIRE; }
  /* TODO | WAND */
  /* TODO | WOR */
  ;
variable_type
  : identifier dimension_S { 
    $$ = new VariableAssign($1, new Identifier("__null")); 
    $$->set_source(parser->source());
    $$->set_line($1->get_line());
    $$->get_lhs()->replace_dim($2); 
  }
  | identifier EQ expression { 
    $$ = new VariableAssign($1, $3); 
    $$->set_source(parser->source());
    $$->set_line($1->get_line());
  }
  ;

/* A.2.2.3 Delays */
delay3 
  : POUND delay_value { $$ = new DelayControl($2); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line);}
  /* TODO | # (mintypmax_expression (, mintypmax_expression, mintypmax_expression?)?) */
  ;
delay_value
  : UNSIGNED_NUM { $$ = new Number($1, Number::UNBASED, 32, false); }
  /* TODO | real_number */
  /* TODO | identifier */
  ;

/* A.2.3 Declaration Lists */
list_of_net_decl_assignments
  : net_decl_assignment { $$ = new Many<VariableAssign>($1); }
  | list_of_net_decl_assignments COMMA net_decl_assignment {
    $$ = $1;
    $$->push_back($3);
  }
  ;
list_of_net_identifiers
  : identifier dimension_S { 
    $$ = new Many<Identifier>($1); 
    $$->back()->replace_dim($2);
  }
  | list_of_net_identifiers COMMA identifier dimension_S {
    $$ = $1;
    $$->push_back($3);
    $$->back()->replace_dim($4);
  }
  ;
list_of_param_assignments 
  : param_assignment { $$ = new Many<VariableAssign>($1); }
  | list_of_param_assignments COMMA param_assignment {
    $$ = $1;
    $$->push_back($3);
  }
  ;
list_of_port_identifiers
  : identifier { $$ = new Many<Identifier>($1); }
  | list_of_port_identifiers COMMA identifier {
    $$ = $1;
    $$->push_back($3);
  }
  ;
list_of_variable_identifiers
  : variable_type { $$ = new Many<VariableAssign>($1); }
  | list_of_variable_identifiers COMMA variable_type {
    $$ = $1;
    $$->push_back($3);
  }
  ;
list_of_variable_port_identifiers
  : identifier eq_ce_Q { $$ = new Many<VariableAssign>(new VariableAssign($1,$2)); }
  | list_of_variable_port_identifiers COMMA identifier eq_ce_Q {
    $$ = $1;
    $$->push_back(new VariableAssign($3,$4));
  }

/* A.2.4 Declaration Assignments */
net_decl_assignment
  : identifier EQ expression { $$ = new VariableAssign($1,$3); $$->set_source(parser->source()); $$->set_line($1->get_line()); }
  ;
param_assignment
  : identifier EQ mintypmax_expression { $$ = new VariableAssign($1, $3); }

/* A.2.5 Declaration Ranges */
dimension
  : OSQUARE expression COLON expression CSQUARE { 
    $$ = new RangeExpression($2, RangeExpression::CONSTANT, $4);
  }
  ;
range
  : OSQUARE expression COLON expression CSQUARE { 
    $$ = new RangeExpression($2, RangeExpression::CONSTANT, $4);
  }
  ;

/* A.2.8 Block Item Declarations */
block_item_declaration
  : attribute_instance_S REG signed_Q range_Q list_of_block_variable_identifiers SCOLON { 
    $$ = new Many<Declaration>();
    while (!$5->empty()) {
      $$->push_back(new RegDeclaration($1->clone(), $5->remove_front(), $3, $4->clone(), new Maybe<Expression>()));
    }
    delete $1;
    delete $4;
    delete $5;
  }
  | attribute_instance_S INTEGER list_of_block_variable_identifiers SCOLON { 
    $$ = new Many<Declaration>();
    while (!$3->empty()) {
      $$->push_back(new IntegerDeclaration($1->clone(),$3->remove_front(),new Maybe<Expression>()));
    }
    delete $1;
    delete $3;
  }
  /* TODO | attribute_instance_S TIME list_of_block_variable_identifiers SCOLON { } */
  /* TODO | attribute_instance_S REAL list_of_block_variable_identifiers SCOLON { } */
  /* TODO | attribute_instance_S REALTIME list_of_block_variable_identifiers SCOLON { } */
  /* TODO | attribute_instance_S event_declaration { } */
  | /*attribute_instance_S*/ local_parameter_declaration SCOLON { $$ = $1; }
  | /*attribute_instance_S*/ parameter_declaration SCOLON { $$ = $1; }
  ;
list_of_block_variable_identifiers 
  : block_variable_type { $$ = new Many<Identifier>($1); } 
  | list_of_block_variable_identifiers COMMA block_variable_type { 
    $$ = $1;
    $$->push_back($3);
  }
  ;
block_variable_type 
  : identifier dimension_S { 
    $$ = $1; 
    $$->replace_dim($2);
  }
  ;

/* A.4.1 Module Instantiation */
module_instantiation 
  : identifier parameter_value_assignment_Q module_instance_P SCOLON {
    $$ = new Many<ModuleItem>();
    while (!$3->empty()) {
      auto mi = $3->remove_front();
      mi->replace_mid($1->clone());
      mi->replace_params($2->clone());
      mi->set_source($1->get_source());
      mi->set_line($1->get_line());
      $$->push_back(mi);
    }
    delete $1;
    delete $2;
    delete $3;
  }
  ;
parameter_value_assignment
  : POUND OPAREN list_of_parameter_assignments CPAREN { $$ = $3; }
  ;
list_of_parameter_assignments
  : ordered_parameter_assignment_P { $$ = $1; }
  | named_parameter_assignment_P { $$ = $1; }
  ;
ordered_parameter_assignment
  : expression { $$ = new ArgAssign(new Maybe<Identifier>(), new Maybe<Expression>($1)); }
  ;
named_parameter_assignment
  : DOT identifier OPAREN mintypmax_expression_Q CPAREN { 
    $$ = new ArgAssign(new Maybe<Identifier>($2), $4);
  } 
  ; 
module_instance
  : name_of_module_instance OPAREN list_of_port_connections CPAREN { 
    $$ = new ModuleInstantiation(new Attributes(new Many<AttrSpec>()), new Identifier(""), $1.first, $1.second, new Many<ArgAssign>(), $3);
  }
  ;
name_of_module_instance
  : identifier range_Q { $$ = std::make_pair($1, $2); }
  ;
list_of_port_connections
  : ordered_port_connection_P { $$ = $1; }
  | named_port_connection_P { $$ = $1; }
  ;
ordered_port_connection
  : /*attribute_instance_S*/ expression_Q { 
    $$ = new ArgAssign(new Maybe<Identifier>(),$1); 
  }
  ;
named_port_connection
  : /*attribute_instance_S*/ DOT identifier OPAREN expression_Q CPAREN {
    $$ = new ArgAssign(new Maybe<Identifier>($2),$4);
  }
  ;

/* A.4.2 Generate Construct */
generate_region
  : GENERATE module_or_generate_item_S ENDGENERATE { $$ = new GenerateRegion($2); }
  ;
genvar_declaration
  : attribute_instance_S GENVAR list_of_genvar_identifiers SCOLON { 
    $$ = new Many<ModuleItem>();
    while (!$3->empty()) {
      $$->push_back(new GenvarDeclaration($1->clone(), $3->remove_front()));
    }
    delete $1;
    delete $3;
  }
  ;
list_of_genvar_identifiers
  : identifier { $$ = new Many<Identifier>($1); }
  | list_of_genvar_identifiers COMMA identifier { 
    $$ = $1;
    $$->push_back($3);
  }
  ;
loop_generate_construct
  : FOR OPAREN genvar_initialization SCOLON genvar_expression SCOLON genvar_iteration CPAREN generate_block { 
    $$ = new LoopGenerateConstruct($3,$5,$7,$9);
  }
  ;
genvar_initialization
  : identifier EQ expression { $$ = new VariableAssign($1,$3); }
  ;
genvar_expression
  : genvar_primary { 
    $$ = $1; 
  }
  | unary_operator /*attribute_instance_S*/ genvar_primary { 
    $$ = new UnaryExpression($1,$2); $$->set_source($2->get_source()); $$->set_line($2->get_line());
  }
  | genvar_expression AAMP /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::AAMP, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression AMP /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::AMP, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression BEEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::BEEQ, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression BEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::BEQ, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression CARAT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::CARAT, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression DIV /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::DIV, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression EEEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::EEEQ, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression EEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::EEQ, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression GEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::GEQ, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression GGGT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::GGGT, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression GGT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::GGT, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression GT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::GT, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression LEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::LEQ, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression LLLT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::LLLT, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression LLT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::LLT, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression LT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::LT, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression MINUS /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::MINUS, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression MOD /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::MOD, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression PIPE /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::PIPE, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression PPIPE /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::PPIPE, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression PLUS /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::PLUS, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression TCARAT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::TCARAT, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression TIMES /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::TIMES, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression TTIMES /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::TTIMES, $3); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  | genvar_expression QMARK /*attribute_instance_S*/ genvar_expression COLON genvar_expression { 
    $$ = new ConditionalExpression($1,$3,$5); $$->set_source($1->get_source()); $$->set_line($1->get_line());
  }
  ;
genvar_iteration
  : identifier EQ genvar_expression { $$ = new VariableAssign($1,$3); }
  ;
genvar_primary
  : primary { $$ = $1; }
  /* NOTE: This is redundant and introduces a reduce/reduce error | identifier { $$ = $1; } */
  ;
conditional_generate_construct
  : if_generate_construct { $$ = $1; }
  | case_generate_construct { $$ = $1; }
  ;
if_generate_construct
  : IF OPAREN expression CPAREN generate_block_or_null %prec THEN { 
    $$ = new IfGenerateConstruct(new Attributes(new Many<AttrSpec>()), new IfGenerateClause($3, $5), new Maybe<GenerateBlock>());
  }
  | IF OPAREN expression CPAREN generate_block_or_null ELSE generate_block_or_null { 
    $$ = new IfGenerateConstruct(new Attributes(new Many<AttrSpec>()), new IfGenerateClause($3, $5), new Maybe<GenerateBlock>());
    // Was the remainder of this parse an empty block?
    if ($7->null()) {
      // Nothing to do.
    }
    // Was it an unscoped if/then?
    else if (!$7->get()->get_scope() && ($7->get()->get_items()->size() == 1) && dynamic_cast<IfGenerateConstruct*>($7->get()->get_items()->front())) {
      auto igc = dynamic_cast<IfGenerateConstruct*>($7->get()->get_items()->remove_front());
      delete $7;
      while (!igc->get_clauses()->empty()) {
        $$->get_clauses()->push_back(igc->get_clauses()->remove_front());
      }
      $$->swap_else(igc->get_else());
      delete igc;
    }
    // Everything else is just an else block
    else {
      $$->replace_else($7);
    }
  }
  ;
case_generate_construct
  : CASE OPAREN expression CPAREN case_generate_item_P ENDCASE { 
    auto has_default = false;
    for (auto cgi : *$5) {
      if (cgi->get_exprs()->empty()) {
        has_default = true;
        break;
      }
    }
    if (!has_default) {
      $5->push_back(new CaseGenerateItem(new Many<Expression>(), new Maybe<GenerateBlock>()));
    }
    $$ = new CaseGenerateConstruct($3,$5);
  }
  ;
case_generate_item
  : expression_P COLON generate_block_or_null { 
    $$ = new CaseGenerateItem($1,$3);
  }
  | DEFAULT colon_Q generate_block_or_null { 
    $$ = new CaseGenerateItem(new Many<Expression>(),$3);
  }
  ; 
generate_block
  : module_or_generate_item { 
    $$ = new GenerateBlock(new Maybe<Identifier>(), false, $1); 
  }
  | BEGIN_ generate_block_id_Q module_or_generate_item_S END { 
    $$ = new GenerateBlock($2, true, $3);   
  }
  ;
generate_block_or_null
  : generate_block { $$ = new Maybe<GenerateBlock>($1); }
  | SCOLON { $$ = new Maybe<GenerateBlock>(); }
  ;

/* A.6.1 Continuous Assignment Statements */
continuous_assign
  : ASSIGN /* TODO drive_strength? */ delay3_Q list_of_net_assignments SCOLON {
    $$ = new Many<ModuleItem>();
    while (!$3->empty()) {
      auto ca = new ContinuousAssign($2->clone(), $3->remove_front());
      ca->set_source(parser->source());
      ca->set_line(ca->get_assign()->get_line());
      $$->push_back(ca);
    }
    delete $2;
    delete $3;
  }
  ;
list_of_net_assignments
  : net_assignment { $$ = new Many<VariableAssign>($1); }
  | list_of_net_assignments COMMA net_assignment {
    $$ = $1;
    $$->push_back($3);
  }
  ;
net_assignment
  : net_lvalue EQ expression { 
    $$ = new VariableAssign($1,$3); 
    $$->set_source(parser->source());
    $$->set_line($1->get_line());
  }
  ;

/* A.6.2 Procedural Blocks and Assignments */
initial_construct
  : INITIAL_ statement { $$ = new InitialConstruct(new Attributes(new Many<AttrSpec>()), $2); }
  ;
always_construct
  : ALWAYS statement { $$ = new AlwaysConstruct($2); }
  ;
blocking_assignment
  : variable_lvalue EQ delay_or_event_control_Q expression {
    $$ = new BlockingAssign($3,new VariableAssign($1,$4));
    $$->set_source(parser->source());
    $$->set_line($1->get_line());
  }
  ;
nonblocking_assignment
  : variable_lvalue LEQ delay_or_event_control_Q expression {
    $$ = new NonblockingAssign($3,new VariableAssign($1,$4));
    $$->set_source(parser->source());
    $$->set_line($1->get_line());
  }
  ;
variable_assignment
  : variable_lvalue EQ expression { $$ = new VariableAssign($1,$3); $$->set_source(parser->source()); $$->set_line($1->get_line()); }
  ;

/* A.6.3 Parallel and Sequential Blocks */
par_block 
  : FORK statement_S JOIN {
    $$ = new ParBlock(new Maybe<Identifier>(), new Many<Declaration>(), $2);
  }
  | FORK COLON identifier block_item_declaration_S statement_S JOIN {
    $$ = new ParBlock(new Maybe<Identifier>($3), $4, $5); 
  }
  ;
seq_block 
  : BEGIN_ statement_S END {
    $$ = new SeqBlock(new Maybe<Identifier>(), new Many<Declaration>(), $2);
  }
  | BEGIN_ COLON identifier block_item_declaration_S statement_S END {
    $$ = new SeqBlock(new Maybe<Identifier>($3), $4, $5); 
  }
  ;

/* A.6.4 Statements */
statement 
  : /*attribute_instance_S*/ blocking_assignment SCOLON { $$ = $1; }
  | /*attribute_instance_S*/ case_statement { $$ = $1; }
  | /*attribute_instance_S*/ conditional_statement { $$ = $1; }
  /* TODO | attribute_instance_S disable_statement */
  /* TODO | attribute_instance_S event_trigger  */
  | /*attribute_instance_S*/ loop_statement { $$ = $1; }
  | /*attribute_instance_S*/ nonblocking_assignment SCOLON { $$ = $1; }
  | /*attribute_instance_S*/ par_block { $$ = $1; }
  /* TODO | attribute_instance_S procedural_continuous_assignments SCOLON  */
  | /*attribute_instance_S*/ procedural_timing_control_statement { $$ = $1; }
  | /*attribute_instance_S*/ seq_block { $$ = $1; }
  | /*attribute_instance_S*/ system_task_enable { $$ = $1; }
  /* TODO | attribute_instance_S task_enable */
  | /*attribute_instance_S*/ wait_statement { $$ = $1; }
  ;
statement_or_null
  : statement { $$ = $1; }
  | /*attribute_instance_S*/ SCOLON { 
    $$ = new SeqBlock(new Maybe<Identifier>(), new Many<Declaration>(), new Many<Statement>()); 
  }
  ;

/* A.6.5 Timing Control Statements */
delay_control
  : POUND delay_value { $$ = new DelayControl($2); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  | POUND OPAREN mintypmax_expression CPAREN { $$ = new DelayControl($3); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  ;
delay_or_event_control 
  : delay_control { $$ = $1; }
  | event_control { $$ = $1; }
  /* TODO | repeat (expression) event_control */
  ;
event_control
  /* TODO : AT hierarchical_event_identifier */ 
  : AT OPAREN event_expression CPAREN { $$ = new EventControl($3); }
  | AT TIMES { $$ = new EventControl(new Many<Event>()); }
  | AT STAR { $$ = new EventControl(new Many<Event>()); }
  ;
event_expression
  : expression {
    $$ = new Many<Event>(new Event(Event::EDGE, $1));
  }
  | POSEDGE expression {
    $$ = new Many<Event>(new Event(Event::POSEDGE, $2));
  }
  | NEGEDGE expression {
    $$ = new Many<Event>(new Event(Event::NEGEDGE, $2));
  }
  | event_expression OR event_expression {
    $$ = $1;
    $$->concat($3);
  }
  | event_expression COMMA event_expression {
    $$ = $1;
    $$->concat($3);
  }
  ;
procedural_timing_control
  : delay_control { $$ = $1; }
  | event_control { $$ = $1; }
  ;
procedural_timing_control_statement
  : procedural_timing_control statement_or_null { $$ = new TimingControlStatement($1,$2); }
  ;
wait_statement
  : WAIT OPAREN expression CPAREN statement_or_null { 
    $$ = new WaitStatement($3,$5); 
    $$->set_source(parser->source());
    $$->set_line($3->get_line());
  }
  ;

/* A.6.6 Conditional Statements */
conditional_statement
  : IF OPAREN expression CPAREN statement_or_null %prec THEN {
    $$ = new ConditionalStatement($3,$5,new SeqBlock(new Maybe<Identifier>(), new Many<Declaration>(), new Many<Statement>()));
  }
  | IF OPAREN expression CPAREN statement_or_null ELSE statement_or_null {
    $$ = new ConditionalStatement($3,$5,$7);
  }
  ; 

/* A.6.7 Case Statements */
case_statement
  : CASE OPAREN expression CPAREN case_item_P ENDCASE {
    $$ = new CaseStatement(CaseStatement::CASE,$3,$5);
  }
  | CASEZ OPAREN expression CPAREN case_item_P ENDCASE {
    $$ = new CaseStatement(CaseStatement::CASEZ,$3,$5);
  }
  | CASEX OPAREN expression CPAREN case_item_P ENDCASE {
    $$ = new CaseStatement(CaseStatement::CASEX,$3,$5);
  }
  ;
case_item
  : expression_P COLON statement_or_null { $$ = new CaseItem($1,$3); }
  | DEFAULT colon_Q statement_or_null { $$ = new CaseItem(new Many<Expression>(),$3); }

/* A.6.8 Looping Statements */
loop_statement
  : FOREVER statement { 
    $$ = new ForeverStatement($2); 
    $$->set_source(parser->source()); 
    $$->set_line($2->get_line());
  }
  | REPEAT OPAREN expression CPAREN statement { 
    $$ = new RepeatStatement($3,$5); 
    $$->set_source(parser->source()); 
    $$->set_line($3->get_line());
  }
  | WHILE OPAREN expression CPAREN statement { 
    $$ = new WhileStatement($3,$5); 
    $$->set_source(parser->source()); 
    $$->set_line($3->get_line());
  }
  | FOR OPAREN variable_assignment SCOLON expression SCOLON variable_assignment CPAREN statement {
    $$ = new ForStatement($3,$5,$7,$9); 
    $$->set_source(parser->source()); 
    $$->set_line($3->get_line());
  }
  ;

/* A.6.9 Task Enable Statements */
system_task_enable
  : SYS_DISPLAY SCOLON { $$ = new DisplayStatement(new Many<Expression>()); }
  | SYS_DISPLAY OPAREN CPAREN SCOLON { $$ = new DisplayStatement(new Many<Expression>()); }
  | SYS_DISPLAY OPAREN expression_P CPAREN SCOLON { $$ = new DisplayStatement($3); }
  | SYS_FINISH SCOLON { $$ = new FinishStatement(new Number(Bits(false), Number::UNBASED)); }
  | SYS_FINISH OPAREN expression CPAREN SCOLON { $$ = new FinishStatement($3); }
  | SYS_WRITE SCOLON { $$ = new WriteStatement(new Many<Expression>()); }
  | SYS_WRITE OPAREN CPAREN SCOLON { $$ = new WriteStatement(new Many<Expression>()); }
  | SYS_WRITE OPAREN expression_P CPAREN SCOLON { $$ = new WriteStatement($3); }
  ;

/* A.8.1 Concatenations */
concatenation 
  : OCURLY expression_P CCURLY { $$ = new Concatenation($2); }
  ;
multiple_concatenation
  : OCURLY expression concatenation CCURLY { $$ = new MultipleConcatenation($2, $3); }
  ;

/* A.8.3 Expressions */
conditional_expression
  : expression QMARK /*attribute_instance_S*/ expression COLON expression {
    $$ = new ConditionalExpression($1,$3,$5);
  }
  ;
expression
  : primary { 
    $$ = $1; 
  }
  | unary_operator /*attribute_instance_S*/ primary { 
    $$ = new UnaryExpression($1,$2); 
  }
  | expression AAMP  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::AAMP, $3); 
  }
  | expression AMP  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::AMP, $3); 
  }
  | expression BEEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::BEEQ, $3); 
  }
  | expression BEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::BEQ, $3); 
  }
  | expression CARAT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::CARAT, $3); 
  }
  | expression DIV  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::DIV, $3); 
  }
  | expression EEEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::EEEQ, $3); 
  }
  | expression EEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::EEQ, $3); 
  }
  | expression GEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::GEQ, $3); 
  }
  | expression GGGT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::GGGT, $3); 
  }
  | expression GGT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::GGT, $3); 
  }
  | expression GT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::GT, $3); 
  }
  | expression LEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::LEQ, $3); 
  }
  | expression LLLT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::LLLT, $3); 
  }
  | expression LLT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::LLT, $3); 
  }
  | expression LT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::LT, $3); 
  }
  | expression MINUS  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::MINUS, $3); 
  }
  | expression MOD  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::MOD, $3); 
  }
  | expression PIPE  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::PIPE, $3); 
  }
  | expression PPIPE  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::PPIPE, $3); 
  }
  | expression PLUS  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::PLUS, $3); 
  }
  | expression TCARAT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::TCARAT, $3); 
  }
  | expression TIMES  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::TIMES, $3); 
  }
  | expression TTIMES  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::TTIMES, $3); 
  }
  | conditional_expression { $$ = $1; }
  ;
mintypmax_expression
  : expression { $$ = $1; }
  /* TODO | expression COLON expression COLON expression */
  ;
range_expression
  : expression { $$ = $1; }
  | expression COLON expression { $$ = new RangeExpression($1, RangeExpression::CONSTANT, $3); }
  | expression PCOLON expression { $$ = new RangeExpression($1, RangeExpression::PLUS, $3); }
  | expression MCOLON expression { $$ = new RangeExpression($1, RangeExpression::MINUS, $3); } 
  ;
          
/* A.8.4 Primaries */
primary
  : number { $$ = $1; }
  | hierarchical_identifier /* [exp]* [rexp]? inlined */ { $$ = $1; }
  | concatenation { $$ = $1; }
  | multiple_concatenation { $$ = $1; }
  /* TODO | function_call */
  /* TODO | system_function_call */
  | OPAREN mintypmax_expression CPAREN { $$ = new NestedExpression($2); }
  | string_ { $$ = $1; $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  ;

/* A.8.5 Expression Left-Side Values */
net_lvalue 
  : hierarchical_identifier /* [exp]* [rexp]? inlined */ { $$ = $1; }
  /* | '{' net_lvalue_P '}' */
  ;
variable_lvalue
  : hierarchical_identifier /* [exp]* [rexp]? inlined */ { $$ = $1; }
  /* TODO | '{' variable_lvalue_P '}' */
  ;

/* A.8.6 Operators */
unary_operator
  : PLUS   { $$ = UnaryExpression::PLUS; }
  | MINUS  { $$ = UnaryExpression::MINUS; }
  | BANG   { $$ = UnaryExpression::BANG; }
  | TILDE  { $$ = UnaryExpression::TILDE; }
  | AMP    { $$ = UnaryExpression::AMP; }
  | TAMP   { $$ = UnaryExpression::TAMP; }
  | PIPE   { $$ = UnaryExpression::PIPE; }
  | TPIPE  { $$ = UnaryExpression::TPIPE; }
  | CARAT  { $$ = UnaryExpression::CARAT; }
  | TCARAT { $$ = UnaryExpression::TCARAT; }
  ;
/* INLINED binary_operator */

/* A.8.7 Numbers */
number
  : decimal_number { $$ = $1; }
  | octal_number { $$ = $1; }
  | binary_number { $$ = $1; }
  | hex_number { $$ = $1; }
  /* TODO | real_number */
  ;
decimal_number
  : UNSIGNED_NUM { $$ = new Number($1, Number::UNBASED, 32, true); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  | DECIMAL_VALUE { $$ = new Number($1.second, Number::DEC, 32, $1.first); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  | size DECIMAL_VALUE { $$ = new Number($2.second, Number::DEC, $1, $2.first); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  /* TODO | [size] decimal_base x_digit _* */
  /* TODO | [size] decimal_base z_digit _* */
  ;
binary_number 
  : BINARY_VALUE { $$ = new Number($1.second, Number::BIN, 32, $1.first); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  | size BINARY_VALUE { $$ = new Number($2.second, Number::BIN, $1, $2.first); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  ;
octal_number 
  : OCTAL_VALUE { $$ = new Number($1.second, Number::OCT, 32, $1.first); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  | size OCTAL_VALUE { $$ = new Number($2.second, Number::OCT, $1, $2.first); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  ;
hex_number 
  : HEX_VALUE { $$ = new Number($1.second, Number::HEX, 32, $1.first); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  | size HEX_VALUE { $$ = new Number($2.second, Number::HEX, $1, $2.first); $$->set_source(parser->source()); $$->set_line(parser->loc().begin.line); }
  ;
size
  : UNSIGNED_NUM { $$ = atoll($1.c_str()); }
  ;
/* ... See Lexer */

/* A.8.8 Strings */
string_
  : STRING { $$ = new String($1); }

/* A.9.1 Attributes */
attribute_instance 
  : OTIMES attr_spec_P CTIMES { $$ = $2; }
  ;
attr_spec          
  : attr_name { $$ = new AttrSpec($1,new Maybe<Expression>()); }
  | attr_name EQ expression { $$ = new AttrSpec($1,new Maybe<Expression>($3)); }
  ;
attr_name
  : identifier { $$ = $1; }
  ;

/* A.9.3 Identifiers */
/* NOTE: This parse deviates from the spec. This is only okay given the */
/* contexts in which we use it: primary, net_lvalue, and variable_lvalue */
hierarchical_identifier
  : simple_id_L braced_rexp_S { 
    const auto id = new Id($1.second, new Maybe<Expression>());
    $$ = new Identifier(new Many<Id>(id), $2); 
    $$->set_source(parser->source());
    $$->set_line($1.first);
    for (auto e : *$2) {
      if ((e != $2->back()) && (dynamic_cast<const RangeExpression*>(e) != nullptr)) {
        error(parser->loc(), "Unexpected range expression in array subscript");
        YYERROR;
      }
    }
  }
  | hierarchical_identifier DOT SIMPLE_ID braced_rexp_S {
    $$ = $1;
    if ($$->get_dim()->size() > 1) {
      error(parser->loc(), "Unexpected multiplie instance selects");
      YYERROR;
    }
    if (!$$->get_dim()->empty()) {
      if (dynamic_cast<RangeExpression*>($$->get_dim()->back()) != nullptr) {
        error(parser->loc(), "Unexpected range expression in array subscript");
        YYERROR;
      }
      $$->get_ids()->back()->get_isel()->replace($$->get_dim()->remove_back());
    }
    $$->get_ids()->push_back(new Id($3, new Maybe<Expression>()));
    $$->replace_dim($4);
    for (auto e : *$4) {
      if ((e != $4->back()) && (dynamic_cast<RangeExpression*>(e) != nullptr)) {
        error(parser->loc(), "Unexpected range expression in array subscript");
        YYERROR;
      }
    }
  }
  ;
identifier 
  : simple_id_L { 
    const auto id = new Id($1.second, new Maybe<Expression>());
    $$ = new Identifier(new Many<Id>(id), new Many<Expression>()); 
    $$->set_source(parser->source());
    $$->set_line($1.first);
  }
  /* TODO | ESCAPED_ID */
  ;
/* NOTE: Rules that delegate to these rules just add clutter */

/* Auxiliary Rules */
attr_spec_P
  : attr_spec { $$ = new Many<AttrSpec>($1); }
  | attr_spec_P COMMA attr_spec {
    $$ = $1;
    $$->push_back($3);
  }
  ;
attribute_instance_S 
  : %empty { $$ = new Attributes(new Many<AttrSpec>()); }
  | attribute_instance_S attribute_instance {
    $$ = $1;
    $$->get_as()->concat($2);
  }
  ;
block_item_declaration_S
  : %empty { $$ = new Many<Declaration>(); }
  | block_item_declaration_S block_item_declaration { 
    $$ = $1;
    $$->concat($2);
  }
  ;
braced_rexp_S
  : %empty { $$ = new Many<Expression>(); }
  | braced_rexp_S OSQUARE range_expression CSQUARE { 
    $$ = $1;
    $$->push_back($3); 
  }
  ;
case_generate_item_P
  : case_generate_item { $$ = new Many<CaseGenerateItem>($1); }
  | case_generate_item_P case_generate_item { 
    $$ = $1;
    $$->push_back($2);
  }
  ;
case_item_P
  : case_item { $$ = new Many<CaseItem>($1); }
  | case_item_P case_item {
    $$ = $1;
    $$->push_back($2);
  }
  ;
colon_Q
  : %empty
  | COLON
  ;
delay3_Q
  : %empty { $$ = new Maybe<DelayControl>(); }
  | delay3 { $$ = new Maybe<DelayControl>($1); }
  ;
delay_or_event_control_Q
  : %empty { $$ = new Maybe<TimingControl>(); }
  | delay_or_event_control { $$ = new Maybe<TimingControl>($1); }
  ;
dimension_S
  : %empty { $$ = new Many<Expression>(); }
  | dimension_S dimension { 
    $$ = $1;
    $$->push_back($2);
  }
  ;
eq_ce_Q
  : %empty { $$ = new Identifier("__null"); }
  | EQ expression { $$ = $2; }
  ;
expression_P
  : expression { $$ = new Many<Expression>($1); }
  | expression_P COMMA expression {
    $$ = $1;
    $$->push_back($3);
  }
  ;
expression_Q
  : %empty { $$ = new Maybe<Expression>(); }
  | expression { $$ = new Maybe<Expression>($1); }
  ;
generate_block_id_Q
  : %empty { $$ = new Maybe<Identifier>(); }
  | COLON identifier { $$ = new Maybe<Identifier>($2); }
  ;
integer_L
  : INTEGER { $$ = parser->loc().begin.line; }
  ;
list_of_port_declarations_Q 
  : %empty { $$ = new Many<ModuleItem>(); } 
  | list_of_port_declarations { $$ = $1; }
  ;
localparam_L
  : LOCALPARAM { $$ = parser->loc().begin.line; }
  ;
mintypmax_expression_Q 
  : %empty { $$ = new Maybe<Expression>(); }
  | mintypmax_expression { $$ = new Maybe<Expression>($1); }
  ;
module_instance_P
  : module_instance { $$ = new Many<ModuleInstantiation>($1); }
  | module_instance_P COMMA module_instance {
    $$ = $1;
    $$->push_back($3);
  }
  ;
module_item_S
  : %empty { $$ = new Many<ModuleItem>(); }
  | module_item_S module_item { 
    $$ = $1;
    $$->concat($2);
  }
  ;
module_keyword_L
  : module_keyword { $$ = parser->loc().begin.line; }
  ;
module_or_generate_item_S
  : %empty { $$ = new Many<ModuleItem>(); }
  | module_or_generate_item_S module_or_generate_item {
    $$ = $1;
    $$->concat($2);
  }
  ;
named_parameter_assignment_P
  : named_parameter_assignment { $$ = new Many<ArgAssign>($1); }
  | named_parameter_assignment_P COMMA named_parameter_assignment { 
    $$ = $1;
    $$->push_back($3);
  }
  ;
named_port_connection_P
  : named_port_connection { $$ = new Many<ArgAssign>($1); }
  | named_port_connection_P COMMA named_port_connection {
    $$ = $1;
    $$->push_back($3);
  }
  ;
net_type_L
  : net_type { $$ = std::make_pair(parser->loc().begin.line, $1); }
  ;
net_type_Q
  : %empty { $$ = NetDeclaration::WIRE; }
  | net_type { $$ = $1; }
  ;
non_port_module_item_S
  : %empty { $$ = new Many<ModuleItem>(); }
  | non_port_module_item_S non_port_module_item {
    $$ = $1;
    $$->concat($2);
  }
  ;
ordered_parameter_assignment_P
  : ordered_parameter_assignment { $$ = new Many<ArgAssign>($1); }
  | ordered_parameter_assignment_P COMMA ordered_parameter_assignment { 
    $$ = $1;
    $$->push_back($3);
  }
  ;
ordered_port_connection_P
  : ordered_port_connection { $$ = new Many<ArgAssign>($1); }
  | ordered_port_connection_P COMMA ordered_port_connection {
    $$ = $1;
    $$->push_back($3);
  }
  ;
parameter_declaration_P
  : parameter_declaration_P COMMA alt_parameter_declaration {
    $$ = $1;
    $$->push_back($3);
  } 
  | parameter_declaration_P COMMA list_of_param_assignments {
    $$ = $1;
    while (!$3->empty()) {
      auto va = $3->remove_front();
      auto pd = dynamic_cast<ParameterDeclaration*>($1->back()->clone());
      pd->replace_id(va->get_lhs()->clone());
      pd->replace_val(va->get_rhs()->clone());
      delete va; 
      $$->push_back(pd);
    }
    delete $3;
  }
  | alt_parameter_declaration {
    $$ = new Many<Declaration>($1);
  }
  ;
parameter_L
  : PARAMETER { $$ = parser->loc().begin.line; }
  ;
parameter_value_assignment_Q
  : %empty { $$ = new Many<ArgAssign>(); }
  | parameter_value_assignment { $$ = $1; }
  ;
port_declaration_P
  : port_declaration_P COMMA alt_port_declaration {
    $$ = $1;
    $$->push_back($3);
  }
  | port_declaration_P COMMA list_of_variable_port_identifiers {
    $$ = $1;
    while (!$3->empty()) {
      auto va = $3->remove_front();
      auto pd = dynamic_cast<PortDeclaration*>($1->back()->clone());
      if (auto nd = dynamic_cast<NetDeclaration*>(pd->get_decl())) {
        nd->replace_id(va->get_lhs()->clone());
        if (!is_null(va->get_rhs())) {
          error(parser->loc(), "Found initialization value in net declaration!");
          YYERROR;
        }
      } else if (auto rd = dynamic_cast<RegDeclaration*>(pd->get_decl())) {
        rd->replace_id(va->get_lhs()->clone());
        if (!is_null(va->get_rhs())) {
          rd->get_val()->replace(va->get_rhs()->clone());
        }
      } else {
        assert(false);
      }
      delete va;
      $$->push_back(pd);
    }
    delete $3;
  }
  | alt_port_declaration {
    $$ = new Many<ModuleItem>($1);
  }
  ;
port_P
  : port { $$ = new Many<ArgAssign>($1); }
  | port_P COMMA port {
    $$ = $1;
    $$->push_back($3);
  }
  ;
range_Q
  : %empty { $$ = new Maybe<RangeExpression>(); }
  | range { $$ = new Maybe<RangeExpression>($1); }
  ;
reg_L
  : REG { $$ = parser->loc().begin.line; }
  ;
signed_Q
  : %empty { $$ = false; }
  | SIGNED { $$ = true; }
  ;
simple_id_L
  : SIMPLE_ID { $$ = make_pair(parser->loc().begin.line, $1); }
statement_S
  : %empty { $$ = new Many<Statement>(); }
  | statement_S statement {
    $$ = $1;
    $$->push_back($2);
  }
  ;

alt_parameter_declaration
  : attribute_instance_S PARAMETER signed_Q range_Q param_assignment {
    $$ = new ParameterDeclaration($1, $3, $4, $5->get_lhs()->clone(), $5->get_rhs()->clone());
    delete $5;
  }
  | attribute_instance_S PARAMETER parameter_type param_assignment {
    $$ = new ParameterDeclaration($1, false, new Maybe<RangeExpression>(), $4->get_lhs()->clone(), $4->get_rhs()->clone());
    delete $4;
  }
  ;

alt_port_declaration 
  : alt_port_type alt_net_type signed_Q range_Q identifier eq_ce_Q {
    // If this is a net declaration and we have an initial value, it's an error
    if (!$2 && !is_null($6)) {
      error(parser->loc(), "Found initialization value in net declaration!");
      YYERROR;
    }
    auto d = $2 ? 
      (Declaration*) new RegDeclaration(new Attributes(new Many<AttrSpec>()), $5, $3, $4, is_null($6) ? new Maybe<Expression>() : new Maybe<Expression>($6->clone())) :
      (Declaration*) new NetDeclaration(new Attributes(new Many<AttrSpec>()), NetDeclaration::WIRE, new Maybe<DelayControl>(), $5, $3, $4);
    delete $6;
    $$ = new PortDeclaration(new Attributes(new Many<AttrSpec>()), $1, d);
  }
  ;
alt_port_type
  : INOUT { $$ = PortDeclaration::INOUT; }
  | INPUT { $$ = PortDeclaration::INPUT; }
  | OUTPUT { $$ = PortDeclaration::OUTPUT; }
  ;
alt_net_type 
  : %empty { $$ = false; }
  | WIRE { $$ = false; }
  | REG { $$ = true; }
  ; 
%%

namespace cascade {

void yyParser::error(const location_type& l, const std::string& m) {
  std::stringstream ss;

  if (parser->source() == "<top>") {
    ss << "In final line of user input:\n";
  } else {
    ss << "In " << parser->source() << " on line " << l.end.line << ":\n";
  }
  ss << m;
  parser->log_.error(ss.str());
}

} // namespace cascade
