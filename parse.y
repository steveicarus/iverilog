
%{
/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: parse.y,v 1.110 2000/12/05 22:32:05 steve Exp $"
#endif

# include  "parse_misc.h"
# include  "pform.h"

extern void lex_start_table();
extern void lex_end_table();

/*
 * These are the strengths to use for net declaration
 * assignments. They are stored here so that scope that contains the
 * net_decl_assigns can change them during specific statements.
 */
static struct str_pair_t decl_strength = { PGate::STRONG, PGate::STRONG };
%}

%union {
      char letter;
	/* text items are C strings allocated by the lexor using
	   strdup. They can be put into lists with the texts type. */
      char*text;
      list<char*>*texts;

      list<string>*strings;

      struct str_pair_t drive;

      PCase::Item*citem;
      svector<PCase::Item*>*citems;

      lgate*gate;
      svector<lgate>*gates;

      Module::port_t *mport;
      svector<Module::port_t*>*mports;

      portname_t*portname;
      svector<portname_t*>*portnames;
      struct parmvalue_t*parmvalue;

      PExpr*expr;
      svector<PExpr*>*exprs;

      svector<PEEvent*>*event_expr;

      NetNet::Type nettype;
      PGBuiltin::Type gatetype;
      NetNet::PortType porttype;

      PTask*task;
      PFunction*function;

      PWire*wire;
      svector<PWire*>*wires;

      PEventStatement*event_statement;
      Statement*statement;
      svector<Statement*>*statement_list;

      verinum* number;

      verireal* realtime;
};

%token <text>   HIDENTIFIER IDENTIFIER PORTNAME SYSTEM_IDENTIFIER STRING
%token <number> NUMBER
%token <realtime> REALTIME
%token K_LE K_GE K_EG K_EQ K_NE K_CEQ K_CNE K_LS K_RS K_SG
%token K_LOR K_LAND K_NAND K_NOR K_NXOR K_TRIGGER
%token K_always K_and K_assign K_begin K_buf K_bufif0 K_bufif1 K_case
%token K_casex K_casez K_cmos K_deassign K_default K_defparam K_disable
%token K_edge K_else K_end K_endcase K_endfunction K_endmodule
%token K_endprimitive K_endspecify K_endtable K_endtask K_event K_for
%token K_force K_forever K_fork K_function K_highz0 K_highz1 K_if
%token K_initial K_inout K_input K_integer K_join K_large K_localparam
%token K_macromodule
%token K_medium K_module K_nand K_negedge K_nmos K_nor K_not K_notif0
%token K_notif1 K_or K_output K_parameter K_pmos K_posedge K_primitive
%token K_pull0 K_pull1 K_pulldown K_pullup K_rcmos K_real K_realtime
%token K_reg K_release K_repeat
%token K_rnmos K_rpmos K_rtran K_rtranif0 K_rtranif1 K_scalered
%token K_signed K_small K_specify
%token K_specparam K_strong0 K_strong1 K_supply0 K_supply1 K_table K_task
%token K_time K_tran K_tranif0 K_tranif1 K_tri K_tri0 K_tri1 K_triand
%token K_trior K_trireg K_vectored K_wait K_wand K_weak0 K_weak1
%token K_while K_wire
%token K_wor K_xnor K_xor

%token KK_attribute

%type <drive>   drive_strength drive_strength_opt dr_strength0 dr_strength1
%type <letter>  udp_input_sym udp_output_sym
%type <text>    udp_input_list udp_sequ_entry udp_comb_entry
%type <strings> udp_entry_list udp_comb_entry_list udp_sequ_entry_list
%type <strings> udp_body udp_port_list
%type <wires>   udp_port_decl udp_port_decls
%type <statement> udp_initial udp_init_opt

%type <text> identifier register_variable
%type <texts> register_variable_list list_of_variables

%type <text> net_decl_assign
%type <texts> net_decl_assigns

%type <mport> port port_opt port_reference port_reference_list
%type <mports> list_of_ports list_of_ports_opt

%type <wires> task_item task_item_list task_item_list_opt
%type <wires> function_item function_item_list

