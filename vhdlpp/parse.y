
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
# include "vtype.h"
# include  <cstdarg>
# include  <list>

inline void FILE_NAME(LineInfo*tmp, const struct yyltype&where)
{
      tmp->set_lineno(where.first_line);
      tmp->set_file(filename_strings.make(where.text));
}


static void yyerror(const char*msg);

int parse_errors = 0;
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

      InterfacePort*interface_element;
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

%type <interface_element> interface_element
%type <interface_list>    interface_list entity_header port_clause
%type <port_mode>  mode

%type <arch_statement> concurrent_statement concurrent_signal_assignment_statement
%type <arch_statement_list> architecture_statement_part

%type <expr> expression expression_logical factor primary relation
%type <expr> shift_expression simple_expression term waveform_element

%type <expr_list> waveform waveform_elements

%type <vtype> subtype_indication

%type <text> identifier_opt logical_name
%type <name_list> logical_name_list

%%

 /* The design_file is the root for the VHDL parse. */
design_file : design_units ;

architecture_body
  : K_architecture IDENTIFIER
    K_of IDENTIFIER
    K_is
    K_begin architecture_statement_part K_end K_architecture_opt identifier_opt ';'
      { Architecture*tmp = new Architecture(lex_strings.make($2), *$7);
	FILE_NAME(tmp, @1);
	bind_architecture_to_entity($4, tmp);
	if ($10 && tmp->get_name() != $10)
	      errormsg(@2, "Architecture name doesn't match closing name\n");
	delete[]$2;
	delete[]$4;
	delete $7;
	if ($10) delete[]$10
      }
  | K_architecture IDENTIFIER
    K_of IDENTIFIER
    K_is
    K_begin error K_end K_architecture_opt ';'
      { errormsg(@1, "Syntax error in architecture statement.\n"); yyerrok; }
  | K_architecture error ';'
      { errormsg(@1, "Syntax error in architecture body.\n"); yyerrok; }
  ;

  /* The architecture_statement_part is a list of concurrent
     statements. */
architecture_statement_part
  : architecture_statement_part concurrent_statement
      { std::list<Architecture::Statement*>*tmp = $1;
	tmp->push_back($2);
	$$ = tmp;
      }
  | concurrent_statement
      { std::list<Architecture::Statement*>*tmp = new std::list<Architecture::Statement*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

concurrent_signal_assignment_statement
  : IDENTIFIER LEQ waveform ';'
      { perm_string targ_name = lex_strings.make($1);
	SignalAssignment*tmp = new SignalAssignment(targ_name, *$3);
	FILE_NAME(tmp, @1);

	$$ = tmp;
	delete[]$1;
	delete $3;
      }
  ;

concurrent_statement
  : concurrent_signal_assignment_statement
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
  : K_entity IDENTIFIER K_is entity_header K_end K_entity_opt ';'
      { Entity*tmp = new Entity(lex_strings.make($2));
	FILE_NAME(tmp, @1);
	  // Transfer the ports
	std::list<InterfacePort*>*ports = $4;
	tmp->set_interface(ports);
	delete ports;
	  // Save the entity in the entity map.
	design_entities[tmp->get_name()] = tmp;
	delete[]$2;
      }
  | K_entity IDENTIFIER K_is entity_header K_end K_entity_opt IDENTIFIER ';'
      { Entity*tmp = new Entity(lex_strings.make($2));
	FILE_NAME(tmp, @1);
	if(tmp->get_name() != $7) {
	      errormsg(@1, "Syntax error in entity clause. \n");
	}
	
	delete[]$2;
	delete[]$7;
	// Transfer the ports
	std::list<InterfacePort*>*ports = $4;
	tmp->set_interface(ports);
	delete ports;
	// Save the entity in the entity map.
	design_entities[tmp->get_name()] = tmp;
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

expression_logical
  : relation K_and relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::AND, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_or relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::OR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_xor relation
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
  | relation K_xnor relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XNOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

factor : primary { $$ = $1; } ;

identifier_opt : IDENTIFIER { $$ = $1; } |  { $$ = 0; } ;

  /* The interface_element is also an interface_declaration */
interface_element
  : IDENTIFIER ':' mode subtype_indication
      { InterfacePort*tmp = new InterfacePort;
	FILE_NAME(tmp, @1);
	tmp->mode = $3;
	tmp->name = lex_strings.make($1);
	tmp->type = $4;
	delete[]$1;
	$$ = tmp;
      }
  ;

interface_list
  : interface_list ';' interface_element
      { std::list<InterfacePort*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | interface_element
      { std::list<InterfacePort*>*tmp = new std::list<InterfacePort*>;
	tmp->push_back($1);
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

port_clause
  : K_port '(' interface_list ')' ';'
      { $$ = $3; }
  | K_port '(' error ')' ';'
      { errormsg(@1, "Syntax error in port list\n");
	yyerrok;
	$$ = 0;
      }
  ;

primary
  : IDENTIFIER
      { ExpName*tmp = new ExpName(lex_strings.make($1));
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }
  | INT_LITERAL
      { ExpInteger*tmp = new ExpInteger($1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

relation : shift_expression { $$ = $1; } ;

selected_name
  : IDENTIFIER '.' K_all
  | IDENTIFIER '.' IDENTIFIER '.' K_all
  ;

selected_names
  : selected_names ',' selected_name
  | selected_name
  ;

shift_expression : simple_expression { $$ = $1; } ;

simple_expression : term { $$ = $1; } ;

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

term : factor { $$ = $1; } ;

use_clause
  : K_use selected_names ';'
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
K_entity_opt : K_entity | ;
%%

static void yyerror(const char*msg)
{
      fprintf(stderr, "%s\n", msg);
      parse_errors += 1;
}

static const char*file_path = "";

void errormsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%d: ", file_path, loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_errors += 1;
}

/*
 * This is used only by the lexor, to set the file path used in error
 * messages.
 */
void yyparse_set_filepath(const char*path)
{
      file_path = path;
}
