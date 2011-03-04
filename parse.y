
%{
/*
 * Copyright (c) 1998-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "parse_misc.h"
# include  "compiler.h"
# include  "pform.h"
# include  <sstream>

extern void lex_start_table();
extern void lex_end_table();

static svector<PExpr*>* active_range = 0;
static bool active_signed = false;

/* Port declaration lists use this structure for context. */
static struct {
      NetNet::Type port_net_type;
      NetNet::PortType port_type;
      bool sign_flag;
      svector<PExpr*>* range;
} port_declaration_context;

/* Later version of bison (including 1.35) will not compile in stack
   extension if the output is compiled with C++ and either the YYSTYPE
   or YYLTYPE are provided by the source code. However, I can get the
   old behavior back by defining these symbols. */
# define YYSTYPE_IS_TRIVIAL 1
# define YYLTYPE_IS_TRIVIAL 1

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

/*
 * These are some common strength pairs that are used as defaults when
 * the user is not otherwise specific.
 */
const static struct str_pair_t pull_strength = { PGate::PULL,  PGate::PULL };
const static struct str_pair_t str_strength = { PGate::STRONG, PGate::STRONG };
%}

%union {
      bool flag;

      char letter;

	/* text items are C strings allocated by the lexor using
	   strdup. They can be put into lists with the texts type. */
      char*text;
      list<perm_string>*perm_strings;

      hname_t*hier;

      list<string>*strings;

      struct str_pair_t drive;

      PCase::Item*citem;
      svector<PCase::Item*>*citems;

      lgate*gate;
      svector<lgate>*gates;

      Module::port_t *mport;
      svector<Module::port_t*>*mports;

      named_pexpr_t*named_pexpr;
      svector<named_pexpr_t*>*named_pexprs;
      struct parmvalue_t*parmvalue;

      PExpr*expr;
      svector<PExpr*>*exprs;

      svector<PEEvent*>*event_expr;

      NetNet::Type nettype;
      PGBuiltin::Type gatetype;
      NetNet::PortType porttype;

      PWire*wire;
      svector<PWire*>*wires;

      PEventStatement*event_statement;
      Statement*statement;
      svector<Statement*>*statement_list;

      PTaskFuncArg function_type;

      struct { svector<PExpr*>*range; svector<PExpr*>*delay; } range_delay;
      net_decl_assign_t*net_decl_assign;

      verinum* number;

      verireal* realtime;
};

%token <text>   IDENTIFIER SYSTEM_IDENTIFIER STRING
%token <text>   PATHPULSE_IDENTIFIER
%token <number> BASED_NUMBER DEC_NUMBER
%token <realtime> REALTIME
%token K_LE K_GE K_EG K_EQ K_NE K_CEQ K_CNE K_LS K_RS K_RSS K_SG
%token K_PO_POS K_PO_NEG
%token K_PSTAR K_STARP
%token K_LOR K_LAND K_NAND K_NOR K_NXOR K_TRIGGER
%token K_always K_and K_assign K_begin K_buf K_bufif0 K_bufif1 K_case
%token K_casex K_casez K_cmos K_deassign K_default K_defparam K_disable
%token K_edge K_edge_descriptor
%token K_else K_end K_endcase K_endfunction K_endmodule
%token K_endprimitive K_endspecify K_endtable K_endtask K_event K_for
%token K_force K_forever K_fork K_function K_highz0 K_highz1 K_if K_ifnone
%token K_initial K_inout K_input K_integer K_join K_large K_localparam
%token K_macromodule
%token K_medium K_module K_nand K_negedge K_nmos K_nor K_not K_notif0
%token K_notif1 K_or K_output K_parameter K_pmos K_posedge K_primitive
%token K_pull0 K_pull1 K_pulldown K_pullup K_rcmos K_real K_realtime
%token K_reg K_release K_repeat
%token K_rnmos K_rpmos K_rtran K_rtranif0 K_rtranif1 K_scalared
%token K_signed K_small K_specify
%token K_specparam K_strong0 K_strong1 K_supply0 K_supply1 K_table K_task
%token K_time K_tran K_tranif0 K_tranif1 K_tri K_tri0 K_tri1 K_triand
%token K_trior K_trireg K_vectored K_wait K_wand K_weak0 K_weak1
%token K_while K_wire
%token K_wor K_xnor K_xor
%token K_Shold K_Speriod K_Srecovery K_Srecrem K_Ssetup K_Swidth K_Ssetuphold

%token KK_attribute

%type <number>  number
%type <flag>    signed_opt udp_reg_opt
%type <drive>   drive_strength drive_strength_opt dr_strength0 dr_strength1
%type <letter>  udp_input_sym udp_output_sym
%type <text>    udp_input_list udp_sequ_entry udp_comb_entry
%type <perm_strings> udp_input_declaration_list
%type <strings> udp_entry_list udp_comb_entry_list udp_sequ_entry_list
%type <strings> udp_body udp_port_list
%type <wires>   udp_port_decl udp_port_decls
%type <statement> udp_initial udp_init_opt
%type <expr>    udp_initial_expr_opt

%type <hier> identifier
%type <text> register_variable
%type <perm_strings> register_variable_list list_of_identifiers

%type <net_decl_assign> net_decl_assign net_decl_assigns

%type <mport> port port_opt port_reference port_reference_list
%type <mport> port_declaration
%type <mports> list_of_ports module_port_list_opt list_of_port_declarations

%type <wires> task_item task_item_list task_item_list_opt
%type <wires> function_item function_item_list

%type <named_pexpr> port_name parameter_value_byname
%type <named_pexprs> port_name_list parameter_value_byname_list

%type <named_pexpr> attribute
%type <named_pexprs> attribute_list attribute_list_opt

%type <citem>  case_item
%type <citems> case_items

%type <gate>  gate_instance
%type <gates> gate_instance_list

%type <expr>  expression expr_primary expr_mintypmax
%type <expr>  lavalue lpvalue
%type <expr>  delay_value delay_value_simple
%type <exprs> delay1 delay3 delay3_opt
%type <exprs> expression_list expression_list_proper
%type <exprs> assign assign_list

%type <exprs> range range_opt
%type <nettype>  net_type var_type net_type_opt
%type <gatetype> gatetype
%type <porttype> port_type
%type <parmvalue> parameter_value_opt

%type <function_type> function_range_or_type_opt
%type <event_expr> event_expression_list
%type <event_expr> event_expression
%type <event_statement> event_control
%type <statement> statement statement_opt
%type <statement_list> statement_list

%type <range_delay> range_delay

%type <letter> spec_polarity
%type <perm_strings>  specify_path_identifiers

%token K_TAND
%right '?' ':'
%left K_LOR
%left K_LAND
%left '|'
%left '^' K_NXOR K_NOR
%left '&' K_NAND
%left K_EQ K_NE K_CEQ K_CNE
%left K_GE K_LE '<' '>'
%left K_LS K_RS K_RSS
%left '+' '-'
%left '*' '/' '%'
%left UNARY_PREC

/* to resolve dangling else ambiguity. */
%nonassoc less_than_K_else
%nonassoc K_else


%%

  /* A degenerate source file can be completely empty. */
main : source_file | ;

source_file
	: description
	| source_file description
	;

number  : BASED_NUMBER
	     { $$ = $1; }
        | DEC_NUMBER
	     { $$ = $1; }
        | DEC_NUMBER BASED_NUMBER
	     { $$ = pform_verinum_with_size($1,$2, @2.text, @2.first_line); }
	;

  /* Verilog-2001 supports attribute lists, which can be attached to a
     variety of different objects. The syntax inside the (* *) is a
     comma separated list of names or names with assigned values. */
attribute_list_opt
	: K_PSTAR attribute_list K_STARP { $$ = $2; }
	| K_PSTAR K_STARP { $$ = 0; }
	| { $$ = 0; }
	;

attribute_list
	: attribute_list ',' attribute
		{ svector<named_pexpr_t*>*tmp =
			new svector<named_pexpr_t*>(*$1,$3);
		  delete $1;
		  $$ = tmp;
		}
	| attribute
		{ svector<named_pexpr_t*>*tmp = new svector<named_pexpr_t*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	;


attribute
	: IDENTIFIER
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($1);
		  tmp->parm = 0;
		  delete $1;
		  $$ = tmp;
		}
	| IDENTIFIER '=' expression
		{ PExpr*tmp = $3;
		  if (tmp && !pform_expression_is_constant(tmp)) {
			yyerror(@3, "error: attribute value "
			            "expression must be constant.");
			delete tmp;
			tmp = 0;
		  }
		  named_pexpr_t*tmp2 = new named_pexpr_t;
		  tmp2->name = lex_strings.make($1);
		  tmp2->parm = tmp;
		  delete $1;
		  $$ = tmp2;
		}
	;


  /* The block_item_decl is used in function definitions, task
     definitions, module definitions and named blocks. Wherever a new
     scope is entered, the source may declare new registers and
     integers. This rule matches those declarations. The containing
     rule has presumably set up the scope. */
block_item_decl
	: K_reg signed_opt range register_variable_list ';'
		{ pform_set_net_range($4, $3, $2);
		}
	| K_reg signed_opt register_variable_list ';'
		{ pform_set_net_range($3, 0, $2);
		}
	| K_integer register_variable_list ';'
		{ pform_set_reg_integer($2);
		}
	| K_time register_variable_list ';'
		{ pform_set_reg_time($2);
		}
	| K_real list_of_identifiers ';'
		{ pform_make_reals($2, @1.text, @1.first_line);
		}
	| K_realtime list_of_identifiers ';'
		{ pform_make_reals($2, @1.text, @1.first_line);
		}
	| K_parameter parameter_assign_decl ';'
	| K_localparam localparam_assign_decl ';'

  /* Recover from errors that happen within variable lists. Use the
     trailing semi-colon to resync the parser. */

	| K_reg error ';'
		{ yyerror(@1, "error: syntax error in reg variable list.");
		  yyerrok;
		}
	| K_integer error ';'
		{ yyerror(@1, "error: syntax error in integer variable list.");
		  yyerrok;
		}
	| K_time error ';'
		{ yyerror(@1, "error: syntax error in time variable list.");
		  yyerrok;
		}
	| K_real error ';'
		{ yyerror(@1, "error: syntax error in real variable list.");
		  yyerrok;
		}
	| K_realtime error ';'
		{ yyerror(@1, "error: syntax error in realtime variable list.");
		  yyerrok;
		}
	| K_parameter error ';'
		{ yyerror(@1, "error: syntax error in parameter list.");
		  yyerrok;
		}
	| K_localparam error ';'
		{ yyerror(@1, "error: syntax error localparam list.");
		  yyerrok;
		}
	;