%type <portname> port_name parameter_value_byname
%type <portnames> port_name_list parameter_value_byname_list

%type <citem>  case_item
%type <citems> case_items

%type <gate>  gate_instance
%type <gates> gate_instance_list

%type <expr>  expression expr_primary
%type <expr>  lavalue lpvalue
%type <expr>  delay_value delay_value_simple
%type <exprs> delay1 delay3 delay3_opt
%type <exprs> expression_list
%type <exprs> assign assign_list

%type <exprs> range range_opt
%type <nettype>  net_type
%type <gatetype> gatetype
%type <porttype> port_type
%type <parmvalue> parameter_value_opt

%type <task> task_body
%type <function> func_body
%type <exprs> range_or_type_opt
%type <event_expr> event_expression_list
%type <event_expr> event_expression
%type <event_statement> event_control
%type <statement> statement statement_opt
%type <statement_list> statement_list

%right '?' ':'
%left K_LOR
%left K_LAND
%left '|'
%left '^' K_NXOR K_NOR
%left '&' K_NAND
%left K_EQ K_NE K_CEQ K_CNE
%left K_GE K_LE '<' '>'
%left K_LS K_RS
%left '+' '-'
%left '*' '/' '%'
%left UNARY_PREC

/* to resolve dangling else ambiguity: */
%nonassoc less_than_K_else
%nonassoc K_else

%%

  /* A degenerate source file can be completely empty. */
main : source_file | ;

source_file
	: description
	| source_file description
	;

  /* The block_item_decl is used in function definitions, task
     definitions, module definitions and named blocks. Wherever a new
     scope is entered, the source may declare new registers and
     integers. This rule matches those declarations. The containing
     rule has presumably set up the scope. */
block_item_decl
	: K_reg range register_variable_list ';'
		{ pform_set_net_range($3, $2);
		}
	| K_reg register_variable_list ';'
		{ delete $2; }
	| K_reg K_signed range register_variable_list ';'
		{ pform_set_net_range($4, $3);
		  yyerror(@2, "sorry: signed reg not supported.");
		}
	| K_reg K_signed register_variable_list ';'
		{ delete $3;
		  yyerror(@2, "sorry: signed reg not supported.");
		}
	| K_integer list_of_variables ';'
		{ pform_set_reg_integer($2);
		}
	| K_time list_of_variables ';'
		{ pform_set_reg_time($2);
		}
	| K_real list_of_variables ';'
		{ delete $2;
		  yyerror(@1, "sorry: real variables not supported.");
		}
	| K_realtime list_of_variables ';'
		{ delete $2;
		  yyerror(@1, "sorry: reatime variables not supported.");
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
		  pform_set_defparam($1, $3);
		  delete $1;
		}
	;

