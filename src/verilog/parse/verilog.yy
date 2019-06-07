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
#include <vector>
#include "base/log/log.h"
#include "verilog/ast/ast.h"

namespace cascade {

class Parser;

// Typedefs, since Bison >3.1.2 uses a macro, and std::pair cannot be used.
typedef std::pair<bool, std::string> SignedNumber;
typedef std::pair<size_t, NetDeclaration::Type> NetList;
typedef std::pair<Identifier*, RangeExpression*> ModuleIdentifier;
typedef std::pair<size_t, std::string> IdList;

} // namespace cascade
}

%param {Parser* parser}

%locations

%define parse.trace
%define parse.error verbose

%code 
{
#include "verilog/parse/parser.h"

#undef yylex
#define yylex parser->lexer_.yylex

namespace {

bool is_null(const cascade::Expression* e) {
  if (e->is(cascade::Node::Tag::identifier)) {
    auto i = static_cast<const cascade::Identifier*>(e);
    return i->eq("__null");
  }
  return false;
}

template <typename InItr>
cascade::SeqBlock* desugar_output(const cascade::Expression* fd, InItr begin, InItr end) {
  // Check args are well-formed
  auto error = false;
  auto simple = false;
  const auto n = end - begin;
  if (n == 0) {
    error = false;
  } else if ((*begin)->is(cascade::Node::Tag::string)) {
    const auto& str = static_cast<const cascade::String*>(*begin)->get_readable_val();
    if (static_cast<size_t>(std::count(str.begin(), str.end(), '%')) != (n-1)) {
      error = true;
    }
  } else {
    error = (n != 1);
    simple = true;
  }

  // Only try to generate a result if args are well-formed
  cascade::SeqBlock* sb = nullptr;
  if (!error) {
    sb = new cascade::SeqBlock(); 
    if (simple) {
      sb->push_back_stmts(new cascade::PutStatement(fd->clone(), new cascade::String("%d"), (*begin)->clone()));
    } else {
      size_t i = 0;
      auto itr = begin;
    
      const auto& str = static_cast<const cascade::String*>(*itr++)->get_readable_val();
      while (i < str.length()) {
        if (str[i] == '%') {
          sb->push_back_stmts(new cascade::PutStatement(fd->clone(), new cascade::String(str.substr(i, 2)), (*itr++)->clone()));
          i += 2;
          continue;
        } 
        const auto ie = str.find_first_of('%', i);
        sb->push_back_stmts(new cascade::PutStatement(fd->clone(), new cascade::String(str.substr(i, ie-i))));
        i = ie;
      }
    }
  }

  // Delete args
  delete fd;
  for (auto i = begin; i != end; ++i) {
    delete *i;
  }

  return sb;
}

} // namespace 
}

/* Control Tokens */
%token END_OF_FILE "<end_of_file>"
%token UNPARSEABLE "<unparseable>"

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
%token WHILE       "while"
%token WIRE        "wire"

/* System Task Identifiers */
%token SYS_DISPLAY  "$display"
%token SYS_ERROR    "$error"
%token SYS_FATAL    "$fatal"
%token SYS_FEOF     "$feof"
%token SYS_FDISPLAY "$fdisplay"
%token SYS_FINISH   "$finish"
%token SYS_FOPEN    "$fopen"
%token SYS_FREAD    "$fread"
%token SYS_FSEEK    "$fseek"
%token SYS_FWRITE   "$fwrite"
%token SYS_GET      "$get"
%token SYS_INFO     "$info"
%token SYS_PUT      "$put"
%token SYS_RESTART  "$restart"
%token SYS_RETARGET "$retarget"
%token SYS_REWIND   "$rewind"
%token SYS_SAVE     "$save"
%token SYS_WARNING  "$warning"
%token SYS_WRITE    "$write"

/* Identifiers and Strings */
%token <std::string> SIMPLE_ID
%token <std::string> STRING

/* Numbers */
%token <std::string> UNSIGNED_NUM
%token <SignedNumber> DECIMAL_VALUE
%token <SignedNumber> BINARY_VALUE
%token <SignedNumber> OCTAL_VALUE
%token <SignedNumber> HEX_VALUE

/* Compiler Directive Tokens */
%token END_INCLUDE "<end_include>"

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

/* A.1.2 Verilog Source Text */
%type <ModuleDeclaration*> module_declaration

/* A.1.3 Module Parameters and Ports */
%type <std::vector<ModuleItem*>> module_parameter_port_list
%type <std::vector<ArgAssign*>> list_of_ports
%type <std::vector<ModuleItem*>> list_of_port_declarations
%type <ArgAssign*> port
%type <Identifier*> port_expression
%type <Identifier*> port_reference
%type <std::vector<ModuleItem*>> port_declaration

/* A.1.4 Module Items */
%type <std::vector<ModuleItem*>> module_item
%type <std::vector<ModuleItem*>> module_or_generate_item
%type <std::vector<ModuleItem*>> module_or_generate_item_declaration
%type <std::vector<ModuleItem*>> non_port_module_item

/* A.2.1.1 Module Parameter Declarations */
%type <std::vector<Declaration*>> local_parameter_declaration
%type <std::vector<Declaration*>> parameter_declaration

/* A.2.1.2 Port Declarations */
%type <std::vector<ModuleItem*>> inout_declaration
%type <std::vector<ModuleItem*>> input_declaration
%type <std::vector<ModuleItem*>> output_declaration

/* A.2.1.3 Type Declarations */
%type <std::vector<ModuleItem*>> integer_declaration
%type <std::vector<ModuleItem*>> net_declaration
%type <std::vector<ModuleItem*>> reg_declaration

/* A.2.2.1 Net and Variable Types */
%type <NetDeclaration::Type> net_type
%type <VariableAssign*> variable_type

/* A.2.2.3 Delays */
%type <DelayControl*> delay3
%type <Expression*> delay_value

/* A.2.3 Declaration Lists */
%type <std::vector<VariableAssign*>> list_of_net_decl_assignments
%type <std::vector<Identifier*>> list_of_net_identifiers
%type <std::vector<VariableAssign*>> list_of_param_assignments
%type <std::vector<Identifier*>> list_of_port_identifiers
%type <std::vector<VariableAssign*>> list_of_variable_identifiers
%type <std::vector<VariableAssign*>> list_of_variable_port_identifiers

/* A.2.4 Declaration Assignments */
%type <VariableAssign*> net_decl_assignment
%type <VariableAssign*> param_assignment

/* A.2.5 Declaration Ranges */
%type <RangeExpression*> dimension
%type <RangeExpression*> range

/* A.2.8 Block Item Declarations */
%type <std::vector<Declaration*>> block_item_declaration
%type <std::vector<Identifier*>> list_of_block_variable_identifiers
%type <Identifier*> block_variable_type

/* A.4.1 Module Instantiation */
%type <std::vector<ModuleItem*>> module_instantiation 
%type <std::vector<ArgAssign*>> parameter_value_assignment
%type <std::vector<ArgAssign*>> list_of_parameter_assignments
%type <ArgAssign*> ordered_parameter_assignment
%type <ArgAssign*> named_parameter_assignment
%type <ModuleInstantiation*> module_instance
%type <ModuleIdentifier> name_of_module_instance
%type <std::vector<ArgAssign*>> list_of_port_connections
%type <ArgAssign*> ordered_port_connection
%type <ArgAssign*> named_port_connection

/* A.4.2 Generate Construct */
%type <GenerateRegion*> generate_region
%type <std::vector<ModuleItem*>> genvar_declaration
%type <std::vector<Identifier*>> list_of_genvar_identifiers
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
%type <GenerateBlock*> generate_block_or_null

/* A.6.1 Continuous Assignment Statements */
%type <std::vector<ModuleItem*>> continuous_assign
%type <std::vector<VariableAssign*>> list_of_net_assignments
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
%type <std::vector<Event*>> event_expression
%type <TimingControl*> procedural_timing_control
%type <TimingControlStatement*> procedural_timing_control_statement

/* A.6.6 Conditional Statements */
%type <ConditionalStatement*> conditional_statement
/* REMOVED IN FAVOR OF BISON CONVENTION %type<ConditionalStatement*> if_else_if_statement */

/* A.6.7 Case Statements */
%type <CaseStatement*> case_statement
%type <CaseItem*> case_item