block_item_decls
	: block_item_decl
	| block_item_decls block_item_decl
	;

block_item_decls_opt
	: block_item_decls
	|
	;

case_item
	: expression_list ':' statement_opt
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->expr = *$1;
		  tmp->stat = $3;
		  delete $1;
		  $$ = tmp;
		}
	| K_default ':' statement_opt
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->stat = $3;
		  $$ = tmp;
		}
	| K_default  statement_opt
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->stat = $2;
		  $$ = tmp;
		}
	| error ':' statement_opt
		{ yyerror(@1, "error: Incomprehensible case expression.");
		  yyerrok;
		}
	;

case_items
	: case_items case_item
		{ svector<PCase::Item*>*tmp;
		  tmp = new svector<PCase::Item*>(*$1, $2);
		  delete $1;
		  $$ = tmp;
		}
	| case_item
		{ svector<PCase::Item*>*tmp = new svector<PCase::Item*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	;

charge_strength
	: '(' K_small ')'
	| '(' K_medium ')'
	| '(' K_large ')'
	;

charge_strength_opt
	: charge_strength
	|
	;

defparam_assign
	: identifier '=' expression
		{ PExpr*tmp = $3;
		  if (!pform_expression_is_constant(tmp)) {
			yyerror(@3, "error: parameter value "
			            "must be constant.");
			delete tmp;
			tmp = 0;
		  }
		  pform_set_defparam(*$1, $3);
		  delete $1;
		}
	;

defparam_assign_list
	: defparam_assign
	| range defparam_assign
		{ yyerror(@1, "error: defparam may not include a range.");
		  delete $1;
		}
	| defparam_assign_list ',' defparam_assign
	;

delay1
	: '#' delay_value_simple
		{ svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = $2;
		  $$ = tmp;
		}
	| '#' '(' delay_value ')'
		{ svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = $3;
		  $$ = tmp;
		}
	;

delay3
	: '#' delay_value_simple
		{ svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = $2;
		  $$ = tmp;
		}
	| '#' '(' delay_value ')'
		{ svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = $3;
		  $$ = tmp;
		}
	| '#' '(' delay_value ',' delay_value ')'
		{ svector<PExpr*>*tmp = new svector<PExpr*>(2);
		  (*tmp)[0] = $3;
		  (*tmp)[1] = $5;
		  $$ = tmp;
		}
	| '#' '(' delay_value ',' delay_value ',' delay_value ')'
		{ svector<PExpr*>*tmp = new svector<PExpr*>(3);
		  (*tmp)[0] = $3;
		  (*tmp)[1] = $5;
		  (*tmp)[2] = $7;
		  $$ = tmp;
		}
	;

delay3_opt
	: delay3 { $$ = $1; }
	|        { $$ = 0; }
	;

delay_value
	: expression
		{ PExpr*tmp = $1;
		  $$ = tmp;
		}
	| expression ':' expression ':' expression
		{ $$ = pform_select_mtm_expr($1, $3, $5); }
	;


delay_value_simple
	: DEC_NUMBER
		{ verinum*tmp = $1;
		  if (tmp == 0) {
			yyerror(@1, "internal error: delay.");
			$$ = 0;
		  } else {
			$$ = new PENumber(tmp);
			$$->set_file(@1.text);
			$$->set_lineno(@1.first_line);
		  }
		}
	| REALTIME
		{ verireal*tmp = $1;
		  if (tmp == 0) {
			yyerror(@1, "internal error: delay.");
			$$ = 0;
		  } else {
			$$ = new PEFNumber(tmp);
			$$->set_file(@1.text);
			$$->set_lineno(@1.first_line);
		  }
		}
	| IDENTIFIER
		{ PEIdent*tmp = new PEIdent(hname_t($1));
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $1;
		}
	;

description
	: module
	| udp_primitive
	| KK_attribute '(' IDENTIFIER ',' STRING ',' STRING ')'
		{ perm_string tmp3 = lex_strings.make($3);
		  pform_set_type_attrib(tmp3, $5, $7);
		  delete $3;
		  delete $5;
		}
	;

drive_strength
	: '(' dr_strength0 ',' dr_strength1 ')'
		{ $$.str0 = $2.str0;
		  $$.str1 = $4.str1;
		}
	| '(' dr_strength1 ',' dr_strength0 ')'
		{ $$.str0 = $4.str0;
		  $$.str1 = $2.str1;
		}
	| '(' dr_strength0 ',' K_highz1 ')'
		{ $$.str0 = $2.str0;
		  $$.str1 = PGate::HIGHZ;
		}
	| '(' dr_strength1 ',' K_highz0 ')'
		{ $$.str0 = PGate::HIGHZ;
		  $$.str1 = $2.str1;
		}
	| '(' K_highz1 ',' dr_strength0 ')'
		{ $$.str0 = $4.str0;
		  $$.str1 = PGate::HIGHZ;
		}
	| '(' K_highz0 ',' dr_strength1 ')'
		{ $$.str0 = PGate::HIGHZ;
		  $$.str1 = $4.str1;
		}
	;

drive_strength_opt
	: drive_strength { $$ = $1; }
	|                { $$.str0 = PGate::STRONG; $$.str1 = PGate::STRONG; }
	;

dr_strength0
	: K_supply0 { $$.str0 = PGate::SUPPLY; }
	| K_strong0 { $$.str0 = PGate::STRONG; }
	| K_pull0   { $$.str0 = PGate::PULL; }
	| K_weak0   { $$.str0 = PGate::WEAK; }
	;

dr_strength1
	: K_supply1 { $$.str1 = PGate::SUPPLY; }
	| K_strong1 { $$.str1 = PGate::STRONG; }
	| K_pull1   { $$.str1 = PGate::PULL; }
	| K_weak1   { $$.str1 = PGate::WEAK; }
	;

event_control
	: '@' identifier
		{ PEIdent*tmpi = new PEIdent(*$2);
		  tmpi->set_file(@2.text);
		  tmpi->set_lineno(@2.first_line);
		  delete $2;
		  PEEvent*tmpe = new PEEvent(PEEvent::ANYEDGE, tmpi);
		  PEventStatement*tmps = new PEventStatement(tmpe);
		  tmps->set_file(@1.text);
		  tmps->set_lineno(@1.first_line);
		  $$ = tmps;
		}
	| '@' '(' event_expression_list ')'
		{ PEventStatement*tmp = new PEventStatement(*$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $3;
		  $$ = tmp;
		}
	| '@' '(' error ')'
		{ yyerror(@1, "error: Malformed event control expression.");
		  $$ = 0;
		}
	;

event_expression_list
	: event_expression
		{ $$ = $1; }
	| event_expression_list K_or event_expression
		{ svector<PEEvent*>*tmp = new svector<PEEvent*>(*$1, *$3);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	| event_expression_list ',' event_expression
		{ svector<PEEvent*>*tmp = new svector<PEEvent*>(*$1, *$3);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	;

event_expression
	: K_posedge expression
		{ PEEvent*tmp = new PEEvent(PEEvent::POSEDGE, $2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	| K_negedge expression
		{ PEEvent*tmp = new PEEvent(PEEvent::NEGEDGE, $2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	| expression
		{ PEEvent*tmp = new PEEvent(PEEvent::ANYEDGE, $1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	;

expression
	: expr_primary
		{ $$ = $1; }
	| '+' expr_primary %prec UNARY_PREC
		{ $$ = $2; }
	| '-' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('-', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| '~' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('~', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| '&' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('&', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| '!' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('!', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| '|' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('|', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| '^' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('^', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| K_NAND expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('A', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| K_NOR expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('N', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| K_NXOR expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('X', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| '!' error %prec UNARY_PREC
		{ yyerror(@1, "error: Operand of unary ! "
			  "is not a primary expression.");
		  $$ = 0;
		}
	| '^' error %prec UNARY_PREC
		{ yyerror(@1, "error: Operand of reduction ^ "
			  "is not a primary expression.");
		  $$ = 0;
		}
	| expression '^' expression
		{ PEBinary*tmp = new PEBinary('^', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '*' expression
		{ PEBinary*tmp = new PEBinary('*', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '/' expression
		{ PEBinary*tmp = new PEBinary('/', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '%' expression
		{ PEBinary*tmp = new PEBinary('%', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '+' expression
		{ PEBinary*tmp = new PEBinary('+', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '-' expression
		{ PEBinary*tmp = new PEBinary('-', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '&' expression
		{ PEBinary*tmp = new PEBinary('&', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '|' expression
		{ PEBinary*tmp = new PEBinary('|', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_NAND expression
		{ PEBinary*tmp = new PEBinary('A', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_NOR expression
		{ PEBinary*tmp = new PEBinary('O', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_NXOR expression
		{ PEBinary*tmp = new PEBinary('X', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '<' expression
		{ PEBinary*tmp = new PEBinary('<', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '>' expression
		{ PEBinary*tmp = new PEBinary('>', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_LS expression
		{ PEBinary*tmp = new PEBinary('l', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_RS expression
		{ PEBinary*tmp = new PEBinary('r', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_RSS expression
		{ PEBinary*tmp = new PEBinary('R', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_EQ expression
		{ PEBinary*tmp = new PEBinary('e', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_CEQ expression
		{ PEBinary*tmp = new PEBinary('E', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_LE expression
		{ PEBinary*tmp = new PEBinary('L', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_GE expression
		{ PEBinary*tmp = new PEBinary('G', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_NE expression
		{ PEBinary*tmp = new PEBinary('n', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_CNE expression
		{ PEBinary*tmp = new PEBinary('N', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_LOR expression
		{ PEBinary*tmp = new PEBinary('o', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression K_LAND expression
		{ PEBinary*tmp = new PEBinary('a', $1, $3);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	| expression '?' expression ':' expression
		{ PETernary*tmp = new PETernary($1, $3, $5);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
	;

expr_mintypmax
	: expression
		{ $$ = $1; }
	| expression ':' expression ':' expression
		{ switch (min_typ_max_flag) {
		      case MIN:
			$$ = $1;
			delete $3;
			delete $5;
			break;
		      case TYP:
			delete $1;
			$$ = $3;
			delete $5;
			break;
		      case MAX:
			delete $1;
			delete $3;
			$$ = $5;
			break;
		  }
		}
	;


  /* Many contexts take a comma separated list of expressions. Null
     expressions can happen anywhere in the list, so there are two
     extra rules for parsing and installing those nulls. */
expression_list
	: expression_list ',' expression
		{ svector<PExpr*>*tmp = new svector<PExpr*>(*$1, $3);
		  delete $1;
		  $$ = tmp;
		}
	| expression
		{ svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	|
		{ svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = 0;
		  $$ = tmp;
		}

	| expression_list ','
		{ svector<PExpr*>*tmp = new svector<PExpr*>(*$1, 0);
		  delete $1;
		  $$ = tmp;
		}
	;

expression_list_proper
	: expression_list_proper ',' expression
		{ svector<PExpr*>*tmp = new svector<PExpr*>(*$1, $3);
		  delete $1;
		  $$ = tmp;
		}
	| expression
		{ svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	;


expr_primary
	: number
		{ assert($1);
		  PENumber*tmp = new PENumber($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| REALTIME
		{ PEFNumber*tmp = new PEFNumber($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| STRING
		{ PEString*tmp = new PEString($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| identifier
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $1;
		}
	| SYSTEM_IDENTIFIER
                { PECallFunction*tmp = new PECallFunction(hname_t($1));
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $1;
		}
	| identifier '[' expression ']'
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  tmp->msb_ = $3;
		  delete $1;
		  $$ = tmp;
		}
	| identifier '[' expression ':' expression ']'
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  tmp->msb_ = $3;
		  tmp->lsb_ = $5;
		  delete $1;
		  $$ = tmp;
		}
	| identifier '(' expression_list ')'
                { PECallFunction*tmp = new PECallFunction(*$1, *$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER '(' expression_list ')'
                { PECallFunction*tmp = new PECallFunction(hname_t($1), *$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| '(' expr_mintypmax ')'
		{ $$ = $2; }
	| '{' expression_list '}'
		{ PEConcat*tmp = new PEConcat(*$2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  for (unsigned idx = 0 ;  idx < (*$2).count() ;  idx += 1) {
			PExpr*ex = (*$2)[idx];
			if (ex == 0) {
			      yyerror(@1, "error: Null arguments not allowed"
				      " in repeat expressions.");
			      break;
			}
		  }
		  delete $2;
		  $$ = tmp;
		}
	| '{' expression '{' expression_list '}' '}'
		{ PExpr*rep = $2;
		  if (!pform_expression_is_constant($2)) {
			yyerror(@2, "error: Repeat expression "
			            "must be constant.");
			delete rep;
			rep = 0;
		  }
		  PEConcat*tmp = new PEConcat(*$4, rep);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $4;
		  $$ = tmp;
		}
	;


  /* A function_item is either a block item (i.e. a reg or integer
     declaration) or an input declaration. There are no output or
     inout ports. */
function_item
	: K_input range_opt list_of_identifiers ';'
                { svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, false,
						$2, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_input K_signed range_opt list_of_identifiers ';'
                { svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, true,
						$3, $4,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output range_opt list_of_identifiers ';'
                { svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, false,
						$2, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		  yyerror(@1, "Functions may not have output ports.");
		}
	| K_output K_signed range_opt list_of_identifiers ';'
                { svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, true,
						$3, $4,
						@1.text, @1.first_line);
		  $$ = tmp;
		  yyerror(@1, "Functions may not have output ports.");
		}
	| K_inout range_opt list_of_identifiers ';'
                { svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, false,
						$2, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		  yyerror(@1, "Functions may not have inout ports.");
		}
	| K_inout K_signed range_opt list_of_identifiers ';'
                { svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, true,
						$3, $4,
						@1.text, @1.first_line);
		  $$ = tmp;
		  yyerror(@1, "Functions may not have inout ports.");
		}
	| attribute_list_opt block_item_decl
                { $$ = 0; }
	;

  /* A function_item_list only lists the input/output/inout
     declarations. The integer and reg declarations are handled in
     place, so are not listed. The list builder needs to account for
     the possibility that the various parts may be NULL. */
function_item_list
	: function_item
                { $$ = $1; }
	| function_item_list function_item
		{ if ($1 && $2) {
		        svector<PWire*>*tmp = new svector<PWire*>(*$1, *$2);
			delete $1;
			delete $2;
			$$ = tmp;
		  } else if ($1) {
			$$ = $1;
		  } else {
			$$ = $2;
		  }
		}
	;

  /* A gate_instance is a module instantiation or a built in part
     type. In any case, the gate has a set of connections to ports. */
gate_instance
	: IDENTIFIER '(' expression_list ')'
		{ lgate*tmp = new lgate;
		  tmp->name = $1;
		  tmp->parms = $3;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete $1;
		  $$ = tmp;
		}

	| IDENTIFIER range '(' expression_list ')'
		{ lgate*tmp = new lgate;
		  svector<PExpr*>*rng = $2;
		  tmp->name = $1;
		  tmp->parms = $4;
		  tmp->range[0] = (*rng)[0];
		  tmp->range[1] = (*rng)[1];
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete $1;
		  delete rng;
		  $$ = tmp;
		}
	| '(' expression_list ')'
		{ lgate*tmp = new lgate;
		  tmp->name = "";
		  tmp->parms = $2;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  $$ = tmp;
		}

  /* Degenerate modules can have no ports. */

	| IDENTIFIER range
		{ lgate*tmp = new lgate;
		  svector<PExpr*>*rng = $2;
		  tmp->name = $1;
		  tmp->parms = 0;
		  tmp->range[0] = (*rng)[0];
		  tmp->range[1] = (*rng)[1];
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete $1;
		  delete rng;
		  $$ = tmp;
		}

  /* Modules can also take ports by port-name expressions. */

	| IDENTIFIER '(' port_name_list ')'
		{ lgate*tmp = new lgate;
		  tmp->name = $1;
		  tmp->parms_by_name = $3;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete $1;
		  $$ = tmp;
		}
	;

gate_instance_list
	: gate_instance_list ',' gate_instance
		{ svector<lgate>*tmp1 = $1;
		  lgate*tmp2 = $3;
		  svector<lgate>*out = new svector<lgate> (*tmp1, *tmp2);
		  delete tmp1;
		  delete tmp2;
		  $$ = out;
		}
	| gate_instance
		{ svector<lgate>*tmp = new svector<lgate>(1);
		  (*tmp)[0] = *$1;
		  delete $1;
		  $$ = tmp;
		}
	;

gatetype
	: K_and  { $$ = PGBuiltin::AND; }
	| K_nand { $$ = PGBuiltin::NAND; }
	| K_or   { $$ = PGBuiltin::OR; }
	| K_nor  { $$ = PGBuiltin::NOR; }
	| K_xor  { $$ = PGBuiltin::XOR; }
	| K_xnor { $$ = PGBuiltin::XNOR; }
	| K_buf  { $$ = PGBuiltin::BUF; }
	| K_bufif0 { $$ = PGBuiltin::BUFIF0; }
	| K_bufif1 { $$ = PGBuiltin::BUFIF1; }
	| K_not    { $$ = PGBuiltin::NOT; }
	| K_notif0 { $$ = PGBuiltin::NOTIF0; }
	| K_notif1 { $$ = PGBuiltin::NOTIF1; }
	| K_nmos  { $$ = PGBuiltin::NMOS; }
	| K_rnmos { $$ = PGBuiltin::RNMOS; }
	| K_pmos  { $$ = PGBuiltin::PMOS; }
	| K_rpmos { $$ = PGBuiltin::RPMOS; }
	| K_cmos  { $$ = PGBuiltin::CMOS; }
	| K_rcmos { $$ = PGBuiltin::RCMOS; }
	| K_tran  { $$ = PGBuiltin::TRAN; }
	| K_rtran { $$ = PGBuiltin::RTRAN; }
	| K_tranif0 { $$ = PGBuiltin::TRANIF0; }
	| K_tranif1 { $$ = PGBuiltin::TRANIF1; }
	| K_rtranif0 { $$ = PGBuiltin::RTRANIF0; }
	| K_rtranif1 { $$ = PGBuiltin::RTRANIF1; }
	;


  /* A general identifier is a hierarchical name, with the right most
     name the base of the identifier. This rule builds up a
     hierarchical name from left to right, forming a list of names. */
identifier
	: IDENTIFIER
		{ $$ = new hname_t($1);
		  delete $1;
		}
	| identifier '.' IDENTIFIER
		{ hname_t * tmp = $1;
		  tmp->append($3);
		  delete $3;
		  $$ = tmp;
		}
	;

  /* This is a list of identifiers. The result is a list of strings,
     each one of the identifiers in the list. These are simple,
     non-hierarchical names separated by ',' characters. */
list_of_identifiers
	: IDENTIFIER
		{ list<perm_string>*tmp = new list<perm_string>;
		  tmp->push_back(lex_strings.make($1));
		  $$ = tmp;
		  delete[]$1;
		}
	| list_of_identifiers ',' IDENTIFIER
		{ list<perm_string>*tmp = $1;
		  tmp->push_back(lex_strings.make($3));
		  $$ = tmp;
		  delete[]$3;
		}
	;


  /* The list_of_ports and list_of_port_declarations rules are the
     port list formats for module ports. The list_of_ports_opt rule is
     only used by the module start rule.

     The first, the list_of_ports, is the 1364-1995 format, a list of
     port names, including .name() syntax.

     The list_of_port_declarations the 1364-2001 format, an in-line
     declaration of the ports.

     In both cases, the list_of_ports and list_of_port_declarations
     returns an array of Module::port_t* items that include the name
     of the port internally and externally. The actual creation of the
     nets/variables is done in the declaration, whether internal to
     the port list or in amongst the module items. */

list_of_ports
	: port_opt
		{ svector<Module::port_t*>*tmp
			  = new svector<Module::port_t*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	| list_of_ports ',' port_opt
		{ svector<Module::port_t*>*tmp
			= new svector<Module::port_t*>(*$1, $3);
		  delete $1;
		  $$ = tmp;
		}
	;

list_of_port_declarations
	: port_declaration
		{ svector<Module::port_t*>*tmp
			  = new svector<Module::port_t*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	| list_of_port_declarations ',' port_declaration
		{ svector<Module::port_t*>*tmp
			= new svector<Module::port_t*>(*$1, $3);
		  delete $1;
		  $$ = tmp;
		}
	| list_of_port_declarations ',' IDENTIFIER
		{ Module::port_t*ptmp;
		  ptmp = pform_module_port_reference($3, @3.text,
						     @3.first_line);
		  svector<Module::port_t*>*tmp
			= new svector<Module::port_t*>(*$1, ptmp);

		    /* Get the port declaration details, the port type
		       and what not, from context data stored by the
		       last port_declaration rule. */
		  pform_module_define_port(@3, $3,
					port_declaration_context.port_type,
					port_declaration_context.port_net_type,
					port_declaration_context.sign_flag,
					port_declaration_context.range, 0);
		  delete $1;
		  $$ = tmp;
		}
        ;

port_declaration
	: attribute_list_opt
          K_input net_type_opt signed_opt range_opt IDENTIFIER
		{ Module::port_t*ptmp;
		  ptmp = pform_module_port_reference($6, @2.text,
						     @2.first_line);
		  pform_module_define_port(@2, $6, NetNet::PINPUT,
					   $3, $4, $5, $1);
		  port_declaration_context.port_type = NetNet::PINPUT;
		  port_declaration_context.port_net_type = $3;
		  port_declaration_context.sign_flag = $4;
		  port_declaration_context.range = $5;
		  delete $1;
		  delete $6;
		  $$ = ptmp;
		}
	| attribute_list_opt
          K_inout  net_type_opt signed_opt range_opt IDENTIFIER
		{ Module::port_t*ptmp;
		  ptmp = pform_module_port_reference($6, @2.text,
						     @2.first_line);
		  pform_module_define_port(@2, $6, NetNet::PINOUT,
					   $3, $4, $5, $1);
		  port_declaration_context.port_type = NetNet::PINOUT;
		  port_declaration_context.port_net_type = $3;
		  port_declaration_context.sign_flag = $4;
		  port_declaration_context.range = $5;
		  delete $1;
		  delete $6;
		  $$ = ptmp;
		}
	| attribute_list_opt
          K_output net_type_opt signed_opt range_opt IDENTIFIER
		{ Module::port_t*ptmp;
		  ptmp = pform_module_port_reference($6, @2.text,
						     @2.first_line);
		  pform_module_define_port(@2, $6, NetNet::POUTPUT,
					   $3, $4, $5, $1);
		  port_declaration_context.port_type = NetNet::POUTPUT;
		  port_declaration_context.port_net_type = $3;
		  port_declaration_context.sign_flag = $4;
		  port_declaration_context.range = $5;
		  delete $1;
		  delete $6;
		  $$ = ptmp;
		}
	| attribute_list_opt
          K_output var_type signed_opt range_opt IDENTIFIER
		{ Module::port_t*ptmp;
		  ptmp = pform_module_port_reference($6, @2.text,
						     @2.first_line);
		  pform_module_define_port(@2, $6, NetNet::POUTPUT,
					   $3, $4, $5, $1);
		  port_declaration_context.port_type = NetNet::POUTPUT;
		  port_declaration_context.port_net_type = $3;
		  port_declaration_context.sign_flag = $4;
		  port_declaration_context.range = $5;
		  delete $1;
		  delete $6;
		  $$ = ptmp;
		}
	| attribute_list_opt
          K_output var_type signed_opt range_opt IDENTIFIER '=' expression
		{ Module::port_t*ptmp;
		  ptmp = pform_module_port_reference($6, @2.text,
						     @2.first_line);
		  pform_module_define_port(@2, $6, NetNet::POUTPUT,
					   $3, $4, $5, $1);
		  port_declaration_context.port_type = NetNet::POUTPUT;
		  port_declaration_context.port_net_type = $3;
		  port_declaration_context.sign_flag = $4;
		  port_declaration_context.range = $5;

		  if (! pform_expression_is_constant($8))
			yyerror(@8, "error: register declaration assignment"
				" value must be a constant expression.");
		  pform_make_reginit(@6, $6, $8);

		  delete $1;
		  delete $6;
		  $$ = ptmp;
		}
	;



net_type_opt
	: net_type { $$ = $1; }
	| { $$ = NetNet::IMPLICIT; }
	;

signed_opt : K_signed { $$ = true; } | {$$ = false; } ;

  /* An lavalue is the expression that can go on the left side of a
     continuous assign statement. This checks (where it can) that the
     expression meets the constraints of continuous assignments. */
lavalue
	: identifier
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| identifier '[' expression ']'
		{ PEIdent*tmp = new PEIdent(*$1);
		  PExpr*sel = $3;
		  if (! pform_expression_is_constant(sel)) {
			yyerror(@2, "error: Bit select in lvalue must "
			        "contain a constant expression.");
			delete sel;
		  } else {
			tmp->msb_ = sel;
		  }
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| identifier range
		{ PEIdent*tmp = new PEIdent(*$1);
		  assert($2->count() == 2);
		  tmp->msb_ = (*$2)[0];
		  tmp->lsb_ = (*$2)[1];
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  delete $2;
		  $$ = tmp;
		}
	| '{' expression_list '}'
		{ PEConcat*tmp = new PEConcat(*$2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $2;
		  $$ = tmp;
		}
	;

  /* An lpvalue is the expression that can go on the left side of a
     procedural assignment. This rule handles only procedural assignments. */
lpvalue
	: identifier
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| identifier '[' expression ']'
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->msb_ = $3;
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;

		  $$ = tmp;
		}
	| identifier '[' expression ':' expression ']'
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->msb_ = $3;
		  tmp->lsb_ = $5;
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| '{' expression_list '}'
		{ PEConcat*tmp = new PEConcat(*$2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $2;
		  $$ = tmp;
		}
	;

assign
	: lavalue '=' expression
		{ svector<PExpr*>*tmp = new svector<PExpr*>(2);
		  (*tmp)[0] = $1;
		  (*tmp)[1] = $3;
		  $$ = tmp;
		}
	;

assign_list
	: assign_list ',' assign
		{ svector<PExpr*>*tmp = new svector<PExpr*>(*$1, *$3);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	| assign
		{ $$ = $1; }
	;


  /* This is the global structure of a module. A module in a start
     section, with optional ports, then an optional list of module
     items, and finally an end marker. */

module  : attribute_list_opt module_start IDENTIFIER
		{ pform_startmodule($3, @2.text, @2.first_line, $1); }
          module_parameter_port_list_opt
	  module_port_list_opt ';'
		{ pform_module_set_ports($6); }
	  module_item_list_opt
	  K_endmodule
		{ pform_endmodule($3);
		  delete $3;
		}

	;

module_start : K_module | K_macromodule ;

range_delay : range_opt delay3_opt
		{ $$.range = $1; $$.delay = $2; }
	;

module_port_list_opt
	: '(' list_of_ports ')' { $$ = $2; }
	| '(' list_of_port_declarations ')' { $$ = $2; }
	|                       { $$ = 0; }
	;

  /* Module declarations include optional ANSI style module parameter
     ports. These are simply advance ways to declare parameters, so
     that the port declarations may use them. */
module_parameter_port_list_opt
	:
        | '#' '(' module_parameter_port_list ')'
	;

module_parameter_port_list
	: K_parameter parameter_assign
	| module_parameter_port_list ',' parameter_assign
	| module_parameter_port_list ',' K_parameter parameter_assign
	;

module_item
	: attribute_list_opt net_type signed_opt range_delay list_of_identifiers ';'
		{ pform_makewire(@2, $4.range, $3, $5, $2,
				 NetNet::NOT_A_PORT, $1);
		  if ($4.delay != 0) {
			yyerror(@4, "sorry: net delays not supported.");
			delete $4.delay;
		  }
		  if ($1) delete $1;
		}
	| attribute_list_opt net_type signed_opt range_delay net_decl_assigns ';'
		{ pform_makewire(@2, $4.range, $3, $4.delay,
				 str_strength, $5, $2);
		  if ($1) {
			yyerror(@3, "sorry: Attributes not supported "
				"on net declaration assignments.");
			delete $1;
		  }
		}
	| attribute_list_opt net_type signed_opt drive_strength net_decl_assigns ';'
		{ pform_makewire(@2, 0, $3, 0, $4, $5, $2);
		  if ($1) {
			yyerror(@4, "sorry: Attributes not supported "
				"on net declaration assignments.");
			delete $1;
		  }
		}
	| K_trireg charge_strength_opt range_delay list_of_identifiers ';'
		{ yyerror(@1, "sorry: trireg nets not supported.");
		  delete $3.range;
		  delete $3.delay;
		}

	| port_type signed_opt range_delay list_of_identifiers ';'
		{ pform_set_port_type(@1, $4, $3.range, $2, $1);
		}

  /* The next two rules handle Verilog 2001 statements of the form:
       input wire signed [h:l] <list>;
     This creates the wire and sets the port type all at once. */

	| port_type net_type signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, $1, 0);
		}

	| K_output var_type signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, NetNet::POUTPUT, 0);
		}

  /* var_type declaration (reg variables) cannot be input or output,
     because the port declaration implies an external driver, which
     cannot be attached to a reg. These rules catch that error early. */

	| K_input var_type signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, NetNet::PINPUT, 0);
		  yyerror(@2, "error: reg variables cannot be inputs.");
		}

	| K_inout var_type signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, NetNet::PINOUT, 0);
		  yyerror(@2, "error: reg variables cannot be inouts.");
		}

	| port_type signed_opt range_delay error ';'
		{ yyerror(@3, "error: Invalid variable list"
			  " in port declaration.");
		  if ($3.range) delete $3.range;
		  if ($3.delay) delete $3.delay;
		  yyerrok;
		}

  /* block_item_decl rule is shared with task blocks and named
     begin/end. */

	| attribute_list_opt block_item_decl
                { ; }

  /* */

	| K_defparam defparam_assign_list ';'
	| K_event list_of_identifiers ';'
		{ pform_make_events($2, @1.text, @1.first_line);
		}

  /* Most gate types have an optional drive strength and optional
     three-value delay. These rules handle the different cases. */

	| attribute_list_opt gatetype gate_instance_list ';'
		{ pform_makegates($2, str_strength, 0, $3, $1);
		}

	| attribute_list_opt gatetype delay3 gate_instance_list ';'
		{ pform_makegates($2, str_strength, $3, $4, $1);
		}

	| attribute_list_opt gatetype drive_strength gate_instance_list ';'
		{ pform_makegates($2, $3, 0, $4, $1);
		}

	| attribute_list_opt gatetype drive_strength delay3 gate_instance_list ';'
		{ pform_makegates($2, $3, $4, $5, $1);
		}

  /* Pullup and pulldown devices cannot have delays, and their
     strengths are limited. */

	| K_pullup gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLUP, pull_strength, 0,
				  $2, 0);
		}
	| K_pulldown gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLDOWN, pull_strength,
				  0, $2, 0);
		}

	| K_pullup '(' dr_strength1 ')' gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLUP, $3, 0, $5, 0);
		}

	| K_pulldown '(' dr_strength0 ')' gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLDOWN, $3, 0, $5, 0);
		}

  /* This rule handles instantiations of modules and user defined
     primitives. These devices to not have delay lists or strengths,
     but then can have parameter lists. */

	| IDENTIFIER parameter_value_opt gate_instance_list ';'
		{ perm_string tmp1 = lex_strings.make($1);
		  pform_make_modgates(tmp1, $2, $3);
		  delete $1;
		}

        | IDENTIFIER parameter_value_opt error ';'
		{ yyerror(@1, "error: Invalid module instantiation");
		}

  /* Continuous assignment can have an optional drive strength, then
     an optional delay3 that applies to all the assignments in the
     assign_list. */

	| K_assign drive_strength_opt delay3_opt assign_list ';'
		{ pform_make_pgassign_list($4, $3, $2, @1.text, @1.first_line); }

  /* Always and initial items are behavioral processes. */

	| attribute_list_opt K_always statement
		{ PProcess*tmp = pform_make_behavior(PProcess::PR_ALWAYS,
						     $3, $1);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		}
	| attribute_list_opt K_initial statement
		{ PProcess*tmp = pform_make_behavior(PProcess::PR_INITIAL,
						     $3, $1);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		}

  /* The task declaration rule matches the task declaration
     header, then pushes the function scope. This causes the
     definitions in the task_body to take on the scope of the task
     instead of the module. */

	| K_task IDENTIFIER ';'
		{ pform_push_scope($2); }
	  task_item_list_opt statement_opt
	  K_endtask
		{ PTask*tmp = new PTask;
		  perm_string tmp2 = lex_strings.make($2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  tmp->set_ports($5);
		  tmp->set_statement($6);
		  pform_set_task(tmp2, tmp);
		  pform_pop_scope();
		  delete $2;
		}

  /* The function declaration rule matches the function declaration
     header, then pushes the function scope. This causes the
     definitions in the func_body to take on the scope of the function
     instead of the module. */

        | K_function function_range_or_type_opt IDENTIFIER ';'
                { pform_push_scope($3); }
          function_item_list statement
          K_endfunction
		{ perm_string name = lex_strings.make($3);
		  PFunction *tmp = new PFunction(name);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  tmp->set_ports($6);
		  tmp->set_statement($7);
		  tmp->set_return($2);
		  pform_set_function(name, tmp);
		  pform_pop_scope();
		  delete $3;
		}

  /* specify blocks are parsed but ignored. */

	| K_specify K_endspecify
		{ /* empty lists are legal syntax. */ }

	| K_specify specify_item_list K_endspecify
		{
		}

	| K_specify error K_endspecify
		{ yyerror(@1, "error: syntax error in specify block");
		  yyerrok;
		}

  /* These rules match various errors that the user can type into
     module items. These rules try to catch them at a point where a
     reasonable error message can be produced. */

	| error ';'
		{ yyerror(@1, "error: invalid module item. "
			  "Did you forget an initial or always?");
		  yyerrok;
		}

	| K_assign error '=' expression ';'
		{ yyerror(@1, "error: syntax error in left side "
			  "of continuous assignment.");
		  yyerrok;
		}

	| K_assign error ';'
		{ yyerror(@1, "error: syntax error in "
			  "continuous assignment");
		  yyerrok;
		}

	| K_function error K_endfunction
		{ yyerror(@1, "error: I give up on this "
			  "function definition.");
		  yyerrok;
		}

  /* These rules are for the Icarus Verilog specific $attribute
     extensions. Then catch the parameters of the $attribute keyword. */

	| KK_attribute '(' IDENTIFIER ',' STRING ',' STRING ')' ';'
		{ perm_string tmp3 = lex_strings.make($3);
		  perm_string tmp5 = lex_strings.make($5);
		  pform_set_attrib(tmp3, tmp5, $7);
		  delete $3;
		  delete $5;
		}
	| KK_attribute '(' error ')' ';'
		{ yyerror(@1, "error: Malformed $attribute parameter list."); }
	;


module_item_list
	: module_item_list module_item
	| module_item
	;

module_item_list_opt
	: module_item_list
	|
	;


  /* A net declaration assignment allows the programmer to combine the
     net declaration and the continuous assignment into a single
     statement.

     Note that the continuous assignment statement is generated as a
     side effect, and all I pass up is the name of the l-value. */

net_decl_assign
	: IDENTIFIER '=' expression
		{ net_decl_assign_t*tmp = new net_decl_assign_t;
		  tmp->next = tmp;
		  tmp->name = $1;
		  tmp->expr = $3;
		  $$ = tmp;
		}
	;

net_decl_assigns
	: net_decl_assigns ',' net_decl_assign
		{ net_decl_assign_t*tmp = $1;
		  $3->next = tmp->next;
		  tmp->next = $3;
		  $$ = tmp;
		}
	| net_decl_assign
		{ $$ = $1;
		}
	;

net_type
	: K_wire    { $$ = NetNet::WIRE; }
	| K_tri     { $$ = NetNet::TRI; }
	| K_tri1    { $$ = NetNet::TRI1; }
	| K_supply0 { $$ = NetNet::SUPPLY0; }
	| K_wand    { $$ = NetNet::WAND; }
	| K_triand  { $$ = NetNet::TRIAND; }
	| K_tri0    { $$ = NetNet::TRI0; }
	| K_supply1 { $$ = NetNet::SUPPLY1; }
	| K_wor     { $$ = NetNet::WOR; }
	| K_trior   { $$ = NetNet::TRIOR; }
	;

var_type
	: K_reg { $$ = NetNet::REG; }
	;

parameter_assign
	: IDENTIFIER '=' expression
		{ PExpr*tmp = $3;
		  if (!pform_expression_is_constant(tmp)) {
			yyerror(@3, "error: parameter value "
			            "must be a constant expression.");
			delete tmp;
			tmp = 0;
		  } else {
			pform_set_parameter(lex_strings.make($1),
					    active_signed,
					    active_range, tmp);
		  }
		  delete $1;
		}
	;

parameter_assign_decl
	: parameter_assign_list
	| range { active_range = $1; active_signed = false; }
          parameter_assign_list
		{ active_range = 0;
		  active_signed = false;
		}
	| K_signed range { active_range = $2; active_signed = true; }
          parameter_assign_list
		{ active_range = 0;
		  active_signed = false;
		}
	;

parameter_assign_list
	: parameter_assign
	| parameter_assign_list ',' parameter_assign
	;


  /* Localparam assignments and assignment lists are broken into
     separate BNF so that I can call slightly different parameter
     handling code. They parse the same as parameters, they just
     behave differently when someone tries to override them. */

localparam_assign
	: IDENTIFIER '=' expression
		{ PExpr*tmp = $3;
		  if (!pform_expression_is_constant(tmp)) {
			yyerror(@3, "error: parameter value "
			            "must be constant.");
			delete tmp;
			tmp = 0;
		  } else {
			pform_set_localparam(lex_strings.make($1),
					     active_signed,
					     active_range, tmp);
		  }
		  delete $1;
		}
	;

localparam_assign_decl
	: localparam_assign_list
	| range { active_range = $1; active_signed = false; }
          localparam_assign_list
		{ active_range = 0;
		  active_signed = false;
		}
	| K_signed range { active_range = $2; active_signed = true; }
          localparam_assign_list
		{ active_range = 0;
		  active_signed = false;
		}
	;

localparam_assign_list
	: localparam_assign
	| localparam_assign_list ',' localparam_assign
	;



  /* The parameters of a module instance can be overridden by writing
     a list of expressions in a syntax much like a delay list. (The
     difference being the list can have any length.) The pform that
     attaches the expression list to the module checks that the
     expressions are constant.

     Although the BNF in IEEE1364-1995 implies that parameter value
     lists must be in parentheses, in practice most compilers will
     accept simple expressions outside of parentheses if there is only
     one value, so I'll accept simple numbers here.

     The parameter value by name syntax is OVI enhancement BTF-B06 as
     approved by WG1364 on 6/28/1998. */

parameter_value_opt
	: '#' '(' expression_list ')'
		{ struct parmvalue_t*tmp = new struct parmvalue_t;
		  tmp->by_order = $3;
		  tmp->by_name = 0;
		  $$ = tmp;
		}
	| '#' '(' parameter_value_byname_list ')'
		{ struct parmvalue_t*tmp = new struct parmvalue_t;
		  tmp->by_order = 0;
		  tmp->by_name = $3;
		  $$ = tmp;
		}
	| '#' DEC_NUMBER
		{ assert($2);
		  PENumber*tmp = new PENumber($2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);

		  struct parmvalue_t*lst = new struct parmvalue_t;
		  lst->by_order = new svector<PExpr*>(1);
		  (*lst->by_order)[0] = tmp;
		  lst->by_name = 0;
		  $$ = lst;
		}
	| '#' error
		{ yyerror(@1, "error: syntax error in parameter value "
			  "assignment list.");
		  $$ = 0;
		}
	|
		{ $$ = 0; }
	;

parameter_value_byname
	: '.' IDENTIFIER '(' expression ')'
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = $4;
		  free($2);
		  $$ = tmp;
		}
	| '.' IDENTIFIER '(' ')'
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = 0;
		  free($2);
		  $$ = tmp;
		}
	;

parameter_value_byname_list
	: parameter_value_byname
		{ svector<named_pexpr_t*>*tmp = new svector<named_pexpr_t*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	| parameter_value_byname_list ',' parameter_value_byname
		{ svector<named_pexpr_t*>*tmp =
			new svector<named_pexpr_t*>(*$1,$3);
		  delete $1;
		  $$ = tmp;
		}
	;


  /* The port (of a module) is a fairly complex item. Each port is
     handled as a Module::port_t object. A simple port reference has a
     name and a PExpr object, but more complex constructs are possible
     where the name can be attached to a list of PWire objects.

     The port_reference returns a Module::port_t, and so does the
     port_reference_list. The port_reference_list may have built up a
     list of PWires in the port_t object, but it is still a single
     Module::port_t object.

     The port rule below takes the built up Module::port_t object and
     tweaks its name as needed. */

port
	: port_reference
		{ $$ = $1; }

  /* This syntax attaches an external name to the port reference so
     that the caller can bind by name to non-trivial port
     references. The port_t object gets its PWire from the
     port_reference, but its name from the IDENTIFIER. */

	| '.' IDENTIFIER '(' port_reference ')'
		{ Module::port_t*tmp = $4;
		  tmp->name = lex_strings.make($2);
		  delete $2;
		  $$ = tmp;
		}

  /* A port can also be a concatenation of port references. In this
     case the port does not have a name available to the outside, only
     positional parameter passing is possible here. */

	| '{' port_reference_list '}'
		{ Module::port_t*tmp = $2;
		  tmp->name = perm_string();
		  $$ = tmp;
		}

  /* This attaches a name to a port reference concatenation list so
     that parameter passing be name is possible. */

	| '.' IDENTIFIER '(' '{' port_reference_list '}' ')'
		{ Module::port_t*tmp = $5;
		  tmp->name = lex_strings.make($2);
		  delete $2;
		  $$ = tmp;
		}
	;

port_opt
	: port { $$ = $1; }
	| { $$ = 0; }
	;


  /* A port reference is an internal (to the module) name of the port,
     possibly with a part of bit select to attach it to specific bits
     of a signal fully declared inside the module.

     The parser creates a PEIdent for every port reference, even if the
     signal is bound to different ports. The elaboration figures out
     the mess that this creates. The port_reference (and the
     port_reference_list below) puts the port reference PEIdent into the
     port_t object to pass it up to the module declaration code. */

port_reference

	: IDENTIFIER
		{ Module::port_t*ptmp;
		  ptmp = pform_module_port_reference($1, @1.text,
						     @1.first_line);
		  delete $1;
		  $$ = ptmp;
		}

	| IDENTIFIER '[' expression ':' expression ']'
		{ PEIdent*wtmp = new PEIdent(hname_t($1));
		  wtmp->set_file(@1.text);
		  wtmp->set_lineno(@1.first_line);
		  if (!pform_expression_is_constant($3)) {
			yyerror(@3, "error: msb expression of "
				"port part select must be constant.");
		  }
		  if (!pform_expression_is_constant($5)) {
			yyerror(@5, "error: lsb expression of "
				"port part select must be constant.");
		  }
		  wtmp->msb_ = $3;
		  wtmp->lsb_ = $5;
		  Module::port_t*ptmp = new Module::port_t;
		  ptmp->name = perm_string();
		  ptmp->expr = svector<PEIdent*>(1);
		  ptmp->expr[0] = wtmp;
		  delete $1;
		  $$ = ptmp;
		}

	| IDENTIFIER '[' expression ']'
		{ PEIdent*tmp = new PEIdent(hname_t($1));
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  if (!pform_expression_is_constant($3)) {
			yyerror(@3, "error: port bit select "
				"must be constant.");
		  }
		  tmp->msb_ = $3;
		  Module::port_t*ptmp = new Module::port_t;
		  ptmp->name = perm_string();
		  ptmp->expr = svector<PEIdent*>(1);
		  ptmp->expr[0] = tmp;
		  delete $1;
		  $$ = ptmp;
		}

	| IDENTIFIER '[' error ']'
		{ yyerror(@1, "error: invalid port bit select");
		  Module::port_t*ptmp = new Module::port_t;
		  PEIdent*wtmp = new PEIdent(hname_t($1));
		  wtmp->set_file(@1.text);
		  wtmp->set_lineno(@1.first_line);
		  ptmp->name = lex_strings.make($1);
		  ptmp->expr = svector<PEIdent*>(1);
		  ptmp->expr[0] = wtmp;
		  delete $1;
		  $$ = ptmp;
		}
	;


port_reference_list
	: port_reference
		{ $$ = $1; }
	| port_reference_list ',' port_reference
		{ Module::port_t*tmp = $1;
		  tmp->expr = svector<PEIdent*>(tmp->expr, $3->expr);
		  delete $3;
		  $$ = tmp;
		}
	;

  /* The port_name rule is used with a module is being *instantiated*,
     and not when it is being declared. See the port rule if you are
     looking for the ports of a module declaration. */

port_name
	: '.' IDENTIFIER '(' expression ')'
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = $4;
		  delete $2;
		  $$ = tmp;
		}
	| '.' IDENTIFIER '(' error ')'
		{ yyerror(@4, "error: invalid port connection expression.");
		  named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = 0;
		  delete $2;
		  $$ = tmp;
		}
	| '.' IDENTIFIER '(' ')'
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = 0;
		  delete $2;
		  $$ = tmp;
		}
	;

port_name_list
	: port_name_list ',' port_name
		{ svector<named_pexpr_t*>*tmp;
		  tmp = new svector<named_pexpr_t*>(*$1, $3);
		  delete $1;
		  $$ = tmp;
		}
	| port_name
		{ svector<named_pexpr_t*>*tmp = new svector<named_pexpr_t*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	;

port_type
	: K_input { $$ = NetNet::PINPUT; }
	| K_output { $$ = NetNet::POUTPUT; }
	| K_inout { $$ = NetNet::PINOUT; }
	;

range
	: '[' expression ':' expression ']'
		{ svector<PExpr*>*tmp = new svector<PExpr*> (2);
		  if (!pform_expression_is_constant($2))
			yyerror(@2, "error: msb of range must be constant.");

		  (*tmp)[0] = $2;

		  if (!pform_expression_is_constant($4))
			yyerror(@4, "error: lsb of range must be constant.");

		  (*tmp)[1] = $4;

		  $$ = tmp;
		}
	;

range_opt
	: range
	| { $$ = 0; }
	;

  /* This is used to express the return type of a function. */
function_range_or_type_opt
	: range      { $$.range = $1; $$.type = PTF_REG; }
	| K_integer  { $$.range = 0;  $$.type = PTF_INTEGER; }
	| K_real     { $$.range = 0;  $$.type = PTF_REAL; }
	| K_realtime { $$.range = 0;  $$.type = PTF_REALTIME; }
	| K_time     { $$.range = 0;  $$.type = PTF_TIME; }
	|            { $$.range = 0;  $$.type = PTF_REG; }
	;

  /* The register_variable rule is matched only when I am parsing
     variables in a "reg" definition. I therefore know that I am
     creating registers and I do not need to let the containing rule
     handle it. The register variable list simply packs them together
     so that bit ranges can be assigned. */
register_variable
	: IDENTIFIER
		{ pform_makewire(@1, $1, NetNet::REG,
				 NetNet::NOT_A_PORT, 0);
		  $$ = $1;
		}
	| IDENTIFIER '=' expression
		{ pform_makewire(@1, $1, NetNet::REG,
				 NetNet::NOT_A_PORT, 0);
		  if (! pform_expression_is_constant($3))
			yyerror(@3, "error: register declaration assignment"
				" value must be a constant expression.");
		  pform_make_reginit(@1, $1, $3);
		  $$ = $1;
		}
	| IDENTIFIER '[' expression ':' expression ']'
		{ pform_makewire(@1, $1, NetNet::REG,
				 NetNet::NOT_A_PORT, 0);
		  if (! pform_expression_is_constant($3))
			yyerror(@3, "error: msb of register range must be constant.");
		  if (! pform_expression_is_constant($5))
			yyerror(@3, "error: lsb of register range must be constant.");
		  pform_set_reg_idx($1, $3, $5);
		  $$ = $1;
		}
	;

register_variable_list
	: register_variable
		{ list<perm_string>*tmp = new list<perm_string>;
		  tmp->push_back(lex_strings.make($1));
		  $$ = tmp;
		  delete[]$1;
		}
	| register_variable_list ',' register_variable
		{ list<perm_string>*tmp = $1;
		  tmp->push_back(lex_strings.make($3));
		  $$ = tmp;
		  delete[]$3;
		}
	;

specify_item
	: K_specparam specparam_list ';'
	| specify_simple_path_decl ';'
		{
		}
	| specify_edge_path_decl ';'
		{
		}
	| K_if '(' expression ')' specify_simple_path_decl ';'
		{
		}
	| K_if '(' expression ')' specify_edge_path_decl ';'
		{
		}
	| K_ifnone specify_simple_path_decl ';'
		{
		}
	| K_Shold '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		}
	| K_Speriod '(' spec_reference_event ',' delay_value
	  spec_notifier_opt ')' ';'
		{ delete $5;
		}
	| K_Srecovery '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		}
	| K_Ssetup '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		}
	| K_Ssetuphold '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		  delete $9;
		}
	| K_Srecrem '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		  delete $9;
		}
	| K_Swidth '(' spec_reference_event ',' delay_value ',' expression
	  spec_notifier_opt ')' ';'
		{ delete $5;
		  delete $7;
		}
	| K_Swidth '(' spec_reference_event ',' delay_value ')' ';'
		{ delete $5;
		}
	;

specify_delay_value_list
	: delay_value
		{ delete $1;
		}
	| specify_delay_value_list ',' delay_value
		{ delete $3;
		}
	;

specify_item_list
	: specify_item
	| specify_item_list specify_item
	;

specify_edge_path_decl
	: specify_edge_path '=' '(' specify_delay_value_list ')'
	| specify_edge_path '=' delay_value_simple
	;

specify_edge_operator
	: K_posedge
	| K_negedge
	;

  /* The first two rules are not in development. */
specify_edge_path
	: '(' specify_edge_operator specify_path_identifiers spec_polarity K_EG IDENTIFIER ')'
	| '(' specify_edge_operator specify_path_identifiers spec_polarity K_SG IDENTIFIER ')'
	| '('                       specify_path_identifiers spec_polarity K_EG '(' specify_path_identifiers polarity_operator expression ')' ')'
	| '(' specify_edge_operator specify_path_identifiers spec_polarity K_EG '(' specify_path_identifiers polarity_operator expression ')' ')'
	| '('                       specify_path_identifiers spec_polarity K_SG '(' specify_path_identifiers polarity_operator expression ')' ')'
	| '(' specify_edge_operator specify_path_identifiers spec_polarity K_SG '(' specify_path_identifiers polarity_operator expression ')' ')'
	;

polarity_operator
        : K_PO_POS
	| K_PO_NEG
	| ':'
	;

specify_simple_path_decl
	: specify_simple_path '=' '(' specify_delay_value_list ')'
	| specify_simple_path '=' delay_value_simple
	| specify_simple_path '=' '(' error ')'
		{ yyerror(@2, "Syntax error in delay value list.");
		  yyerrok;
		}
	;

specify_simple_path
	: '(' specify_path_identifiers spec_polarity
              K_EG specify_path_identifiers ')'
		{ pform_make_specify_path($2, $3, false, $5); }
	| '(' specify_path_identifiers spec_polarity
              K_SG specify_path_identifiers ')'
		{ pform_make_specify_path($2, $3, true, $5); }
	| '(' error ')'
		{ yyerror(@2, "Invalid simple path");
		  yyerrok;
		}
	;

specify_path_identifiers
	: IDENTIFIER
		{ list<perm_string>*tmp = new list<perm_string>;
		  tmp->push_back(lex_strings.make($1));
		  $$ = tmp;
		  delete[]$1;
		}
	| IDENTIFIER '[' expr_primary ']'
		{ list<perm_string>*tmp = new list<perm_string>;
		  tmp->push_back(lex_strings.make($1));
		  $$ = tmp;
		  delete[]$1;
		}
	| specify_path_identifiers ',' IDENTIFIER
		{ list<perm_string>*tmp = $1;
		  tmp->push_back(lex_strings.make($3));
		  $$ = tmp;
		  delete[]$3;
		}
	| specify_path_identifiers ',' IDENTIFIER '[' expr_primary ']'
		{ list<perm_string>*tmp = $1;
		  tmp->push_back(lex_strings.make($3));
		  $$ = tmp;
		  delete[]$3;
		}
	;

specparam
	: IDENTIFIER '=' expression
		{ PExpr*tmp = $3;
		  if (!pform_expression_is_constant(tmp)) {
			yyerror(@3, "error: specparam value "
			            "must be a constant expression.");
			delete tmp;
			tmp = 0;
		  } else {
			pform_set_specparam(lex_strings.make($1), tmp);
		  }
		  delete $1;
		}
	| IDENTIFIER '=' expression ':' expression ':' expression
		{ delete $1;
		  delete $3;
		  delete $5;
		  delete $7;
		}
	| PATHPULSE_IDENTIFIER '=' expression
		{ delete $1;
		  delete $3;
		}
	| PATHPULSE_IDENTIFIER '=' '(' expression ',' expression ')'
		{ delete $1;
		  delete $4;
		  delete $6;
		}
	;

specparam_list
	: specparam
	| specparam_list ',' specparam
	;

spec_polarity
	: '+'  { $$ = '+'; }
	| '-'  { $$ = '-'; }
	|      { $$ = 0;   }
	;

spec_reference_event
	: K_posedge expression
		{ delete $2; }
	| K_negedge expression
		{ delete $2; }
	| K_posedge expr_primary K_TAND expression
		{ delete $2;
		  delete $4;
		}
	| K_negedge expr_primary K_TAND expression
		{ delete $2;
		  delete $4;
		}
	| K_edge '[' edge_descriptor_list ']' expr_primary
		{ delete $5; }
	| K_edge '[' edge_descriptor_list ']' expr_primary K_TAND expression
		{ delete $5;
		  delete $7;
		}
	| expr_primary K_TAND expression
		{ delete $1;
		  delete $3;
		}
        | expr_primary
		{ delete $1; }
	;

edge_descriptor_list
	: edge_descriptor_list ',' K_edge_descriptor
	| K_edge_descriptor
	;

spec_notifier_opt
	: /* empty */
		{  }
	| spec_notifier
		{  }
	;
spec_notifier
	: ','
		{  }
	| ','  identifier
		{ delete $2; }
	| spec_notifier ','
		{  }
	| spec_notifier ',' identifier
                { delete $3; }
	| spec_notifier ',' identifier '[' expr_primary ']'
                { delete $3;
		  delete $5;
		}
	| IDENTIFIER
		{ delete $1; }
	;


statement

  /* assign and deassign statements are procedural code to do
     structural assignments, and to turn that structural assignment
     off. This stronger then any other assign, but weaker then the
     force assignments. */

	: K_assign lavalue '=' expression ';'
		{ PCAssign*tmp = new PCAssign($2, $4);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}

	| K_deassign lavalue ';'
		{ PDeassign*tmp = new PDeassign($2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}


  /* Force and release statements are similar to assignments,
     syntactically, but they will be elaborated differently. */

	| K_force lavalue '=' expression ';'
		{ PForce*tmp = new PForce($2, $4);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_release lavalue ';'
		{ PRelease*tmp = new PRelease($2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}

  /* begin-end blocks come in a variety of forms, including named and
     anonymous. The named blocks can also carry their own reg
     variables, which are placed in the scope created by the block
     name. These are handled by pushing the scope name then matching
     the declarations. The scope is popped at the end of the block. */

	| K_begin statement_list K_end
		{ PBlock*tmp = new PBlock(PBlock::BL_SEQ, *$2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $2;
		  $$ = tmp;
		}
	| K_begin ':' IDENTIFIER
		{ pform_push_scope($3); }
	  block_item_decls_opt
	  statement_list K_end
		{ pform_pop_scope();
		  PBlock*tmp = new PBlock(lex_strings.make($3),
					  PBlock::BL_SEQ, *$6);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $3;
		  delete $6;
		  $$ = tmp;
		}
	| K_begin K_end
		{ PBlock*tmp = new PBlock(PBlock::BL_SEQ);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_begin ':' IDENTIFIER K_end
		{ PBlock*tmp = new PBlock(PBlock::BL_SEQ);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
/*	| K_begin error K_end
		{ yyerrok; }
*/
  /* fork-join blocks are very similar to begin-end blocks. In fact,
     from the parser's perspective there is no real difference. All we
     need to do is remember that this is a parallel block so that the
     code generator can do the right thing. */

	| K_fork ':' IDENTIFIER
		{ pform_push_scope($3); }
	  block_item_decls_opt
	  statement_list K_join
		{ pform_pop_scope();
		  PBlock*tmp = new PBlock(lex_strings.make($3),
					  PBlock::BL_PAR, *$6);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $3;
		  delete $6;
		  $$ = tmp;
		}
	| K_fork K_join
		{ PBlock*tmp = new PBlock(PBlock::BL_PAR);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_fork ':' IDENTIFIER K_join
		{ PBlock*tmp = new PBlock(PBlock::BL_PAR);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $3;
		  $$ = tmp;
		}

	| K_disable identifier ';'
		{ PDisable*tmp = new PDisable(*$2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $2;
		  $$ = tmp;
		}
	| K_TRIGGER identifier ';'
		{ PTrigger*tmp = new PTrigger(*$2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  delete $2;
		  $$ = tmp;
		}
	| K_forever statement
		{ PForever*tmp = new PForever($2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_fork statement_list K_join
		{ PBlock*tmp = new PBlock(PBlock::BL_PAR, *$2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $2;
		  $$ = tmp;
		}
	| K_repeat '(' expression ')' statement
		{ PRepeat*tmp = new PRepeat($3, $5);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_case '(' expression ')' case_items K_endcase
		{ PCase*tmp = new PCase(NetCase::EQ, $3, $5);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_casex '(' expression ')' case_items K_endcase
		{ PCase*tmp = new PCase(NetCase::EQX, $3, $5);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_casez '(' expression ')' case_items K_endcase
		{ PCase*tmp = new PCase(NetCase::EQZ, $3, $5);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_case '(' expression ')' error K_endcase
		{ yyerrok; }
	| K_casex '(' expression ')' error K_endcase
		{ yyerrok; }
	| K_casez '(' expression ')' error K_endcase
		{ yyerrok; }
	| K_if '(' expression ')' statement_opt %prec less_than_K_else
		{ PCondit*tmp = new PCondit($3, $5, 0);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_if '(' expression ')' statement_opt K_else statement_opt
		{ PCondit*tmp = new PCondit($3, $5, $7);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_if '(' error ')' statement_opt %prec less_than_K_else
		{ yyerror(@1, "error: Malformed conditional expression.");
		  $$ = $5;
		}
	| K_if '(' error ')' statement_opt K_else statement_opt
		{ yyerror(@1, "error: Malformed conditional expression.");
		  $$ = $5;
		}
	| K_for '(' lpvalue '=' expression ';' expression ';'
	  lpvalue '=' expression ')' statement
		{ PForStatement*tmp = new PForStatement($3, $5, $7, $9, $11, $13);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_for '(' lpvalue '=' expression ';' expression ';'
	  error ')' statement
		{ $$ = 0;
		  yyerror(@9, "error: Error in for loop step assignment.");
		}
	| K_for '(' lpvalue '=' expression ';' error ';'
	  lpvalue '=' expression ')' statement
		{ $$ = 0;
		  yyerror(@7, "error: Error in for loop condition expression.");
		}
	| K_for '(' error ')' statement
		{ $$ = 0;
		  yyerror(@3, "error: Incomprehensible for loop.");
		}
	| K_while '(' expression ')' statement
		{ PWhile*tmp = new PWhile($3, $5);
		  $$ = tmp;
		}
	| K_while '(' error ')' statement
		{ $$ = 0;
		  yyerror(@3, "error: Error in while loop condition.");
		}
	| delay1 statement_opt
		{ PExpr*del = (*$1)[0];
		  assert($1->count() == 1);
		  PDelayStatement*tmp = new PDelayStatement(del, $2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| event_control attribute_list_opt statement_opt
		{ PEventStatement*tmp = $1;
		  if (tmp == 0) {
			yyerror(@1, "error: Invalid event control.");
			$$ = 0;
		  } else {
			pform_attach_attributes($3, $2);
			tmp->set_statement($3);
			$$ = tmp;
		  }
		  if ($2) delete $2;
		}
	| '@' '*' attribute_list_opt statement_opt
		{ PEventStatement*tmp = new PEventStatement;
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  pform_attach_attributes($4, $3);
		  tmp->set_statement($4);
		  if ($3) delete $3;
		  $$ = tmp;
		}
	| '@' '(' '*' ')' attribute_list_opt statement_opt
		{ PEventStatement*tmp = new PEventStatement;
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  pform_attach_attributes($6, $5);
		  tmp->set_statement($6);
		  if ($5) delete $5;
		  $$ = tmp;
		}
	| lpvalue '=' expression ';'
		{ PAssign*tmp = new PAssign($1,$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| lpvalue K_LE expression ';'
		{ PAssignNB*tmp = new PAssignNB($1,$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| lpvalue '=' delay1 expression ';'
		{ assert($3->count() == 1);
		  PExpr*del = (*$3)[0];
		  PAssign*tmp = new PAssign($1,del,$4);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| lpvalue K_LE delay1 expression ';'
		{ assert($3->count() == 1);
		  PExpr*del = (*$3)[0];
		  PAssignNB*tmp = new PAssignNB($1,del,$4);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| lpvalue '=' event_control expression ';'
		{ PAssign*tmp = new PAssign($1,$3,$4);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| lpvalue '=' K_repeat '(' expression ')' event_control expression ';'
		{ PAssign*tmp = new PAssign($1,$7,$8);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  yyerror(@3, "sorry: repeat event control not supported.");
		  delete $5;
		  $$ = tmp;
		}
	| lpvalue K_LE event_control expression ';'
		{ yyerror(@1, "sorry: Event controls not supported here.");
		  PAssignNB*tmp = new PAssignNB($1,$4);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| lpvalue K_LE K_repeat '(' expression ')' event_control expression ';'
		{ yyerror(@1, "sorry: Event controls not supported here.");
		  delete $5;
		  PAssignNB*tmp = new PAssignNB($1,$8);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_wait '(' expression ')' statement_opt
		{ PEventStatement*tmp;
		  PEEvent*etmp = new PEEvent(PEEvent::POSITIVE, $3);
		  tmp = new PEventStatement(etmp);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  tmp->set_statement($5);
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER '(' expression_list ')' ';'
		{ PCallTask*tmp = new PCallTask(hname_t($1), *$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER ';'
		{ svector<PExpr*>pt (0);
		  PCallTask*tmp = new PCallTask(hname_t($1), pt);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| identifier '(' expression_list_proper ')' ';'
		{ PCallTask*tmp = new PCallTask(*$1, *$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}

 /* NOTE: The standard doesn't really support an empty argument list
    between parentheses, but it seems natural, and people commonly
    want it. So accept it explicitly. */

	| identifier '(' ')' ';'
		{ svector<PExpr*>pt (0);
		  PCallTask*tmp = new PCallTask(*$1, pt);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| identifier ';'
		{ svector<PExpr*>pt (0);
		  PCallTask*tmp = new PCallTask(*$1, pt);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| error ';'
		{ yyerror(@1, "error: malformed statement");
		  yyerrok;
		  $$ = new PNoop;
		}
	;

statement_list
	: statement_list attribute_list_opt statement
                { pform_attach_attributes($3, $2);
		  svector<Statement*>*tmp = new svector<Statement*>(*$1, $3);
		  delete $1;
		  if ($2) delete $2;
		  $$ = tmp;
		}
	| attribute_list_opt statement
		{ pform_attach_attributes($2, $1);
		  svector<Statement*>*tmp = new svector<Statement*>(1);
		  (*tmp)[0] = $2;
		  if ($1) delete $1;
		  $$ = tmp;
		}
	;

statement_opt
	: statement
	| ';' { $$ = 0; }
	;


task_item
	: block_item_decl
	    { $$ = new svector<PWire*>(0); }
	| K_input range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, false,
						$2, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_input K_signed range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, true,
						$3, $4,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT, false,
						$2, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output K_signed range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT, true,
						$3, $4,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT, false,
						$2, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout K_signed range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT, true,
						$3, $4,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	;

task_item_list
	: task_item_list task_item
		{ svector<PWire*>*tmp = new svector<PWire*>(*$1, *$2);
		  delete $1;
		  delete $2;
		  $$ = tmp;
		}
	| task_item
		{ $$ = $1; }
	;

task_item_list_opt
	: task_item_list
		{ $$ = $1; }
	|
		{ $$ = 0; }
	;

udp_body
	: K_table { lex_start_table(); }
	    udp_entry_list
	  K_endtable { lex_end_table(); $$ = $3; }
	;

udp_entry_list
	: udp_comb_entry_list
	| udp_sequ_entry_list
	;

udp_comb_entry
	: udp_input_list ':' udp_output_sym ';'
		{ char*tmp = new char[strlen($1)+3];
		  strcpy(tmp, $1);
		  char*tp = tmp+strlen(tmp);
		  *tp++ = ':';
		  *tp++ = $3;
		  *tp++ = 0;
		  delete[]$1;
		  $$ = tmp;
		}
	;

udp_comb_entry_list
	: udp_comb_entry
		{ list<string>*tmp = new list<string>;
		  tmp->push_back($1);
		  delete $1;
		  $$ = tmp;
		}
	| udp_comb_entry_list udp_comb_entry
		{ list<string>*tmp = $1;
		  tmp->push_back($2);
		  delete $2;
		  $$ = tmp;
		}
	;

udp_sequ_entry_list
	: udp_sequ_entry
		{ list<string>*tmp = new list<string>;
		  tmp->push_back($1);
		  delete $1;
		  $$ = tmp;
		}
	| udp_sequ_entry_list udp_sequ_entry
		{ list<string>*tmp = $1;
		  tmp->push_back($2);
		  delete $2;
		  $$ = tmp;
		}
	;

udp_sequ_entry
	: udp_input_list ':' udp_input_sym ':' udp_output_sym ';'
		{ char*tmp = new char[strlen($1)+5];
		  strcpy(tmp, $1);
		  char*tp = tmp+strlen(tmp);
		  *tp++ = ':';
		  *tp++ = $3;
		  *tp++ = ':';
		  *tp++ = $5;
		  *tp++ = 0;
		  $$ = tmp;
		}
	;

udp_initial
	: K_initial IDENTIFIER '=' number ';'
		{ PExpr*etmp = new PENumber($4);
		  PEIdent*itmp = new PEIdent(hname_t($2));
		  PAssign*atmp = new PAssign(itmp, etmp);
		  atmp->set_file(@2.text);
		  atmp->set_lineno(@2.first_line);
		  delete $2;
		  $$ = atmp;
		}
	;

udp_init_opt
	: udp_initial  { $$ = $1; }
	|              { $$ = 0; }
	;

udp_input_list
	: udp_input_sym
		{ char*tmp = new char[2];
		  tmp[0] = $1;
		  tmp[1] = 0;
		  $$ = tmp;
		}
	| udp_input_list udp_input_sym
		{ char*tmp = new char[strlen($1)+2];
		  strcpy(tmp, $1);
		  char*tp = tmp+strlen(tmp);
		  *tp++ = $2;
		  *tp++ = 0;
		  delete[]$1;
		  $$ = tmp;
		}
	;

udp_input_sym
	: '0' { $$ = '0'; }
	| '1' { $$ = '1'; }
	| 'x' { $$ = 'x'; }
	| '?' { $$ = '?'; }
	| 'b' { $$ = 'b'; }
	| '*' { $$ = '*'; }
	| '%' { $$ = '%'; }
	| 'f' { $$ = 'f'; }
	| 'F' { $$ = 'F'; }
	| 'l' { $$ = 'l'; }
	| 'h' { $$ = 'H'; }
	| 'B' { $$ = 'B'; }
	| 'r' { $$ = 'r'; }
	| 'R' { $$ = 'R'; }
	| 'M' { $$ = 'M'; }
	| 'n' { $$ = 'n'; }
	| 'N' { $$ = 'N'; }
	| 'p' { $$ = 'p'; }
	| 'P' { $$ = 'P'; }
	| 'Q' { $$ = 'Q'; }
	| 'q' { $$ = 'q'; }
	| '_' { $$ = '_'; }
	| '+' { $$ = '+'; }
	;

udp_output_sym
	: '0' { $$ = '0'; }
	| '1' { $$ = '1'; }
	| 'x' { $$ = 'x'; }
	| '-' { $$ = '-'; }
	;

udp_port_decl
	: K_input list_of_identifiers ';'
		{ $$ = pform_make_udp_input_ports($2); }
	| K_output IDENTIFIER ';'
		{ PWire*pp = new PWire($2, NetNet::IMPLICIT, NetNet::POUTPUT);
		  svector<PWire*>*tmp = new svector<PWire*>(1);
		  (*tmp)[0] = pp;
		  $$ = tmp;
		}
	| K_reg IDENTIFIER ';'
		{ PWire*pp = new PWire($2, NetNet::REG, NetNet::PIMPLICIT);
		  svector<PWire*>*tmp = new svector<PWire*>(1);
		  (*tmp)[0] = pp;
		  $$ = tmp;
		}
	| K_reg K_output IDENTIFIER ';'
		{ PWire*pp = new PWire($3, NetNet::REG, NetNet::POUTPUT);
		  svector<PWire*>*tmp = new svector<PWire*>(1);
		  (*tmp)[0] = pp;
		  $$ = tmp;
		}
	;

udp_port_decls
	: udp_port_decl
		{ $$ = $1; }
	| udp_port_decls udp_port_decl
		{ svector<PWire*>*tmp = new svector<PWire*>(*$1, *$2);
		  delete $1;
		  delete $2;
		  $$ = tmp;
		}
	;

udp_port_list
	: IDENTIFIER
		{ list<string>*tmp = new list<string>;
		  tmp->push_back($1);
		  delete $1;
		  $$ = tmp;
		}
	| udp_port_list ',' IDENTIFIER
		{ list<string>*tmp = $1;
		  tmp->push_back($3);
		  delete $3;
		  $$ = tmp;
		}
	;

udp_reg_opt: K_reg  { $$ = true; } | { $$ = false; };

udp_initial_expr_opt
	: '=' expression { $$ = $2; }
	|                { $$ = 0; }
	;

udp_input_declaration_list
        : K_input IDENTIFIER
		{ list<perm_string>*tmp = new list<perm_string>;
		  tmp->push_back(lex_strings.make($2));
		  $$ = tmp;
		  delete[]$2;
		}
	| udp_input_declaration_list ',' K_input IDENTIFIER
		{ list<perm_string>*tmp = $1;
		  tmp->push_back(lex_strings.make($4));
		  $$ = tmp;
		  delete[]$4;
		}
	;

udp_primitive
        /* This is the syntax for primitives that uses the IEEE1364-1995
	   format. The ports are simply names in the port list, and the
	   declarations are in the body. */

	: K_primitive IDENTIFIER '(' udp_port_list ')' ';'
	    udp_port_decls
	    udp_init_opt
	    udp_body
	  K_endprimitive

		{ perm_string tmp2 = lex_strings.make($2);
		  pform_make_udp(tmp2, $4, $7, $9, $8,
				 @2.text, @2.first_line);
		  delete[]$2;
		}

        /* This is the syntax for IEEE1364-2001 format definitions. The port
	   names and declarations are all in the parameter list. */

	| K_primitive IDENTIFIER
	    '(' K_output udp_reg_opt IDENTIFIER udp_initial_expr_opt ','
	    udp_input_declaration_list ')' ';'
	    udp_body
	  K_endprimitive

		{ perm_string tmp2 = lex_strings.make($2);
		  perm_string tmp6 = lex_strings.make($6);
		  pform_make_udp(tmp2, $5, tmp6, $7, $9, $12,
				 @2.text, @2.first_line);
		  delete[]$2;
		  delete[]$6;
		}
	;