defparam_assign_list
	: defparam_assign
	| range defparam_assign
		{ yywarn(@1, "Ranges in parameter definition "
		             "are not supported.");
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
	: NUMBER
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
	| IDENTIFIER
		{ PEIdent*tmp = new PEIdent($1);
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
		{ pform_set_type_attrib($3, $5, $7);
		  delete $3;
		  delete $5;
		  delete $7;
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
	: '@' IDENTIFIER
		{ PEIdent*tmpi = new PEIdent($2);
		  tmpi->set_file(@2.text);
		  tmpi->set_lineno(@2.first_line);
		  delete[]$2;
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


expr_primary
	: NUMBER
		{ assert($1);
		  PENumber*tmp = new PENumber($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| REALTIME
		{ yyerror(@1, "sorry: real constants not supported.");
		  delete $1;
		  $$ = 0;
		}
	| STRING
		{ PEString*tmp = new PEString($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $1;
		}
	| identifier
		{ PEIdent*tmp = new PEIdent($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $1;
		}
	| SYSTEM_IDENTIFIER
                { PECallFunction*tmp = new PECallFunction($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $1
		}
	| identifier '[' expression ']'
		{ PEIdent*tmp = new PEIdent($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  tmp->msb_ = $3;
		  delete $1;
		  $$ = tmp;
		}
	| identifier '[' expression ':' expression ']'
		{ PEIdent*tmp = new PEIdent($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  tmp->msb_ = $3;
		  tmp->lsb_ = $5;
		  delete $1;
		  $$ = tmp;
		}
	| identifier '(' expression_list ')'
                { PECallFunction*tmp = new PECallFunction($1, *$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER '(' expression_list ')'
                { PECallFunction*tmp = new PECallFunction($1, *$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| '(' expression ')'
		{ $$ = $2; }
	| '{' expression_list '}'
		{ PEConcat*tmp = new PEConcat(*$2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $2;
		  $$ = tmp;
		}
	| '{' expression '{' expression_list '}' '}'
		{ PExpr*rep = $2;
		  if (!pform_expression_is_constant($2)) {
			yyerror(@2, "error: Repeat expression "
			            "must be constant.");
			delete rep;
			delete $2;
			rep = 0;
		  }
		  PEConcat*tmp = new PEConcat(*$4, rep);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $4;
		  $$ = tmp;
		}
	;


func_body
	: function_item_list statement
                { $$ = new PFunction($1, $2); }
	| function_item_list
		{ yyerror(@1, "error: function body has no statement.");
		  $$ = new PFunction($1, 0);
		}
	;

  /* A function_item is either a block item (i.e. a reg or integer
     declaration) or an input declaration. There are no output or
     inout ports. */
function_item
	: K_input range_opt list_of_variables ';'
                { svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, $2, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| block_item_decl
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

identifier
	: IDENTIFIER
		{ $$ = $1; }
	| HIDENTIFIER
		{ $$ = $1;
		}
	;

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

list_of_ports_opt
	: '(' list_of_ports ')' { $$ = $2; }
	|                       { $$ = 0; }
	;

list_of_variables
	: IDENTIFIER
		{ list<char*>*tmp = new list<char*>;
		  tmp->push_back($1);
		  $$ = tmp;
		}
	| list_of_variables ',' IDENTIFIER
		{ list<char*>*tmp = $1;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	;

  /* An lavalue is the expression that can go on the left side of a
     continuous assign statement. This checks (where it can) that the
     expression meets the constraints of continuous assignments. */
lavalue
	: identifier
		{ PEIdent*tmp = new PEIdent($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| identifier '[' expression ']'
		{ PEIdent*tmp = new PEIdent($1);
		  PExpr*sel = $3;
		  if (! pform_expression_is_constant(sel)) {
			yyerror(@2, "eror: Bit select in lvalue must "
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
		{ PEIdent*tmp = new PEIdent($1);
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
		{ PEIdent*tmp = new PEIdent($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| identifier '[' expression ']'
		{ PEIdent*tmp = new PEIdent($1);
		  tmp->msb_ = $3;
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;

		  $$ = tmp;
		}
	| identifier '[' expression ':' expression ']'
		{ PEIdent*tmp = new PEIdent($1);
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

module
	: K_module IDENTIFIER list_of_ports_opt ';'
		{ pform_startmodule($2, $3);
		}
	  module_item_list
	  K_endmodule
		{ pform_endmodule($2);
		  delete $2;
		}
	| K_module IDENTIFIER list_of_ports_opt ';'
		{ pform_startmodule($2, $3);
		}
	  K_endmodule
		{ pform_endmodule($2);
		  delete $2;
		}
	;

module_item
	: net_type range_opt list_of_variables ';'
		{ pform_makewire(@1, $2, $3, $1);
		}
	| net_type range_opt net_decl_assigns ';'
		{ pform_makewire(@1, $2, $3, $1);
		}
	| net_type drive_strength { decl_strength = $2;} net_decl_assigns ';'
		{ pform_makewire(@1, 0, $4, $1);
		    /* The strengths are handled in the
		       net_decl_assigns using the decl_strength that I
		       set in the rule. Right here, just restore the
		       defaults for other rules. */
		  decl_strength.str0 = PGate::STRONG;
		  decl_strength.str1 = PGate::STRONG;
		}
	| K_trireg charge_strength_opt range_opt delay3_opt list_of_variables ';'
		{ yyerror(@1, "sorry: trireg nets not supported.");
		  delete $3;
		}

	| port_type range_opt list_of_variables ';'
		{ pform_set_port_type($3, $2, $1);
		}
	| port_type range_opt error ';'
		{ yyerror(@3, "error: Invalid variable list"
			  " in port declaration.");
		  if ($2) delete $2;
		  yyerrok;
		}
	| block_item_decl
	| K_defparam defparam_assign_list ';'
	| K_event list_of_variables ';'
		{ pform_make_events($2, @1.text, @1.first_line);
		}
	| K_parameter parameter_assign_list ';'
	| K_localparam localparam_assign_list ';' { }

  /* Most gate types have an optional drive strength and optional
     three-value delay. These rules handle the different cases. */

	| gatetype gate_instance_list ';'
		{ pform_makegates($1, decl_strength, 0, $2);
		}

	| gatetype delay3 gate_instance_list ';'
		{ pform_makegates($1, decl_strength, $2, $3);
		}

	| gatetype drive_strength gate_instance_list ';'
		{ pform_makegates($1, $2, 0, $3);
		}

	| gatetype drive_strength delay3 gate_instance_list ';'
		{ pform_makegates($1, $2, $3, $4);
		}

  /* Pullup and pulldown devices cannot have delays, and their
     strengths are limited. */

	| K_pullup gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLUP, decl_strength, 0, $2);
		}
	| K_pulldown gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLDOWN, decl_strength, 0, $2);
		}

  /* This rule handles instantiations of modules and user defined
     primitives. These devices to not have delay lists or strengths,
     but then can have parameter lists. */

	| IDENTIFIER parameter_value_opt gate_instance_list ';'
		{ pform_make_modgates($1, $2, $3);
		  delete $1;
		}

  /* Continuous assignment can have an optional drive strength, then
     an optional delay3 that applies to all the assignments in the
     assign_list. */

	| K_assign drive_strength_opt delay3_opt assign_list ';'
		{ pform_make_pgassign_list($4, $3, $2, @1.text, @1.first_line); }
	| K_assign error '=' expression ';'

	| K_always statement
		{ PProcess*tmp = pform_make_behavior(PProcess::PR_ALWAYS, $2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		}
	| K_initial statement
		{ PProcess*tmp = pform_make_behavior(PProcess::PR_INITIAL, $2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		}

  /* The task declaration rule matches the task declaration
     header, then pushes the function scope. This causes the
     definitions in the task_body to take on the scope of the task
     instead of the module. */

	| K_task IDENTIFIER ';'
		{ pform_push_scope($2); }
	  task_body
		{ pform_pop_scope(); }
	  K_endtask
		{ PTask*tmp = $5;
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  pform_set_task($2, $5);
		  delete $2;
		}

  /* The function declaration rule matches the function declaration
     header, then pushes the function scope. This causes the
     definitions in the func_body to take on the scope of the function
     instead of the module. */

        | K_function range_or_type_opt IDENTIFIER ';'
                { pform_push_scope($3); }
          func_body
                { pform_pop_scope(); }
          K_endfunction
                { PFunction *tmp = $6;
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  pform_set_function($3, $2, $6);
		  delete $3;
		}

  /* specify blocks are parsed but ignored. */

	| K_specify specify_item_list K_endspecify
		{
		}

  /* These rules are for the Icarus VErilog specific $attribute
     extensions. Then catch the parameters of the $attribute keyword. */

	| KK_attribute '(' IDENTIFIER ',' STRING ',' STRING ')' ';'
		{ pform_set_attrib($3, $5, $7);
		  delete $3;
		  delete $5;
		  delete $7;
		}
	| KK_attribute '(' error ')' ';'
		{ yyerror(@1, "error: Misformed $attribute parameter list."); }
	;


module_item_list
	: module_item_list module_item
	| module_item
	;


  /* A net declaration assignment allows the programmer to combine the
     net declaration and the continuous assignment into a single
     statement.

     Note that the continuous assignment statement is generated as a
     side effect, and all I pass up is the name of the l-value. */

net_decl_assign
	: IDENTIFIER '=' expression
		{ PEIdent*id = new PEIdent($1);
		  PGAssign*tmp = pform_make_pgassign(id, $3, 0, decl_strength);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = $1;
		}
	| delay1 IDENTIFIER '=' expression
		{ PEIdent*id = new PEIdent($2);
		  PGAssign*tmp = pform_make_pgassign(id, $4, $1,
						     decl_strength);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = $2;
		}
	;

net_decl_assigns
	: net_decl_assigns ',' net_decl_assign
		{ list<char*>*tmp = $1;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	| net_decl_assign
		{ list<char*>*tmp = new list<char*>;
		  tmp->push_back($1);
		  $$ = tmp;
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

parameter_assign
	: IDENTIFIER '=' expression
		{ PExpr*tmp = $3;
		  if (!pform_expression_is_constant(tmp)) {
			yyerror(@3, "error: parameter value "
			            "must be a constant expression.");
			delete tmp;
			tmp = 0;
		  } else {
			pform_set_parameter($1, tmp);
		  }
		  delete $1;
		}
	;

parameter_assign_list
	: parameter_assign
	| range parameter_assign
		{ yywarn(@1, "Ranges in parameter definition "
		          "are not supported.");
		  delete $1;
		}
	| parameter_assign_list ',' parameter_assign
	;


  /* Localparam assignments and asignment lists are broken into
     separate BNF so that I can call slightly different paramter
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
		  }
		  pform_set_localparam($1, tmp);
		  delete $1;
		}
	;

localparam_assign_list
	: localparam_assign
	| range localparam_assign
		{ yywarn(@1, "Ranges in localparam definition "
		          "are not supported.");
		  delete $1;
		}
	| localparam_assign_list ',' localparam_assign
	;


  /* The parameters of a module instance can be overridden by writing
     a list of expressions in a syntax much line a delay list. (The
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
	| '#' NUMBER
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
	: PORTNAME '(' expression ')'
		{ portname_t*tmp = new portname_t;
		  tmp->name = $1;
		  tmp->parm = $3;
		  free($1);
		  $$ = tmp;
		}
	| PORTNAME '(' ')'
		{ portname_t*tmp = new portname_t;
		  tmp->name = $1;
		  tmp->parm = 0;
		  free($1);
		  $$ = tmp;
		}
	;

parameter_value_byname_list
	: parameter_value_byname
		{ svector<portname_t*>*tmp = new svector<portname_t*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	| parameter_value_byname_list ',' parameter_value_byname
		{ svector<portname_t*>*tmp = new svector<portname_t*>(*$1,$3);
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
     port_reference, but its name from the PORTNAME. */

	| PORTNAME '(' port_reference ')'
		{ Module::port_t*tmp = $3;
		  tmp->name = $1;
		  delete $1;
		  $$ = tmp;
		}

  /* A port can also be a concatenation of port references. In this
     case the port does not have a name available to the outside, only
     positional parameter passing is possible here. */

	| '{' port_reference_list '}'
		{ Module::port_t*tmp = $2;
		  tmp->name = "";
		  $$ = tmp;
		}

  /* This attaches a name to a port reference contatenation list so
     that parameter passing be name is possible. */

	| PORTNAME '(' '{' port_reference_list '}' ')'
		{ Module::port_t*tmp = $4;
		  tmp->name = $1;
		  delete $1;
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
		{ Module::port_t*ptmp = new Module::port_t;
		  PEIdent*tmp = new PEIdent($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  ptmp->name = $1;
		  ptmp->expr = svector<PEIdent*>(1);
		  ptmp->expr[0] = tmp;
		  delete $1;
		  $$ = ptmp;
		}

	| IDENTIFIER '[' expression ':' expression ']'
		{ PEIdent*wtmp = new PEIdent($1);
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
		  ptmp->name = "";
		  ptmp->expr = svector<PEIdent*>(1);
		  ptmp->expr[0] = wtmp;
		  delete $1;
		  $$ = ptmp;
		}

	| IDENTIFIER '[' expression ']'
		{ PEIdent*tmp = new PEIdent($1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  if (!pform_expression_is_constant($3)) {
			yyerror(@3, "error: port bit select "
				"must be constant.");
		  }
		  tmp->msb_ = $3;
		  Module::port_t*ptmp = new Module::port_t;
		  ptmp->name = "";
		  ptmp->expr = svector<PEIdent*>(1);
		  ptmp->expr[0] = tmp;
		  delete $1;
		  $$ = ptmp;
		}

	| IDENTIFIER '[' error ']'
		{ yyerror(@1, "error: invalid port bit select");
		  Module::port_t*ptmp = new Module::port_t;
		  PEIdent*wtmp = new PEIdent($1);
		  wtmp->set_file(@1.text);
		  wtmp->set_lineno(@1.first_line);
		  ptmp->name = $1;
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
	: PORTNAME '(' expression ')'
		{ portname_t*tmp = new portname_t;
		  tmp->name = $1;
		  tmp->parm = $3;
		  delete $1;
		  $$ = tmp;
		}
	| PORTNAME '(' error ')'
		{ yyerror(@3, "error: invalid port connection expression.");
		  portname_t*tmp = new portname_t;
		  tmp->name = $1;
		  tmp->parm = 0;
		  delete $1;
		  $$ = tmp;
		}
	| PORTNAME '(' ')'
		{ portname_t*tmp = new portname_t;
		  tmp->name = $1;
		  tmp->parm = 0;
		  delete $1;
		  $$ = tmp;
		}
	;

port_name_list
	: port_name_list ',' port_name
		{ svector<portname_t*>*tmp;
		  tmp = new svector<portname_t*>(*$1, $3);
		  delete $1;
		  $$ = tmp;
		}
	| port_name
		{ svector<portname_t*>*tmp = new svector<portname_t*>(1);
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
			yyerror(@4, "error: msb of range must be constant.");

		  (*tmp)[1] = $4;

		  $$ = tmp;
		}
	;

range_opt
	: range
	| { $$ = 0; }
	;

  /* This is used to express the retur type of a function. */
range_or_type_opt
	: range { $$ = $1; }
	| K_integer { $$ = 0; }
	| K_real { $$ = 0; }
	| K_realtime { $$ = 0; }
	| K_time { $$ = 0; }
	| { $$ = 0; }
	;

  /* The register_variable rule is matched only when I am parsing
     variables in a "reg" definition. I therefore know that I am
     creating registers and I do not need to let the containing rule
     handle it. The register variable list simply packs them together
     so that bit ranges can be assigned. */
register_variable
	: IDENTIFIER
		{ pform_makewire(@1, $1, NetNet::REG);
		  $$ = $1;
		}
	| IDENTIFIER '=' expression
		{ pform_makewire(@1, $1, NetNet::REG);
		  if (! pform_expression_is_constant($3))
			yyerror(@3, "error: register declaration assignment"
				" value must be a constant expression.");
		  pform_make_reginit(@1, $1, $3);
		  $$ = $1;
		}
	| IDENTIFIER '[' expression ':' expression ']'
		{ pform_makewire(@1, $1, NetNet::REG);
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
		{ list<char*>*tmp = new list<char*>;
		  tmp->push_back($1);
		  $$ = tmp;
		}
	| register_variable_list ',' register_variable
		{ list<char*>*tmp = $1;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	;

specify_item
	: K_specparam specparam_list ';'
	| specify_simple_path '=' '(' delay_value ')' ';'
		{
		}
	| specify_simple_path '=' '(' delay_value ',' delay_value ')' ';'
		{
		}
	| specify_simple_path '=' '(' delay_value ',' delay_value ',' delay_value ')' ';'
		{
		}
	| specify_simple_path '=' delay_value_simple ';'
		{
		}
	;

specify_item_list
	: specify_item
	| specify_item_list specify_item
	;

specify_simple_path
	: '(' IDENTIFIER spec_polarity K_EG IDENTIFIER ')'
	| '(' IDENTIFIER spec_polarity K_SG IDENTIFIER ')'
	;

specparam
	: IDENTIFIER '=' expression
		{ delete $1;
		  delete $3;
		}
	;

specparam_list
	: specparam
	| specparam_list ',' specparam
	;

spec_polarity: '+' | '-' | ;


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

	| K_deassign lavalue';'
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
		  PBlock*tmp = new PBlock($3, PBlock::BL_SEQ, *$6);
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
	| K_begin error K_end
		{ yyerrok; }

  /* fork-join blocks are very similar to begin-end blocks. In fact,
     from the parser's perspective there is no real difference. All we
     need to do is remember that this is a parallel block so that the
     code generator can do the right thing. */

	| K_fork ':' IDENTIFIER
		{ pform_push_scope($3); }
	  block_item_decls_opt
	  statement_list K_join
		{ pform_pop_scope();
		  PBlock*tmp = new PBlock($3, PBlock::BL_PAR, *$6);
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
		{ PDisable*tmp = new PDisable($2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $2;
		  $$ = tmp;
		}
	| K_TRIGGER IDENTIFIER ';'
		{ PTrigger*tmp = new PTrigger($2);
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
		  yyerror(@9, "error: Error in for loop step assigment.");
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
	| event_control statement_opt
		{ PEventStatement*tmp = $1;
		  if (tmp == 0) {
			yyerror(@1, "error: Invalid event control.");
			$$ = 0;
		  } else {
			tmp->set_statement($2);
			$$ = tmp;
		  }
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
		  tmp->set_statement($5);
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER '(' expression_list ')' ';'
		{ PCallTask*tmp = new PCallTask($1, *$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER ';'
		{ svector<PExpr*>pt (0);
		  PCallTask*tmp = new PCallTask($1, pt);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| identifier '(' expression_list ')' ';'
		{ PCallTask*tmp = new PCallTask($1, *$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	| identifier ';'
		{ svector<PExpr*>pt (0);
		  PCallTask*tmp = new PCallTask($1, pt);
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
	: statement_list statement
		{ svector<Statement*>*tmp = new svector<Statement*>(*$1, $2);
		  delete $1;
		  $$ = tmp;
		}
	| statement
		{ svector<Statement*>*tmp = new svector<Statement*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	;

statement_opt
	: statement
	| ';' { $$ = 0; }
	;

task_body
	: task_item_list_opt statement_opt
		{ PTask*tmp = new PTask($1, $2);
		  $$ = tmp;
		}
	;

task_item
	: block_item_decl
	    { $$ = new svector<PWire*>(0); }
	| K_input range_opt list_of_variables ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT, $2,
						$3, @1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output range_opt list_of_variables ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT, $2, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout range_opt list_of_variables ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT, $2, $3,
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
	: K_initial IDENTIFIER '=' NUMBER ';'
		{ PExpr*etmp = new PENumber($4);
		  PEIdent*itmp = new PEIdent($2);
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
	| 'r' { $$ = 'r'; }
	| 'R' { $$ = 'R'; }
	| 'n' { $$ = 'n'; }
	| 'N' { $$ = 'N'; }
	| 'p' { $$ = 'p'; }
	| 'P' { $$ = 'P'; }
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
	: K_input list_of_variables ';'
		{ $$ = pform_make_udp_input_ports($2); }
	| K_output IDENTIFIER ';'
		{ PWire*pp = new PWire($2, NetNet::IMPLICIT, NetNet::POUTPUT);
		  svector<PWire*>*tmp = new svector<PWire*>(1);
		  (*tmp)[0] = pp;
		  delete $2;
		  $$ = tmp;
		}
	| K_reg IDENTIFIER ';'
		{ PWire*pp = new PWire($2, NetNet::REG, NetNet::PIMPLICIT);
		  svector<PWire*>*tmp = new svector<PWire*>(1);
		  (*tmp)[0] = pp;
		  delete $2;
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

udp_primitive
	: K_primitive IDENTIFIER '(' udp_port_list ')' ';'
	    udp_port_decls
	    udp_init_opt
	    udp_body
	  K_endprimitive
		{ pform_make_udp($2, $4, $7, $9, $8);
		  delete[]$2;
		}
	;
