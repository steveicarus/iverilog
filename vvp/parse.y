
%{
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: parse.y,v 1.10 2001/03/21 05:13:03 steve Exp $"
#endif

# include  "parse_misc.h"
# include  "compile.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <assert.h>

/*
 * These are bits in the lexor.
 */
extern FILE*yyin;

%}

%union {
      char*text;
      long numb;

      comp_operands_t opa;

      struct symb_s  symb;
      struct symbv_s symbv;

      struct argv_s argv;
      vpiHandle vpi;
};


%token K_FUNCTOR K_SCOPE K_THREAD K_VAR K_vpi_call

%token <text> T_INSTR
%token <text> T_LABEL
%token <numb> T_NUMBER
%token <text> T_STRING
%token <text> T_SYMBOL

%type <symb>  symbol
%type <symbv> symbols
%type <text> label_opt
%type <opa>  operand operands operands_opt

%type <argv> argument_opt argument_list
%type <vpi>  argument

%%

  /* A program is simply a list of statements. No other structure. */
program
	: statement
	| program statement
	;


  /* A statement can be any of the following. In all cases, the
     statment is terminated by a semi-colon. In general, a statement
     has a label, an opcode of some source, and operands. The
     structure of the operands depends on the opcode. */

statement

  /* Functor statements define functors. The functor must have a
     label and a type name, and may have operands. */

	: T_LABEL K_FUNCTOR T_SYMBOL ',' T_NUMBER ',' symbols ';'
		{ struct symbv_s obj = $7;
		  compile_functor($1, $3, $5, obj.cnt, obj.vect);
		}
	| T_LABEL K_FUNCTOR T_SYMBOL','  T_NUMBER ';'
		{ compile_functor($1, $3, $5, 0, 0); }

  /* Instructions may have a label, and have zero or more
     operands. The meaning of and restrictions on the operands depends
     on the specific instruction. */

	| label_opt T_INSTR operands_opt ';'
		{ compile_code($1, $2, $3);
		}


  /* %vpi_call statements are instructions that have unusual operand
     requirements so are handled by their own rules. */

	| label_opt K_vpi_call T_STRING argument_opt ';'
		{ compile_vpi_call($1, $3, $4.argc, $4.argv); }


  /* Scope statements come in two forms. There are the scope
     declaration and the scope recall. */

	| T_LABEL K_SCOPE T_STRING ';'
		{ compile_scope_decl($1, $3, 0); }

	| T_LABEL K_SCOPE T_STRING ',' T_SYMBOL ';'
		{ compile_scope_decl($1, $3, $5); }

	|         K_SCOPE T_SYMBOL ';'
		{ compile_scope_recall($2); }

  /* Thread statements declare a thread with its starting address. The
     starting address must already be defined. */

	|         K_THREAD T_SYMBOL ';'
		{ compile_thread($2); }

  /* Var statements declare a bit of a variable. This also implicitly
     creates a functor with the same name that acts as the output of
     the variable in the netlist. */

	| T_LABEL K_VAR T_STRING ',' T_NUMBER ',' T_NUMBER ';'
		{ compile_variable($1, $3, $5, $7); }

  /* Oh and by the way, empty statements are OK as well. */

	| ';'
	;


  /* There are a few places where the label is optional. This rule
     returns the label value if present, or 0 if not. */

label_opt
	: T_LABEL { $$ = $1; }
	|         { $$ = 0; }
	;

operands_opt
	: operands { $$ = $1; }
	|          { $$ = 0; }
	;

operands
	: operands ',' operand
		{ comp_operands_t opa = $1;
		  assert(opa->argc < 3);
		  assert($3->argc == 1);
		  opa->argv[opa->argc] = $3->argv[0];
		  opa->argc += 1;
		  free($3);
		  $$ =  opa;
		}
	| operand
		{ $$ = $1; }
	;

operand
	: symbol
		{ comp_operands_t opa = (comp_operands_t)
			calloc(1, sizeof(struct comp_operands_s));
		  opa->argc = 1;
		  opa->argv[0].ltype = L_SYMB;
		  opa->argv[0].symb = $1;
		  $$ = opa;
		}
	| T_NUMBER
		{ comp_operands_t opa = (comp_operands_t)
			calloc(1, sizeof(struct comp_operands_s));
		  opa->argc = 1;
		  opa->argv[0].ltype = L_NUMB;
		  opa->argv[0].numb = $1;
		  $$ = opa;
		}
	;


  /* The argument_list is a list of vpiHandle objects that can be
     passed to a %vpi_call statement (and hence built into a
     vpiCallSysTask handle). We build up an arbitrary sized list with
     the struct argv_s type.

     Each argument of the call is represented as a vpiHandle
     object. If the argument is a symbol, it is located in the sym_vpi
     symbol table. if it is someother supported object, the necessary
     vpiHandle object is created to support it. */

argument_opt
	: ',' argument_list
		{ $$ = $2; }
	|
		{ struct argv_s tmp;
		  argv_init(&tmp);
		  $$ = tmp;
		}
	;

argument_list
	: argument
		{ struct argv_s tmp;
		  argv_init(&tmp);
		  argv_add(&tmp, $1);
		  $$ = tmp;
		}
	| argument_list ',' argument
		{ struct argv_s tmp = $1;
		  argv_add(&tmp, $3);
		  $$ = tmp;
		}
	;

argument
	: T_STRING
		{ $$ = vpip_make_string_const($1); }
	| T_SYMBOL
		{ $$ = compile_vpi_lookup($1);
		  free($1);
		}
	;


  /* functor operands can only be a list of symbols. */
symbols
	: symbol
		{ struct symbv_s obj;
		  symbv_init(&obj);
		  symbv_add(&obj, $1);
		  $$ = obj;
		}
	| symbols ',' symbol
		{ struct symbv_s obj = $1;
		  symbv_add(&obj, $3);
		  $$ = obj;
		}
	;


  /* In some cases, simple pointer arithmetic is allowed. In
     particular, functor vectors can be indexed with the [] syntax,
     with values from 0 up. */

symbol
	: T_SYMBOL
		{ $$.text = $1;
		  $$.idx = 0;
		}
	| T_SYMBOL '[' T_NUMBER ']'
		{ $$.text = $1;
		  $$.idx = $3;
		}
	;

%%

int compile_design(const char*path)
{
      yypath = path;
      yyline = 1;
      yyin = fopen(path, "r");
      int rc = yyparse();
      return rc;
}


/*
 * $Log: parse.y,v $
 * Revision 1.10  2001/03/21 05:13:03  steve
 *  Allow var objects as vpiHandle arguments to %vpi_call.
 *
 * Revision 1.9  2001/03/20 06:16:24  steve
 *  Add support for variable vectors.
 *
 * Revision 1.8  2001/03/20 02:48:40  steve
 *  Copyright notices.
 *
 */