/* A.6.8 Looping Statements */
%type <LoopStatement*> loop_statement

/* A.6.9 Task Enable Statements */
%type <Statement*> system_task_enable

/* A.8.1 Concatenations */
%type <Concatenation*> concatenation
%type <MultipleConcatenation*> multiple_concatenation

/* A.8.3 Expressions */
%type <ConditionalExpression*> conditional_expression
%type <FeofExpression*> feof_expression
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
%type <std::vector<AttrSpec*>> attribute_instance 
%type <AttrSpec*> attr_spec          
%type <Identifier*> attr_name

/* A.9.3 Identifiers */
%type <Identifier*> hierarchical_identifier
%type <Identifier*> identifier 

/* Auxiliary Rules */
%type <std::vector<AttrSpec*>> attr_spec_P
%type <Attributes*> attribute_instance_S 
%type <std::vector<Declaration*>> block_item_declaration_S
%type <std::vector<Expression*>> braced_rexp_S
%type <std::vector<CaseGenerateItem*>> case_generate_item_P
%type <std::vector<CaseItem*>> case_item_P
%type <DelayControl*> delay3_Q
%type <TimingControl*> delay_or_event_control_Q
%type <std::vector<Expression*>> dimension_S
%type <Expression*> eq_ce_Q
%type <std::vector<Expression*>> expression_P
%type <Expression*> expression_Q
%type <Identifier*> generate_block_id_Q
%type <size_t> integer_L
%type <std::vector<ModuleItem*>> list_of_port_declarations_Q
%type <size_t> localparam_L
%type <Expression*> mintypmax_expression_Q
%type <std::vector<ModuleInstantiation*>> module_instance_P
%type <std::vector<ModuleItem*>> module_item_S
%type <size_t> module_keyword_L
%type <std::vector<ModuleItem*>> module_or_generate_item_S
%type <std::vector<ArgAssign*>> named_parameter_assignment_P
%type <std::vector<ArgAssign*>> named_port_connection_P
%type <NetList> net_type_L
%type <NetDeclaration::Type> net_type_Q
%type <std::vector<ModuleItem*>> non_port_module_item_S
%type <std::vector<ArgAssign*>> ordered_parameter_assignment_P
%type <std::vector<ArgAssign*>> ordered_port_connection_P
%type <std::vector<Declaration*>> parameter_declaration_P
%type <size_t> parameter_L
%type <std::vector<ArgAssign*>> parameter_value_assignment_Q
%type <std::vector<ModuleItem*>> port_declaration_P
%type <std::vector<ArgAssign*>> port_P
%type <RangeExpression*> range_Q
%type <size_t> reg_L
%type <bool> signed_Q
%type <IdList> simple_id_L
%type <std::vector<Statement*>> statement_S

/* Alternate Rules */
/* These rules deviate from the Verilog Spec, due to LALR(1) parser quirks */
%type <Declaration*> alt_parameter_declaration
%type <PortDeclaration*> alt_port_declaration
%type <PortDeclaration::Type> alt_port_type
%type <bool> alt_net_type

%%

main 
  : restore module_declaration { 
    parser->res_.push_back($2); 
    YYACCEPT; 
  }
  | restore non_port_module_item backup { 
    parser->res_.insert(parser->res_.end(), $2.begin(), $2.end()); 
    YYACCEPT; 
  }
  | restore END_OF_FILE { 
    parser->eof_ = true; 
    YYACCEPT;
  }
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

/* A.1.2 Verilog Source Text */
module_declaration
  : attribute_instance_S module_keyword_L identifier module_parameter_port_list
      list_of_ports SCOLON module_item_S
      ENDMODULE {
      $4.insert($4.end(), $7.begin(), $7.end());
      $$ = new ModuleDeclaration($1, $3, $5.begin(), $5.end(), $4.begin(), $4.end());
      parser->set_loc($$, $2);
    }
  | attribute_instance_S module_keyword_L identifier module_parameter_port_list 
      list_of_port_declarations_Q SCOLON non_port_module_item_S
      ENDMODULE {
      std::vector<ArgAssign*> ps;
      for (auto p : $5) {
        assert(p->is(Node::Tag::port_declaration));
        auto* d = static_cast<PortDeclaration*>(p)->get_decl();
        ps.push_back(new ArgAssign(nullptr, d->get_id()->clone()));
      }
      $4.insert($4.end(), $5.begin(), $5.end());
      $4.insert($4.end(), $7.begin(), $7.end());
      $$ = new ModuleDeclaration($1, $3, ps.begin(), ps.end(), $4.begin(), $4.end());
      parser->set_loc($$, $2);
    }
  ;
module_keyword
  : MODULE 
  | MACROMODULE 
  ;

/* A.1.3 Module Parameters and Ports */
module_parameter_port_list
  : %empty { }
  | POUND OPAREN parameter_declaration_P CPAREN { 
    $$.insert($$.end(), $3.begin(), $3.end());
  }
  ;
list_of_ports
  : OPAREN port_P CPAREN { $$ = $2; }
  ;
list_of_port_declarations 
  : OPAREN port_declaration_P CPAREN { $$ = $2; }
  ;
