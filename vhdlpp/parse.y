
%pure-parser
%lex-param { yyscan_t yyscanner }
%parse-param {yyscan_t yyscanner  }
%parse-param {const char*file_path}
%parse-param {bool parsing_work   }
%{
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include "vhdlpp_config.h"
# include "vhdlint.h"
# include "vhdlreal.h"
# include "compiler.h"
# include "parse_api.h"
# include "parse_misc.h"
# include "architec.h"
# include "expression.h"
# include "sequential.h"
# include  "package.h"
# include "vsignal.h"
# include "vtype.h"
# include  <cstdarg>
# include  <cstring>
# include  <list>
# include  <stack>
# include  <map>
# include  <vector>
# include  "parse_types.h"
# include  <assert.h>

inline void FILE_NAME(LineInfo*tmp, const struct yyltype&where)
{
      tmp->set_lineno(where.first_line);
      tmp->set_file(filename_strings.make(where.text));
}


/* Recent version of bison expect that the user supply a
   YYLLOC_DEFAULT macro that makes up a yylloc value from existing
   values. I need to supply an explicit version to account for the
   text field, that otherwise won't be copied. */
# define YYLLOC_DEFAULT(Current, Rhs, N)  do {       \
  (Current).first_line   = (Rhs)[1].first_line;      \
  (Current).text         = file_path; /*(Rhs)[1].text;*/   } while (0)

static void yyerror(YYLTYPE*yyllocp,yyscan_t yyscanner,const char*file_path,bool, const char*msg);

int parse_errors = 0;
int parse_sorrys = 0;

/*
 * The parser calls yylex to get the next lexical token. It is only
 * called by the bison-generated parser.
 */
extern int yylex(union YYSTYPE*yylvalp,YYLTYPE*yyllocp,yyscan_t yyscanner);


/*
 * Create an initial scope that collects all the global
 * declarations. Also save a stack of previous scopes, as a way to
 * manage lexical scopes.
 */
static ActiveScope*active_scope = new ActiveScope;
static stack<ActiveScope*> scope_stack;

/*
 * When a scope boundary starts, call the push_scope function to push
 * a scope context. Preload this scope context with the contents of
 * the parent scope, then make this the current scope. When the scope
 * is done, the pop_scope function pops the scope off the stack and
 * resumes the scope that was the parent.
 */
static void push_scope(void)
{
      assert(active_scope);
      scope_stack.push(active_scope);
      active_scope = new ActiveScope (active_scope);
}

static void pop_scope(void)
{
      delete active_scope;
      assert(scope_stack.size() > 0);
      active_scope = scope_stack.top();
      scope_stack.pop();
}


void preload_global_types(void)
{
      generate_global_types(active_scope);
}

//Remove the scope created at the beginning of parser's work.
//After the parsing active_scope should keep it's address

static void delete_global_scope(void)
{
    active_scope->destroy_global_scope();
    delete active_scope;
}

//delete global entities that were gathered over the parsing process 
static void delete_design_entities(void)
{
      for(map<perm_string,Entity*>::iterator cur = design_entities.begin()
      ; cur != design_entities.end(); ++cur)
        delete cur->second;
}

//clean the mess caused by the parser
void parser_cleanup(void)
{
    delete_design_entities();
    delete_global_scope();
    lex_strings.cleanup();
}

const VType*parse_type_by_name(perm_string name)
{
      return active_scope->find_type(name);
}

%}


%union {
      port_mode_t port_mode;

      char*text;

      std::list<perm_string>* name_list;
      std::vector<perm_string>* compound_name;
      std::list<std::vector<perm_string>* >* compound_name_list;

      bool flag;
      int64_t uni_integer;
      double  uni_real;

      Expression*expr;
      std::list<Expression*>* expr_list;

      SequentialStmt* sequ;
      std::list<SequentialStmt*>*sequ_list;

      IfSequential::Elsif*elsif;
      std::list<IfSequential::Elsif*>*elsif_list;
      
      CaseSeqStmt::CaseStmtAlternative* case_alt;
      std::list<CaseSeqStmt::CaseStmtAlternative*>* case_alt_list;

      named_expr_t*named_expr;
      std::list<named_expr_t*>*named_expr_list;
      entity_aspect_t* entity_aspect;
      instant_list_t* instantiation_list;
      std::pair<instant_list_t*, ExpName*>* component_specification;

      const VType* vtype;

      range_t* range;

      std::list<InterfacePort*>* interface_list;

      Architecture::Statement* arch_statement;
      std::list<Architecture::Statement*>* arch_statement_list;
};

  /* The keywords are all tokens. */
%token K_abs K_access K_after K_alias K_all K_and K_architecture
%token K_array K_assert K_assume K_assume_guarantee K_attribute
%token K_begin K_block K_body K_buffer K_bus
%token K_case K_component K_configuration K_constant K_context K_cover
%token K_default K_disconnect K_downto
%token K_else K_elsif K_end K_entity K_exit
%token K_fairness K_file K_for K_force K_function
%token K_generate K_generic K_group K_guarded
%token K_if K_impure K_in K_inertial K_inout K_is
%token K_label K_library K_linkage K_literal K_loop
%token K_map K_mod
%token K_nand K_new K_next K_nor K_not K_null
%token K_of K_on K_open K_or K_others K_out
%token K_package K_parameter K_port K_postponed K_procedure K_process
%token K_property K_protected K_pure
%token K_range K_record K_register K_reject K_release K_rem K_report
%token K_restrict K_restrict_guarantee K_return K_rol K_ror
%token K_select K_sequence K_severity K_signal K_shared
%token K_sla K_sll K_sra K_srl K_strong K_subtype
%token K_then K_to K_transport K_type
%token K_unaffected K_units K_until K_use
%token K_variable K_vmode K_vprop K_vunit
%token K_wait K_when K_while K_with
%token K_xnor K_xor
 /* Identifiers that are not keywords are identifiers. */
