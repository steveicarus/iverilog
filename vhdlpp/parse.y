
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
# include "vsignal.h"
# include "vtype.h"
# include  <cstdarg>
# include  <list>
# include  <map>
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
  (Current).first_column = (Rhs)[1].first_column;    \
  (Current).last_line    = (Rhs)[N].last_line;       \
  (Current).last_column  = (Rhs)[N].last_column;     \
  (Current).text         = (Rhs)[1].text;   } while (0)

static void yyerror(const char*msg);

int parse_errors = 0;
int parse_sorrys = 0;

/*
 * This map accumulates signals that are matched in the declarations
 * section of a block. When the declarations are over, these are
 * transferred to a map for the block proper.
 */
static map<perm_string, Signal*> block_signals;

/*
 * This map accumulates component declarations.
 */
static map<perm_string, ComponentBase*> block_components;
%}


%union {
      port_mode_t port_mode;

      char*text;

      std::list<perm_string>* name_list;

      bool flag;
      int64_t uni_integer;
      double  uni_real;
      
      Expression*expr;
      std::list<Expression*>* expr_list;

      const VType* vtype;

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
%token <text> STRING_LITERAL CHARACTER_LITERAL
 /* compound symbols */
%token LEQ GEQ VASSIGN NE BOX EXP ARROW DLT DGT

 /* The rules may have types. */

%type <flag> direction

%type <interface_list> interface_element interface_list entity_header
%type <interface_list> port_clause port_clause_opt
%type <port_mode>  mode

%type <arch_statement> concurrent_statement component_instantiation_statement concurrent_signal_assignment_statement
%type <arch_statement_list> architecture_statement_part

%type <expr> expression factor primary relation
%type <expr> expression_logical expression_logical_and expression_logical_or
%type <expr> expression_logical_xnor expression_logical_xor
%type <expr> name
%type <expr> shift_expression simple_expression term waveform_element

%type <expr_list> waveform waveform_elements

%type <vtype> subtype_indication

%type <text> identifier_opt logical_name
%type <name_list> logical_name_list identifier_list

%%

 /* The design_file is the root for the VHDL parse. */
design_file : design_units ;

architecture_body
  : K_architecture IDENTIFIER
    K_of IDENTIFIER
    K_is block_declarative_items_opt
    K_begin architecture_statement_part K_end K_architecture_opt identifier_opt ';'
      { Architecture*tmp = new Architecture(lex_strings.make($2),
					    block_signals,
					    block_components, *$8);
	FILE_NAME(tmp, @1);
	bind_architecture_to_entity($4, tmp);
	if ($11 && tmp->get_name() != $11)
	      errormsg(@2, "Architecture name doesn't match closing name\n");
	delete[]$2;
	delete[]$4;
	delete $8;
	block_signals.clear();
	block_components.clear();
	if ($11) delete[]$11;
      }
  | K_architecture IDENTIFIER
    K_of IDENTIFIER
    K_is block_declarative_items_opt
    K_begin error K_end K_architecture_opt identifier_opt ';'
      { errormsg(@8, "Errors in architecture statements.\n"); yyerrok; }
  | K_architecture error ';'
      { errormsg(@2, "Errors in architecture body.\n"); yyerrok; }
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
  : name ARROW name
  ;

association_list
  : association_list ',' association_element
  | association_element
  ;

block_declarative_item
  : K_signal identifier_list ':' subtype_indication ';'
      { /* Save the signal declaration in the block_signals map. */
	for (std::list<perm_string>::iterator cur = $2->begin()
		   ; cur != $2->end() ; ++cur) {
	      Signal*sig = new Signal(*cur, $4);
	      FILE_NAME(sig, @1);
	      block_signals[*cur] = sig;
	}
	delete $2;
      }

  | K_component IDENTIFIER K_is_opt
    port_clause_opt
    K_end K_component identifier_opt ';'
      { perm_string name = lex_strings.make($2);
	if ($7 && name != $7) {
	      errormsg(@7, "Identifier %s doesn't match component name %s\n",
		       $7, name.str());
	}

	ComponentBase*comp = new ComponentBase(name);
	if ($4) comp->set_interface($4);
	block_components[name] = comp;
	delete[]$2;
	delete[]$7;
      }

      /* Various error handling rules for block_declarative_item... */

  | K_signal error ';'
      { errormsg(@2, "Syntax error declaring signals.\n"); yyerrok; }
  | error ';'
      { errormsg(@1, "Syntax error in block declarations.\n"); yyerrok; }

  | K_component IDENTIFIER K_is_opt error K_end K_component identifier_opt ';'
      { errormsg(@4, "Syntax error in component declaration.\n"); yyerrok; }
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

component_instantiation_statement
  : IDENTIFIER ':' IDENTIFIER port_map_aspect_opt ';'
      { sorrymsg(@1, "Component instantiation statements are not supported.\n");
	delete[]$1;
	delete[]$3;
	$$ = 0;
      }
  | IDENTIFIER ':' IDENTIFIER error ';'
      { errormsg(@4, "Errors in component instantiation.\n");
	delete[]$1;
	delete[]$3;
	$$ = 0;
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
  ;

context_clause : context_items | ;

context_item
  : library_clause
  | use_clause
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
        }
        delete $7;
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
  : entity_declaration
  | architecture_body
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

port_clause
  : K_port '(' interface_list ')' ';'
      { $$ = $3; }
  | K_port '(' error ')' ';'
      { errormsg(@1, "Syntax error in port list\n");
	yyerrok;
	$$ = 0;
      }
  ;

port_clause_opt : port_clause {$$ = $1;} | {$$ = 0;} ;

port_map_aspect
  : K_port K_map '(' association_list ')'
  ;

port_map_aspect_opt
  : port_map_aspect
  |
  ;

primary
  : name
      { $$ = $1; }
  | INT_LITERAL
      { ExpInteger*tmp = new ExpInteger($1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | '(' expression ')'
      { $$ = $2; }
  ;

relation : shift_expression { $$ = $1; } ;

  /* The *_use variant of selected_name is used by the "use"
     clause. It is syntactically identical to other selected_name
     rules, but is a convenient place to attach use_clause actions. */
selected_name_use
  : IDENTIFIER '.' K_all
      { library_use(@1, 0, $1, 0);
	delete[]$1;
      }
  | IDENTIFIER '.' IDENTIFIER '.' K_all
      { library_use(@1, $1, $3, 0);
	delete[]$1;
	delete[]$3;
      }
  ;

selected_names_use
  : selected_names_use ',' selected_name_use
  | selected_name_use
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
  ;

subtype_indication
  : IDENTIFIER
      { const VType*tmp = global_types[lex_strings.make($1)];
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER '(' simple_expression direction simple_expression ')'
      { const VType*tmp = calculate_subtype($1, $3, $4, $5);
	delete[]$1;
	$$ = tmp;
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
  : K_use selected_names_use ';'
  | K_use error ';'
     { errormsg(@1, "Syntax error in use clause.\n"); yyerrok; }
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
K_entity_opt       : K_entity       | ;
K_is_opt           : K_is           | ;
%%

static void yyerror(const char* /*msg*/)
{
	//fprintf(stderr, "%s\n", msg);
      parse_errors += 1;
}

static const char*file_path = "";

void errormsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%d: error: ", file_path, loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_errors += 1;
}

void sorrymsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%d: sorry: ", file_path, loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_sorrys += 1;
}

/*
 * This is used only by the lexor, to set the file path used in error
 * messages.
 */
void yyparse_set_filepath(const char*path)
{
      file_path = path;
}