port
  : %empty { $$ = new ArgAssign(nullptr, nullptr); }
  | port_expression { $$ = new ArgAssign(nullptr, $1); }
  | DOT identifier OPAREN CPAREN { $$ = new ArgAssign($2, nullptr); }
  | DOT identifier OPAREN port_expression CPAREN { $$ = new ArgAssign($2, $4); }
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
    for (auto pd : $2) {
      assert(pd->is(Node::Tag::port_declaration));
      static_cast<PortDeclaration*>(pd)->replace_attrs($1->clone());
    }
    delete $1;
    $$ = $2;
  }
  | attribute_instance_S input_declaration {
    for (auto pd : $2) {
      assert(pd->is(Node::Tag::port_declaration));
      static_cast<PortDeclaration*>(pd)->replace_attrs($1->clone());
    }
    delete $1;
    $$ = $2;
  }
  | attribute_instance_S output_declaration {
    for (auto pd : $2) {
      assert(pd->is(Node::Tag::port_declaration));
      static_cast<PortDeclaration*>(pd)->replace_attrs($1->clone());
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
    $$.insert($$.end(), $1.begin(), $1.end());
  }
  /* TODO | attribute_instance_S parameter_override */
  | /*attribute_instance_S*/ continuous_assign { $$ = $1; }
  /* TODO | attribute_instance_S gate_instantiation */
  /* TODO | attribute_instance_S udp_instantiation */
  | attribute_instance_S module_instantiation { 
    for (auto mi : $2) {
      static_cast<ModuleInstantiation*>(mi)->replace_attrs($1->clone());
    }
    delete $1;
    $$ = $2;
  }
  | attribute_instance_S initial_construct { 
    $2->replace_attrs($1); 
    $$.push_back($2); 
  }
  | /*attribute_instance_S*/ always_construct { 
    $$.push_back($1); 
  }
  | /*attribute_instance_S*/ loop_generate_construct { 
    $$.push_back($1); 
  }
  | attribute_instance_S conditional_generate_construct { 
    if ($2->is(Node::Tag::if_generate_construct)) {
      assert($2->is(Node::Tag::if_generate_construct));
      auto igc = static_cast<IfGenerateConstruct*>($2);
      igc->replace_attrs($1);
    } else {
      delete $1;
    }
    $$.push_back($2); 
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
  | generate_region { 
    $$.push_back($1); 
  }
  /* TODO | specify_block */
  | /*attribute_instance_S*/ parameter_declaration SCOLON { 
    $$.insert($$.end(), $1.begin(), $1.end());
  }
  /* TODO | attribute_instance_S specparam_declaration SCOLON */
  ;

/* A.2.1.1 Module Parameter Declarations */
local_parameter_declaration
  : attribute_instance_S localparam_L signed_Q range_Q list_of_param_assignments {
    for (auto va : $5) {
      auto lpd = new LocalparamDeclaration($1->clone(), $3, $4 == nullptr ? $4 : $4->clone(), va->get_lhs()->clone(), va->get_rhs()->clone());
      delete va;
      parser->set_loc(lpd, $2);
      parser->set_loc(lpd->get_id(), $2);
      parser->set_loc(lpd->get_val(), $2);
      $$.push_back(lpd);
    }
    delete $1;
    if ($4 != nullptr) {
      delete $4;
    }
  }
  | attribute_instance_S localparam_L parameter_type list_of_param_assignments {
    for (auto va : $4) {
      auto lpd = new LocalparamDeclaration($1->clone(), false, nullptr, va->get_lhs()->clone(), va->get_rhs()->clone());
      delete va;
      parser->set_loc(lpd, $2);
      parser->set_loc(lpd->get_id(), $2);
      parser->set_loc(lpd->get_val(), $2);
      $$.push_back(lpd);
    }
    delete $1;
  }
  ;
parameter_declaration
  : attribute_instance_S parameter_L signed_Q range_Q list_of_param_assignments {
    for (auto va : $5) {
      auto pd = new ParameterDeclaration($1->clone(), $3, $4 == nullptr ? $4 : $4->clone(), va->get_lhs()->clone(), va->get_rhs()->clone());
      delete va;
      parser->set_loc(pd, $2);
      parser->set_loc(pd->get_id(), $2);
      parser->set_loc(pd->get_val(), $2);
      $$.push_back(pd);
    }
    delete $1;
    if ($4 == nullptr) {
      delete $4;
    }
  }
  | attribute_instance_S parameter_L parameter_type list_of_param_assignments {
    for (auto va : $4) {
      auto pd = new ParameterDeclaration($1->clone(), false, nullptr, va->get_lhs()->clone(), va->get_rhs()->clone());
      delete va;
      parser->set_loc(pd, $2);
      parser->set_loc(pd->get_id(), $2);
      parser->set_loc(pd->get_val(), $2);
      $$.push_back(pd);
    }
    delete $1;
  }
  ;
parameter_type
  : integer_L 
  ;
/* A.2.1.2 Port Declarations */
inout_declaration
  : INOUT net_type_Q signed_Q range_Q list_of_port_identifiers {
    for (auto id : $5) {
      auto t = PortDeclaration::Type::INOUT;
      auto d = new NetDeclaration(new Attributes(), $2, nullptr, id, $3, $4 == nullptr ? $4 : $4->clone());
      $$.push_back(new PortDeclaration(new Attributes(), t, d));
    }
    if ($4 != nullptr) {
      delete $4;
    }
  }
  ;
input_declaration
  : INPUT net_type_Q signed_Q range_Q list_of_port_identifiers {
    for (auto id : $5) {
      auto t = PortDeclaration::Type::INPUT;
      auto d = new NetDeclaration(new Attributes(), $2, nullptr, id, $3, $4 == nullptr ? $4 : $4->clone());
      $$.push_back(new PortDeclaration(new Attributes(), t, d));
    }
    if ($4 != nullptr) {
      delete $4;
    }
  }
  ;
output_declaration
  : OUTPUT net_type_Q signed_Q range_Q list_of_port_identifiers {
    for (auto id : $5) {
      auto t = PortDeclaration::Type::OUTPUT;
      auto d = new NetDeclaration(new Attributes(), $2, nullptr, id, $3, $4 == nullptr ? $4 : $4->clone());
      $$.push_back(new PortDeclaration(new Attributes(), t, d));
    }
    if ($4 != nullptr) {
      delete $4;
    }
  }
  | OUTPUT REG signed_Q range_Q list_of_variable_port_identifiers {
    for (auto va : $5) {
      auto t = PortDeclaration::Type::OUTPUT;
      auto d = new RegDeclaration(new Attributes(), va->get_lhs()->clone(), $3, $4 == nullptr ? $4 : $4->clone(), !is_null(va->get_rhs()) ? va->get_rhs()->clone() : nullptr);
      delete va;
      $$.push_back(new PortDeclaration(new Attributes(), t, d));
    }
    if ($4 != nullptr) {
      delete $4;
    }
  }
  /* TODO | OUTPUT output_variable_type list_of_variable_port_identifiers */
  ;

/* A.2.1.3 Type Declarations */
integer_declaration
  : attribute_instance_S integer_L list_of_variable_identifiers SCOLON {
    for (auto va : $3) {
      auto id = new RegDeclaration($1->clone(), va->get_lhs()->clone(), true, new RangeExpression(32, 0), !is_null(va->get_rhs()) ? va->get_rhs()->clone() : nullptr);
      delete va;
      parser->set_loc(id, $2);
      parser->set_loc(id->get_id(), $2);
      $$.push_back(id);
    }
    delete $1;
  }
net_declaration 
  /** TODO: Combining cases with below due to lack of support for vectored|scalared */
  : attribute_instance_S net_type_L /* [vectored|scalared] */ signed_Q range_Q delay3_Q list_of_net_identifiers SCOLON {
    for (auto id : $6) {
      auto nd = new NetDeclaration($1->clone(), $2.second, $5 == nullptr ? $5 : $5->clone(), id, $3, $4 == nullptr ? $4 : $4->clone());
      parser->set_loc(nd, $2.first);
      parser->set_loc(nd->get_id(), $2.first);
      $$.push_back(nd);
    }
    delete $1;
    if ($4 != nullptr) {
      delete $4;
    }
    if ($5 != nullptr) {
      delete $5;
    }
  }
  /** TODO: Combining cases with below due to lack of support for vectored|scalared */
  | attribute_instance_S net_type_L /* drive_strength [vectored|scalared] */ signed_Q range_Q delay3_Q list_of_net_decl_assignments SCOLON {
    for (auto va : $6) {
      auto nd = new NetDeclaration($1->clone(), $2.second, $5 == nullptr ? $5 : $5->clone(), va->get_lhs()->clone(), $3, $4 == nullptr ? $4 : $4->clone());
      parser->set_loc(nd, $2.first);
      parser->set_loc(nd->get_id(), $2.first);
      $$.push_back(nd);

      auto ca = new ContinuousAssign(va);
      $$.push_back(ca);
    }
    delete $1;
    if ($4 != nullptr) {
      delete $4;
    }
    if ($5 != nullptr) {
      delete $5;
    }
  }
  /* TODO | ... lots of cases */
  ;
reg_declaration
  : attribute_instance_S reg_L signed_Q range_Q list_of_variable_identifiers SCOLON {
    for (auto va : $5) {
      auto rd = new RegDeclaration($1->clone(), va->get_lhs()->clone(), $3, $4 == nullptr ? $4 : $4->clone(), !is_null(va->get_rhs()) ? va->get_rhs()->clone() : nullptr);
      delete va;
      parser->set_loc(rd, $2);
      parser->set_loc(rd->get_id(), $2);
      $$.push_back(rd);
    }
    delete $1;
    if ($4 != nullptr) {
      delete $4;
    }
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
  : WIRE { $$ = NetDeclaration::Type::WIRE; }
  /* TODO | WAND */
  /* TODO | WOR */
  ;
variable_type
  : identifier dimension_S { 
    $$ = new VariableAssign($1, new Identifier("__null")); 
    $$->get_lhs()->purge_dim(); 
    $$->get_lhs()->push_back_dim($2.begin(), $2.end()); 
    parser->set_loc($$, $1);
  }
  | identifier EQ expression { 
    $$ = new VariableAssign($1, $3); 
    parser->set_loc($$, $1);
  }
  | identifier EQ SYS_FOPEN OPAREN string_ CPAREN { 
    $$ = new VariableAssign($1, new FopenExpression($5)); 
    parser->set_loc($$, $1);
  }
  ;

/* A.2.2.3 Delays */
delay3 
  : POUND delay_value { 
    $$ = new DelayControl($2); 
    parser->set_loc($$);
  }
  /* TODO | # (mintypmax_expression (, mintypmax_expression, mintypmax_expression?)?) */
  ;
delay_value
  : UNSIGNED_NUM { $$ = new Number($1, Number::Format::UNBASED, 32, false); }
  /* TODO | real_number */
  /* TODO | identifier */
  ;

/* A.2.3 Declaration Lists */
list_of_net_decl_assignments
  : net_decl_assignment { 
    $$.push_back($1); 
  }
  | list_of_net_decl_assignments COMMA net_decl_assignment {
    $$ = $1;
    $$.push_back($3);
  }
  ;
list_of_net_identifiers
  : identifier dimension_S { 
    $$.push_back($1); 
    $$.back()->purge_dim();
    $$.back()->push_back_dim($2.begin(), $2.end());
  }
  | list_of_net_identifiers COMMA identifier dimension_S {
    $$ = $1;
    $$.push_back($3);
    $$.back()->purge_dim();
    $$.back()->push_back_dim($4.begin(), $4.end());
  }
  ;
list_of_param_assignments 
  : param_assignment { 
    $$.push_back($1); 
  }
  | list_of_param_assignments COMMA param_assignment {
    $$ = $1;
    $$.push_back($3);
  }
  ;
list_of_port_identifiers
  : identifier { 
    $$.push_back($1); 
  }
  | list_of_port_identifiers COMMA identifier {
    $$ = $1;
    $$.push_back($3);
  }
  ;
list_of_variable_identifiers
  : variable_type { 
    $$.push_back($1); 
  }
  | list_of_variable_identifiers COMMA variable_type {
    $$ = $1;
    $$.push_back($3);
  }
  ;
list_of_variable_port_identifiers
  : identifier eq_ce_Q { 
    $$.push_back(new VariableAssign($1, $2)); 
  }
  | list_of_variable_port_identifiers COMMA identifier eq_ce_Q {
    $$ = $1;
    $$.push_back(new VariableAssign($3,$4));
  }

/* A.2.4 Declaration Assignments */
net_decl_assignment
  : identifier EQ expression { 
    $$ = new VariableAssign($1,$3); 
    parser->set_loc($$, $1);
  }
  ;
param_assignment
  : identifier EQ mintypmax_expression { $$ = new VariableAssign($1, $3); }

/* A.2.5 Declaration Ranges */
dimension
  : OSQUARE expression COLON expression CSQUARE { 
    $$ = new RangeExpression($2, RangeExpression::Type::CONSTANT, $4);
  }
  ;
range
  : OSQUARE expression COLON expression CSQUARE { 
    $$ = new RangeExpression($2, RangeExpression::Type::CONSTANT, $4);
  }
  ;

/* A.2.8 Block Item Declarations */
block_item_declaration
  : attribute_instance_S REG signed_Q range_Q list_of_block_variable_identifiers SCOLON { 
    for (auto id : $5) {
      $$.push_back(new RegDeclaration($1->clone(), id, $3, $4 == nullptr ? $4 : $4->clone(), nullptr));
    }
    delete $1;
    if ($4 != nullptr) {
      delete $4;
    }
  }
  | attribute_instance_S integer_L list_of_block_variable_identifiers SCOLON { 
    for (auto id : $3) {
      $$.push_back(new RegDeclaration($1->clone(), id, true, new RangeExpression(32, 0), nullptr));
    }
    delete $1;
  }
  /* TODO | attribute_instance_S TIME list_of_block_variable_identifiers SCOLON { } */
  /* TODO | attribute_instance_S REAL list_of_block_variable_identifiers SCOLON { } */
  /* TODO | attribute_instance_S REALTIME list_of_block_variable_identifiers SCOLON { } */
  /* TODO | attribute_instance_S event_declaration { } */
  | /*attribute_instance_S*/ local_parameter_declaration SCOLON { $$ = $1; }
  | /*attribute_instance_S*/ parameter_declaration SCOLON { $$ = $1; }
  ;
list_of_block_variable_identifiers 
  : block_variable_type { 
    $$.push_back($1); } 
  | list_of_block_variable_identifiers COMMA block_variable_type { 
    $$ = $1;
    $$.push_back($3);
  }
  ;
block_variable_type 
  : identifier dimension_S { 
    $$ = $1; 
    $$->purge_dim();
    $$->push_back_dim($2.begin(), $2.end());
  }
  ;

/* A.4.1 Module Instantiation */
module_instantiation 
  : identifier parameter_value_assignment_Q module_instance_P SCOLON {
    for (auto mi : $3) {
      mi->replace_mid($1->clone());
      mi->purge_params();
      mi->push_back_params($2.begin(), $2.end());
      parser->set_loc(mi, $1);
      $$.push_back(mi);
    }
    delete $1;
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
  : expression { $$ = new ArgAssign(nullptr, $1); }
  ;
named_parameter_assignment
  : DOT identifier OPAREN mintypmax_expression_Q CPAREN { 
    $$ = new ArgAssign($2, $4);
  } 
  ; 
module_instance
  : name_of_module_instance OPAREN list_of_port_connections CPAREN { 
    $$ = new ModuleInstantiation(new Attributes(), new Identifier(""), $1.first);
    $$->replace_range($1.second);
    $$->push_back_ports($3.begin(), $3.end());
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
    $$ = new ArgAssign(nullptr, $1); 
  }
  ;
named_port_connection
  : /*attribute_instance_S*/ DOT identifier OPAREN expression_Q CPAREN {
    $$ = new ArgAssign($2, $4);
  }
  ;

/* A.4.2 Generate Construct */
generate_region
  : GENERATE module_or_generate_item_S ENDGENERATE { $$ = new GenerateRegion($2.begin(), $2.end()); }
  ;
genvar_declaration
  : attribute_instance_S GENVAR list_of_genvar_identifiers SCOLON { 
    for (auto id : $3) {
      $$.push_back(new GenvarDeclaration($1->clone(), id));
    }
    delete $1;
  }
  ;
list_of_genvar_identifiers
  : identifier { 
    $$.push_back($1); 
  }
  | list_of_genvar_identifiers COMMA identifier { 
    $$ = $1;
    $$.push_back($3);
  }
  ;
loop_generate_construct
  : FOR OPAREN genvar_initialization SCOLON genvar_expression SCOLON genvar_iteration CPAREN generate_block { 
    $$ = new LoopGenerateConstruct($3, $5, $7, $9);
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
    $$ = new UnaryExpression($1,$2); parser->set_loc($$, $2); 
  }
  | genvar_expression AAMP /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::AAMP, $3); parser->set_loc($$, $1);
  }
  | genvar_expression AMP /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::AMP, $3); parser->set_loc($$, $1);
  }
  | genvar_expression BEEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::BEEQ, $3); parser->set_loc($$, $1);
  }
  | genvar_expression BEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::BEQ, $3); parser->set_loc($$, $1);
  }
  | genvar_expression CARAT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::CARAT, $3); parser->set_loc($$, $1);
  }
  | genvar_expression DIV /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::DIV, $3); parser->set_loc($$, $1);
  }
  | genvar_expression EEEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::EEEQ, $3); parser->set_loc($$, $1);
  }
  | genvar_expression EEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::EEQ, $3); parser->set_loc($$, $1);
  }
  | genvar_expression GEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::GEQ, $3); parser->set_loc($$, $1);
  }
  | genvar_expression GGGT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::GGGT, $3); parser->set_loc($$, $1);
  }
  | genvar_expression GGT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::GGT, $3); parser->set_loc($$, $1);
  }
  | genvar_expression GT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::GT, $3); parser->set_loc($$, $1);
  }
  | genvar_expression LEQ /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::LEQ, $3); parser->set_loc($$, $1);
  }
  | genvar_expression LLLT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::LLLT, $3); parser->set_loc($$, $1);
  }
  | genvar_expression LLT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::LLT, $3); parser->set_loc($$, $1);
  }
  | genvar_expression LT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::LT, $3); parser->set_loc($$, $1);
  }
  | genvar_expression MINUS /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::MINUS, $3); parser->set_loc($$, $1);
  }
  | genvar_expression MOD /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::MOD, $3); parser->set_loc($$, $1);
  }
  | genvar_expression PIPE /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::PIPE, $3); parser->set_loc($$, $1);
  }
  | genvar_expression PPIPE /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::PPIPE, $3); parser->set_loc($$, $1);
  }
  | genvar_expression PLUS /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::PLUS, $3); parser->set_loc($$, $1);
  }
  | genvar_expression TCARAT /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::TCARAT, $3); parser->set_loc($$, $1);
  }
  | genvar_expression TIMES /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::TIMES, $3); parser->set_loc($$, $1);
  }
  | genvar_expression TTIMES /*attribute_instance_S*/ genvar_expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::TTIMES, $3); parser->set_loc($$, $1);
  }
  | genvar_expression QMARK /*attribute_instance_S*/ genvar_expression COLON genvar_expression { 
    $$ = new ConditionalExpression($1,$3,$5); parser->set_loc($$, $1);
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
    $$ = new IfGenerateConstruct(new Attributes(), new IfGenerateClause($3, $5), nullptr);
  }
  | IF OPAREN expression CPAREN generate_block_or_null ELSE generate_block_or_null { 
    $$ = new IfGenerateConstruct(new Attributes(), new IfGenerateClause($3, $5), nullptr);
    // Was the remainder of this parse an empty block?
    if ($7 == nullptr) {
      // Nothing to do.
    }
    // Was it an unscoped if/then?
    else if (!$7->get_scope() && ($7->size_items() == 1) && $7->front_items()->is(Node::Tag::if_generate_construct)) {
      auto igc = static_cast<IfGenerateConstruct*>($7->remove_front_items());
      delete $7;
      while (!igc->empty_clauses()) {
        $$->push_back_clauses(igc->remove_front_clauses());
      }
      auto e = igc->get_else();
      igc->set_else(nullptr); 
      $$->replace_else(e);
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
    for (auto cgi : $5) {
      if (cgi->empty_exprs()) {
        has_default = true;
        break;
      }
    }
    if (!has_default) {
      $5.push_back(new CaseGenerateItem());
    }
    $$ = new CaseGenerateConstruct($3, $5.begin(), $5.end());
  }
  ;
case_generate_item
  : expression_P COLON generate_block_or_null { 
    $$ = new CaseGenerateItem($1.begin(), $1.end(), $3);
  }
  | DEFAULT colon_Q generate_block_or_null { 
    $$ = new CaseGenerateItem();
    $$->replace_block($3);
  }
  ; 
generate_block
  : module_or_generate_item { 
    $$ = new GenerateBlock(nullptr, false, $1.begin(), $1.end()); 
  }
  | BEGIN_ generate_block_id_Q module_or_generate_item_S END { 
    $$ = new GenerateBlock($2, true, $3.begin(), $3.end());   
  }
  ;
generate_block_or_null
  : SCOLON { $$ = nullptr; }
  | generate_block { $$ = $1; }
  ;

/* A.6.1 Continuous Assignment Statements */
continuous_assign
  : ASSIGN /* TODO drive_strength? */ delay3_Q list_of_net_assignments SCOLON {
    for (auto id : $3) {
      auto ca = new ContinuousAssign($2 == nullptr ? $2 : $2->clone(), id);
      parser->set_loc(ca, ca->get_assign());
      $$.push_back(ca);
    }
    if ($2 != nullptr) {
      delete $2;
    }
  }
  ;
list_of_net_assignments
  : net_assignment { 
    $$.push_back($1); 
  }
  | list_of_net_assignments COMMA net_assignment {
    $$ = $1;
    $$.push_back($3);
  }
  ;
net_assignment
  : net_lvalue EQ expression { 
    $$ = new VariableAssign($1,$3); 
    parser->set_loc($$, $1);
  }
  ;

/* A.6.2 Procedural Blocks and Assignments */
initial_construct
  : INITIAL_ statement { $$ = new InitialConstruct(new Attributes(), $2); }
  ;
always_construct
  : ALWAYS statement { $$ = new AlwaysConstruct($2); }
  ;
blocking_assignment
  : variable_lvalue EQ delay_or_event_control_Q expression {
    $$ = new BlockingAssign($3, new VariableAssign($1,$4));
    parser->set_loc($$, $1);
  }
  ;
nonblocking_assignment
  : variable_lvalue LEQ delay_or_event_control_Q expression {
    $$ = new NonblockingAssign($3, new VariableAssign($1,$4));
    parser->set_loc($$, $1);
  }
  ;
variable_assignment
  : variable_lvalue EQ expression { 
    $$ = new VariableAssign($1,$3); 
    parser->set_loc($$, $1);
  }
  ;

/* A.6.3 Parallel and Sequential Blocks */
par_block 
  : FORK statement_S JOIN {
    $$ = new ParBlock();
    $$->push_back_stmts($2.begin(), $2.end());
  }
  | FORK COLON identifier block_item_declaration_S statement_S JOIN {
    $$ = new ParBlock($3, $4.begin(), $4.end(), $5.begin(), $5.end()); 
  }
  ;
seq_block 
  : BEGIN_ statement_S END {
    $$ = new SeqBlock();
    $$->push_back_stmts($2.begin(), $2.end());
  }
  | BEGIN_ COLON identifier block_item_declaration_S statement_S END {
    $$ = new SeqBlock($3, $4.begin(), $4.end(), $5.begin(), $5.end()); 
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
  /* TODO | wait_statement */
  ;
statement_or_null
  : statement { $$ = $1; }
  | /*attribute_instance_S*/ SCOLON { 
    $$ = new SeqBlock();
  }
  ;

/* A.6.5 Timing Control Statements */
delay_control
  : POUND delay_value { 
    $$ = new DelayControl($2); 
    parser->set_loc($$);
  }
  | POUND OPAREN mintypmax_expression CPAREN { 
    $$ = new DelayControl($3); 
    parser->set_loc($$);
  }
  ;
delay_or_event_control 
  : delay_control { $$ = $1; }
  | event_control { $$ = $1; }
  /* TODO | repeat (expression) event_control */
  ;
event_control
  /* TODO : AT hierarchical_event_identifier */ 
  : AT OPAREN event_expression CPAREN { $$ = new EventControl($3.begin(), $3.end()); }
  | AT TIMES { $$ = new EventControl(); }
  | AT STAR { $$ = new EventControl(); }
  ;
event_expression
  : expression {
    $$.push_back(new Event(Event::Type::EDGE, $1));
  }
  | POSEDGE expression {
    $$.push_back(new Event(Event::Type::POSEDGE, $2));
  }
  | NEGEDGE expression {
    $$.push_back(new Event(Event::Type::NEGEDGE, $2));
  }
  | event_expression OR event_expression {
    $$ = $1;
    $$.insert($$.end(), $3.begin(), $3.end());
  }
  | event_expression COMMA event_expression {
    $$ = $1;
    $$.insert($$.end(), $3.begin(), $3.end());
  }
  ;
procedural_timing_control
  : delay_control { $$ = $1; }
  | event_control { $$ = $1; }
  ;
procedural_timing_control_statement
  : procedural_timing_control statement_or_null { $$ = new TimingControlStatement($1,$2); }
  ;
/* wait_statement */

/* A.6.6 Conditional Statements */
conditional_statement
  : IF OPAREN expression CPAREN statement_or_null %prec THEN {
    $$ = new ConditionalStatement($3, $5, new SeqBlock());
  }
  | IF OPAREN expression CPAREN statement_or_null ELSE statement_or_null {
    $$ = new ConditionalStatement($3, $5, $7);
  }
  ; 

/* A.6.7 Case Statements */
case_statement
  : CASE OPAREN expression CPAREN case_item_P ENDCASE {
    $$ = new CaseStatement(CaseStatement::Type::CASE, $3, $5.begin(), $5.end());
  }
  | CASEZ OPAREN expression CPAREN case_item_P ENDCASE {
    $$ = new CaseStatement(CaseStatement::Type::CASEZ, $3, $5.begin(), $5.end());
  }
  | CASEX OPAREN expression CPAREN case_item_P ENDCASE {
    $$ = new CaseStatement(CaseStatement::Type::CASEX, $3, $5.begin(), $5.end());
  }
  ;
case_item
  : expression_P COLON statement_or_null { $$ = new CaseItem($1.begin(), $1.end(), $3); }
  | DEFAULT colon_Q statement_or_null { $$ = new CaseItem($3); }

/* A.6.8 Looping Statements */
loop_statement
  /* forever statement */
  : REPEAT OPAREN expression CPAREN statement { 
    $$ = new RepeatStatement($3,$5); 
    parser->set_loc($$);
  }
  | WHILE OPAREN expression CPAREN statement { 
    $$ = new WhileStatement($3,$5); 
    parser->set_loc($$);
  }
  | FOR OPAREN variable_assignment SCOLON expression SCOLON variable_assignment CPAREN statement {
    $$ = new ForStatement($3,$5,$7,$9); 
    parser->set_loc($$);
  }
  ;

/* A.6.9 Task Enable Statements */
system_task_enable
  : SYS_DISPLAY SCOLON { 
    $$ = new PutStatement(new Identifier("STDOUT"), new String("\n")); 
    parser->set_loc($$);
  }
  | SYS_DISPLAY OPAREN CPAREN SCOLON { 
    $$ = new PutStatement(new Identifier("STDOUT"), new String("\n")); 
    parser->set_loc($$);
  }
  | SYS_DISPLAY OPAREN expression_P CPAREN SCOLON { 
    auto* sb = desugar_output(new Identifier("STDOUT"), $3.begin(), $3.end()); 
    if (sb == nullptr) {
      error(parser->get_loc(), "Found incorrectly formatted $display() statement!");
      YYERROR;
    }
    sb->push_back_stmts(new PutStatement(new Identifier("STDOUT"), new String("\n")));
    $$ = sb;
    parser->set_loc($$);
  }
  | SYS_ERROR SCOLON { 
    $$ = new PutStatement(new Identifier("STDERR"), new String("\n")); 
    parser->set_loc($$);
  }
  | SYS_ERROR OPAREN CPAREN SCOLON { 
    $$ = new PutStatement(new Identifier("STDERR"), new String("\n")); 
    parser->set_loc($$);
  }
  | SYS_ERROR OPAREN expression_P CPAREN SCOLON { 
    auto* sb = desugar_output(new Identifier("STDERR"), $3.begin(), $3.end()); 
    if (sb == nullptr) {
      error(parser->get_loc(), "Found incorrectly formatted $error() statement!");
      YYERROR;
    }
    sb->push_back_stmts(new PutStatement(new Identifier("STDERR"), new String("\n")));
    $$ = sb;
    parser->set_loc($$);
  }
  | SYS_FATAL SCOLON { 
    $$ = new FinishStatement(new Number(Bits(false), Number::Format::UNBASED)); 
    parser->set_loc($$);
  }
  | SYS_FATAL OPAREN CPAREN SCOLON { 
    $$ = new FinishStatement(new Number(Bits(false), Number::Format::UNBASED)); 
    parser->set_loc($$);
  }
  | SYS_FATAL OPAREN number CPAREN SCOLON { 
    $$ = new FinishStatement($3); 
    parser->set_loc($$);
  }
  | SYS_FATAL OPAREN number COMMA expression_P CPAREN SCOLON { 
    auto* es = desugar_output(new Identifier("STDERR"), $5.begin(), $5.end()); 
    if (es == nullptr) {
      error(parser->get_loc(), "Found incorrectly formatted $fatal() statement!");
      YYERROR;
    }
    es->push_back_stmts(new PutStatement(new Identifier("STDERR"), new String("\n")));
    auto* sb = new SeqBlock(es);
    sb->push_back_stmts(new FinishStatement($3));
    $$ = sb;
    parser->set_loc($$);
  }
  | SYS_FDISPLAY OPAREN expression CPAREN SCOLON { 
    $$ = new PutStatement($3, new String("\n")); 
    parser->set_loc($$);
  }
  | SYS_FDISPLAY OPAREN expression COMMA expression_P CPAREN SCOLON { 
    auto* fd = $3->clone();
    auto* sb = desugar_output($3, $5.begin(), $5.end()); 
    if (sb == nullptr) {
      error(parser->get_loc(), "Found incorrectly formatted $fdisplay() statement!");
      YYERROR;
    }
    sb->push_back_stmts(new PutStatement(fd, new String("\n")));
    $$ = sb;
    parser->set_loc($$);
  }
  | SYS_FINISH SCOLON { 
    $$ = new FinishStatement(new Number(Bits(false), Number::Format::UNBASED)); 
    parser->set_loc($$);
  }
  | SYS_FINISH OPAREN CPAREN SCOLON { 
    $$ = new FinishStatement(new Number(Bits(false), Number::Format::UNBASED)); 
    parser->set_loc($$);
  }
  | SYS_FINISH OPAREN expression CPAREN SCOLON { 
    $$ = new FinishStatement($3); 
    parser->set_loc($$);
  }
  | SYS_FWRITE OPAREN expression CPAREN SCOLON { 
    $$ = new PutStatement($3, new String("\n")); 
    parser->set_loc($$);
  }
  | SYS_FWRITE OPAREN expression COMMA expression_P CPAREN SCOLON { 
    auto* fd = $3->clone();
    auto* sb = desugar_output($3, $5.begin(), $5.end()); 
    if (sb == nullptr) {
      error(parser->get_loc(), "Found incorrectly formatted $fwrite() statement!");
      YYERROR;
    }
    $$ = sb;
    parser->set_loc($$);
  }
  | SYS_GET OPAREN expression COMMA string_ CPAREN SCOLON {
    $$ = new GetStatement($3, $5); 
    parser->set_loc($$);
  }
  | SYS_GET OPAREN expression COMMA string_ COMMA identifier CPAREN SCOLON {
    $$ = new GetStatement($3, $5, $7); 
    parser->set_loc($$);
  }
  | SYS_INFO SCOLON { 
    $$ = new PutStatement(new Identifier("STDINFO"), new String("\n"));
    parser->set_loc($$);
  }
  | SYS_INFO OPAREN CPAREN SCOLON { 
    $$ = new PutStatement(new Identifier("STDINFO"), new String("\n"));
    parser->set_loc($$);
  }
  | SYS_INFO OPAREN expression_P CPAREN SCOLON { 
    auto* sb = desugar_output(new Identifier("STDINFO"), $3.begin(), $3.end()); 
    if (sb == nullptr) {
      error(parser->get_loc(), "Found incorrectly formatted $info() statement!");
      YYERROR;
    }
    sb->push_back_stmts(new PutStatement(new Identifier("STDINFO"), new String("\n")));
    $$ = sb;
    parser->set_loc($$);
  }
  | SYS_PUT OPAREN expression COMMA string_ CPAREN SCOLON {
    $$ = new PutStatement($3, $5);
  }
  | SYS_PUT OPAREN expression COMMA string_ COMMA expression CPAREN SCOLON {
    $$ = new PutStatement($3, $5, $7);
  }
  | SYS_RESTART OPAREN string_ CPAREN SCOLON {
    $$ = new RestartStatement($3);
    parser->set_loc($$);
  }
  | SYS_RETARGET OPAREN string_ CPAREN SCOLON {
    $$ = new RetargetStatement($3);
    parser->set_loc($$);
  }
  | SYS_REWIND OPAREN expression CPAREN SCOLON {
    $$ = new FseekStatement($3, new Number(Bits(false)), new Number(Bits(false)));
    parser->set_loc($$);
  }
  | SYS_SAVE OPAREN string_ CPAREN SCOLON {
    $$ = new SaveStatement($3);
    parser->set_loc($$);
  }
  | SYS_FREAD OPAREN expression COMMA identifier CPAREN SCOLON {
    $$ = new GetStatement($3, new String("%u"), $5);
    parser->set_loc($$);
  }
  | SYS_FSEEK OPAREN expression COMMA number COMMA number CPAREN SCOLON {
    $$ = new FseekStatement($3, $5, $7);
    parser->set_loc($$);
  }
  | SYS_WARNING SCOLON { 
    $$ = new PutStatement(new Identifier("STDWARN"), new String("\n"));
    parser->set_loc($$);
  }
  | SYS_WARNING OPAREN CPAREN SCOLON { 
    $$ = new PutStatement(new Identifier("STDWARN"), new String("\n"));
    parser->set_loc($$);
  }
  | SYS_WARNING OPAREN expression_P CPAREN SCOLON { 
    auto* sb = desugar_output(new Identifier("STDWARN"), $3.begin(), $3.end()); 
    if (sb == nullptr) {
      error(parser->get_loc(), "Found incorrectly formatted $warning() statement!");
      YYERROR;
    }
    sb->push_back_stmts(new PutStatement(new Identifier("STDWARN"), new String("\n")));
    $$ = sb;
    parser->set_loc($$);
  }
  | SYS_WRITE SCOLON { 
    $$ = new PutStatement(new Identifier("STDOUT"), new String("")); 
    parser->set_loc($$);
  }
  | SYS_WRITE OPAREN CPAREN SCOLON { 
    $$ = new PutStatement(new Identifier("STDOUT"), new String("")); 
    parser->set_loc($$);
  }
  | SYS_WRITE OPAREN expression_P CPAREN SCOLON { 
    auto* sb = desugar_output(new Identifier("STDOUT"), $3.begin(), $3.end()); 
    if (sb == nullptr) {
      error(parser->get_loc(), "Found incorrectly formatted $write() statement!");
      YYERROR;
    }
    $$ = sb;
    parser->set_loc($$);
  }
  ;

/* A.8.1 Concatenations */
concatenation 
  : OCURLY expression_P CCURLY { $$ = new Concatenation($2.begin(), $2.end()); }
  ;
multiple_concatenation
  : OCURLY expression concatenation CCURLY { $$ = new MultipleConcatenation($2, $3); }
  ;

/* A.8.3 Expressions */
conditional_expression
  : expression QMARK /*attribute_instance_S*/ expression COLON expression {
    $$ = new ConditionalExpression($1, $3, $5);
  }
  ;
feof_expression
  : SYS_FEOF OPAREN expression CPAREN {
    $$ = new FeofExpression($3);
    parser->set_loc($$);
  }
expression
  : primary { 
    $$ = $1; 
  }
  | unary_operator /*attribute_instance_S*/ primary { 
    $$ = new UnaryExpression($1,$2); 
  }
  | expression AAMP  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::AAMP, $3); 
  }
  | expression AMP  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::AMP, $3); 
  }
  | expression BEEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::BEEQ, $3); 
  }
  | expression BEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::BEQ, $3); 
  }
  | expression CARAT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::CARAT, $3); 
  }
  | expression DIV  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::DIV, $3); 
  }
  | expression EEEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::EEEQ, $3); 
  }
  | expression EEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::EEQ, $3); 
  }
  | expression GEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::GEQ, $3); 
  }
  | expression GGGT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::GGGT, $3); 
  }
  | expression GGT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::GGT, $3); 
  }
  | expression GT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::GT, $3); 
  }
  | expression LEQ  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::LEQ, $3); 
  }
  | expression LLLT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::LLLT, $3); 
  }
  | expression LLT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::LLT, $3); 
  }
  | expression LT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::LT, $3); 
  }
  | expression MINUS  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::MINUS, $3); 
  }
  | expression MOD  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::MOD, $3); 
  }
  | expression PIPE  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::PIPE, $3); 
  }
  | expression PPIPE  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::PPIPE, $3); 
  }
  | expression PLUS  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::PLUS, $3); 
  }
  | expression TCARAT  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::TCARAT, $3); 
  }
  | expression TIMES  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::TIMES, $3); 
  }
  | expression TTIMES  /*attribute_instance_S*/ expression { 
    $$ = new BinaryExpression($1, BinaryExpression::Op::TTIMES, $3); 
  }
  | conditional_expression { $$ = $1; }
  | feof_expression { $$ = $1; }
  ;
