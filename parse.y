
%{
/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: parse.y,v 1.5 1998/11/11 03:13:04 steve Exp $"
#endif

# include  "parse_misc.h"
# include  "pform.h"

%}

%union {
      string*text;
      list<string>*strings;

      lgate*gate;
      list<lgate>*gates;

      PExpr*expr;
      list<PExpr*>*exprs;

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

%token <text>   IDENTIFIER SYSTEM_IDENTIFIER STRING
%token <number> NUMBER
%token K_LE K_GE K_EQ K_NE K_CEQ K_CNE
%token K_always K_and K_assign K_begin K_buf K_bufif0 K_bufif1 K_case
%token K_casex K_casez K_cmos K_deassign K_default K_defparam K_disable
%token K_edge K_else K_end K_endcase K_endfunction K_endmodule
%token K_endprimitive K_endspecify K_endtable K_endtask K_event K_for
%token K_force K_forever K_fork K_function K_highz0 K_highz1 K_if
%token K_initial K_inout K_input K_integer K_join K_large K_macromodule
%token K_medium K_module K_nand K_negedge K_nmos K_nor K_not K_notif0
%token K_notif1 K_or K_output K_pmos K_posedge K_primitive K_pull0
%token K_pull1 K_pulldown K_pullup K_rcmos K_reg K_release K_repeat
%token K_rnmos K_rpmos K_rtran K_rtranif0 K_rtranif1 K_scalered
%token K_small K_specify
%token K_specparam K_strong0 K_strong1 K_supply0 K_supply1 K_table K_task
%token K_time K_tran K_tranif0 K_tranif1 K_tri K_tri0 K_tri1 K_triand
%token K_trior K_vectored K_wait K_wand K_weak0 K_weak1 K_while K_wire
%token K_wor K_xnor K_xor

%type <text> identifier lvalue register_variable
%type <strings> list_of_register_variables
%type <strings> list_of_variables

%type <wire> port
%type <wires> list_of_ports list_of_ports_opt

%type <gate>  gate_instance
%type <gates> gate_instance_list

%type <expr>  bitsel delay delay_opt expression expr_primary const_expression
%type <exprs> expression_list

%type <exprs> range range_opt
%type <nettype>  net_type
%type <gatetype> gatetype
%type <porttype> port_type

%type <event_statement> event_control event_expression
%type <statement> statement statement_opt
%type <statement_list> statement_list

%left UNARY_PREC
%left '+' '-'
%left K_GE K_LE '<' '>'
%left K_EQ K_NE K_CEQ K_CNE
%left '&'
%left '^'

%%

source_file
	: description
	| source_file description
	;

bitsel
	: '[' const_expression ']'
		{ $$ = $2; }
	;

const_expression
	: NUMBER
		{ $$ = new PENumber($1);
		}
	| STRING
		{ $$ = new PEString(*$1);
		  delete $1;
		}
	;

delay
	: '#' NUMBER
		{ $$ = new PENumber($2);
		}
	| '#' IDENTIFIER
		{ $$ = new PEIdent(*$2);
		  delete $2;
		}
	;

delay_opt
	: delay { $$ = $1; }
	|       { $$ = 0; }
	;

description
	: module
	| primitive
	;

event_control
	: '@' IDENTIFIER
		{ yyerror(@1, "Sorry, event control not supported.");
		  $$ = 0;
		}
	| '@' '(' event_expression ')'
		{ $$ = $3;
		}
	;

event_expression
	: K_posedge expression
		{ $$ = new PEventStatement(NetPEvent::POSEDGE, $2);
		}
	| K_negedge expression
		{ $$ = new PEventStatement(NetPEvent::NEGEDGE, $2);
		}
	;

expression
	: expr_primary
		{ $$ = $1; }
	| '(' expression ')'
		{ $$ = $2; }
	| '~' expression %prec UNARY_PREC
		{ $$ = new PEUnary('~', $2);
		}
	| '&' expression %prec UNARY_PREC
		{ $$ = new PEUnary('&', $2);
		}
	| expression '^' expression
		{ $$ = new PEBinary('^', $1, $3);
		}
	| expression '+' expression
		{ $$ = new PEBinary('+', $1, $3);
		}
	| expression '-' expression
		{ $$ = new PEBinary('-', $1, $3);
		}
	| expression '&' expression
		{ $$ = new PEBinary('&', $1, $3);
		}
	| expression K_EQ expression
		{ $$ = new PEBinary('e', $1, $3);
		}
	| expression K_CEQ expression
		{ $$ = new PEBinary('E', $1, $3);
		}
	| expression K_NE expression
		{ $$ = new PEBinary('n', $1, $3);
		}
	| expression K_CNE expression
		{ $$ = new PEBinary('N', $1, $3);
		}
	;


expression_list
	: expression_list ',' expression
		{ list<PExpr*>*tmp = $1;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	| expression
		{ list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($1);
		  $$ = tmp;
		}
	| expression_list ','
		{ list<PExpr*>*tmp = $1;
		  tmp->push_back(0);
		  $$ = tmp;
		}
	;


expr_primary
	: NUMBER
		{ $$ = new PENumber($1);
		}
	| STRING
		{ $$ = new PEString(*$1);
		  delete $1;
		}
	| identifier
		{ $$ = new PEIdent(*$1);
		  delete $1;
		}
	| SYSTEM_IDENTIFIER
		{ $$ = new PEIdent(*$1);
		  delete $1;
		}
	| identifier '[' expression ']'
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->msb_ = $3;
		  delete $1;
		  $$ = tmp;
		}
	| identifier '[' expression ':' expression ']'
		{ PEIdent*tmp = new PEIdent(*$1);
		  tmp->msb_ = $3;
		  tmp->lsb_ = $5;
		  delete $1;
		  $$ = tmp;
		}
	;

gate_instance
	: IDENTIFIER '(' expression_list ')'
		{ lgate*tmp = new lgate;
		  tmp->name = *$1;
		  tmp->parms = $3;
		  delete $1;
		  $$ = tmp;
		}
	| '(' expression_list ')'
		{ lgate*tmp = new lgate;
		  tmp->name = "";
		  tmp->parms = $2;
		  $$ = tmp;
		}
	;

gate_instance_list
	: gate_instance_list ',' gate_instance
		{ list<lgate>*tmp = $1;
		  tmp->push_back(*$3);
		  delete $3;
		  $$ = tmp;
		}
	| gate_instance
		{ list<lgate>*tmp = new list<lgate>;
		  tmp->push_back(*$1);
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

list_of_register_variables
	: register_variable
		{ list<string>*tmp = new list<string>;
		  tmp->push_back(*$1);
		  delete $1;
		  $$ = tmp;
		}
	| list_of_register_variables ',' register_variable
		{ list<string>*tmp = $1;
		  tmp->push_back(*$3);
		  delete $3;
		  $$ = tmp;
		}
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

lvalue
	: identifier { $$ = $1; }
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
	| K_reg range_opt list_of_register_variables ';'
		{ pform_makewire($3, NetNet::REG);
		  if ($2) {
			pform_set_net_range($3, $2);
			delete $2;
		  }
		  delete $3;
		}
	| gatetype delay_opt gate_instance_list ';'
		{ pform_makegates($1, $2, $3);
		}
	| IDENTIFIER gate_instance_list ';'
		{ pform_make_modgates(*$1, $2);
		  delete $1;
		}
	| K_assign IDENTIFIER '=' expression ';'
		{ pform_make_pgassign(*$2, $4);
		  delete $2;
		}
	| K_assign IDENTIFIER bitsel '=' expression ';'
		{ pform_make_pgassign(*$2, $3, $5);
		  delete $2;
		}
	| K_assign IDENTIFIER range '=' expression ';'
		{ pform_make_pgassign(*$2, $5);
		  yyerror(@3, "Sorry, lvalue bit range not supported.");
		  delete $2;
		  delete $3;
		}
	| K_always statement
		{ pform_make_behavior(PProcess::PR_ALWAYS, $2);
		}
	| K_initial statement
		{ pform_make_behavior(PProcess::PR_INITIAL, $2);
		}
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

port
	: IDENTIFIER
		{ $$ = new PWire(*$1, NetNet::IMPLICIT);
		  $$->port_type = NetNet::PIMPLICIT;
		  delete $1;
		}
	| IDENTIFIER '[' const_expression ':' const_expression ']'
		{ $$ = new PWire(*$1, NetNet::IMPLICIT);
		  $$->port_type = NetNet::PIMPLICIT;
		  $$->msb = $3;
		  $$->lsb = $5;
		  delete $1;
		}
	| IDENTIFIER '[' error ']'
		{ yyerror(@1, "invalid port bit select");
		  $$ = new PWire(*$1, NetNet::IMPLICIT);
		  $$->port_type = NetNet::PIMPLICIT;
		  delete $1;
		}
	;

port_type
	: K_input { $$ = NetNet::PINPUT; }
	| K_output { $$ = NetNet::POUTPUT; }
	| K_inout { $$ = NetNet::PINOUT; }
	;

primitive
	: K_primitive IDENTIFIER '(' error ')' ';' K_endprimitive
		{ yyerror(@1, "Sorry, primitives not supported."); }
	;

range
	: '[' const_expression ':' const_expression ']'
		{ list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($2);
		  tmp->push_back($4);
		  $$ = tmp;
		}
	;

range_opt
	: range
	| { $$ = 0; }
	;

register_variable
	: IDENTIFIER
		{ $$ = $1; }
	| IDENTIFIER '[' error ']'
		{ yyerror(@1, "Sorry, register regions not implemented.");
		  $$ = $1;
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
	| K_for '(' lvalue '=' expression ';' expression ';'
	  lvalue '=' expression ')' statement
		{ $$ = new PForStatement(*$3, $5, $7, *$9, $11, $13);
		  delete $3;
		  delete $9;
		}
	| K_for '(' lvalue '=' expression ';' expression ';'
	  error ')' statement
		{ $$ = 0;
		  yyerror(@9, "Error in for loop step assigment.");
		}
	| K_for '(' lvalue '=' expression ';' error ';'
	  lvalue '=' expression ')' statement
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
		  $$ = tmp;
		}
	| event_control statement_opt
		{ PEventStatement*tmp = $1;
		  tmp->set_statement($2);
		  $$ = tmp;
		}
	| lvalue '=' expression ';'
		{ $$ = pform_make_assignment($1, $3);
		}
	| lvalue K_LE expression ';'
		{ $$ = pform_make_assignment($1, $3);
		  yyerror(@1, "Sorry, non-blocking assignment not implemented.");
		}
	| K_wait '(' expression ')' statement_opt
		{ PEventStatement*tmp;
		  tmp = new PEventStatement(NetPEvent::POSITIVE, $3);
		  tmp->set_statement($5);
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER '(' expression_list ')' ';'
		{ $$ = pform_make_calltask($1, $3);
		}
	| SYSTEM_IDENTIFIER ';'
		{ $$ = pform_make_calltask($1);
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
