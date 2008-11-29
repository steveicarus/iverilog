
%{
/*
 * Copyright (c) 1998-2008 Stephen Williams (steve@icarus.com)
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
# include  "Statement.h"
# include  "PSpec.h"
# include  <stack>
# include  <cstring>
# include  <sstream>

class PSpecPath;

extern void lex_start_table();
extern void lex_end_table();

static svector<PExpr*>* param_active_range = 0;
static bool param_active_signed = false;
static ivl_variable_type_t param_active_type = IVL_VT_LOGIC;

/* Port declaration lists use this structure for context. */
static struct {
      NetNet::Type port_net_type;
      NetNet::PortType port_type;
      ivl_variable_type_t var_type;
      bool sign_flag;
      svector<PExpr*>* range;
} port_declaration_context = {NetNet::NONE, NetNet::NOT_A_PORT,
                              IVL_VT_NO_TYPE, false, 0};

/* The task and function rules need to briefly hold the pointer to the
   task/function that is currently in progress. */
static PTask* current_task = 0;
static PFunction* current_function = 0;
static stack<PBlock*> current_block_stack;

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

static list<perm_string>* list_from_identifier(char*id)
{
      list<perm_string>*tmp = new list<perm_string>;
      tmp->push_back(lex_strings.make(id));
      delete[]id;
      return tmp;
}

static list<perm_string>* list_from_identifier(list<perm_string>*tmp, char*id)
{
      tmp->push_back(lex_strings.make(id));
      delete[]id;
      return tmp;
}

static svector<PExpr*>* copy_range(svector<PExpr*>* orig)
{
      svector<PExpr*>*copy = 0;

      if (orig) {
	    copy = new svector<PExpr*>(2);
	    (*copy)[0] = (*orig)[0];
	    (*copy)[1] = (*orig)[1];
      }

      return copy;
}

template <class T> void append(vector<T>&out, const vector<T>&in)
{
      for (size_t idx = 0 ; idx < in.size() ; idx += 1)
	    out.push_back(in[idx]);
}

/*
 * This is a shorthand for making a PECallFunction that takes a single
 * arg. This is used by some of the code that detects built-ins.
 */
static PECallFunction*make_call_function(perm_string tn, PExpr*arg)
{
      svector<PExpr*> parms(1);
      parms[0] = arg;
      PECallFunction*tmp = new PECallFunction(tn, parms);
      return tmp;
}

static PECallFunction*make_call_function(perm_string tn, PExpr*arg1, PExpr*arg2)
{
      svector<PExpr*> parms(2);
      parms[0] = arg1;
      parms[1] = arg2;
      PECallFunction*tmp = new PECallFunction(tn, parms);
      return tmp;
}

%}

%union {
      bool flag;

      char letter;

	/* text items are C strings allocated by the lexor using
	   strdup. They can be put into lists with the texts type. */
      char*text;
      list<perm_string>*perm_strings;
      pform_name_t*pform_name;

      ivl_discipline_t discipline;

      hname_t*hier;

      list<string>*strings;

      struct str_pair_t drive;

      PCase::Item*citem;
      svector<PCase::Item*>*citems;

      lgate*gate;
      svector<lgate>*gates;

      Module::port_t *mport;
      LexicalScope::range_t* value_range;
      vector<Module::port_t*>*mports;

      named_pexpr_t*named_pexpr;
      svector<named_pexpr_t*>*named_pexprs;
      struct parmvalue_t*parmvalue;

      PExpr*expr;
      svector<PExpr*>*exprs;

      svector<PEEvent*>*event_expr;

      NetNet::Type nettype;
      PGBuiltin::Type gatetype;
      NetNet::PortType porttype;
      ivl_variable_type_t datatype;

      PWire*wire;
      svector<PWire*>*wires;

      PEventStatement*event_statement;
      Statement*statement;
      svector<Statement*>*statement_list;

      PTaskFuncArg function_type;

      net_decl_assign_t*net_decl_assign;

      verinum* number;

      verireal* realtime;

      PSpecPath* specpath;
      list<index_component_t> *dimensions;
};

%token <text>   IDENTIFIER SYSTEM_IDENTIFIER STRING
%token <discipline> DISCIPLINE_IDENTIFIER
%token <text>   PATHPULSE_IDENTIFIER
%token <number> BASED_NUMBER DEC_NUMBER
%token <realtime> REALTIME
%token K_LE K_GE K_EG K_EQ K_NE K_CEQ K_CNE K_LS K_RS K_RSS K_SG
 /* K_CONTRIBUTE is <+, the contribution assign. */
%token K_CONTRIBUTE
%token K_PO_POS K_PO_NEG K_POW
%token K_PSTAR K_STARP
%token K_LOR K_LAND K_NAND K_NOR K_NXOR K_TRIGGER
%token K_abs K_abstol K_access K_acos K_acosh K_asin K_analog K_asinh
%token K_atan K_atanh K_atan2 K_automatic
%token K_always K_and K_assign K_begin K_bool K_buf K_bufif0 K_bufif1 K_case
%token K_casex K_casez K_ceil K_cmos K_continuous K_cos K_cosh
%token K_ddt_nature K_deassign K_default K_defparam K_disable K_discrete
%token K_domain K_edge K_edge_descriptor K_discipline
%token K_else K_end K_endcase K_enddiscipline K_endfunction K_endgenerate
%token K_endmodule K_endnature
%token K_endprimitive K_endspecify K_endtable K_endtask K_event
%token K_exclude K_exp K_floor K_flow K_from
%token K_for K_force K_forever K_fork K_function K_generate K_genvar
%token K_ground K_highz0 K_highz1 K_hypot K_idt_nature K_if K_ifnone K_inf
%token K_initial K_inout K_input K_integer K_join K_large K_ln K_localparam
%token K_log K_logic K_macromodule K_max
%token K_medium K_min K_module K_nand K_nature K_negedge
%token K_nmos K_nor K_not K_notif0
%token K_notif1 K_or K_output K_parameter K_pmos K_posedge K_potential
%token K_pow K_primitive
%token K_pull0 K_pull1 K_pulldown K_pullup K_rcmos K_real K_realtime
%token K_reg K_release K_repeat
%token K_rnmos K_rpmos K_rtran K_rtranif0 K_rtranif1 K_scalared
%token K_signed K_sin K_sinh K_small K_specify
%token K_specparam K_sqrt K_string K_strong0 K_strong1 K_supply0 K_supply1
%token K_table
%token K_tan K_tanh K_task
%token K_time K_tran K_tranif0 K_tranif1 K_tri K_tri0 K_tri1 K_triand
%token K_trior K_trireg K_units K_vectored K_wait K_wand K_weak0 K_weak1
%token K_while K_wire
%token K_wone K_wor K_xnor K_xor
%token K_Shold K_Speriod K_Srecovery K_Srecrem K_Ssetup K_Swidth K_Ssetuphold

%token KK_attribute

%type <flag>    from_exclude
%type <number>  number
%type <flag>    signed_opt udp_reg_opt edge_operator automatic_opt
%type <drive>   drive_strength drive_strength_opt dr_strength0 dr_strength1
%type <letter>  udp_input_sym udp_output_sym
%type <text>    udp_input_list udp_sequ_entry udp_comb_entry
%type <perm_strings> udp_input_declaration_list
%type <strings> udp_entry_list udp_comb_entry_list udp_sequ_entry_list
%type <strings> udp_body
%type <perm_strings> udp_port_list
%type <wires>   udp_port_decl udp_port_decls
%type <statement> udp_initial udp_init_opt
%type <expr>    udp_initial_expr_opt

%type <text> register_variable net_variable real_variable
%type <perm_strings> register_variable_list net_variable_list real_variable_list list_of_identifiers

%type <net_decl_assign> net_decl_assign net_decl_assigns

%type <mport> port port_opt port_reference port_reference_list
%type <mport> port_declaration
%type <mports> list_of_ports module_port_list_opt list_of_port_declarations
%type <value_range> parameter_value_range parameter_value_ranges
%type <value_range> parameter_value_ranges_opt
%type <expr> value_range_expression

%type <wires> task_item task_item_list task_item_list_opt
%type <wires> task_port_item task_port_decl task_port_decl_list
%type <wires> function_item function_item_list

%type <named_pexpr> port_name parameter_value_byname
%type <named_pexprs> port_name_list parameter_value_byname_list

%type <named_pexpr> attribute
%type <named_pexprs> attribute_list attribute_list_opt

%type <citem>  case_item
%type <citems> case_items

%type <gate>  gate_instance
%type <gates> gate_instance_list

%type <pform_name> hierarchy_identifier
%type <expr>  expression expr_primary expr_mintypmax
%type <expr>  lpvalue
%type <expr>  branch_probe_expression
%type <expr>  delay_value delay_value_simple
%type <exprs> delay1 delay3 delay3_opt delay_value_list
%type <exprs> expression_list_with_nuls expression_list_proper
%type <exprs> cont_assign cont_assign_list

%type <exprs> range range_opt
%type <dimensions> dimensions_opt dimensions
%type <nettype>  net_type var_type net_type_opt
%type <gatetype> gatetype
%type <porttype> port_type
%type <datatype> primitive_type primitive_type_opt
%type <parmvalue> parameter_value_opt

%type <function_type> function_range_or_type_opt
%type <event_expr> event_expression_list
%type <event_expr> event_expression
%type <event_statement> event_control
%type <statement> statement statement_or_null
%type <statement_list> statement_list

%type <statement> analog_statement

%type <letter> spec_polarity
%type <perm_strings>  specify_path_identifiers

%type <specpath> specify_simple_path specify_simple_path_decl
%type <specpath> specify_edge_path specify_edge_path_decl

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
%left K_POW
%left UNARY_PREC


/* to resolve dangling else ambiguity. */
%nonassoc less_than_K_else
%nonassoc K_else

 /* to resolve exclude (... ambiguity */
%nonassoc '('
%nonassoc K_exclude

%%

  /* A degenerate source file can be completely empty. */
main : source_file | ;

source_file
	: description
	| source_file description
	;

number  : BASED_NUMBER
	     { $$ = $1; based_size = 0;}
        | DEC_NUMBER
	     { $$ = $1; based_size = 0;}
        | DEC_NUMBER BASED_NUMBER
	     { $$ = pform_verinum_with_size($1,$2, @2.text, @2.first_line);
	       based_size = 0; }
	;

  /* real and realtime are exactly the same so save some code
   * with a common matching rule. */
real_or_realtime
	: K_real
	| K_realtime
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
		  delete[]$1;
		  $$ = tmp;
		}
	| IDENTIFIER '=' expression
		{ PExpr*tmp = $3;
		  named_pexpr_t*tmp2 = new named_pexpr_t;
		  tmp2->name = lex_strings.make($1);
		  tmp2->parm = tmp;
		  delete[]$1;
		  $$ = tmp2;
		}
	;


  /* The block_item_decl is used in function definitions, task
     definitions, module definitions and named blocks. Wherever a new
     scope is entered, the source may declare new registers and
     integers. This rule matches those declarations. The containing
     rule has presumably set up the scope. */