mintypmax_expression
  : expression { $$ = $1; }
  /* TODO | expression COLON expression COLON expression */
  ;
range_expression
  : expression { $$ = $1; }
  | expression COLON expression { $$ = new RangeExpression($1, RangeExpression::Type::CONSTANT, $3); }
  | expression PCOLON expression { $$ = new RangeExpression($1, RangeExpression::Type::PLUS, $3); }
  | expression MCOLON expression { $$ = new RangeExpression($1, RangeExpression::Type::MINUS, $3); } 
  ;
          
/* A.8.4 Primaries */
primary
  : number { $$ = $1; }
  | hierarchical_identifier /* [exp]* [rexp]? inlined */ { 
    $$ = $1; 
  }
  | concatenation { $$ = $1; }
  | multiple_concatenation { $$ = $1; }
  /* TODO | function_call */
  /* TODO | system_function_call */
  | OPAREN mintypmax_expression CPAREN { $$ = $2; }
  | string_ { 
    $$ = $1; 
    parser->set_loc($$);
  }
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
  : PLUS   { $$ = UnaryExpression::Op::PLUS; }
  | MINUS  { $$ = UnaryExpression::Op::MINUS; }
  | BANG   { $$ = UnaryExpression::Op::BANG; }
  | TILDE  { $$ = UnaryExpression::Op::TILDE; }
  | AMP    { $$ = UnaryExpression::Op::AMP; }
  | TAMP   { $$ = UnaryExpression::Op::TAMP; }
  | PIPE   { $$ = UnaryExpression::Op::PIPE; }
  | TPIPE  { $$ = UnaryExpression::Op::TPIPE; }
  | CARAT  { $$ = UnaryExpression::Op::CARAT; }
  | TCARAT { $$ = UnaryExpression::Op::TCARAT; }
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
  : UNSIGNED_NUM { $$ = new Number($1, Number::Format::UNBASED, 32, true); parser->set_loc($$); }
  | DECIMAL_VALUE { $$ = new Number($1.second, Number::Format::DEC, 32, $1.first); parser->set_loc($$); }
  | size DECIMAL_VALUE { $$ = new Number($2.second, Number::Format::DEC, $1, $2.first); parser->set_loc($$); }
  /* TODO | [size] decimal_base x_digit _* */
  /* TODO | [size] decimal_base z_digit _* */
  ;
