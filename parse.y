
%{
/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: parse.y,v 1.28 1999/05/27 03:31:29 steve Exp $"
#endif

# include  "parse_misc.h"
# include  "pform.h"

extern void lex_start_table();
extern void lex_end_table();
%}

%union {
      char letter;
      string*text;
      list<string>*strings;

      PCase::Item*citem;
      list<PCase::Item*>*citems;

      lgate*gate;
      svector<lgate>*gates;

      PExpr*expr;
      svector<PExpr*>*exprs;

      svector<PEEvent*>*event_expr;

      NetNet::Type nettype;
      PGBuiltin::Type gatetype;
      NetNet::PortType porttype;

      PWire*wire;
      list<PWire*>*wires;

      PEventStatement*event_statement;
      Statement*statement;
      list<Statement*>*statement_list;

      verinum* number;
};

%token <text>   IDENTIFIER PORTNAME SYSTEM_IDENTIFIER STRING
%token <number> NUMBER
%token K_LE K_GE K_EQ K_NE K_CEQ K_CNE K_LS K_RS
%token K_LOR K_LAND
%token K_always K_and K_assign K_begin K_buf K_bufif0 K_bufif1 K_case
%token K_casex K_casez K_cmos K_deassign K_default K_defparam K_disable
%token K_edge K_else K_end K_endcase K_endfunction K_endmodule
%token K_endprimitive K_endspecify K_endtable K_endtask K_event K_for
%token K_force K_forever K_fork K_function K_highz0 K_highz1 K_if
%token K_initial K_inout K_input K_integer K_join K_large K_macromodule
%token K_medium K_module K_nand K_negedge K_nmos K_nor K_not K_notif0
%token K_notif1 K_or K_output K_parameter K_pmos K_posedge K_primitive
%token K_pull0 K_pull1 K_pulldown K_pullup K_rcmos K_real K_realtime
%token K_reg K_release K_repeat
%token K_rnmos K_rpmos K_rtran K_rtranif0 K_rtranif1 K_scalered
%token K_small K_specify
%token K_specparam K_strong0 K_strong1 K_supply0 K_supply1 K_table K_task
%token K_time K_tran K_tranif0 K_tranif1 K_tri K_tri0 K_tri1 K_triand
%token K_trior K_vectored K_wait K_wand K_weak0 K_weak1 K_while K_wire
%token K_wor K_xnor K_xor

%token KK_attribute

%type <letter>  udp_input_sym udp_output_sym
%type <text>    udp_input_list udp_sequ_entry udp_comb_entry
%type <strings> udp_entry_list udp_comb_entry_list udp_sequ_entry_list
%type <strings> udp_body udp_port_list
%type <wires>   udp_port_decl udp_port_decls
%type <statement> udp_initial udp_init_opt

%type <text> identifier register_variable
%type <strings> register_variable_list
%type <strings> list_of_variables

%type <wire> port
%type <wires> list_of_ports list_of_ports_opt

%type <citem>  case_item
%type <citems> case_items

%type <gate>  gate_instance
%type <gates> gate_instance_list

%type <expr>  delay delay_opt expression expr_primary
%type <expr>  lavalue lpvalue
%type <exprs> expression_list

%type <exprs> range range_opt
%type <nettype>  net_type
%type <gatetype> gatetype
%type <porttype> port_type

%type <event_expr> event_expression
%type <event_statement> event_control
%type <statement> statement statement_opt
%type <statement_list> statement_list

%left '?' ':'
%left K_LOR
%left K_LAND
%left '|'
%left '^'
%left '&'
%left K_EQ K_NE K_CEQ K_CNE
%left K_GE K_LE '<' '>'
%left K_LS K_RS
%left '+' '-'
%left '*' '/' '%'
%left UNARY_PREC

%%

source_file
	: description
	| source_file description
	;

case_item
	: expression ':' statement_opt
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->expr = $1;
		  tmp->stat = $3;
		  $$ = tmp;
		}
	| K_default ':' statement_opt
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->expr = 0;
		  tmp->stat = $3;
		  $$ = tmp;
		}
	| K_default  statement_opt
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->expr = 0;
		  tmp->stat = $2;
		  $$ = tmp;
		}
	;