block_item_decl
	: attribute_list_opt K_reg
          primitive_type_opt signed_opt range
          register_variable_list ';'
		{ ivl_variable_type_t dtype = $3;
		  if (dtype == IVL_VT_NO_TYPE)
			dtype = IVL_VT_LOGIC;
		  pform_set_net_range($6, $5, $4, dtype);
		  if ($1) delete $1;
		}

  /* This differs from the above pattern only in the absence of the
     range. This is the rule for a scalar. */

	| attribute_list_opt K_reg
          primitive_type_opt signed_opt
          register_variable_list ';'
		{ ivl_variable_type_t dtype = $3;
		  if (dtype == IVL_VT_NO_TYPE)
			dtype = IVL_VT_LOGIC;
		  pform_set_net_range($5, 0, $4, dtype);
		  if ($1) delete $1;
		}

  /* Integer declarations are simpler in that they do not have all the
     trappings of a general variable declaration. All of that is
     implicit in the "integer" of the declaration. */

	| attribute_list_opt K_integer register_variable_list ';'
		{ pform_set_reg_integer($3);
		  if ($1) delete $1;
		}

	| attribute_list_opt K_time register_variable_list ';'
		{ pform_set_reg_time($3);
		}

  /* real declarations are fairly simple as there is no range of
     signed flag in the declaration. Create the real as a NetNet::REG
     with real value. Note that real and realtime are interchangeable
     in this context. */

        | attribute_list_opt K_real real_variable_list ';'
                { delete $3; }
        | attribute_list_opt K_realtime real_variable_list ';'
                { delete $3; }

	| K_event list_of_identifiers ';'
		{ pform_make_events($2, @1.text, @1.first_line);
		}

	| K_parameter parameter_assign_decl ';'
	| K_localparam localparam_assign_decl ';'

  /* Recover from errors that happen within variable lists. Use the
     trailing semi-colon to resync the parser. */

	| attribute_list_opt K_reg error ';'
		{ yyerror(@2, "error: syntax error in reg variable list.");
		  yyerrok;
		  if ($1) delete $1;
		}
	| attribute_list_opt K_integer error ';'
		{ yyerror(@2, "error: syntax error in integer variable list.");
		  yyerrok;
		  if ($1) delete $1;
		}
	| attribute_list_opt K_time error ';'
		{ yyerror(@2, "error: syntax error in time variable list.");
		  yyerrok;
		}
	| attribute_list_opt K_real error ';'
		{ yyerror(@2, "error: syntax error in real variable list.");
		  yyerrok;
		}
	| attribute_list_opt K_realtime error ';'
		{ yyerror(@2, "error: syntax error in realtime variable list.");
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
	: expression_list_proper ':' statement_or_null
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->expr = *$1;
		  tmp->stat = $3;
		  delete $1;
		  $$ = tmp;
		}
	| K_default ':' statement_or_null
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->stat = $3;
		  $$ = tmp;
		}
	| K_default  statement_or_null
		{ PCase::Item*tmp = new PCase::Item;
		  tmp->stat = $2;
		  $$ = tmp;
		}
	| error ':' statement_or_null
		{ yyerror(@2, "error: Incomprehensible case expression.");
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
	: hierarchy_identifier '=' expression
		{ pform_set_defparam(*$1, $3);
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

delay_value_list
        : delay_value
              { svector<PExpr*>*tmp = new svector<PExpr*>(1);
	        (*tmp)[0] = $1;
		$$ = tmp;
	      }
	| delay_value_list ',' delay_value
              { svector<PExpr*>*tmp = new svector<PExpr*>(*$1, $3);
		delete $1;
		$$ = tmp;
	      }
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
			FILE_NAME($$, @1);
		  }
		  based_size = 0;
		}
	| REALTIME
		{ verireal*tmp = $1;
		  if (tmp == 0) {
			yyerror(@1, "internal error: delay.");
			$$ = 0;
		  } else {
			$$ = new PEFNumber(tmp);
			FILE_NAME($$, @1);
		  }
		}
	| IDENTIFIER
                { PEIdent*tmp = new PEIdent(lex_strings.make($1));
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		  delete[]$1;
		}
	;

description
  : module
  | udp_primitive
  | nature_declaration
  | discipline_declaration
  | KK_attribute '(' IDENTIFIER ',' STRING ',' STRING ')'
      { perm_string tmp3 = lex_strings.make($3);
	pform_set_type_attrib(tmp3, $5, $7);
	delete[]$3;
	delete $5;
      }
  ;

  /* The discipline and nature declarations used to take no ';' after
     the identifier. The 2.3 LRM adds the ';', but since there are
     programs written to the 2.1 and 2.2 standard that don't, we
     choose to make the ';' optional in this context. */
optional_semicolon : ';' | ;

discipline_declaration
  : K_discipline IDENTIFIER optional_semicolon
      { pform_start_discipline($2); }
    discipline_items K_enddiscipline
      { pform_end_discipline(@1); delete[] $2; }
  ;

discipline_items
  : discipline_items discipline_item
  | discipline_item
  ;

discipline_item
  : K_domain K_discrete ';'
      { pform_discipline_domain(@1, IVL_DIS_DISCRETE); }
  | K_domain K_continuous ';'
      { pform_discipline_domain(@1, IVL_DIS_CONTINUOUS); }
  | K_potential IDENTIFIER ';'
      { pform_discipline_potential(@1, $2); delete[] $2; }
  | K_flow IDENTIFIER ';'
      { pform_discipline_flow(@1, $2); delete[] $2; }
  ;

nature_declaration
  : K_nature IDENTIFIER optional_semicolon
      { pform_start_nature($2); }
    nature_items
    K_endnature
      { pform_end_nature(@1); delete[] $2; }
  ;

nature_items
  : nature_items nature_item
  | nature_item
  ;