binary_number 
  : BINARY_VALUE { $$ = new Number($1.second, Number::Format::BIN, 32, $1.first); parser->set_loc($$); }
  | size BINARY_VALUE { $$ = new Number($2.second, Number::Format::BIN, $1, $2.first); parser->set_loc($$); }
  ;
octal_number 
  : OCTAL_VALUE { $$ = new Number($1.second, Number::Format::OCT, 32, $1.first); parser->set_loc($$); }
  | size OCTAL_VALUE { $$ = new Number($2.second, Number::Format::OCT, $1, $2.first); parser->set_loc($$); }
  ;
hex_number 
  : HEX_VALUE { $$ = new Number($1.second, Number::Format::HEX, 32, $1.first); parser->set_loc($$); }
  | size HEX_VALUE { $$ = new Number($2.second, Number::Format::HEX, $1, $2.first); parser->set_loc($$); }
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
  : attr_name { $$ = new AttrSpec($1, nullptr); }
  | attr_name EQ expression { $$ = new AttrSpec($1, $3); }
  ;
attr_name
  : identifier { $$ = $1; }
  ;

/* A.9.3 Identifiers */
/* NOTE: This parse deviates from the spec. This is only okay given the */
/* contexts in which we use it: primary, net_lvalue, and variable_lvalue */
hierarchical_identifier
  : simple_id_L braced_rexp_S { 
    $$ = new Identifier(new Id($1.second), $2.begin(), $2.end()); 
    parser->set_loc($$, $1.first);
    for (auto e : $2) {
      if ((e != $2.back()) && e->is(Node::Tag::range_expression)) {
        error(parser->get_loc(), "Unexpected range expression in array subscript");
        YYERROR;
      }
    }
  }
  | hierarchical_identifier DOT SIMPLE_ID braced_rexp_S {
    $$ = $1;
    if ($$->size_dim() > 1) {
      error(parser->get_loc(), "Unexpected multiplie instance selects");
      YYERROR;
    }
    if (!$$->empty_dim()) {
      if ($$->back_dim()->is(Node::Tag::range_expression)) {
        error(parser->get_loc(), "Unexpected range expression in array subscript");
        YYERROR;
      }
      $$->back_ids()->replace_isel($$->remove_back_dim());
    }
    $$->push_back_ids(new Id($3));
    $$->purge_dim();
    $$->push_back_dim($4.begin(), $4.end());
    for (auto e : $4) {
      if ((e != $4.back()) && e->is(Node::Tag::range_expression)) {
        error(parser->get_loc(), "Unexpected range expression in array subscript");
        YYERROR;
      }
    }
  }
  ;