%token <text> IDENTIFIER
%token <uni_integer> INT_LITERAL
%token <uni_real> REAL_LITERAL
%token <text> STRING_LITERAL CHARACTER_LITERAL BITSTRING_LITERAL
 /* compound symbols */
%token LEQ GEQ VASSIGN NE BOX EXP ARROW DLT DGT CC M_EQ M_NE M_LT M_LEQ M_GT M_GEQ

 /* The rules may have types. */

%type <flag> direction

%type <interface_list> interface_element interface_list entity_header
%type <interface_list> port_clause port_clause_opt
%type <port_mode>  mode

%type <entity_aspect> entity_aspect entity_aspect_opt binding_indication binding_indication_semicolon_opt
%type <instantiation_list> instantiation_list
%type <component_specification> component_specification

%type <arch_statement> concurrent_statement component_instantiation_statement concurrent_signal_assignment_statement
%type <arch_statement> process_statement
%type <arch_statement_list> architecture_statement_part

%type <expr> choice expression factor primary relation
%type <expr> expression_logical expression_logical_and expression_logical_or
%type <expr> expression_logical_xnor expression_logical_xor
%type <expr> name 
%type <expr> shift_expression simple_expression term waveform_element

%type <expr_list> waveform waveform_elements
%type <expr_list> name_list
%type <expr_list> process_sensitivity_list process_sensitivity_list_opt

%type <named_expr> association_element
%type <named_expr_list> association_list port_map_aspect port_map_aspect_opt

%type <vtype> subtype_indication

%type <text> architecture_body_start package_declaration_start
%type <text> identifier_opt identifier_colon_opt logical_name suffix
%type <name_list> logical_name_list identifier_list
%type <compound_name> prefix selected_name
%type <compound_name_list> selected_names use_clause

%type <sequ_list> sequence_of_statements if_statement_else
%type <sequ> sequential_statement if_statement signal_assignment_statement
%type <sequ> case_statement procedure_call procedure_call_statement
%type <sequ> loop_statement

%type <range> range

%type <case_alt> case_statement_alternative
%type <case_alt_list> case_statement_alternative_list

%type <elsif> if_statement_elsif
%type <elsif_list> if_statement_elsif_list if_statement_elsif_list_opt

%%

 /* The design_file is the root for the VHDL parse. This rule is also
    where I put some run-time initializations. */
design_file : { yylloc.text = file_path; } design_units ;

architecture_body
  : architecture_body_start
    K_of IDENTIFIER
    K_is block_declarative_items_opt
    K_begin architecture_statement_part K_end K_architecture_opt identifier_opt ';'
      { Architecture*tmp = new Architecture(lex_strings.make($1),
					    *active_scope, *$7);
	FILE_NAME(tmp, @1);
	bind_architecture_to_entity($3, tmp);
	if ($10 && tmp->get_name() != $10)
	      errormsg(@1, "Architecture name doesn't match closing name.\n");
	delete[]$1;
	delete[]$3;
	delete $7;
	pop_scope();
	if ($10) delete[]$10;
      }
  | architecture_body_start
    K_of IDENTIFIER
    K_is block_declarative_items_opt
    K_begin error K_end K_architecture_opt identifier_opt ';'
      { errormsg(@8, "Errors in architecture statements.\n");
	yyerrok;
	pop_scope();
      }
  ;

architecture_body_start
  : K_architecture IDENTIFIER
      { $$ = $2;
	push_scope();
      }
  ;
/*
 * The architecture_statement_part is a list of concurrent
 * statements.
 */
architecture_statement_part
  : architecture_statement_part concurrent_statement
      { std::list<Architecture::Statement*>*tmp = $1;
	if ($2) tmp->push_back($2);
	$$ = tmp;
      }
  | concurrent_statement
      { std::list<Architecture::Statement*>*tmp = new std::list<Architecture::Statement*>;
	if ($1) tmp->push_back($1);
	$$ = tmp;
      }

  | error ';'
      { $$ = 0;
	errormsg(@1, "Syntax error in architecture statement.\n");
	yyerrok;
      }
  ;

