
%{
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
      unsigned long numb;
      struct textv_s textv;
      comp_operands_t opa;

      struct argv_s argv;
      vpiHandle vpi;
};


%token K_FUNCTOR K_SCOPE K_THREAD K_VAR K_vpi_call

%token <text> T_INSTR
%token <text> T_LABEL
%token <numb> T_NUMBER
%token <text> T_STRING
%token <text> T_SYMBOL

%type <textv> symbols
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
		{ struct textv_s obj = $7;
		  compile_functor($1, $3, $5, obj.cnt, obj.text);
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

	| T_LABEL K_VAR ';'
		{ compile_variable($1); }

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
	: T_SYMBOL
		{ comp_operands_t opa = (comp_operands_t)
			calloc(1, sizeof(struct comp_operands_s));
		  opa->argc = 1;
		  opa->argv[0].ltype = L_TEXT;
		  opa->argv[0].text = $1;
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
     the struct argv_s type. */

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
	;


  /* functor operands can only be a list of symbols. */
symbols
	: T_SYMBOL
		{ struct textv_s obj;
		  textv_init(&obj);
		  textv_add(&obj, $1);
		  $$ = obj;
		}
	| symbols ',' T_SYMBOL
		{ struct textv_s obj = $1;
		  textv_add(&obj, $3);
		  $$ = obj;
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