identifier 
  : simple_id_L { 
    $$ = new Identifier(new Id($1.second)); 
    parser->set_loc($$, $1.first);
  }
  /* TODO | ESCAPED_ID */
  ;

/* Auxiliary Rules */
attr_spec_P
  : attr_spec { 
    $$.push_back($1); 
  }
  | attr_spec_P COMMA attr_spec {
    $$ = $1;
    $$.push_back($3);
  }
  ;
attribute_instance_S 
  : %empty { $$ = new Attributes(); }
  | attribute_instance_S attribute_instance {
    $$ = $1;
    $$->push_back_as($2.begin(), $2.end());
  }
  ;
block_item_declaration_S
  : %empty { }
  | block_item_declaration_S block_item_declaration { 
    $$ = $1;
    $$.insert($$.end(), $2.begin(), $2.end());
  }
  ;
braced_rexp_S
  : %empty { }
  | braced_rexp_S OSQUARE range_expression CSQUARE { 
    $$ = $1;
    $$.push_back($3); 
  }
  ;
case_generate_item_P
  : case_generate_item { 
    $$.push_back($1); 
  }
  | case_generate_item_P case_generate_item { 
    $$ = $1;
    $$.push_back($2);
  }
  ;
case_item_P
  : case_item { 
    $$.push_back($1); 
  }
  | case_item_P case_item {
    $$ = $1;
    $$.push_back($2);
  }
  ;