association_element
  : IDENTIFIER ARROW expression
      { named_expr_t*tmp = new named_expr_t(lex_strings.make($1), $3);
	delete[]$1;
	$$ = tmp;
	}
  | IDENTIFIER ARROW K_open
      { named_expr_t*tmp = new named_expr_t(lex_strings.make($1), 0);
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER ARROW error
      { errormsg(@3, "Invalid target for port map association.\n");
	yyerrok;
	$$ = 0;
      }
  ;

association_list
  : association_list ',' association_element
      { std::list<named_expr_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | association_element
      { std::list<named_expr_t*>*tmp = new std::list<named_expr_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

binding_indication
  : K_use entity_aspect_opt
   port_map_aspect_opt
   generic_map_aspect_opt
      { //TODO: do sth with generic map aspect
   $$ = $2;
      }
  ;

binding_indication_semicolon_opt
  : binding_indication ';' { $$ = $1; }
  | { $$ = 0; }
  ;

block_configuration
  : K_for IDENTIFIER
    use_clauses_opt
    configuration_items_opt
    K_end K_for ';'
    { delete[] $2; }
 ;

block_configuration_opt
  : block_configuration
  |
  ;

block_declarative_item
  : K_signal identifier_list ':' subtype_indication ';'
      { /* Save the signal declaration in the block_signals map. */
	for (std::list<perm_string>::iterator cur = $2->begin()
		   ; cur != $2->end() ; ++cur) {
	      Signal*sig = new Signal(*cur, $4);
	      FILE_NAME(sig, @1);
	      active_scope->bind_name(*cur, sig);
	}
	delete $2;
      }

  | component_declaration

  | constant_declaration

  | use_clause_lib

      /* Various error handling rules for block_declarative_item... */

  | K_signal error ';'
      { errormsg(@2, "Syntax error declaring signals.\n"); yyerrok; }
  | error ';'
      { errormsg(@1, "Syntax error in block declarations.\n"); yyerrok; }

  ;

/*
 * The block_declarative_items rule matches "{ block_declarative_item }"
 * which is a synonym for "architecture_declarative_part" and
 * "block_declarative_part".
 */
block_declarative_items
  : block_declarative_items block_declarative_item
  | block_declarative_item
  ;

block_declarative_items_opt
  : block_declarative_items
  |
  ;
case_statement
  : K_case expression K_is
    case_statement_alternative_list
    K_end K_case ';'
      {
    sorrymsg(@1, "Case statement is not yet supported");
    CaseSeqStmt* tmp = new CaseSeqStmt($2, $4);
    FILE_NAME(tmp, @1);
    delete $4;
    $$ = tmp;
      }
    ;
case_statement_alternative_list
  : case_statement_alternative_list case_statement_alternative
      { std::list<CaseSeqStmt::CaseStmtAlternative*>* tmp = $1;
    tmp->push_back($2);
    $$ = tmp;
      }
  | case_statement_alternative
      {
    std::list<CaseSeqStmt::CaseStmtAlternative*>*tmp = 
        new std::list<CaseSeqStmt::CaseStmtAlternative*>();
    tmp->push_back($1);
    $$ = tmp;
      }
   ;

case_statement_alternative
  : K_when choice ARROW sequence_of_statements
      {
    CaseSeqStmt::CaseStmtAlternative* tmp = 
        new CaseSeqStmt::CaseStmtAlternative($2, $4);
    FILE_NAME(tmp, @1);
    delete $4;
    $$ = tmp;
      }
   ;

choice
  : simple_expression
      { $$ = $1;}
  | K_others
      { $$ = 0; }
  ;

component_configuration
  : K_for component_specification
    binding_indication_semicolon_opt
    block_configuration_opt
    K_end K_for ';'
      {
    sorrymsg(@1, "Component configuration in not yet supported");
    if($3) delete $3;
    delete $2;
      }
  | K_for component_specification error K_end K_for
      {
    errormsg(@1, "Error in component configuration statement.\n");
    delete $2;
      }
  ;

component_declaration
  : K_component IDENTIFIER K_is_opt
    port_clause_opt
    K_end K_component identifier_opt ';'
      { perm_string name = lex_strings.make($2);
	if($7 && name != $7) {
	      errormsg(@7, "Identifier %s doesn't match component name %s.\n",
		       $7, name.str());
	}

	ComponentBase*comp = new ComponentBase(name);
	if ($4) {
        comp->set_interface($4);
        delete $4;
    }
	active_scope->bind_name(name, comp);
	delete[]$2;
	if ($7) delete[] $7;
      }

  | K_component IDENTIFIER K_is_opt error K_end K_component identifier_opt ';'
      { errormsg(@4, "Syntax error in component declaration.\n");
	delete[] $2;
	if($7) delete[] $7;
	yyerrok;
      }
  ;

component_instantiation_statement
  : IDENTIFIER ':' K_component_opt IDENTIFIER port_map_aspect_opt ';'
      { perm_string iname = lex_strings.make($1);
	perm_string cname = lex_strings.make($4);
	ComponentInstantiation*tmp = new ComponentInstantiation(iname, cname, $5);
    delete $5;
	FILE_NAME(tmp, @1);
	delete[]$1;
	delete[]$4;
	$$ = tmp;
      }
  | IDENTIFIER ':' K_component_opt IDENTIFIER error ';'
      { errormsg(@4, "Errors in component instantiation.\n");
	delete[]$1;
	delete[]$4;
	$$ = 0;
      }
  ;

component_specification
  : instantiation_list ':' name
      {
    ExpName* name = dynamic_cast<ExpName*>($3);
    std::pair<instant_list_t*, ExpName*>* tmp = new std::pair<instant_list_t*, ExpName*>($1, name);
    $$ = tmp;
      }
  ;

concurrent_signal_assignment_statement
  : name LEQ waveform ';'
      { ExpName*name = dynamic_cast<ExpName*> ($1);
	assert(name);
	SignalAssignment*tmp = new SignalAssignment(name, *$3);
	FILE_NAME(tmp, @1);

	$$ = tmp;
	delete $3;
      }
  | name LEQ waveform K_when expression K_else waveform ';'
      { ExpConditional*tmp = new ExpConditional($5, $3, $7);
	FILE_NAME(tmp, @3);
	delete $3;
	delete $7;

        ExpName*name = dynamic_cast<ExpName*> ($1);
	assert(name);
	SignalAssignment*tmpa = new SignalAssignment(name, tmp);
	FILE_NAME(tmpa, @1);

	$$ = tmpa;
      }
  | name LEQ error ';'
      { errormsg(@2, "Syntax error in signal assignment waveform.\n");
	delete $1;
	$$ = 0;
	yyerrok;
      }
  ;

concurrent_statement
  : component_instantiation_statement
  | concurrent_signal_assignment_statement
  | process_statement
  ;

configuration_declaration
  : K_configuration IDENTIFIER K_of IDENTIFIER K_is
  configuration_declarative_part
  block_configuration
  K_end K_configuration_opt identifier_opt ';'
     {
  if(design_entities.find(lex_strings.make($4)) == design_entities.end())
      errormsg(@4, "Couldn't find entity %s used in configuration declaration", $4);
  //choose_architecture_for_entity();
  sorrymsg(@1, "Configuration declaration is not yet supported.\n");
     }
  | K_configuration error K_end K_configuration_opt identifier_opt ';'
      { errormsg(@2, "Too many errors, giving up on configuration declaration.\n");
    if($5) delete $5;
    yyerrok;
      }
  ;
//TODO: this list is only a sketch. It must be filled out later
configuration_declarative_item
  : use_clause
  ;

configuration_declarative_items
  : configuration_declarative_items configuration_declarative_item
  | configuration_declarative_item
  ;

configuration_declarative_part
  : configuration_declarative_items
  |
  ;

configuration_item
  : block_configuration
  | component_configuration
  ;

configuration_items
  : configuration_items configuration_item
  | configuration_item
  ;

configuration_items_opt
  : configuration_items
  |
  ;

constant_declaration
  : K_constant identifier_list ':' subtype_indication VASSIGN expression ';'
      { // The syntax allows mutliple names to have the same type/value.
	for (std::list<perm_string>::iterator cur = $2->begin()
		   ; cur != $2->end() ; ++cur) {
	      active_scope->bind_name(*cur, $4, $6);
	}
	delete $2;
      }
  | K_constant identifier_list ':' subtype_indication ';'
      { sorrymsg(@1, "Deferred constant declarations not supported\n");
	delete $2;
      }
  ;

context_clause : context_items | ;

context_item
  : library_clause
  | use_clause_lib
  ;

context_items
  : context_items context_item
  | context_item
  ;


design_unit
  : context_clause library_unit
  | error { errormsg(@1, "Invalid design_unit\n"); }
  ;

design_units
  : design_units design_unit
  | design_unit
  ;

  /* Indicate the direction as a flag, with "downto" being TRUE. */
direction : K_to { $$ = false; } | K_downto { $$ = true; } ;

  /* As an entity is declared, add it to the map of design entities. */
entity_aspect
  : K_entity name
      {
    ExpName* name = dynamic_cast<ExpName*>($2);
    entity_aspect_t* tmp = new entity_aspect_t(entity_aspect_t::ENTITY, name);
    $$ = tmp;
      }
  | K_configuration name
      {
    ExpName* name = dynamic_cast<ExpName*>($2);
    entity_aspect_t* tmp = new entity_aspect_t(entity_aspect_t::CONFIGURATION, name);
    $$ = tmp;
      }
  | K_open
      {
    entity_aspect_t* tmp = new entity_aspect_t(entity_aspect_t::OPEN, 0);
    $$ = tmp;
      }
  ;

entity_aspect_opt
  : entity_aspect { $$ = $1; }
  | { $$ = 0; }
  ;

entity_declaration
  : K_entity IDENTIFIER K_is entity_header K_end K_entity_opt identifier_opt';'
      { Entity*tmp = new Entity(lex_strings.make($2));
	FILE_NAME(tmp, @1);
	  // Transfer the ports
	std::list<InterfacePort*>*ports = $4;
	tmp->set_interface(ports);
	delete ports;
	  // Save the entity in the entity map.
	design_entities[tmp->get_name()] = tmp;
	delete[]$2;
    if($7) {
        if(tmp->get_name() != $7) {
            errormsg(@1, "Syntax error in entity clause. Closing name doesn't match.\n");
            yyerrok;
        }
        delete[]$7;
    }
      }
  | K_entity error K_end K_entity_opt identifier_opt ';'
      { errormsg(@1, "Too many errors, giving up on entity declaration.\n");
	yyerrok;
	if ($5) delete[]$5;
      }
  ;

entity_header
  : port_clause
      { $$ = $1; }
  ;

expression
  : expression_logical
      { $$ = $1; }
  ;

/*
 * The expression_logical matches the logical_expression from the
 * standard. Note that this is a little more tricky then usual because
 * of the strange VHDL rules for logical expressions. We have to
 * account for things like this:
 *
 *        <exp> and <exp> and <exp>...
 *
 * which is matched by the standard rule:
 *
 *        logical_expression ::=
 *           ...
 *             relation { and relation }
 *
 * The {} is important, and implies that "and" can be strung together
 * with other "and" operators without parentheses. This is true for
 * "and", "or", "xor", and "xnor". The tricky part is that these
 * cannot be mixed. For example, this is not OK:
 *
 *        <exp> and <exp> or <exp>
 *
 * Also note that "nand" and "nor" can not be chained in this manner.
 */
expression_logical
  : relation { $$ = $1; }
  | relation K_and expression_logical_and
      { ExpLogical*tmp = new ExpLogical(ExpLogical::AND, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_or expression_logical_or
      { ExpLogical*tmp = new ExpLogical(ExpLogical::OR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_xor expression_logical_xor
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_nand relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::NAND, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_nor relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::NOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_xnor expression_logical_xnor
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XNOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_logical_and
  : relation
      { $$ = $1; }
  | expression_logical_and K_and relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::AND, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_logical_or
  : relation
      { $$ = $1; }
  | expression_logical_or K_or relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::OR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_logical_xnor
  : relation
      { $$ = $1; }
  | expression_logical_xnor K_xnor relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XNOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_logical_xor
  : relation
      { $$ = $1; }
  | expression_logical_xor K_xor relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

factor
  : primary
      { $$ = $1; }
  | primary EXP primary
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::POW, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | K_abs primary
      { ExpUAbs*tmp = new ExpUAbs($2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_not primary
      { ExpUNot*tmp = new ExpUNot($2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;
generic_map_aspect_opt
  : generic_map_aspect
  |
  ;

generic_map_aspect
  : K_generic K_map '(' association_list ')'
      {
    sorrymsg(@1, "Generic map aspect not yet supported.\n");
      }
  ;

identifier_list
  : identifier_list ',' IDENTIFIER
      { std::list<perm_string>* tmp = $1;
	tmp->push_back(lex_strings.make($3));
	delete[]$3;
	$$ = tmp;
      }
  | IDENTIFIER
      { std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  ;

identifier_opt : IDENTIFIER { $$ = $1; } |  { $$ = 0; } ;

identifier_colon_opt : IDENTIFIER ':' { $$ = $1; } | { $$ = 0; };

if_statement
  : K_if expression K_then sequence_of_statements
    if_statement_elsif_list_opt if_statement_else
    K_end K_if ';'
      { IfSequential*tmp = new IfSequential($2, $4, $5, $6);
	FILE_NAME(tmp, @1);
	delete $4;
	delete $5;
	delete $6;
	$$ = tmp;
      }

  | K_if error K_then sequence_of_statements
    if_statement_elsif_list_opt if_statement_else
    K_end K_if ';'
      { errormsg(@2, "Error in if_statement condition expression.\n");
	yyerrok;
	$$ = 0;
	delete $4;
      }

  | K_if expression K_then error K_end K_if ';'
      { errormsg(@4, "Too many errors in sequence within if_statement.\n");
	yyerrok;
	$$ = 0;
      }

  | K_if error K_end K_if ';'
      { errormsg(@2, "Too many errors in if_statement.\n");
	yyerrok;
	$$ = 0;
      }
  ;

if_statement_elsif_list_opt
  : if_statement_elsif_list { $$ = $1; }
  |                         { $$ = 0;  }
  ;

if_statement_elsif_list
  : if_statement_elsif_list if_statement_elsif
      { list<IfSequential::Elsif*>*tmp = $1;
	tmp->push_back($2);
	$$ = tmp;
      }
  | if_statement_elsif
      { list<IfSequential::Elsif*>*tmp = new list<IfSequential::Elsif*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

if_statement_elsif
  : K_elsif expression K_then sequence_of_statements
      { IfSequential::Elsif*tmp = new IfSequential::Elsif($2, $4);
	FILE_NAME(tmp, @1);
	delete $4;
	$$ = tmp;
      }
  | K_elsif expression K_then error
      { errormsg(@4, "Too many errors in elsif sub-statements.\n");
	yyerrok;
	$$ = 0;
      }
  ;

if_statement_else
  : K_else sequence_of_statements
      { $$ = $2; }
  | K_else error
      { errormsg(@2, "Too many errors in else sub-statements.\n");
	yyerrok;
	$$ = 0;
      }
  |
      { $$ = 0; }
  ;

instantiation_list
  : identifier_list
     {
  instant_list_t* tmp = new instant_list_t(instant_list_t::NONE, $1);
  $$ = tmp;
     }
  | K_others
     {
  instant_list_t* tmp = new instant_list_t(instant_list_t::OTHERS, 0);
  $$ = tmp;
     }
  | K_all
   {
  instant_list_t* tmp = new instant_list_t(instant_list_t::ALL, 0);
  $$ = tmp;
   }
  ;

  /* The interface_element is also an interface_declaration */
interface_element
  : identifier_list ':' mode subtype_indication
      { std::list<InterfacePort*>*tmp = new std::list<InterfacePort*>;
	for (std::list<perm_string>::iterator cur = $1->begin()
		   ; cur != $1->end() ; ++cur) {
	      InterfacePort*port = new InterfacePort;
	      FILE_NAME(port, @1);
	      port->mode = $3;
	      port->name = *(cur);
	      port->type = $4;
	      tmp->push_back(port);
	}
	delete $1;
	$$ = tmp;
      }
  ;

interface_list
  : interface_list ';' interface_element
      { std::list<InterfacePort*>*tmp = $1;
	tmp->splice(tmp->end(), *$3);
	delete $3;
	$$ = tmp;
      }
  | interface_element
      { std::list<InterfacePort*>*tmp = $1;
	$$ = tmp;
      }
  ;

library_clause
  : K_library logical_name_list ';'
      { library_import(@1, $2);
	delete $2;
      }
  | K_library error ';'
     { errormsg(@1, "Syntax error in library clause.\n"); yyerrok; }
  ;

  /* Collapse the primary_unit and secondary_unit of the library_unit
     into this single set of rules. */
library_unit
  : primary_unit
  | secondary_unit
  ;

logical_name : IDENTIFIER { $$ = $1; } ;

logical_name_list
  : logical_name_list ',' logical_name
      { std::list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	delete[]$3;
	$$ = tmp;
      }
  | logical_name
      { std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  ;

loop_statement
  : identifier_colon_opt
    K_while expression_logical K_loop
    sequence_of_statements
    K_end K_loop identifier_opt ';'
      {
    if($1 && strcmp($1, $8))
            errormsg(@1, "Loop statement name doesn't match closing name.\n");
    if($1) delete[]$1;
    if($8) delete[]$8;

    ExpLogical* cond = dynamic_cast<ExpLogical*>($3);
    if(!cond) {
        errormsg(@3, "Iteration condition is not a correct logical expression");
    }
    WhileLoopStatement* tmp = new WhileLoopStatement(cond, $5);
    FILE_NAME(tmp, @1);

    sorrymsg(@1, "Loop statements are not supported");
    $$ = tmp;
      }
  | identifier_colon_opt K_for
    IDENTIFIER K_in range K_loop
    sequence_of_statements
    K_end K_loop identifier_opt ';'
      {
    if($1 && strcmp($1, $10))
        errormsg(@1, "Loop statement name doesn't match closing name.\n");
    if($1)  delete[] $1;
    if($10) delete[] $10;

    ForLoopStatement* tmp = new ForLoopStatement(lex_strings.make($3), $5, $7);
    delete[]$3;
    FILE_NAME(tmp, @1);

    sorrymsg(@1, "Loop statements are not supported");
    $$ = tmp;
      }
  | identifier_colon_opt K_loop
    sequence_of_statements
    K_end K_loop identifier_opt ';'
      {
    if($1 && strcmp($1, $6))
        errormsg(@1, "Loop statement name doesn't match closing name.\n");
    if($1) delete[]$1;
    if($6) delete[]$6;

    BasicLoopStatement* tmp = new BasicLoopStatement($3);
    FILE_NAME(tmp, @1);

    sorrymsg(@1, "Loop statements are not supported");
    $$ = tmp;
      };
      
mode
  : K_in  { $$ = PORT_IN; }
  | K_out { $$ = PORT_OUT; }
  ;

name
  : IDENTIFIER
      { ExpName*tmp = new ExpName(lex_strings.make($1));
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER '('  expression ')'
      { ExpName*tmp = new ExpName(lex_strings.make($1), $3);
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }
  ;

  /* Handle name lists as lists of expressions. */
name_list
  : name_list ',' name
      { std::list<Expression*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | name
      { std::list<Expression*>*tmp = new std::list<Expression*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

package_declaration
  : package_declaration_start K_is
    package_declarative_part_opt
    K_end K_package_opt identifier_opt ';'
      { perm_string name = lex_strings.make($1);
	if($6 && name != $6) {
	      errormsg(@1, "Identifier %s doesn't match package name %s.\n",
		       $6, name.str());
        }
	Package*tmp = new Package(name, *active_scope);
	FILE_NAME(tmp, @1);
	delete[]$1;
        if ($6) delete[]$6;
	pop_scope();
	  /* Put this package into the work library. */
	library_save_package(0, tmp, parsing_work);
      }
  | package_declaration_start K_is error K_end K_package_opt identifier_opt ';'
    { errormsg(@3, "Syntax error in package clause.\n");
      yyerrok;
      pop_scope();
    }
  ;

package_declaration_start
  : K_package IDENTIFIER
      { push_scope();
	$$ = $2;
      }
  ;

/* TODO: this list must be extended in the future
   presently it is only a sketch */
package_body_declarative_item
  : use_clause
  ;

package_body_declarative_items
  : package_body_declarative_items package_body_declarative_item
  | package_body_declarative_item
  ;
package_body_declarative_part_opt
  : package_body_declarative_items
  |
  ;

package_declarative_item
  : component_declaration
  | constant_declaration
  | subtype_declaration
  | use_clause
  ;

package_declarative_items
  : package_declarative_items package_declarative_item
  | package_declarative_item
  ;

package_declarative_part_opt
  : package_declarative_items
  |
  ;

package_body
  : K_package K_body IDENTIFIER K_is
    package_body_declarative_part_opt
    K_end K_package_opt identifier_opt ';'
      {
    sorrymsg(@1, "Package body is not yet supported.\n");
    delete[] $3;
    if($8) delete[] $8;
      }

  | K_package K_body IDENTIFIER K_is
    error
    K_end K_package_opt identifier_opt ';'
      {
    errormsg(@1, "Errors in package body.\n");
      }
  ;

port_clause
  : K_port '(' interface_list ')' ';'
      { $$ = $3; }
  | K_port '(' error ')' ';'
      { errormsg(@1, "Syntax error in port list.\n");
	yyerrok;
	$$ = 0;
      }
  ;

port_clause_opt : port_clause {$$ = $1;} | {$$ = 0;} ;

port_map_aspect
  : K_port K_map '(' association_list ')'
      { $$ = $4; }
  | K_port K_map '(' error ')'
      { errormsg(@1, "Syntax error in port map aspect.\n");
      }
  ;

port_map_aspect_opt
  : port_map_aspect  { $$ = $1; }
  |                  { $$ = 0; }
  ;

prefix
  : IDENTIFIER
      { std::vector<perm_string>* tmp = new std::vector<perm_string>();
	tmp->push_back(lex_strings.make($1));
	delete[] $1;
	$$ = tmp;
      }
  | STRING_LITERAL
      { std::vector<perm_string>* tmp = new std::vector<perm_string>();
	tmp->push_back(lex_strings.make($1));
	delete[] $1;
	$$ = tmp;
      }
  | selected_name
      { $$ = $1; }
  ;

primary
  : name
      { $$ = $1; }
  | name '\'' IDENTIFIER
      { perm_string name = lex_strings.make($3);
	ExpName*base = dynamic_cast<ExpName*> ($1);
	ExpAttribute*tmp = new ExpAttribute(base, name);
	FILE_NAME(tmp, @3);
	delete[]$3;
	$$ = tmp;
      }
  | CHARACTER_LITERAL
      { ExpCharacter*tmp = new ExpCharacter($1[0]);
	FILE_NAME(tmp,@1);
	delete[]$1;
	$$ = tmp;
      }
  | INT_LITERAL
      { ExpInteger*tmp = new ExpInteger($1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | STRING_LITERAL
      { ExpString*tmp = new ExpString($1);
    FILE_NAME(tmp,@1);
    delete[]$1;
    $$ = tmp;
      }
  | BITSTRING_LITERAL
      { ExpBitstring*tmp = new ExpBitstring($1);
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }
  | '(' expression ')'
      { $$ = $2; }
  ;

primary_unit
  : entity_declaration
  | configuration_declaration
  | package_declaration
  ;

procedure_call
  : IDENTIFIER
      {
    ProcedureCall* tmp = new ProcedureCall(lex_strings.make($1));
    sorrymsg(@1, "Procedure calls are not supported.\n");
    $$ = tmp;
      }
  | IDENTIFIER '(' association_list ')'
      {
    ProcedureCall* tmp = new ProcedureCall(lex_strings.make($1), $3);
    sorrymsg(@1, "Procedure calls are not supported.\n");
    $$ = tmp;
      }
  | IDENTIFIER '(' error ')'
      {
    errormsg(@1, "Errors in procedure call.\n");
    yyerrok;
    delete[]$1;
    $$ = 0;
      };

procedure_call_statement
  : IDENTIFIER ':' procedure_call { $$ = $3; }
  | procedure_call { $$ = $1; }
  ;

process_statement
  : IDENTIFIER ':' K_postponed_opt K_process
    process_sensitivity_list_opt K_is_opt
    K_begin sequence_of_statements
    K_end K_postponed_opt K_process identifier_opt ';'
      { perm_string iname = lex_strings.make($1);
	if ($12) {
	      if (iname != $12)
		    errormsg(@12, "Process name %s does not match opening name %s.\n",
			     $12, $1);
	      delete[]$12;
	}

	ProcessStatement*tmp = new ProcessStatement(iname, $5, $8);
	FILE_NAME(tmp, @4);
	delete $5;
	delete $8;
	$$ = tmp;
      }

  | IDENTIFIER ':' K_postponed_opt K_process
    process_sensitivity_list_opt K_is_opt
    K_begin error
    K_end K_postponed_opt K_process identifier_opt ';'
      { errormsg(@8, "Too many errors in process sequential statements.\n");
	yyerrok;
	$$ = 0;
      }
  ;

/*
 * A process_sentitivity_list is:
 *     <nil>  if the list is not present, or
 *     or a non-empty list of actual expressions.
 */
process_sensitivity_list_opt
  : '(' process_sensitivity_list ')'
      { $$ = $2; }
  | '(' error ')'
      { errormsg(@2, "Error in process sensitivity list\n");
	yyerrok;
	$$ = 0;
      }
  |
      { $$ = 0; }
  ;

process_sensitivity_list
  : K_all
      { std::list<Expression*>*tmp = new std::list<Expression*>;
	ExpName*all = new ExpNameALL;
	FILE_NAME(all, @1);
	tmp->push_back(all);
	$$ = tmp;
      }
  | name_list
      { $$ = $1; }
  ;
range
  : simple_expression direction simple_expression
      {
    range_t* tmp = new range_t($1, $3, $2);
    $$ = tmp;
      }
  ;

relation
  : shift_expression
      { $$ = $1; }
  | shift_expression '=' shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::EQ, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression '<' shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::LT, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression '>' shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::GT, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression LEQ shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::LE, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression GEQ shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::GE, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression NE shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::NEQ, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

secondary_unit
  : architecture_body
  | package_body
  ;

selected_name
  : prefix '.' suffix
      {
    std::vector<perm_string>* tmp = $1;
    tmp->push_back(lex_strings.make($3));
    delete[] $3;

    $$ = tmp;
      }
  ;

selected_names
  : selected_names ',' selected_name
      {
    std::list<std::vector<perm_string>* >* tmp = $1;
    tmp->push_back($3);
    $$ = tmp;
      }
  | selected_name
      {
    std::list<std::vector<perm_string>* >* tmp = new std::list<std::vector<perm_string>* >();
    tmp->push_back($1);
    $$ = tmp;
      }
  ;

  /* The *_use variant of selected_name is used by the "use"
     clause. It is syntactically identical to other selected_name
     rules, but is a convenient place to attach use_clause actions. */
selected_name_use
  : IDENTIFIER '.' K_all
      { library_use(@1, active_scope, 0, $1, 0);
	delete[]$1;
      }
  | IDENTIFIER '.' IDENTIFIER '.' K_all
      { library_use(@1, active_scope, $1, $3, 0);
	delete[]$1;
	delete[]$3;
      }
  | IDENTIFIER '.' IDENTIFIER '.' IDENTIFIER
      { library_use(@1, active_scope, $1, $3, $5);
	delete[]$1;
	delete[]$3;
	delete[]$5;
      }
  ;

selected_names_use
  : selected_names_use ',' selected_name_use
  | selected_name_use
  ;


sequence_of_statements
  : sequence_of_statements sequential_statement
      { std::list<SequentialStmt*>*tmp = $1;
    if($2)
        tmp->push_back($2);
	$$ = tmp;
      }
  | sequential_statement
      { std::list<SequentialStmt*>*tmp = new std::list<SequentialStmt*>;
    if($1)
        tmp->push_back($1);
	$$ = tmp;
      }
  ;

sequential_statement
  : if_statement                { $$ = $1; }
  | signal_assignment_statement { $$ = $1; }
  | case_statement { $$ = $1; }
  | procedure_call_statement { $$ = $1; }
  | loop_statement { $$ = $1; }
  | K_null ';' { $$ = 0; }
  ;

shift_expression : simple_expression { $$ = $1; } ;

simple_expression
  : term
      { $$ = $1; }
  | term '+' term
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::PLUS, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | term '-' term
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::MINUS, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | term '&' term
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::CONCAT, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

signal_assignment_statement
  : name LEQ waveform ';'
      { SignalSeqAssignment*tmp = new SignalSeqAssignment($1, $3);
	FILE_NAME(tmp, @1);
	delete $3;
	$$ = tmp;
      }
  | name LEQ waveform K_when expression K_else waveform ';'
      { SignalSeqAssignment*tmp = new SignalSeqAssignment($1, $3);
	FILE_NAME(tmp, @1);
	sorrymsg(@4, "Conditional signal assignment not supported.\n");
	$$ = tmp;
      }
  ;

subtype_declaration
  : K_subtype IDENTIFIER K_is subtype_indication ';'
      { perm_string name = lex_strings.make($2);
	if ($4 == 0) {
	      errormsg(@1, "Failed to declare type name %s.\n", name.str());
	} else {
	      active_scope->bind_name(name, $4);
	}
      }
  ;

subtype_indication
  : IDENTIFIER
      { const VType*tmp = parse_type_by_name(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER '(' simple_expression direction simple_expression ')'
      { const VType*tmp = calculate_subtype_array(@1, $1, active_scope, $3, $4, $5);
	if (tmp == 0) {
	      errormsg(@1, "Unable to calculate bounds for array of %s.\n", $1);
	}
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER K_range simple_expression direction simple_expression
      { const VType*tmp = calculate_subtype_range(@1, $1, active_scope, $3, $4, $5);
	if (tmp == 0) {
	      errormsg(@1, "Unable to calculate bounds for range of %s.\n", $1);
	}
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER '(' error ')'
      {
    errormsg(@1, "Syntax error in subtype indication.\n");
      }
  ;

suffix
  : IDENTIFIER
      {
    $$ = $1;
      }
  | CHARACTER_LITERAL
      {
   $$ = $1;
      }
  | K_all
      {
  //do not have now better idea than using char constant
    $$ = strcpy(new char[strlen("all"+1)], "all");
      }
  ;

term
  : factor
      { $$ = $1; }
  | factor '*' factor
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::MULT, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | factor '/' factor
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::DIV, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | factor K_mod factor
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::MOD, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | factor K_rem factor
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::REM, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

use_clause
  : K_use selected_names ';'
     {
    $$ = $2;
     }
  | K_use error ';'
     { errormsg(@1, "Syntax error in use clause.\n"); yyerrok; }
  ;

use_clause_lib
  : K_use selected_names_use ';'
  | K_use error ';'
     { errormsg(@1, "Syntax error in use clause.\n"); yyerrok; }
  ;

use_clauses
  : use_clauses use_clause
  | use_clause
  ;

use_clauses_opt
  : use_clauses
  |
  ;

waveform
  : waveform_elements
      { $$ = $1; }
  | K_unaffected
      { $$ = 0; }
  ;

waveform_elements
  : waveform_elements ',' waveform_element
      { std::list<Expression*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | waveform_element
      { std::list<Expression*>*tmp = new std::list<Expression*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

waveform_element
  : expression
      { $$ = $1; }
  | K_null
      { $$ = 0; }
  ;

  /* Some keywords are optional in some contexts. In all such cases, a
     similar rule is used, as described here. */
K_architecture_opt : K_architecture | ;
K_component_opt    : K_component    | ;
K_configuration_opt: K_configuration| ;
K_entity_opt       : K_entity       | ;
K_package_opt      : K_package      | ;
K_postponed_opt    : K_postponed    | ;
K_is_opt           : K_is           | ;
%%

static void yyerror(YYLTYPE*, yyscan_t, const char*, bool, const char* /*msg*/)
{
	//fprintf(stderr, "%s\n", msg);
      parse_errors += 1;
}

void errormsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%d: error: ", loc.text, loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_errors += 1;
}

void sorrymsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%d: sorry: ", loc.text, loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_sorrys += 1;
}


/*
 * The reset_lexor function takes the fd and makes it the input file
 * for the lexor. The path argument is used in lexor/parser error messages.
 */
extern yyscan_t prepare_lexor(FILE*fd);
extern void destroy_lexor(yyscan_t scanner);

int parse_source_file(const char*file_path, bool work_library_flag)
{
      FILE*fd = fopen(file_path, "r");
      if (fd == 0) {
	    perror(file_path);
	    return -1;
      }

      yyscan_t scanner = prepare_lexor(fd);
      int rc = yyparse(scanner, file_path, work_library_flag);
      fclose(fd);
      destroy_lexor(scanner);

      return rc;
}