nature_item
  : K_units '=' STRING ';'
      { delete[] $3; }
  | K_abstol '=' expression ';'
  | K_access '=' IDENTIFIER ';'
      { pform_nature_access(@1, $3); delete[] $3; }
  | K_idt_nature '=' IDENTIFIER ';'
      { delete[] $3; }
  | K_ddt_nature '=' IDENTIFIER ';'
      { delete[] $3; }
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
	: '@' hierarchy_identifier
		{ PEIdent*tmpi = new PEIdent(*$2);
		  PEEvent*tmpe = new PEEvent(PEEvent::ANYEDGE, tmpi);
		  PEventStatement*tmps = new PEventStatement(tmpe);
		  FILE_NAME(tmps, @1);
		  $$ = tmps;
		  delete $2;
		}
	| '@' '(' event_expression_list ')'
		{ PEventStatement*tmp = new PEventStatement(*$3);
		  FILE_NAME(tmp, @1);
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
		  FILE_NAME(tmp, @1);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	| K_negedge expression
		{ PEEvent*tmp = new PEEvent(PEEvent::NEGEDGE, $2);
		  FILE_NAME(tmp, @1);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	| expression
		{ PEEvent*tmp = new PEEvent(PEEvent::ANYEDGE, $1);
		  FILE_NAME(tmp, @1);
		  svector<PEEvent*>*tl = new svector<PEEvent*>(1);
		  (*tl)[0] = tmp;
		  $$ = tl;
		}
	;

  /* A branch probe expression applies a probe function (potential or
     flow) to a branch. The branch may be implicit as a pair of nets
     or explicit as a named branch. Elaboration will check that the
     function name really is a nature attribute identifier. */
branch_probe_expression
  : IDENTIFIER '(' IDENTIFIER ',' IDENTIFIER ')'
      { $$ = pform_make_branch_probe_expression(@1, $1, $3, $5); }
  | IDENTIFIER '(' IDENTIFIER ')'
      { $$ = pform_make_branch_probe_expression(@1, $1, $3); }
  ;

expression
	: expr_primary
		{ $$ = $1; }
	| '+' expr_primary %prec UNARY_PREC
		{ $$ = $2; }
	| '-' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('-', $2);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| '~' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('~', $2);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| '&' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('&', $2);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| '!' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('!', $2);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| '|' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('|', $2);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| '^' expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('^', $2);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| K_NAND expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('A', $2);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| K_NOR expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('N', $2);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| K_NXOR expr_primary %prec UNARY_PREC
		{ PEUnary*tmp = new PEUnary('X', $2);
		  FILE_NAME(tmp, @2);
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
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_POW expression
		{ PEBinary*tmp = new PEBPower('p', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '*' expression
		{ PEBinary*tmp = new PEBinary('*', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '/' expression
		{ PEBinary*tmp = new PEBinary('/', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '%' expression
		{ PEBinary*tmp = new PEBinary('%', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '+' expression
		{ PEBinary*tmp = new PEBinary('+', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '-' expression
		{ PEBinary*tmp = new PEBinary('-', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '&' expression
		{ PEBinary*tmp = new PEBinary('&', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '|' expression
		{ PEBinary*tmp = new PEBinary('|', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_NAND expression
		{ PEBinary*tmp = new PEBinary('A', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_NOR expression
		{ PEBinary*tmp = new PEBinary('O', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_NXOR expression
		{ PEBinary*tmp = new PEBinary('X', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '<' expression
		{ PEBinary*tmp = new PEBComp('<', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '>' expression
		{ PEBinary*tmp = new PEBComp('>', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_LS expression
		{ PEBinary*tmp = new PEBShift('l', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_RS expression
		{ PEBinary*tmp = new PEBShift('r', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_RSS expression
		{ PEBinary*tmp = new PEBShift('R', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_EQ expression
		{ PEBinary*tmp = new PEBComp('e', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_CEQ expression
		{ PEBinary*tmp = new PEBComp('E', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_LE expression
		{ PEBinary*tmp = new PEBComp('L', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_GE expression
		{ PEBinary*tmp = new PEBComp('G', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_NE expression
		{ PEBinary*tmp = new PEBComp('n', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_CNE expression
		{ PEBinary*tmp = new PEBComp('N', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_LOR expression
		{ PEBinary*tmp = new PEBLogic('o', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression K_LAND expression
		{ PEBinary*tmp = new PEBLogic('a', $1, $3);
		  FILE_NAME(tmp, @2);
		  $$ = tmp;
		}
	| expression '?' expression ':' expression
		{ PETernary*tmp = new PETernary($1, $3, $5);
		  FILE_NAME(tmp, @2);
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
		  if (min_typ_max_warn > 0) {
		        cerr << $$->get_fileline() << ": warning: choosing ";
		        switch (min_typ_max_flag) {
		            case MIN:
		              cerr << "min";
		              break;
		            case TYP:
		              cerr << "typ";
		              break;
		            case MAX:
		              cerr << "max";
		              break;
		        }
		        cerr << " expression." << endl;
		        min_typ_max_warn -= 1;
		  }
		}
	;


  /* Many contexts take a comma separated list of expressions. Null
     expressions can happen anywhere in the list, so there are two
     extra rules in expression_list_with_nuls for parsing and
     installing those nulls.

     The expression_list_proper rules do not allow null items in the
     expression list, so can be used where nul expressions are not allowed. */

expression_list_with_nuls
	: expression_list_with_nuls ',' expression
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

	| expression_list_with_nuls ','
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
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| REALTIME
		{ PEFNumber*tmp = new PEFNumber($1);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| STRING
		{ PEString*tmp = new PEString($1);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER
                { perm_string tn = lex_strings.make($1);
		  PECallFunction*tmp = new PECallFunction(tn);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		  delete[]$1;
		}

  /* The hierarchy_identifier rule matches simple identifiers as well as
     indexed arrays and part selects */

    | hierarchy_identifier
        { PEIdent*tmp = new PEIdent(*$1);
	  FILE_NAME(tmp, @1);
	  $$ = tmp;
	  delete $1;
	}

  /* An identifier followed by an expression list in parentheses is a
     function call. If a system identifier, then a system function
     call. */

  | hierarchy_identifier '(' expression_list_proper ')'
      { PECallFunction*tmp = new PECallFunction(*$1, *$3);
	FILE_NAME(tmp, @1);
	delete $1;
	$$ = tmp;
      }
  | SYSTEM_IDENTIFIER '(' expression_list_proper ')'
      { perm_string tn = lex_strings.make($1);
	PECallFunction*tmp = new PECallFunction(tn, *$3);
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }

  /* Many of the VAMS built-in functions are available as builtin
     functions with $system_function equivalents. */

  | K_acos '(' expression ')'
      { perm_string tn = perm_string::literal("$acos");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_acosh '(' expression ')'
      { perm_string tn = perm_string::literal("$acosh");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_asin '(' expression ')'
      { perm_string tn = perm_string::literal("$asin");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_asinh '(' expression ')'
      { perm_string tn = perm_string::literal("$asinh");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_atan '(' expression ')'
      { perm_string tn = perm_string::literal("$atan");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_atanh '(' expression ')'
      { perm_string tn = perm_string::literal("$atanh");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_atan2 '(' expression ',' expression ')'
      { perm_string tn = perm_string::literal("$atan2");
	PECallFunction*tmp = make_call_function(tn, $3, $5);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_ceil '(' expression ')'
      { perm_string tn = perm_string::literal("$ceil");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_cos '(' expression ')'
      { perm_string tn = perm_string::literal("$cos");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_cosh '(' expression ')'
      { perm_string tn = perm_string::literal("$cosh");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_exp '(' expression ')'
      { perm_string tn = perm_string::literal("$exp");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_floor '(' expression ')'
      { perm_string tn = perm_string::literal("$floor");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_hypot '(' expression ',' expression ')'
      { perm_string tn = perm_string::literal("$hypot");
	PECallFunction*tmp = make_call_function(tn, $3, $5);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_ln '(' expression ')'
      { perm_string tn = perm_string::literal("$ln");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_log '(' expression ')'
      { perm_string tn = perm_string::literal("$log10");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_pow '(' expression ',' expression ')'
      { perm_string tn = perm_string::literal("$pow");
        PECallFunction*tmp = make_call_function(tn, $3, $5);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_sin '(' expression ')'
      { perm_string tn = perm_string::literal("$sin");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_sinh '(' expression ')'
      { perm_string tn = perm_string::literal("$sinh");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_sqrt '(' expression ')'
      { perm_string tn = perm_string::literal("$sqrt");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_tan '(' expression ')'
      { perm_string tn = perm_string::literal("$tan");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_tanh '(' expression ')'
      { perm_string tn = perm_string::literal("$tanh");
	PECallFunction*tmp = make_call_function(tn, $3);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  /* These mathematical functions are conveniently expressed as unary
     and binary expressions. They behave much like unary/binary
     operators, even though they are parsed as functions.  */

  | K_abs '(' expression ')'
      { PEUnary*tmp = new PEUnary('m', $3);
        FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_max '(' expression ',' expression ')'
      { PEBinary*tmp = new PEBinary('M', $3, $5);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | K_min '(' expression ',' expression ')'
      { PEBinary*tmp = new PEBinary('m', $3, $5);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  /* Parenthesized expressions are primaries. */

	| '(' expr_mintypmax ')'
		{ $$ = $2; }

  /* Various kinds of concatenation expressions. */

	| '{' expression_list_proper '}'
		{ PEConcat*tmp = new PEConcat(*$2);
		  FILE_NAME(tmp, @1);
		  delete $2;
		  $$ = tmp;
		}
	| '{' expression '{' expression_list_proper '}' '}'
		{ PExpr*rep = $2;
		  PEConcat*tmp = new PEConcat(*$4, rep);
		  FILE_NAME(tmp, @1);
		  delete $4;
		  $$ = tmp;
		}
	| '{' expression '{' expression_list_proper '}' error '}'
		{ PExpr*rep = $2;
		  PEConcat*tmp = new PEConcat(*$4, rep);
		  FILE_NAME(tmp, @1);
		  delete $4;
		  $$ = tmp;
		  yyerror(@5, "error: Syntax error between internal '}' "
			  "and closing '}' of repeat concatenation.");
		  yyerrok;
		}
	;

  /* A function_item_list borrows the task_port_item run to match
     declarations of ports. We check later to make sure there are no
     output or inout ports actually used. */
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

function_item
  : task_port_item
      { $$ = $1; }
  | block_item_decl
      { $$ = 0; }
  ;

  /* A gate_instance is a module instantiation or a built in part
     type. In any case, the gate has a set of connections to ports. */
gate_instance
	: IDENTIFIER '(' expression_list_with_nuls ')'
		{ lgate*tmp = new lgate;
		  tmp->name = $1;
		  tmp->parms = $3;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete[]$1;
		  $$ = tmp;
		}

	| IDENTIFIER range '(' expression_list_with_nuls ')'
		{ lgate*tmp = new lgate;
		  svector<PExpr*>*rng = $2;
		  tmp->name = $1;
		  tmp->parms = $4;
		  tmp->range[0] = (*rng)[0];
		  tmp->range[1] = (*rng)[1];
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete[]$1;
		  delete rng;
		  $$ = tmp;
		}
	| '(' expression_list_with_nuls ')'
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
		  tmp->parms_by_name = 0;
		  tmp->range[0] = (*rng)[0];
		  tmp->range[1] = (*rng)[1];
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete[]$1;
		  delete rng;
		  $$ = tmp;
		}

  /* Modules can also take ports by port-name expressions. */

	| IDENTIFIER '(' port_name_list ')'
		{ lgate*tmp = new lgate;
		  tmp->name = $1;
		  tmp->parms = 0;
		  tmp->parms_by_name = $3;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete[]$1;
		  $$ = tmp;
		}

	| IDENTIFIER range '(' port_name_list ')'
		{ lgate*tmp = new lgate;
		  svector<PExpr*>*rng = $2;
		  tmp->name = $1;
		  tmp->parms = 0;
		  tmp->parms_by_name = $4;
		  tmp->range[0] = (*rng)[0];
		  tmp->range[1] = (*rng)[1];
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  delete[]$1;
		  delete rng;
		  $$ = tmp;
		}

	| IDENTIFIER '(' error ')'
		{ lgate*tmp = new lgate;
		  tmp->name = $1;
		  tmp->parms = 0;
		  tmp->parms_by_name = 0;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  yyerror(@2, "error: Syntax error in instance port "
			  "expression(s).");
		  delete[]$1;
		  $$ = tmp;
		}

	| IDENTIFIER range '(' error ')'
		{ lgate*tmp = new lgate;
		  tmp->name = $1;
		  tmp->parms = 0;
		  tmp->parms_by_name = 0;
		  tmp->file  = @1.text;
		  tmp->lineno = @1.first_line;
		  yyerror(@3, "error: Syntax error in instance port "
			  "expression(s).");
		  delete[]$1;
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
     hierarchical name from the left to the right, forming a list of
     names. */

hierarchy_identifier
    : IDENTIFIER
        { $$ = new pform_name_t;
	  $$->push_back(name_component_t(lex_strings.make($1)));
	  delete[]$1;
	}
    | hierarchy_identifier '.' IDENTIFIER
        { pform_name_t * tmp = $1;
	  tmp->push_back(name_component_t(lex_strings.make($3)));
	  delete[]$3;
	  $$ = tmp;
	}
    | hierarchy_identifier '[' expression ']'
        { pform_name_t * tmp = $1;
	  name_component_t&tail = tmp->back();
	  index_component_t itmp;
	  itmp.sel = index_component_t::SEL_BIT;
	  itmp.msb = $3;
	  tail.index.push_back(itmp);
	  $$ = tmp;
	}
    | hierarchy_identifier '[' expression ':' expression ']'
        { pform_name_t * tmp = $1;
	  name_component_t&tail = tmp->back();
	  index_component_t itmp;
	  itmp.sel = index_component_t::SEL_PART;
	  itmp.msb = $3;
	  itmp.lsb = $5;
	  tail.index.push_back(itmp);
	  $$ = tmp;
	}
    | hierarchy_identifier '[' expression K_PO_POS expression ']'
        { pform_name_t * tmp = $1;
	  name_component_t&tail = tmp->back();
	  index_component_t itmp;
	  itmp.sel = index_component_t::SEL_IDX_UP;
	  itmp.msb = $3;
	  itmp.lsb = $5;
	  tail.index.push_back(itmp);
	  $$ = tmp;
	}
    | hierarchy_identifier '[' expression K_PO_NEG expression ']'
        { pform_name_t * tmp = $1;
	  name_component_t&tail = tmp->back();
	  index_component_t itmp;
	  itmp.sel = index_component_t::SEL_IDX_DO;
	  itmp.msb = $3;
	  itmp.lsb = $5;
	  tail.index.push_back(itmp);
	  $$ = tmp;
	}
    ;

  /* This is a list of identifiers. The result is a list of strings,
     each one of the identifiers in the list. These are simple,
     non-hierarchical names separated by ',' characters. */
list_of_identifiers
	: IDENTIFIER
                { $$ = list_from_identifier($1); }
	| list_of_identifiers ',' IDENTIFIER
                { $$ = list_from_identifier($1, $3); }
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
		{ vector<Module::port_t*>*tmp
			  = new vector<Module::port_t*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	| list_of_ports ',' port_opt
	        { vector<Module::port_t*>*tmp = $1;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	;

list_of_port_declarations
	: port_declaration
		{ vector<Module::port_t*>*tmp
			  = new vector<Module::port_t*>(1);
		  (*tmp)[0] = $1;
		  $$ = tmp;
		}
	| list_of_port_declarations ',' port_declaration
	        { vector<Module::port_t*>*tmp = $1;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	| list_of_port_declarations ',' IDENTIFIER
		{ Module::port_t*ptmp;
		  perm_string name = lex_strings.make($3);
		  ptmp = pform_module_port_reference(name, @3.text,
						     @3.first_line);
		  vector<Module::port_t*>*tmp = $1;
		  tmp->push_back(ptmp);

		    /* Get the port declaration details, the port type
		       and what not, from context data stored by the
		       last port_declaration rule. */
		  pform_module_define_port(@3, name,
					port_declaration_context.port_type,
					port_declaration_context.port_net_type,
					port_declaration_context.sign_flag,
					port_declaration_context.range, 0);
		  delete[]$3;
		  $$ = tmp;
		}
	| list_of_port_declarations ','
		{
		  yyerror(@2, "error: NULL port declarations are not "
		              "allowed.");
		}
	| list_of_port_declarations ';'
		{
		  yyerror(@2, "error: ';' is an invalid port declaration "
		              "separator.");
		}
        ;

port_declaration
  : attribute_list_opt
    K_input net_type_opt signed_opt range_opt IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($6);
	ptmp = pform_module_port_reference(name, @2.text,
					   @2.first_line);
	pform_module_define_port(@2, name, NetNet::PINPUT,
				 $3, $4, $5, $1);
	port_declaration_context.port_type = NetNet::PINPUT;
	port_declaration_context.port_net_type = $3;
	port_declaration_context.sign_flag = $4;
	delete port_declaration_context.range;
	port_declaration_context.range = $5;
	delete $1;
	delete[]$6;
	$$ = ptmp;
      }
  | attribute_list_opt
    K_inout  net_type_opt signed_opt range_opt IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($6);
	ptmp = pform_module_port_reference(name, @2.text,
					   @2.first_line);
	pform_module_define_port(@2, name, NetNet::PINOUT,
				 $3, $4, $5, $1);
	port_declaration_context.port_type = NetNet::PINOUT;
	port_declaration_context.port_net_type = $3;
	port_declaration_context.sign_flag = $4;
	delete port_declaration_context.range;
	port_declaration_context.range = $5;
	delete $1;
	delete[]$6;
	$$ = ptmp;
      }
  | attribute_list_opt
    K_output net_type_opt signed_opt range_opt IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($6);
	ptmp = pform_module_port_reference(name, @2.text,
					   @2.first_line);
	pform_module_define_port(@2, name, NetNet::POUTPUT,
				 $3, $4, $5, $1);
	port_declaration_context.port_type = NetNet::POUTPUT;
	port_declaration_context.port_net_type = $3;
	port_declaration_context.sign_flag = $4;
	delete port_declaration_context.range;
	port_declaration_context.range = $5;
	delete $1;
	delete[]$6;
	$$ = ptmp;
      }
  | attribute_list_opt
    K_output var_type signed_opt range_opt IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($6);
	ptmp = pform_module_port_reference(name, @2.text,
					   @2.first_line);
	pform_module_define_port(@2, name, NetNet::POUTPUT,
				 $3, $4, $5, $1);
	port_declaration_context.port_type = NetNet::POUTPUT;
	port_declaration_context.port_net_type = $3;
	port_declaration_context.sign_flag = $4;
	delete port_declaration_context.range;
	port_declaration_context.range = $5;
	delete $1;
	delete[]$6;
	$$ = ptmp;
      }
  | attribute_list_opt
    K_output var_type signed_opt range_opt IDENTIFIER '=' expression
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($6);
	ptmp = pform_module_port_reference(name, @2.text,
					   @2.first_line);
	pform_module_define_port(@2, name, NetNet::POUTPUT,
				 $3, $4, $5, $1);
	port_declaration_context.port_type = NetNet::POUTPUT;
	port_declaration_context.port_net_type = $3;
	port_declaration_context.sign_flag = $4;
	delete port_declaration_context.range;
	port_declaration_context.range = $5;

	pform_make_reginit(@6, name, $8);

	delete $1;
	delete[]$6;
	$$ = ptmp;
      }
  ;



net_type_opt
	: net_type { $$ = $1; }
	| { $$ = NetNet::IMPLICIT; }
	;

signed_opt : K_signed { $$ = true; } | {$$ = false; } ;

  /* An lpvalue is the expression that can go on the left side of a
     procedural assignment. This rule handles only procedural
     assignments. It is more limited then the general expr_primary
     rule to reflect the rules for assignment l-values. */
lpvalue
    : hierarchy_identifier
        { PEIdent*tmp = new PEIdent(*$1);
	  FILE_NAME(tmp, @1);
	  $$ = tmp;
	  delete $1;
	}
    | '{' expression_list_proper '}'
	{ PEConcat*tmp = new PEConcat(*$2);
	  FILE_NAME(tmp, @1);
	  delete $2;
	  $$ = tmp;
	}
    ;


  /* Continuous assignments have a list of individual assignments. */

cont_assign
	: lpvalue '=' expression
		{ svector<PExpr*>*tmp = new svector<PExpr*>(2);
		  (*tmp)[0] = $1;
		  (*tmp)[1] = $3;
		  $$ = tmp;
		}
	;

cont_assign_list
	: cont_assign_list ',' cont_assign
		{ svector<PExpr*>*tmp = new svector<PExpr*>(*$1, *$3);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	| cont_assign
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
		  delete[]$3;
		}

	;

module_start : K_module | K_macromodule ;

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

  /* This rule detects net declarations that possibly include a
     primitive type, an optional vector range and signed flag. This
     also includes an optional delay set. The values are then applied
     to a list of names. If the primitive type is not specified, then
     resort to the default type LOGIC. */

	: attribute_list_opt net_type
          primitive_type_opt signed_opt range_opt
          delay3_opt
          net_variable_list ';'

		{ ivl_variable_type_t dtype = $3;
		  if (dtype == IVL_VT_NO_TYPE)
			dtype = IVL_VT_LOGIC;
		  pform_makewire(@2, $5, $4, $7, $2,
				 NetNet::NOT_A_PORT, dtype, $1);
		  if ($6 != 0) {
			yyerror(@6, "sorry: net delays not supported.");
			delete $6;
		  }
		  if ($1) delete $1;
		}

  /* Very similar to the rule above, but this takes a list of
     net_decl_assigns, which are <name> = <expr> assignment
     declarations. */

	| attribute_list_opt net_type
          primitive_type_opt signed_opt range_opt
          delay3_opt net_decl_assigns ';'

		{ ivl_variable_type_t dtype = $3;
		  if (dtype == IVL_VT_NO_TYPE)
			dtype = IVL_VT_LOGIC;
		  pform_makewire(@2, $5, $4, $6,
				 str_strength, $7, $2, dtype);
		  if ($1) {
			yyerror(@2, "sorry: Attributes not supported "
				"on net declaration assignments.");
			delete $1;
		  }
		}

  /* This form doesn't have the range, but does have strengths. This
     gives strength to the assignment drivers. */

	| attribute_list_opt net_type
          primitive_type_opt signed_opt
          drive_strength net_decl_assigns ';'

		{ ivl_variable_type_t dtype = $3;
		  if (dtype == IVL_VT_NO_TYPE)
			dtype = IVL_VT_LOGIC;
		  pform_makewire(@2, 0, $4, 0, $5, $6, $2, dtype);
		  if ($1) {
			yyerror(@2, "sorry: Attributes not supported "
				"on net declaration assignments.");
			delete $1;
		  }
		}

	| K_trireg charge_strength_opt range_opt delay3_opt list_of_identifiers ';'
		{ yyerror(@1, "sorry: trireg nets not supported.");
		  delete $3;
		  delete $4;
		}

	| port_type signed_opt range_opt delay3_opt list_of_identifiers ';'
		{ pform_set_port_type(@1, $5, $3, $2, $1);
		}

  /* The next two rules handle Verilog 2001 statements of the form:
       input wire signed [h:l] <list>;
     This creates the wire and sets the port type all at once. */

	| port_type net_type signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, $1, IVL_VT_NO_TYPE, 0,
		                 SR_BOTH);
		}

	| K_output var_type signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, NetNet::POUTPUT,
				 IVL_VT_NO_TYPE, 0, SR_BOTH);
		}

  /* var_type declaration (reg variables) cannot be input or output,
     because the port declaration implies an external driver, which
     cannot be attached to a reg. These rules catch that error early. */

	| K_input var_type signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, NetNet::PINPUT,
				 IVL_VT_NO_TYPE, 0);
		  yyerror(@2, "error: reg variables cannot be inputs.");
		}

	| K_inout var_type signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, NetNet::PINOUT,
				 IVL_VT_NO_TYPE, 0);
		  yyerror(@2, "error: reg variables cannot be inouts.");
		}

	| port_type signed_opt range_opt delay3_opt error ';'
		{ yyerror(@1, "error: Invalid variable list"
			  " in port declaration.");
		  if ($3) delete $3;
		  if ($4) delete $4;
		  yyerrok;
		}

  /* Maybe this is a discipline declaration? If so, then the lexor
     will see the discipline name as an identifier. We match it to the
     discipline or type name semantically. */
  | DISCIPLINE_IDENTIFIER list_of_identifiers ';'
  { pform_attach_discipline(@1, $1, $2); }

  /* block_item_decl rule is shared with task blocks and named
     begin/end. */

	| block_item_decl

  /* */

	| K_defparam defparam_assign_list ';'

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

	| attribute_list_opt
	  IDENTIFIER parameter_value_opt gate_instance_list ';'
		{ perm_string tmp1 = lex_strings.make($2);
		  pform_make_modgates(tmp1, $3, $4);
		  delete[]$2;
		  if ($1) delete $1;
		}

        | attribute_list_opt
	  IDENTIFIER parameter_value_opt error ';'
		{ yyerror(@2, "error: Invalid module instantiation");
		  delete[]$2;
		  if ($1) delete $1;
		}

  /* Continuous assignment can have an optional drive strength, then
     an optional delay3 that applies to all the assignments in the
     cont_assign_list. */

	| K_assign drive_strength_opt delay3_opt cont_assign_list ';'
		{ pform_make_pgassign_list($4, $3, $2, @1.text, @1.first_line); }

  /* Always and initial items are behavioral processes. */

  | attribute_list_opt K_always statement
      { PProcess*tmp = pform_make_behavior(IVL_PR_ALWAYS, $3, $1);
	FILE_NAME(tmp, @2);
      }
  | attribute_list_opt K_initial statement
      { PProcess*tmp = pform_make_behavior(IVL_PR_INITIAL, $3, $1);
	FILE_NAME(tmp, @2);
      }

  | attribute_list_opt K_analog analog_statement
      { pform_make_analog_behavior(@2, IVL_PR_ALWAYS, $3); }

  /* The task declaration rule matches the task declaration
     header, then pushes the function scope. This causes the
     definitions in the task_body to take on the scope of the task
     instead of the module. Note that these runs accept for the task
     body statement_or_null, although the standard does not allow null
     statements in the task body. But we continue to accept it as an
     extension. */

  | K_task automatic_opt IDENTIFIER ';'
      { assert(current_task == 0);
	current_task = pform_push_task_scope($3, $2);
	FILE_NAME(current_task, @1);
      }
    task_item_list_opt
    statement_or_null
    K_endtask
      { current_task->set_ports($6);
	current_task->set_statement($7);
	pform_pop_scope();
	current_task = 0;
	delete[]$3;
      }

  | K_task automatic_opt IDENTIFIER
      { assert(current_task == 0);
	current_task = pform_push_task_scope($3, $2);
	FILE_NAME(current_task, @1);
      }
    '(' task_port_decl_list ')' ';'
    block_item_decls_opt
    statement_or_null
    K_endtask
      { current_task->set_ports($6);
	current_task->set_statement($10);
	pform_pop_scope();
	current_task = 0;
	delete[]$3;
      }
  | K_task automatic_opt IDENTIFIER error K_endtask
      {
	pform_pop_scope();
	current_task = 0;
	delete[]$3;
      }

  /* The function declaration rule matches the function declaration
     header, then pushes the function scope. This causes the
     definitions in the func_body to take on the scope of the function
     instead of the module. */

  | K_function automatic_opt function_range_or_type_opt IDENTIFIER ';'
      { assert(current_function == 0);
	current_function = pform_push_function_scope($4, $2);
	FILE_NAME(current_function, @1);
      }
    function_item_list statement
    K_endfunction
      { current_function->set_ports($7);
	current_function->set_statement($8);
	current_function->set_return($3);
	pform_pop_scope();
	current_function = 0;
	delete[]$4;
      }

  | K_function automatic_opt function_range_or_type_opt IDENTIFIER
      { assert(current_function == 0);
	current_function = pform_push_function_scope($4, $2);
	FILE_NAME(current_function, @1);
      }
    '(' task_port_decl_list ')' ';'
    block_item_decls_opt
    statement
    K_endfunction
      { current_function->set_ports($7);
	current_function->set_statement($11);
	current_function->set_return($3);
	pform_pop_scope();
	current_function = 0;
	delete[]$4;
      }
  | K_function automatic_opt function_range_or_type_opt IDENTIFIER error K_endfunction
      {
	pform_pop_scope();
	current_task = 0;
	delete[]$4;
      }

  /* A generate region can contain further module items. Actually, it
     is supposed to be limited to certain kinds of module items, but
     the semantic tests will check that for us. */

  | K_generate module_item_list_opt K_endgenerate

  | K_genvar list_of_identifiers ';'
      { pform_genvars(@1, $2); }

  | K_for '(' IDENTIFIER '=' expression ';'
              expression ';'
              IDENTIFIER '=' expression ')'
      { pform_start_generate_for(@1, $3, $5, $7, $9, $11); }
    generate_block
      { pform_endgenerate(); }

  | generate_if
    generate_block_opt
    K_else
      { pform_start_generate_else(@1); }
    generate_block
      { pform_endgenerate(); }

  | generate_if
    generate_block_opt %prec less_than_K_else
      { pform_endgenerate(); }

  | K_case '(' expression ')'
      { pform_start_generate_case(@1, $3); }
    generate_case_items
    K_endcase
      { pform_endgenerate(); }

  /* Handle some anachronistic syntax cases. */
  | K_generate K_begin module_item_list_opt K_end K_endgenerate
      { /* Detect and warn about anachronistic begin/end use */
	if (generation_flag > GN_VER2001) {
	      warn_count += 1;
	      cerr << @2 << ": warning: Anachronistic use of begin/end to surround generate schemes." << endl;
	}
      }
  | K_generate K_begin ':' IDENTIFIER {
	pform_start_generate_nblock(@1, $4);
      } module_item_list_opt K_end K_endgenerate
      { /* Detect and warn about anachronistic named begin/end use */
	if (generation_flag > GN_VER2001) {
	      warn_count += 1;
	      cerr << @2 << ": warning: Anachronistic use of named begin/end to surround generate schemes." << endl;
	}
	pform_endgenerate();
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

	| K_module error ';'
		{ yyerror(@1, "error: missing endmodule or attempt to "
		              "nest modules.");
		  pform_error_nested_modules();
		  yyerrok;
		}

	| error ';'
		{ yyerror(@2, "error: invalid module item.");
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
		  delete[]$3;
		  delete $5;
		}
	| KK_attribute '(' error ')' ';'
		{ yyerror(@1, "error: Malformed $attribute parameter list."); }
	;

automatic_opt
	: K_automatic { $$ = true; }
	| { $$ = false;}
	;

generate_if : K_if '(' expression ')' { pform_start_generate_if(@1, $3); }

generate_case_items
  : generate_case_items generate_case_item
  | generate_case_item
  ;

generate_case_item
  : expression_list_proper ':' { pform_generate_case_item(@1, $1); } generate_block_opt
      { pform_endgenerate(); }
  | K_default ':' { pform_generate_case_item(@1, 0); } generate_block_opt
      { pform_endgenerate(); }
  ;

module_item_list
	: module_item_list module_item
	| module_item
	;

module_item_list_opt
	: module_item_list
	|
	;

  /* A generate block is the thing within a generate scheme. It may be
     a single module item, an anonymous block of module items, or a
     named module item. In all cases, the meat is in the module items
     inside, and the processing is done by the module_item rules. We
     only need to take note here of the scope name, if any. */

generate_block
        : module_item
        | K_begin module_item_list_opt K_end
        | K_begin ':' IDENTIFIER module_item_list_opt K_end
             { pform_generate_block_name($3); }
        ;

generate_block_opt : generate_block | ';' ;


  /* A net declaration assignment allows the programmer to combine the
     net declaration and the continuous assignment into a single
     statement.

     Note that the continuous assignment statement is generated as a
     side effect, and all I pass up is the name of the l-value. */

net_decl_assign
  : IDENTIFIER '=' expression
      { net_decl_assign_t*tmp = new net_decl_assign_t;
	tmp->next = tmp;
	tmp->name = lex_strings.make($1);
	tmp->expr = $3;
	delete[]$1;
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

primitive_type
        : K_logic { $$ = IVL_VT_LOGIC; }
        | K_bool { $$ = IVL_VT_BOOL; }
        | K_real { $$ = IVL_VT_REAL; }
        ;

primitive_type_opt : primitive_type { $$ = $1; } | { $$ = IVL_VT_NO_TYPE; } ;

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
	| K_wone    { $$ = NetNet::WONE; }
	;

var_type
	: K_reg { $$ = NetNet::REG; }
	;

  /* In this rule we have matched the "parameter" keyword. The rule
     generates a type (optional) and a list of assignments. */

parameter_assign_decl
  : parameter_assign_list
  | range
      { param_active_range = $1;
        param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
    parameter_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_signed range
      { param_active_range = $2;
	param_active_signed = true;
	param_active_type = IVL_VT_LOGIC;
      }
    parameter_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_integer
      { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
        PExpr*re;
        re = new PENumber(new verinum(integer_width-1, integer_width));
        (*range_stub)[0] = re;
        re = new PENumber(new verinum((uint64_t)0, integer_width));
        (*range_stub)[1] = re;
        /* The default range is [31:0] */
        param_active_range = range_stub;
	param_active_signed = true;
	param_active_type = IVL_VT_LOGIC;
      }
    parameter_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_time
      { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
        PExpr*re;
        re = new PENumber(new verinum((uint64_t)63, integer_width));
        (*range_stub)[0] = re;
        re = new PENumber(new verinum((uint64_t)0, integer_width));
        (*range_stub)[1] = re;
        /* The range is [63:0] */
        param_active_range = range_stub;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
    parameter_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | real_or_realtime
      { param_active_range = 0;
	param_active_signed = true;
	param_active_type = IVL_VT_REAL;
      }
    parameter_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  ;

parameter_assign_list
	: parameter_assign
	| parameter_assign_list ',' parameter_assign
	;

parameter_assign
  : IDENTIFIER '=' expression parameter_value_ranges_opt
      { PExpr*tmp = $3;
	pform_set_parameter(@1, lex_strings.make($1), param_active_type,
			    param_active_signed, param_active_range, tmp, $4);
	delete[]$1;
      }
  ;

parameter_value_ranges_opt : parameter_value_ranges { $$ = $1; } | { $$ = 0; } ;

parameter_value_ranges
  : parameter_value_ranges parameter_value_range
      { $$ = $2; $$->next = $1; }
  | parameter_value_range
      { $$ = $1; $$->next = 0; }
  ;

parameter_value_range
  : from_exclude '[' value_range_expression ':' value_range_expression ']'
      { $$ = pform_parameter_value_range($1, false, $3, false, $5); }
  | from_exclude '[' value_range_expression ':' value_range_expression ')'
      { $$ = pform_parameter_value_range($1, false, $3, true, $5); }
  | from_exclude '(' value_range_expression ':' value_range_expression ']'
      { $$ = pform_parameter_value_range($1, true, $3, false, $5); }
  | from_exclude '(' value_range_expression ':' value_range_expression ')'
      { $$ = pform_parameter_value_range($1, true, $3, true, $5); }
  | K_exclude expression
      { $$ = pform_parameter_value_range(true, false, $2, false, $2); }
  ;

value_range_expression
  : expression { $$ = $1; }
  | K_inf { $$ = 0; }
  | '+' K_inf { $$ = 0; }
  | '-' K_inf { $$ = 0; }
  ;

from_exclude : K_from { $$ = false; } | K_exclude { $$ = true; } ;

  /* Localparam assignments and assignment lists are broken into
     separate BNF so that I can call slightly different parameter
     handling code. They parse the same as parameters, they just
     behave differently when someone tries to override them. */

localparam_assign
  : IDENTIFIER '=' expression
      { PExpr*tmp = $3;
	pform_set_localparam(@1, lex_strings.make($1),
			     param_active_type,
			     param_active_signed,
			     param_active_range, tmp);
	delete[]$1;
      }
  ;

localparam_assign_decl
  : localparam_assign_list
  | range
      { param_active_range = $1;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
    localparam_assign_list
      { param_active_range = 0;
        param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_signed range
      { param_active_range = $2;
	param_active_signed = true;
	param_active_type = IVL_VT_LOGIC;
      }
    localparam_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_integer
      { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
        PExpr*re;
        re = new PENumber(new verinum(integer_width-1, integer_width));
        (*range_stub)[0] = re;
        re = new PENumber(new verinum((uint64_t)0, integer_width));
        (*range_stub)[1] = re;
        /* The default range is [31:0] */
        param_active_range = range_stub;
	param_active_signed = true;
	param_active_type = IVL_VT_LOGIC;
      }
    localparam_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_time
      { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
        PExpr*re;
        re = new PENumber(new verinum((uint64_t)63, integer_width));
        (*range_stub)[0] = re;
        re = new PENumber(new verinum((uint64_t)0, integer_width));
        (*range_stub)[1] = re;
        /* The range is [63:0] */
        param_active_range = range_stub;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
    localparam_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | real_or_realtime
      { param_active_range = 0;
	param_active_signed = true;
	param_active_type = IVL_VT_REAL;
      }
    localparam_assign_list
      { param_active_range = 0;
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
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
	: '#' '(' expression_list_with_nuls ')'
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
		  FILE_NAME(tmp, @1);

		  struct parmvalue_t*lst = new struct parmvalue_t;
		  lst->by_order = new svector<PExpr*>(1);
		  (*lst->by_order)[0] = tmp;
		  lst->by_name = 0;
		  $$ = lst;
		  based_size = 0;
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
		  delete[]$2;
		  $$ = tmp;
		}
	| '.' IDENTIFIER '(' ')'
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = 0;
		  delete[]$2;
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
		  delete[]$2;
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
		  delete[]$2;
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
	  perm_string name = lex_strings.make($1);
	  ptmp = pform_module_port_reference(name, @1.text, @1.first_line);
	  delete[]$1;
	  $$ = ptmp;
	}

    | IDENTIFIER '[' expression ':' expression ']'
	{ index_component_t itmp;
	  itmp.sel = index_component_t::SEL_PART;
	  itmp.msb = $3;
	  itmp.lsb = $5;

	  name_component_t ntmp (lex_strings.make($1));
	  ntmp.index.push_back(itmp);

	  pform_name_t pname;
	  pname.push_back(ntmp);

	  PEIdent*wtmp = new PEIdent(pname);
	  FILE_NAME(wtmp, @1);

	  Module::port_t*ptmp = new Module::port_t;
	  ptmp->name = perm_string();
	  ptmp->expr.push_back(wtmp);

	  delete[]$1;
	  $$ = ptmp;
	}

    | IDENTIFIER '[' expression ']'
	{ index_component_t itmp;
	  itmp.sel = index_component_t::SEL_BIT;
	  itmp.msb = $3;
	  itmp.lsb = 0;

	  name_component_t ntmp (lex_strings.make($1));
	  ntmp.index.push_back(itmp);

	  pform_name_t pname;
	  pname.push_back(ntmp);

	  PEIdent*tmp = new PEIdent(pname);
	  FILE_NAME(tmp, @1);

	  Module::port_t*ptmp = new Module::port_t;
	  ptmp->name = perm_string();
	  ptmp->expr.push_back(tmp);
	  delete[]$1;
	  $$ = ptmp;
	}

    | IDENTIFIER '[' error ']'
        { yyerror(@1, "error: invalid port bit select");
	  Module::port_t*ptmp = new Module::port_t;
	  PEIdent*wtmp = new PEIdent(lex_strings.make($1));
	  FILE_NAME(wtmp, @1);
	  ptmp->name = lex_strings.make($1);
	  ptmp->expr.push_back(wtmp);
	  delete[]$1;
	  $$ = ptmp;
	}
    ;


port_reference_list
	: port_reference
		{ $$ = $1; }
	| port_reference_list ',' port_reference
		{ Module::port_t*tmp = $1;
		  append(tmp->expr, $3->expr);
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
		  delete[]$2;
		  $$ = tmp;
		}
	| '.' IDENTIFIER '(' error ')'
		{ yyerror(@3, "error: invalid port connection expression.");
		  named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = 0;
		  delete[]$2;
		  $$ = tmp;
		}
	| '.' IDENTIFIER '(' ')'
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = 0;
		  delete[]$2;
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
		  (*tmp)[0] = $2;
		  (*tmp)[1] = $4;
		  $$ = tmp;
		}
	;

range_opt
	: range
	| { $$ = 0; }
	;
dimensions_opt
	: { $$ = 0; }
	| dimensions { $$ = $1; }

dimensions
	: '[' expression ':' expression ']'
		{ list<index_component_t> *tmp = new list<index_component_t>;
		  index_component_t index;
		  index.msb = $2;
		  index.lsb = $4;
		  tmp->push_back(index);
		  $$ = tmp;
		}
	| dimensions '[' expression ':' expression ']'
		{ list<index_component_t> *tmp = $1;
		  index_component_t index;
		  index.msb = $3;
		  index.lsb = $5;
		  tmp->push_back(index);
		  $$ = tmp;
		}

  /* This is used to express the return type of a function. */
function_range_or_type_opt
	: range           { $$.range = $1; $$.type = PTF_REG; }
	| K_signed range  { $$.range = $2; $$.type = PTF_REG_S; }
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
  : IDENTIFIER dimensions_opt
      { perm_string ident_name = lex_strings.make($1);
	pform_makewire(@1, ident_name, NetNet::REG,
		       NetNet::NOT_A_PORT, IVL_VT_NO_TYPE, 0);
	if ($2 != 0) {
	      index_component_t index;
	      if ($2->size() > 1) {
		    yyerror(@2, "sorry: only 1 dimensional arrays "
			    "are currently supported.");
	      }
	      index = $2->front();
	      pform_set_reg_idx(ident_name, index.msb, index.lsb);
	      delete $2;
	}
	$$ = $1;
      }
  | IDENTIFIER '=' expression
      { perm_string ident_name = lex_strings.make($1);
	pform_makewire(@1, ident_name, NetNet::REG,
		       NetNet::NOT_A_PORT, IVL_VT_NO_TYPE, 0);
	pform_make_reginit(@1, ident_name, $3);
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

real_variable
  : IDENTIFIER dimensions_opt
      { perm_string name = lex_strings.make($1);
        pform_makewire(@1, name, NetNet::REG, NetNet::NOT_A_PORT, IVL_VT_REAL, 0);
        if ($2 != 0) {
	      index_component_t index;
	      if ($2->size() > 1) {
		    yyerror(@2, "sorry: only 1 dimensional arrays "
			    "are currently supported.");
	      }
	      index = $2->front();
	      pform_set_reg_idx(name, index.msb, index.lsb);
	      delete $2;
        }
	$$ = $1;
      }
  | IDENTIFIER '=' expression
      { perm_string name = lex_strings.make($1);
	pform_makewire(@1, name, NetNet::REG, NetNet::NOT_A_PORT, IVL_VT_REAL, 0);
	pform_make_reginit(@1, name, $3);
	$$ = $1;
      }
  ;

real_variable_list
  : real_variable
      { list<perm_string>*tmp = new list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	$$ = tmp;
	delete[]$1;
      }
  | real_variable_list ',' real_variable
      { list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	$$ = tmp;
	delete[]$3;
      }
  ;

net_variable
  : IDENTIFIER dimensions_opt
      { perm_string name = lex_strings.make($1);
	pform_makewire(@1, name, NetNet::IMPLICIT,
		       NetNet::NOT_A_PORT, IVL_VT_NO_TYPE, 0);
	if ($2 != 0) {
	      index_component_t index;
	      if ($2->size() > 1) {
		    yyerror(@2, "sorry: only 1 dimensional arrays "
			    "are currently supported.");
	      }
	      index = $2->front();
	      pform_set_reg_idx(name, index.msb, index.lsb);
	      delete $2;
	}
	$$ = $1;
      }
  ;

net_variable_list
	: net_variable
		{ list<perm_string>*tmp = new list<perm_string>;
		  tmp->push_back(lex_strings.make($1));
		  $$ = tmp;
		  delete[]$1;
		}
	| net_variable_list ',' net_variable
		{ list<perm_string>*tmp = $1;
		  tmp->push_back(lex_strings.make($3));
		  $$ = tmp;
		  delete[]$3;
		}
	;

specify_item
	: K_specparam specparam_list ';'
	| specify_simple_path_decl ';'
                { pform_module_specify_path($1);
		}
	| specify_edge_path_decl ';'
		{ pform_module_specify_path($1);
		}
	| K_if '(' expression ')' specify_simple_path_decl ';'
                { PSpecPath*tmp = $5;
		  if (tmp) {
			tmp->conditional = true;
			tmp->condition = $3;
		  }
		  pform_module_specify_path(tmp);
		}
	| K_if '(' expression ')' specify_edge_path_decl ';'
                { PSpecPath*tmp = $5;
		  if (tmp) {
			tmp->conditional = true;
			tmp->condition = $3;
		  }
		  pform_module_specify_path(tmp);
		}
	| K_ifnone specify_simple_path_decl ';'
                { PSpecPath*tmp = $2;
		  if (tmp) {
			tmp->conditional = true;
			tmp->condition = 0;
		  }
		  pform_module_specify_path(tmp);
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

specify_item_list
	: specify_item
	| specify_item_list specify_item
	;

specify_edge_path_decl
	: specify_edge_path '=' '(' delay_value_list ')'
                { $$ = pform_assign_path_delay($1, $4); }
	| specify_edge_path '=' delay_value_simple
                { svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = $3;
		  $$ = pform_assign_path_delay($1, tmp);
		}
	;

edge_operator : K_posedge { $$ = true; } | K_negedge { $$ = false; } ;

specify_edge_path
	: '('               specify_path_identifiers spec_polarity
	      K_EG '(' specify_path_identifiers polarity_operator expression ')' ')'
                    { int edge_flag = 0;
		      $$ = pform_make_specify_edge_path(@1, edge_flag, $2, $3, false, $6, $8); }
	| '(' edge_operator specify_path_identifiers spec_polarity
	      K_EG '(' specify_path_identifiers polarity_operator expression ')' ')'
                    { int edge_flag = $2? 1 : -1;
		      $$ = pform_make_specify_edge_path(@1, edge_flag, $3, $4, false, $7, $9);}
	| '('               specify_path_identifiers spec_polarity
	      K_SG  '(' specify_path_identifiers polarity_operator expression ')' ')'
                    { int edge_flag = 0;
		      $$ = pform_make_specify_edge_path(@1, edge_flag, $2, $3, true, $6, $8); }
	| '(' edge_operator specify_path_identifiers spec_polarity
	      K_SG '(' specify_path_identifiers polarity_operator expression ')' ')'
                    { int edge_flag = $2? 1 : -1;
		      $$ = pform_make_specify_edge_path(@1, edge_flag, $3, $4, true, $7, $9); }
	;

polarity_operator
        : K_PO_POS
	| K_PO_NEG
	| ':'
	;

specify_simple_path_decl
	: specify_simple_path '=' '(' delay_value_list ')'
                { $$ = pform_assign_path_delay($1, $4); }
	| specify_simple_path '=' delay_value_simple
                { svector<PExpr*>*tmp = new svector<PExpr*>(1);
		  (*tmp)[0] = $3;
		  $$ = pform_assign_path_delay($1, tmp);
		}
	| specify_simple_path '=' '(' error ')'
		{ yyerror(@3, "Syntax error in delay value list.");
		  yyerrok;
		  $$ = 0;
		}
	;

specify_simple_path
	: '(' specify_path_identifiers spec_polarity
              K_EG specify_path_identifiers ')'
                { $$ = pform_make_specify_path(@1, $2, $3, false, $5); }
	| '(' specify_path_identifiers spec_polarity
              K_SG specify_path_identifiers ')'
                { $$ = pform_make_specify_path(@1, $2, $3, true, $5); }
	| '(' error ')'
		{ yyerror(@1, "Invalid simple path");
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
		  pform_set_specparam(lex_strings.make($1), tmp);
		  delete[]$1;
		}
	| IDENTIFIER '=' expression ':' expression ':' expression
                { PExpr*tmp = 0;
		  switch (min_typ_max_flag) {
		      case MIN:
			tmp = $3;
			delete $5;
			delete $7;
			break;
		      case TYP:
			delete $3;
			tmp = $5;
			delete $7;
			break;
		      case MAX:
			delete $3;
			delete $5;
			tmp = $7;
			break;
		  }
		  if (min_typ_max_warn > 0) {
		        cerr << tmp->get_fileline() << ": warning: choosing ";
		        switch (min_typ_max_flag) {
		            case MIN:
		              cerr << "min";
		              break;
		            case TYP:
		              cerr << "typ";
		              break;
		            case MAX:
		              cerr << "max";
		              break;
		        }
		        cerr << " expression." << endl;
		        min_typ_max_warn -= 1;
		  }
		  pform_set_specparam(lex_strings.make($1), tmp);
		  delete[]$1;
		}
	| PATHPULSE_IDENTIFIER '=' expression
		{ delete[]$1;
		  delete $3;
		}
	| PATHPULSE_IDENTIFIER '=' '(' expression ',' expression ')'
		{ delete[]$1;
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

  /* The edge_descriptor is detected by the lexor as the various
     2-letter edge sequences that are supported here. For now, we
     don't care what they are, because we do not yet support specify
     edge events. */
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
	| ','  hierarchy_identifier
		{ delete $2; }
	| spec_notifier ','
		{  }
	| spec_notifier ',' hierarchy_identifier
                { delete $3; }
	| IDENTIFIER
		{ delete[]$1; }
	;


statement

  /* assign and deassign statements are procedural code to do
     structural assignments, and to turn that structural assignment
     off. This stronger then any other assign, but weaker then the
     force assignments. */

	: K_assign lpvalue '=' expression ';'
		{ PCAssign*tmp = new PCAssign($2, $4);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}

	| K_deassign lpvalue ';'
		{ PDeassign*tmp = new PDeassign($2);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}


  /* Force and release statements are similar to assignments,
     syntactically, but they will be elaborated differently. */

	| K_force lpvalue '=' expression ';'
		{ PForce*tmp = new PForce($2, $4);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_release lpvalue ';'
		{ PRelease*tmp = new PRelease($2);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}

  /* begin-end blocks come in a variety of forms, including named and
     anonymous. The named blocks can also carry their own reg
     variables, which are placed in the scope created by the block
     name. These are handled by pushing the scope name then matching
     the declarations. The scope is popped at the end of the block. */

  | K_begin statement_list K_end
      { PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	FILE_NAME(tmp, @1);
	tmp->set_statement(*$2);
	delete $2;
	$$ = tmp;
      }
  | K_begin ':' IDENTIFIER
      { PBlock*tmp = pform_push_block_scope($3, PBlock::BL_SEQ);
	FILE_NAME(tmp, @1);
	current_block_stack.push(tmp);
      }
    block_item_decls_opt
    statement_list K_end
      { pform_pop_scope();
	assert(! current_block_stack.empty());
	PBlock*tmp = current_block_stack.top();
	current_block_stack.pop();
	tmp->set_statement(*$6);
	delete[]$3;
	delete $6;
	$$ = tmp;
      }
  | K_begin K_end
      { PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_begin ':' IDENTIFIER K_end
      { PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	FILE_NAME(tmp, @1);
	delete[]$3;
	$$ = tmp;
      }
  | K_begin error K_end
      { yyerrok; }

  /* fork-join blocks are very similar to begin-end blocks. In fact,
     from the parser's perspective there is no real difference. All we
     need to do is remember that this is a parallel block so that the
     code generator can do the right thing. */

  | K_fork ':' IDENTIFIER
      { PBlock*tmp = pform_push_block_scope($3, PBlock::BL_PAR);
	FILE_NAME(tmp, @1);
	current_block_stack.push(tmp);
      }
    block_item_decls_opt
    statement_list K_join
      { pform_pop_scope();
        assert(! current_block_stack.empty());
	PBlock*tmp = current_block_stack.top();
	current_block_stack.pop();
	tmp->set_statement(*$6);
	delete[]$3;
	delete $6;
	$$ = tmp;
      }
  | K_fork K_join
      { PBlock*tmp = new PBlock(PBlock::BL_PAR);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_fork ':' IDENTIFIER K_join
      { PBlock*tmp = new PBlock(PBlock::BL_PAR);
	FILE_NAME(tmp, @1);
	delete[]$3;
	$$ = tmp;
      }

	| K_disable hierarchy_identifier ';'
		{ PDisable*tmp = new PDisable(*$2);
		  FILE_NAME(tmp, @1);
		  delete $2;
		  $$ = tmp;
		}
	| K_TRIGGER hierarchy_identifier ';'
		{ PTrigger*tmp = new PTrigger(*$2);
		  FILE_NAME(tmp, @1);
		  delete $2;
		  $$ = tmp;
		}
	| K_forever statement
		{ PForever*tmp = new PForever($2);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_fork statement_list K_join
		{ PBlock*tmp = new PBlock(PBlock::BL_PAR);
		  FILE_NAME(tmp, @1);
		  tmp->set_statement(*$2);
		  delete $2;
		  $$ = tmp;
		}
	| K_repeat '(' expression ')' statement
		{ PRepeat*tmp = new PRepeat($3, $5);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_case '(' expression ')' case_items K_endcase
		{ PCase*tmp = new PCase(NetCase::EQ, $3, $5);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_casex '(' expression ')' case_items K_endcase
		{ PCase*tmp = new PCase(NetCase::EQX, $3, $5);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_casez '(' expression ')' case_items K_endcase
		{ PCase*tmp = new PCase(NetCase::EQZ, $3, $5);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_case '(' expression ')' error K_endcase
		{ yyerrok; }
	| K_casex '(' expression ')' error K_endcase
		{ yyerrok; }
	| K_casez '(' expression ')' error K_endcase
		{ yyerrok; }
	| K_if '(' expression ')' statement_or_null %prec less_than_K_else
		{ PCondit*tmp = new PCondit($3, $5, 0);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_if '(' expression ')' statement_or_null K_else statement_or_null
		{ PCondit*tmp = new PCondit($3, $5, $7);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_if '(' error ')' statement_or_null %prec less_than_K_else
		{ yyerror(@1, "error: Malformed conditional expression.");
		  $$ = $5;
		}
	| K_if '(' error ')' statement_or_null K_else statement_or_null
		{ yyerror(@1, "error: Malformed conditional expression.");
		  $$ = $5;
		}
	| K_for '(' lpvalue '=' expression ';' expression ';'
	  lpvalue '=' expression ')' statement
		{ PForStatement*tmp = new PForStatement($3, $5, $7, $9, $11, $13);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_for '(' lpvalue '=' expression ';' expression ';'
	  error ')' statement
		{ $$ = 0;
		  yyerror(@1, "error: Error in for loop step assignment.");
		}
	| K_for '(' lpvalue '=' expression ';' error ';'
	  lpvalue '=' expression ')' statement
		{ $$ = 0;
		  yyerror(@1, "error: Error in for loop condition expression.");
		}
	| K_for '(' error ')' statement
		{ $$ = 0;
		  yyerror(@1, "error: Incomprehensible for loop.");
		}
	| K_while '(' expression ')' statement
		{ PWhile*tmp = new PWhile($3, $5);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_while '(' error ')' statement
		{ $$ = 0;
		  yyerror(@1, "error: Error in while loop condition.");
		}
	| delay1 statement_or_null
		{ PExpr*del = (*$1)[0];
		  assert($1->count() == 1);
		  PDelayStatement*tmp = new PDelayStatement(del, $2);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
  | event_control attribute_list_opt statement_or_null
      { PEventStatement*tmp = $1;
	if (tmp == 0) {
	      yyerror(@1, "error: Invalid event control.");
	      $$ = 0;
	} else {
	      if ($3) pform_bind_attributes($3->attributes,$2);
	      tmp->set_statement($3);
	      $$ = tmp;
	}
      }
  | '@' '*' attribute_list_opt statement_or_null
      { PEventStatement*tmp = new PEventStatement;
	FILE_NAME(tmp, @1);
	if ($4) pform_bind_attributes($4->attributes,$3);
	tmp->set_statement($4);
	$$ = tmp;
      }
  | '@' '(' '*' ')' attribute_list_opt statement_or_null
      { PEventStatement*tmp = new PEventStatement;
	FILE_NAME(tmp, @1);
	if ($6) pform_bind_attributes($6->attributes,$5);
	tmp->set_statement($6);
	$$ = tmp;
      }
	| lpvalue '=' expression ';'
		{ PAssign*tmp = new PAssign($1,$3);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| error '=' expression ';'
                { yyerror(@2, "Syntax in assignment statement l-value.");
		  yyerrok;
		  $$ = new PNoop;
		}
	| lpvalue K_LE expression ';'
		{ PAssignNB*tmp = new PAssignNB($1,$3);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| error K_LE expression ';'
                { yyerror(@2, "Syntax in assignment statement l-value.");
		  yyerrok;
		  $$ = new PNoop;
		}
	| lpvalue '=' delay1 expression ';'
		{ assert($3->count() == 1);
		  PExpr*del = (*$3)[0];
		  PAssign*tmp = new PAssign($1,del,$4);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| lpvalue K_LE delay1 expression ';'
		{ assert($3->count() == 1);
		  PExpr*del = (*$3)[0];
		  PAssignNB*tmp = new PAssignNB($1,del,$4);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| lpvalue '=' event_control expression ';'
		{ PAssign*tmp = new PAssign($1,0,$3,$4);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| lpvalue '=' K_repeat '(' expression ')' event_control expression ';'
		{ PAssign*tmp = new PAssign($1,$5,$7,$8);
		  FILE_NAME(tmp,@1);
		  tmp->set_lineno(@1.first_line);
		  $$ = tmp;
		}
	| lpvalue K_LE event_control expression ';'
		{ PAssignNB*tmp = new PAssignNB($1,0,$3,$4);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| lpvalue K_LE K_repeat '(' expression ')' event_control expression ';'
		{ PAssignNB*tmp = new PAssignNB($1,$5,$7,$8);
		  FILE_NAME(tmp, @1);
		  $$ = tmp;
		}
	| K_wait '(' expression ')' statement_or_null
		{ PEventStatement*tmp;
		  PEEvent*etmp = new PEEvent(PEEvent::POSITIVE, $3);
		  tmp = new PEventStatement(etmp);
		  FILE_NAME(tmp,@1);
		  tmp->set_statement($5);
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER '(' expression_list_with_nuls ')' ';'
                { PCallTask*tmp = new PCallTask(lex_strings.make($1), *$3);
		  FILE_NAME(tmp,@1);
		  delete[]$1;
		  delete $3;
		  $$ = tmp;
		}
	| SYSTEM_IDENTIFIER ';'
		{ svector<PExpr*>pt (0);
		  PCallTask*tmp = new PCallTask(lex_strings.make($1), pt);
		  FILE_NAME(tmp,@1);
		  delete[]$1;
		  $$ = tmp;
		}
	| hierarchy_identifier '(' expression_list_proper ')' ';'
		{ PCallTask*tmp = new PCallTask(*$1, *$3);
		  FILE_NAME(tmp, @1);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}

  /* NOTE: The standard doesn't really support an empty argument list
     between parentheses, but it seems natural, and people commonly
     want it. So accept it explicitly. */

	| hierarchy_identifier '(' ')' ';'
		{ svector<PExpr*>pt (0);
		  PCallTask*tmp = new PCallTask(*$1, pt);
		  FILE_NAME(tmp, @1);
		  delete $1;
		  $$ = tmp;
		}
	| hierarchy_identifier ';'
		{ svector<PExpr*>pt (0);
		  PCallTask*tmp = new PCallTask(*$1, pt);
		  FILE_NAME(tmp, @1);
		  delete $1;
		  $$ = tmp;
		}
	| error ';'
		{ yyerror(@2, "error: malformed statement");
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

statement_or_null
  : statement
      { $$ = $1; }
  | ';'
      { $$ = 0; }
  ;

analog_statement
  : branch_probe_expression K_CONTRIBUTE expression ';'
      { $$ = pform_contribution_statement(@2, $1, $3); }
  ;

  /* Task items are, other than the statement, task port items and
     other block items. */
task_item
        : block_item_decl  { $$ = new svector<PWire*>(0); }
        | task_port_item   { $$ = $1; }
        ;

reg_opt
	: K_reg
	|
	;

task_port_item

	: K_input reg_opt signed_opt range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT,
						IVL_VT_NO_TYPE, $3,
						$4, $5,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output reg_opt signed_opt range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT,
						IVL_VT_LOGIC, $3,
						$4, $5,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout reg_opt signed_opt range_opt list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT,
						IVL_VT_LOGIC, $3,
						$4, $5,
						@1.text, @1.first_line);
		  $$ = tmp;
		}

  /* When the port is an integer, infer a signed vector of the integer
     shape. Generate a range ([31:0]) to make it work. */

	| K_input K_integer list_of_identifiers ';'
                { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum(integer_width-1,
						integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT,
						IVL_VT_LOGIC, true,
						range_stub, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output K_integer list_of_identifiers ';'
		{ svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum(integer_width-1,
						integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT,
						IVL_VT_LOGIC, true,
						range_stub, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout K_integer list_of_identifiers ';'
		{ svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum(integer_width-1,
						integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT,
						IVL_VT_LOGIC, true,
						range_stub, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}

  /* Ports can be time with a width of [63:0] (unsigned). */

	| K_input K_time list_of_identifiers ';'
                { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum((uint64_t)63, integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT,
						IVL_VT_LOGIC, false,
						range_stub, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output K_time list_of_identifiers ';'
		{ svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum((uint64_t)63, integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT,
						IVL_VT_LOGIC, false,
						range_stub, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout K_time list_of_identifiers ';'
		{ svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum((uint64_t)63, integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT,
						IVL_VT_LOGIC, false,
						range_stub, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}

  /* Ports can be real or realtime. */

	| K_input real_or_realtime list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT,
						IVL_VT_REAL, false,
						0, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output real_or_realtime list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT,
						IVL_VT_REAL, true,
						0, $3,
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout real_or_realtime list_of_identifiers ';'
		{ svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT,
						IVL_VT_REAL, true,
						0, $3,
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

task_port_decl

	: K_input reg_opt signed_opt range_opt IDENTIFIER
		{ port_declaration_context.port_type = NetNet::PINPUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = $3;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range($4);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT,
						IVL_VT_LOGIC, $3,
						$4, list_from_identifier($5),
						@1.text, @1.first_line);
		  $$ = tmp;
		}

	| K_output reg_opt signed_opt range_opt IDENTIFIER
		{ port_declaration_context.port_type = NetNet::POUTPUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = $3;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range($4);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT,
						IVL_VT_LOGIC, $3,
						$4, list_from_identifier($5),
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout reg_opt signed_opt range_opt IDENTIFIER
		{ port_declaration_context.port_type = NetNet::PINOUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = $3;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range($4);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT,
						IVL_VT_LOGIC, $3,
						$4, list_from_identifier($5),
						@1.text, @1.first_line);
		  $$ = tmp;
		}

  /* Ports can be integer with a width of [31:0]. */

	| K_input K_integer IDENTIFIER
                { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum(integer_width-1,
						integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  port_declaration_context.port_type = NetNet::PINPUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = true;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range(range_stub);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT,
						IVL_VT_LOGIC, true,
						range_stub,
						list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output K_integer IDENTIFIER
                { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum(integer_width-1,
						integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  port_declaration_context.port_type = NetNet::POUTPUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = true;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range(range_stub);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT,
						IVL_VT_LOGIC, true,
						range_stub,
						list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout K_integer IDENTIFIER
                { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum(integer_width-1,
						integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  port_declaration_context.port_type = NetNet::PINOUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = true;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range(range_stub);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT,
						IVL_VT_LOGIC, true,
						range_stub,
						list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}

  /* Ports can be time with a width of [63:0] (unsigned). */

	| K_input K_time IDENTIFIER
                { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum((uint64_t)63, integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  port_declaration_context.port_type = NetNet::PINPUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = false;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range(range_stub);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT,
						IVL_VT_LOGIC, false,
						range_stub,
						list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output K_time IDENTIFIER
                { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum((uint64_t)63, integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  port_declaration_context.port_type = NetNet::POUTPUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = false;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range(range_stub);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT,
						IVL_VT_LOGIC, false,
						range_stub,
						list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout K_time IDENTIFIER
                { svector<PExpr*>*range_stub = new svector<PExpr*>(2);
		  PExpr*re;
		  re = new PENumber(new verinum((uint64_t)63, integer_width));
		  (*range_stub)[0] = re;
		  re = new PENumber(new verinum((uint64_t)0, integer_width));
		  (*range_stub)[1] = re;
		  port_declaration_context.port_type = NetNet::PINOUT;
		  port_declaration_context.var_type = IVL_VT_LOGIC;
		  port_declaration_context.sign_flag = false;
		  delete port_declaration_context.range;
		  port_declaration_context.range = copy_range(range_stub);
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT,
						IVL_VT_LOGIC, false,
						range_stub,
						list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}

  /* Ports can be real or realtime. */

	| K_input real_or_realtime IDENTIFIER
		{ port_declaration_context.port_type = NetNet::PINPUT;
		  port_declaration_context.var_type = IVL_VT_REAL;
		  port_declaration_context.sign_flag = false;
		  delete port_declaration_context.range;
		  port_declaration_context.range = 0;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINPUT,
						IVL_VT_REAL, false,
						0, list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_output real_or_realtime IDENTIFIER
		{ port_declaration_context.port_type = NetNet::POUTPUT;
		  port_declaration_context.var_type = IVL_VT_REAL;
		  port_declaration_context.sign_flag = false;
		  delete port_declaration_context.range;
		  port_declaration_context.range = 0;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::POUTPUT,
						IVL_VT_REAL, false,
						0, list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}
	| K_inout real_or_realtime IDENTIFIER
		{ port_declaration_context.port_type = NetNet::PINOUT;
		  port_declaration_context.var_type = IVL_VT_REAL;
		  port_declaration_context.sign_flag = false;
		  delete port_declaration_context.range;
		  port_declaration_context.range = 0;
		  svector<PWire*>*tmp
			= pform_make_task_ports(NetNet::PINOUT,
						IVL_VT_REAL, false,
						0, list_from_identifier($3),
						@1.text, @1.first_line);
		  $$ = tmp;
		}
        ;

task_port_decl_list
	: task_port_decl_list ',' task_port_decl
		{ svector<PWire*>*tmp = new svector<PWire*>(*$1, *$3);
		  delete $1;
		  delete $3;
		  $$ = tmp;
		}
	| task_port_decl
		{ $$ = $1; }
	| task_port_decl_list ',' IDENTIFIER
		{ svector<PWire*>*new_decl
			= pform_make_task_ports(
				port_declaration_context.port_type,
				port_declaration_context.var_type,
				port_declaration_context.sign_flag,
				copy_range(port_declaration_context.range),
				list_from_identifier($3),
				@3.text, @3.first_line);
		  svector<PWire*>*tmp = new svector<PWire*>(*$1, *new_decl);
		  delete $1;
		  delete new_decl;
		  $$ = tmp;
		}
	| task_port_decl_list ','
		{
		  yyerror(@2, "error: NULL port declarations are not "
		              "allowed.");
		}
	| task_port_decl_list ';'
		{
		  yyerror(@2, "error: ';' is an invalid port declaration "
		              "separator.");
		}
        ;
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
		  delete[]$1;
		  $$ = tmp;
		}
	| udp_comb_entry_list udp_comb_entry
		{ list<string>*tmp = $1;
		  tmp->push_back($2);
		  delete[]$2;
		  $$ = tmp;
		}
	;

udp_sequ_entry_list
	: udp_sequ_entry
		{ list<string>*tmp = new list<string>;
		  tmp->push_back($1);
		  delete[]$1;
		  $$ = tmp;
		}
	| udp_sequ_entry_list udp_sequ_entry
		{ list<string>*tmp = $1;
		  tmp->push_back($2);
		  delete[]$2;
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
		  PEIdent*itmp = new PEIdent(lex_strings.make($2));
		  PAssign*atmp = new PAssign(itmp, etmp);
		  FILE_NAME(atmp, @2);
		  delete[]$2;
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

  /* Port declarations create wires for the inputs and the output. The
     makes for these ports are scoped within the UDP, so there is no
     hierarchy involved. */
udp_port_decl
  : K_input list_of_identifiers ';'
      { $$ = pform_make_udp_input_ports($2); }
  | K_output IDENTIFIER ';'
      { perm_string pname = lex_strings.make($2);
	PWire*pp = new PWire(pname, NetNet::IMPLICIT, NetNet::POUTPUT, IVL_VT_LOGIC);
	svector<PWire*>*tmp = new svector<PWire*>(1);
	(*tmp)[0] = pp;
	$$ = tmp;
	delete[]$2;
      }
  | K_reg IDENTIFIER ';'
      { perm_string pname = lex_strings.make($2);
	PWire*pp = new PWire(pname, NetNet::REG, NetNet::PIMPLICIT, IVL_VT_LOGIC);
	svector<PWire*>*tmp = new svector<PWire*>(1);
	(*tmp)[0] = pp;
	$$ = tmp;
	delete[]$2;
      }
  | K_reg K_output IDENTIFIER ';'
      { perm_string pname = lex_strings.make($3);
	PWire*pp = new PWire(pname, NetNet::REG, NetNet::POUTPUT, IVL_VT_LOGIC);
	svector<PWire*>*tmp = new svector<PWire*>(1);
	(*tmp)[0] = pp;
	$$ = tmp;
	delete[]$3;
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
      { list<perm_string>*tmp = new list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  | udp_port_list ',' IDENTIFIER
      { list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	delete[]$3;
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