colon_Q
  : %empty
  | COLON
  ;
delay3_Q
  : %empty { $$ = nullptr; }
  | delay3 { $$ = $1; }
  ;
delay_or_event_control_Q
  : %empty { $$ = nullptr; }
  | delay_or_event_control { $$ = $1; }
  ;
dimension_S
  : %empty { }
  | dimension_S dimension { 
    $$ = $1;
    $$.push_back($2);
  }
  ;
eq_ce_Q
  : %empty { $$ = new Identifier("__null"); }
  | EQ expression { $$ = $2; }
  ;
expression_P
  : expression { 
    $$.push_back($1); 
  }
  | expression_P COMMA expression {
    $$ = $1;
    $$.push_back($3);
  }
  ;
expression_Q
  : %empty { $$ = nullptr; }
  | expression { $$ = $1; }
  ;
generate_block_id_Q
  : %empty { $$ = nullptr; }
  | COLON identifier { $$ = $2; }
  ;
integer_L
  : INTEGER { $$ = parser->get_loc().begin.line; }
  ;
list_of_port_declarations_Q 
  : %empty { }
  | list_of_port_declarations { $$ = $1; }
  ;
localparam_L
  : LOCALPARAM { $$ = parser->get_loc().begin.line; }
  ;
mintypmax_expression_Q 
  : %empty { $$ = nullptr; }
  | mintypmax_expression { $$ = $1; }
  ;