case_items
	: case_items case_item
		{ list<PCase::Item*>*tmp = $1;
		  tmp->push_back($2);
		  $$ = tmp;
		}
	| case_item
		{ list<PCase::Item*>*tmp = new list<PCase::Item*>;
		  tmp->push_back($1);
		  $$ = tmp;
		}
	;

delay
	: '#' NUMBER
		{ verinum*tmp = $2;
		  if (tmp == 0) {
			yyerror(@2, "XXXX internal error: delay.");
			$$ = 0;
		  } else {
			$$ = new PENumber(tmp);
		  }
		}
	| '#' IDENTIFIER
		{ PEIdent*tmp = new PEIdent(*$2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		  delete $2;
		}
	| '#' '(' IDENTIFIER ')'
		{ PEIdent*tmp = new PEIdent(*$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $3;
		}
	| '#' '(' expression ')'
		{ $$ = $3;
		}
	;

delay_opt
	: delay { $$ = $1; }
	|       { $$ = 0; }
	;

description
	: module
	| udp_primitive
	| KK_attribute '(' IDENTIFIER ',' STRING ',' STRING ')'
		{ pform_set_type_attrib(*$3, *$5, *$7);
		  delete $3;
		  delete $5;
		  delete $7;
		}
	;

event_control
	: '@' IDENTIFIER
		{ yyerror(@1, "Sorry, event control not supported.");
		  $$ = 0;
		}
	| '@' '(' event_expression ')'
		{ PEventStatement*tmp = new PEventStatement(*$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $3;
		  $$ = tmp;
		}
	| '@' '(' error ')'
		{ yyerror(@1, "Malformed event control expression.");
		  $$ = 0;
		}
	;

event_expression
	: K_posedge expression
		{ PEEvent*tmp = new PEEvent(NetNEvent::POSEDGE, $2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	| K_negedge expression
		{ PEEvent*tmp = new PEEvent(NetNEvent::NEGEDGE, $2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	| expression
		{ PEEvent*tmp = new PEEvent(NetNEvent::ANYEDGE, $1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	| event_expression K_or event_expression
		{ svector<PEEvent*>*tmp = new svector<PEEvent*>(*$1, *$3);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	;

expression
	: expr_primary
		{ $$ = $1; }
	| '(' expression ')'
		{ $$ = $2; }
	| '{' expression_list '}'
		{ PEConcat*tmp = new PEConcat(*$2);
		  delete $2;
		  $$ = tmp;
		}
	| '+' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('+', $2);
		  tmp->set_file(@2.text);
		  tmp->set_lineno(@2.first_line);
		  $$ = tmp;
		}
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
		{ yyerror(@2, "Sorry, ?: operator not supported.");
		  $$ = 0;
		}
	;


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
	| expression_list ','
		{ svector<PExpr*>*tmp = new svector<PExpr*>(*$1, 0);
		  delete $1;
		  $$ = tmp;
		}
	;


expr_primary
	: NUMBER
		{ if ($1 == 0) {
		        yyerror(@1, "XXXX No number value in primary?");
			$$ = 0;
		  } else {
			PENumber*tmp = new PENumber($1);
			tmp->set_file(@1.text);
			tmp->set_lineno(@1.first_line);
			$$ = tmp;
		  }
		}
	| STRING
		{ PEString*tmp = new PEString(*$1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $1;
		}
	| identifier
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		  delete $1;
		}
	| SYSTEM_IDENTIFIER
		{ PEIdent*tmp = new PEIdent(*$1);
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
	;

  /* A gate_instance is a module instantiation or a built in part
     type. In any case, the gate has a set of connections to ports. */
gate_instance
	: IDENTIFIER '(' expression_list ')'
		{ lgate*tmp = new lgate;
		  tmp->name = *$1;
		  tmp->parms = $3;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete $1;
		  $$ = tmp;
		}
	| IDENTIFIER range '(' expression_list ')'
		{ lgate*tmp = new lgate;
		  svector<PExpr*>*rng = $2;
		  tmp->name = *$1;
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
	| IDENTIFIER '(' port_name_list ')'
		{ lgate*tmp = new lgate;
		  tmp->name = *$1;
		  tmp->parms = 0;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete $1;
		  yyerror(@1, "Sorry, named port connections not supported.");
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
	| K_pulldown { $$ = PGBuiltin::PULLDOWN; }
	| K_pullup   { $$ = PGBuiltin::PULLUP; }
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
	: identifier '.' IDENTIFIER
		{ yyerror(@1, "Sorry, qualified identifiers not supported.");
		  $$ = $3;
		  delete $1;
		}
	| IDENTIFIER
		{ $$ = $1; }
	;

list_of_ports
	: port
		{ list<PWire*>*tmp = new list<PWire*>;
		  tmp->push_back($1);
		  $$ = tmp;
		}
	| list_of_ports ',' port
		{ list<PWire*>*tmp = $1;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	;

list_of_ports_opt
	: '(' list_of_ports ')' { $$ = $2; }
	| '(' ')'               { $$ = 0; }
	|                       { $$ = 0; }
	;

list_of_variables
	: IDENTIFIER
		{ list<string>*tmp = new list<string>;
		  tmp->push_back(*$1);
		  delete $1;
		  $$ = tmp;
		}
	| list_of_variables ',' IDENTIFIER
		{ list<string>*tmp = $1;
		  tmp->push_back(*$3);
		  delete $3;
		  $$ = tmp;
		}
	;

  /* An lavalue is the expression that can go on the left side of a
     continuous assign statement. This checks (where it can) that the
     expression meets the constraints of continuous assignments. */
lavalue
	: IDENTIFIER
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  $$ = tmp;
		}
	| IDENTIFIER '[' expression ']'
		{ PEIdent*tmp = new PEIdent(*$1);
		  PExpr*sel = $3;
		  if (! pform_expression_is_constant(sel)) {
			yyerror(@2, "Bit select in lvalue must "
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
	| IDENTIFIER range
		{ PEIdent*tmp = new PEIdent(*$1);
		  yyerror(@3, "Sorry, lvalue bit range not supported.");
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  delete $1;
		  delete $2;
		  $$ = tmp;
		}
	| '{' expression_list '}'
		{ yyerror(@1, "Sorry, concatenation expressions"
		          " not supported in lvalue.");
		  $$ = 0;
		  delete $2;
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
		{ yyerror(@1, "Sorry, part selects"
		          " not supported in lvalue.");
		  $$ = 0;
		  delete $1;
		  delete $3;
		  delete $5;
		}
	| '{' expression_list '}'
		{ yyerror(@1, "Sorry, concatenation expressions"
		          " not supported in lvalue.");
		  $$ = 0;
		  delete $2;
		}
	;

module
	: K_module IDENTIFIER list_of_ports_opt ';'
		{ pform_startmodule(*$2, $3);
		}
	  module_item_list
	  K_endmodule
		{ pform_endmodule(*$2);
		  delete $2;
		}
	;

module_item
	: net_type range_opt list_of_variables ';'
		{ pform_makewire($3, $1);
		  if ($2) {
			pform_set_net_range($3, $2);
			delete $2;
		  }
		  delete $3;
		}
	| port_type range_opt list_of_variables ';'
		{ pform_set_port_type($3, $1);
		  if ($2) {
			pform_set_net_range($3, $2);
			delete $2;
		  }
		  delete $3;
		}
	| K_reg range register_variable_list ';'
		{ pform_set_net_range($3, $2);
		  delete $2;
		  delete $3;
		}
	| K_reg register_variable_list ';'
		{ delete $2; }
	| K_integer list_of_variables ';'
		{ yyerror(@1, "Sorry, integer types not supported."); }
	| K_parameter parameter_assign_list ';'
	| gatetype delay_opt gate_instance_list ';'
		{ pform_makegates($1, $2, $3);
		}
	| IDENTIFIER gate_instance_list ';'
		{ pform_make_modgates(*$1, $2);
		  delete $1;
		}
	| K_assign lavalue '=' expression ';'
		{ PGAssign*tmp = pform_make_pgassign($2, $4);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		}
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
	| K_task IDENTIFIER ';' statement K_endtask
		{ yyerror(@1, "Sorry, task declarations not supported.");
		}
	| K_function range_or_type_opt  IDENTIFIER ';' statement K_endfunction
		{ yyerror(@1, "Sorry, function declarations not supported.");
		}
	| KK_attribute '(' IDENTIFIER ',' STRING ',' STRING ')' ';'
		{ pform_set_attrib(*$3, *$5, *$7);
		  delete $3;
		  delete $5;
		  delete $7;
		}
	| KK_attribute '(' error ')' ';'
		{ yyerror(@1, "Misformed $attribute parameter list."); }
	;

module_item_list
	: module_item_list module_item
	| module_item
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
			yyerror(@3, "parameter value must be constant.");
			delete tmp;
			tmp = 0;
		  }
		  pform_set_parameter(*$1, tmp);
		  delete $1;
		}
	;

parameter_assign_list
	: parameter_assign
	| range parameter_assign
		{ yyerror(@1, "Ranges in parameter definition "
		          "are not supported.");
		  delete $1;
		}
	| parameter_assign_list ',' parameter_assign
	;

port
	: IDENTIFIER
		{ $$ = new PWire(*$1, NetNet::IMPLICIT);
		  $$->port_type = NetNet::PIMPLICIT;
		  delete $1;
		}
	| IDENTIFIER '[' expression ':' expression ']'
		{ PWire*tmp = new PWire(*$1, NetNet::IMPLICIT);
		  tmp->port_type = NetNet::PIMPLICIT;
		  if (!pform_expression_is_constant($3)) {
			yyerror(@3, "msb expression of port bit select "
			        "must be constant.");
			delete $3;
		  } else {
			tmp->msb = $3;
		  }
		  if (!pform_expression_is_constant($5)) {
			yyerror(@3, "lsb expression of port bit select "
			        "must be constant.");
			delete $5;
		  } else {
			tmp->msb = $5;
		  }
		  delete $1;
		  $$ = tmp;
		}
	| IDENTIFIER '[' error ']'
		{ yyerror(@1, "invalid port bit select");
		  $$ = new PWire(*$1, NetNet::IMPLICIT);
		  $$->port_type = NetNet::PIMPLICIT;
		  delete $1;
		}
	;

port_name
	: PORTNAME '(' expression ')'
		{ delete $1;
		  delete $3;
		}
	| PORTNAME '(' error ')'
		{ yyerror(@3, "invalid port connection expression.");
		  delete $1;
		}
	| PORTNAME '(' ')'
		{ delete $1;
		}
	;

port_name_list
	: port_name_list ',' port_name
	| port_name
	;

port_type
	: K_input { $$ = NetNet::PINPUT; }
	| K_output { $$ = NetNet::POUTPUT; }
	| K_inout { $$ = NetNet::PINOUT; }
	;

range
	: '[' expression ':' expression ']'
		{ svector<PExpr*>*tmp = new svector<PExpr*> (2);
		  if (!pform_expression_is_constant($2)) {
			yyerror(@2, "msb of range must be constant.");
			delete $2;
		  } else {
			(*tmp)[0] = $2;
		  }
		  if (!pform_expression_is_constant($4)) {
			yyerror(@4, "msb of range must be constant.");
			delete $4;
		  } else {
			(*tmp)[1] = $4;
		  }
		  $$ = tmp;
		}
	;

range_opt
	: range
	| { $$ = 0; }
	;

range_or_type_opt
	: range { }
	| K_integer
	| K_real
	| K_realtime
	| K_time
	|
	;
  /* The register_variable rule is matched only when I am parsing
     variables in a "reg" definition. I therefore know that I am
     creating registers and I do not need to let the containing rule
     handle it. The register variable list simply packs them together
     so that bit ranges can be assigned. */
register_variable
	: IDENTIFIER
		{ pform_makewire(*$1, NetNet::REG);
		  $$ = $1;
		}
	| IDENTIFIER '[' expression ':' expression ']'
		{ pform_makewire(*$1, NetNet::REG);
		  if (! pform_expression_is_constant($3))
			yyerror(@3, "msb of register range must be constant.");
		  if (! pform_expression_is_constant($5))
			yyerror(@3, "lsb of register range must be constant.");
		  pform_set_reg_idx(*$1, $3, $5);
		  $$ = $1;
		}
	;

register_variable_list
	: register_variable
		{ list<string>*tmp = new list<string>;
		  tmp->push_back(*$1);
		  delete $1;
		  $$ = tmp;
		}
	| register_variable_list ',' register_variable
		{ list<string>*tmp = $1;
		  tmp->push_back(*$3);
		  delete $3;
		  $$ = tmp;
		}
	;

statement
	: K_begin statement_list K_end
		{ $$ = pform_make_block(PBlock::BL_SEQ, $2); }
	| K_fork statement_list K_join
		{ $$ = pform_make_block(PBlock::BL_PAR, $2); }
	| K_begin K_end
		{ $$ = pform_make_block(PBlock::BL_SEQ, 0); }
	| K_fork K_join
		{ $$ = pform_make_block(PBlock::BL_PAR, 0); }
	| K_case '(' expression ')' case_items K_endcase
		{ PCase*tmp = new PCase($3, $5);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_if '(' expression ')' statement_opt
		{ PCondit*tmp = new PCondit($3, $5, 0);
		  $$ = tmp;
		}
	| K_if '(' expression ')' statement_opt K_else statement_opt
		{ PCondit*tmp = new PCondit($3, $5, $7);
		  $$ = tmp;
		}
	| K_if '(' error ')' statement_opt
		{ yyerror(@1, "Malformed conditional expression.");
		  $$ = $5;
		}
	| K_if '(' error ')' statement_opt K_else statement_opt
		{ yyerror(@1, "Malformed conditional expression.");
		  $$ = $5;
		}
	| K_for '(' lpvalue '=' expression ';' expression ';'
	  lpvalue '=' expression ')' statement
		{ $$ = new PForStatement($3, $5, $7, $9, $11, $13);
		}
	| K_for '(' lpvalue '=' expression ';' expression ';'
	  error ')' statement
		{ $$ = 0;
		  yyerror(@9, "Error in for loop step assigment.");
		}
	| K_for '(' lpvalue '=' expression ';' error ';'
	  lpvalue '=' expression ')' statement
		{ $$ = 0;
		  yyerror(@7, "Error in for loop condition expression.");
		}
	| K_for '(' error ')' statement
		{ $$ = 0;
		  yyerror(@3, "Incomprehensible for loop.");
		}
	| K_while '(' expression ')' statement
		{ PWhile*tmp = new PWhile($3, $5);
		  $$ = tmp;
		}
	| K_while '(' error ')' statement
		{ $$ = 0;
		  yyerror(@3, "Error in while loop condition.");
		}
	| delay statement_opt
		{ PDelayStatement*tmp = new PDelayStatement($1, $2);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| event_control statement_opt
		{ PEventStatement*tmp = $1;
		  if (tmp == 0) {
			yyerror(@1, "Invalid event control.");
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
		{ yyerror(@1, "Sorry, non-blocking assignment not implemented.");
		  PAssign*tmp = new PAssign($1,$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| lpvalue '=' delay expression ';'
		{ yyerror(@1, "Sorry, assignment timing control not implemented.");
		  PAssign*tmp = new PAssign($1,$3);
		  tmp->set_file(@1.text);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| K_wait '(' expression ')' statement_opt
		{ PEventStatement*tmp;
		  PEEvent*etmp = new PEEvent(NetNEvent::POSITIVE, $3);
		  tmp = new PEventStatement(etmp);
		  tmp->set_statement($5);
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER '(' expression_list ')' ';'
		{ $$ = pform_make_calltask($1, $3);
		}
	| SYSTEM_IDENTIFIER ';'
		{ $$ = pform_make_calltask($1);
		}
	| IDENTIFIER '(' expression_list ')' ';'
		{ yyerror(@1, "Sorry, task enabling not implemented.");
		  $$ = new PNoop;
		}
	| IDENTIFIER ';'
		{ yyerror(@1, "Sorry, task enabling not implemented.");
		  $$ = new PNoop;
		}
	| error ';'
		{ yyerror(@1, "malformed statement");
		  $$ = new PNoop;
		}
	;

statement_list
	: statement_list statement
		{ list<Statement*>*tmp = $1;
		  tmp->push_back($2);
		  $$ = tmp;
		}
	| statement
		{ list<Statement*>*tmp = new list<Statement*>();
		  tmp->push_back($1);
		  $$ = tmp;
		}
	;

statement_opt
	: statement
	| ';' { $$ = 0; }
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
		{ string*tmp = $1;
		  *tmp += ':';
		  *tmp += $3;
		  $$ = tmp;
		}
	;

udp_comb_entry_list
	: udp_comb_entry
		{ list<string>*tmp = new list<string>;
		  tmp->push_back(*$1);
		  delete $1;
		  $$ = tmp;
		}
	| udp_comb_entry_list udp_comb_entry
		{ list<string>*tmp = $1;
		  tmp->push_back(*$2);
		  delete $2;
		  $$ = tmp;
		}
	;

udp_sequ_entry_list
	: udp_sequ_entry
		{ list<string>*tmp = new list<string>;
		  tmp->push_back(*$1);
		  delete $1;
		  $$ = tmp;
		}
	| udp_sequ_entry_list udp_sequ_entry
		{ list<string>*tmp = $1;
		  tmp->push_back(*$2);
		  delete $2;
		  $$ = tmp;
		}
	;

udp_sequ_entry
	: udp_input_list ':' udp_input_sym ':' udp_output_sym ';'
		{ string*tmp = $1;
		  *tmp += ':';
		  *tmp += $3;
		  *tmp += ':';
		  *tmp += $5;
		  $$ = tmp;
		}
	;

udp_initial
	: K_initial IDENTIFIER '=' NUMBER ';'
		{ PExpr*etmp = new PENumber($4);
		  PEIdent*itmp = new PEIdent(*$2);
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
		{ string*tmp = new string;
		  *tmp += $1;
		  $$ = tmp;
		}
	| udp_input_list udp_input_sym
		{ string*tmp = $1;
		  *tmp += $2;
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
	| 'f' { $$ = 'f'; }
	| 'r' { $$ = 'r'; }
	| 'n' { $$ = 'n'; }
	| 'p' { $$ = 'p'; }
	| '_' { $$ = '_'; }
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
		{ PWire*pp = new PWire(*$2);
		  pp->port_type = NetNet::POUTPUT;
		  list<PWire*>*tmp = new list<PWire*>;
		  tmp->push_back(pp);
		  delete $2;
		  $$ = tmp;
		}
	| K_reg IDENTIFIER ';'
		{ PWire*pp = new PWire(*$2, NetNet::REG);
		  pp->port_type = NetNet::PIMPLICIT;
		  list<PWire*>*tmp = new list<PWire*>;
		  tmp->push_back(pp);
		  delete $2;
		  $$ = tmp;
		}
	;

udp_port_decls
	: udp_port_decl
		{ $$ = $1; }
	| udp_port_decls udp_port_decl
		{ list<PWire*>*tmp = $1;
		  tmp->merge(*$2);
		  delete $2;
		  $$ = tmp;
		}
	;

udp_port_list
	: IDENTIFIER
		{ list<string>*tmp = new list<string>;
		  tmp->push_back(*$1);
		  delete $1;
		  $$ = tmp;
		}
	| udp_port_list ',' IDENTIFIER
		{ list<string>*tmp = $1;
		  tmp->push_back(*$3);
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
		{ pform_make_udp($2, $4, $7, $9, $8); }
	;