module_instance_P
  : module_instance { 
    $$.push_back($1); 
  }
  | module_instance_P COMMA module_instance {
    $$ = $1;
    $$.push_back($3);
  }
  ;
module_item_S
  : %empty { }
  | module_item_S module_item { 
    $$ = $1;
    $$.insert($$.end(), $2.begin(), $2.end());
  }
  ;
module_keyword_L
  : module_keyword { $$ = parser->get_loc().begin.line; }
  ;
module_or_generate_item_S
  : %empty { }
  | module_or_generate_item_S module_or_generate_item {
    $$ = $1;
    $$.insert($$.end(), $2.begin(), $2.end());
  }
  ;
named_parameter_assignment_P
  : named_parameter_assignment { 
    $$.push_back($1);
  }
  | named_parameter_assignment_P COMMA named_parameter_assignment { 
    $$ = $1;
    $$.push_back($3);
  }
  ;
named_port_connection_P
  : named_port_connection { 
    $$.push_back($1);
  }
  | named_port_connection_P COMMA named_port_connection {
    $$ = $1;
    $$.push_back($3);
  }
  ;
net_type_L
  : net_type { $$ = std::make_pair(parser->get_loc().begin.line, $1); }
  ;
net_type_Q
  : %empty { $$ = NetDeclaration::Type::WIRE; }
  | net_type { $$ = $1; }
  ;
non_port_module_item_S
  : %empty { }
  | non_port_module_item_S non_port_module_item {
    $$ = $1;
    $$.insert($$.end(), $2.begin(), $2.end());
  }
  ;
ordered_parameter_assignment_P
  : ordered_parameter_assignment { 
    $$.push_back($1);
  }
  | ordered_parameter_assignment_P COMMA ordered_parameter_assignment { 
    $$ = $1;
    $$.push_back($3);
  }
  ;
ordered_port_connection_P
  : ordered_port_connection { 
    $$.push_back($1);
  }
  | ordered_port_connection_P COMMA ordered_port_connection {
    $$ = $1;
    $$.push_back($3);
  }
  ;
parameter_declaration_P
  : parameter_declaration_P COMMA alt_parameter_declaration {
    $$ = $1;
    $$.push_back($3);
  } 
  | parameter_declaration_P COMMA list_of_param_assignments {
    $$ = $1;
    for (auto va : $3) {
      assert($1.back()->is(Node::Tag::parameter_declaration));
      auto pd = static_cast<ParameterDeclaration*>($1.back()->clone());
      pd->replace_id(va->get_lhs()->clone());
      pd->replace_val(va->get_rhs()->clone());
      delete va; 
      $$.push_back(pd);
    }
  }
  | alt_parameter_declaration {
    $$.push_back($1);
  }
  ;
parameter_L
  : PARAMETER { $$ = parser->get_loc().begin.line; }
  ;
parameter_value_assignment_Q
  : %empty { }
  | parameter_value_assignment { $$ = $1; }
  ;
port_declaration_P
  : port_declaration_P COMMA alt_port_declaration {
    $$ = $1;
    $$.push_back($3);
  }
  | port_declaration_P COMMA list_of_variable_port_identifiers {
    $$ = $1;
    for (auto va : $3) {
      assert($1.back()->is(Node::Tag::port_declaration));
      auto pd = static_cast<PortDeclaration*>($1.back()->clone());
      if (pd->get_decl()->is(Node::Tag::net_declaration)) {
        auto nd = static_cast<NetDeclaration*>(pd->get_decl());
        nd->replace_id(va->get_lhs()->clone());
        if (!is_null(va->get_rhs())) {
          error(parser->get_loc(), "Found initialization value in net declaration!");
          YYERROR;
        }
      } else if (pd->get_decl()->is(Node::Tag::reg_declaration)) {
        auto rd = static_cast<RegDeclaration*>(pd->get_decl());
        rd->replace_id(va->get_lhs()->clone());
        if (!is_null(va->get_rhs())) {
          rd->replace_val(va->get_rhs()->clone());
        }
      } else {
        assert(false);
      }
      delete va;
      $$.push_back(pd);
    }
  }
  | alt_port_declaration {
    $$.push_back($1);
  }
  ;
port_P
  : port { 
    $$.push_back($1);
  }
  | port_P COMMA port {
    $$ = $1;
    $$.push_back($3);
  }
  ;
range_Q
  : %empty { $$ = nullptr; }
  | range { $$ = $1; }
  ;
reg_L
  : REG { $$ = parser->get_loc().begin.line; }
  ;
signed_Q
  : %empty { $$ = false; }
  | SIGNED { $$ = true; }
  ;
simple_id_L
  : SIMPLE_ID { $$ = make_pair(parser->get_loc().begin.line, $1); }
statement_S
  : %empty { }
  | statement_S statement {
    $$ = $1;
    $$.push_back($2);
  }
  ;

alt_parameter_declaration
  : attribute_instance_S PARAMETER signed_Q range_Q param_assignment {
    $$ = new ParameterDeclaration($1, $3, $4, $5->get_lhs()->clone(), $5->get_rhs()->clone());
    delete $5;
  }
  | attribute_instance_S PARAMETER parameter_type param_assignment {
    $$ = new ParameterDeclaration($1, false, nullptr, $4->get_lhs()->clone(), $4->get_rhs()->clone());
    delete $4;
  }
  ;

alt_port_declaration 
  : alt_port_type alt_net_type signed_Q range_Q identifier eq_ce_Q {
    // If this is a net declaration and we have an initial value, it's an error
    if (!$2 && !is_null($6)) {
      error(parser->get_loc(), "Found initialization value in net declaration!");
      YYERROR;
    }
    auto d = $2 ? 
      (Declaration*) new RegDeclaration(new Attributes(), $5, $3, $4, is_null($6) ? nullptr : $6->clone()) :
      (Declaration*) new NetDeclaration(new Attributes(), NetDeclaration::Type::WIRE, nullptr, $5, $3, $4);
    delete $6;
    $$ = new PortDeclaration(new Attributes(), $1, d);
  }
  ;
alt_port_type
  : INOUT { $$ = PortDeclaration::Type::INOUT; }
  | INPUT { $$ = PortDeclaration::Type::INPUT; }
  | OUTPUT { $$ = PortDeclaration::Type::OUTPUT; }
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

  if (parser->get_path() == "<top>") {
    ss << "In final line of user input:\n";
  } else {
    ss << "In " << parser->get_path() << " on line " << l.end.line << ":\n";
  }
  ss << m;
  parser->log_->error(ss.str());
}

} // namespace cascade
