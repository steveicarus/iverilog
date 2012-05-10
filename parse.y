
%{
/*
 * Copyright (c) 1998-2012 Stephen Williams (steve@icarus.com)
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

bool have_timeunit_decl = false;
bool have_timeprec_decl = false;

static list<pform_range_t>* param_active_range = 0;
static bool param_active_signed = false;
static ivl_variable_type_t param_active_type = IVL_VT_LOGIC;

/* Port declaration lists use this structure for context. */
static struct {
      NetNet::Type port_net_type;
      NetNet::PortType port_type;
      ivl_variable_type_t var_type;
      bool sign_flag;
      data_type_t* data_type;
      list<pform_range_t>* range;
} port_declaration_context = {NetNet::NONE, NetNet::NOT_A_PORT,
                              IVL_VT_NO_TYPE, false, 0, 0};

/* The task and function rules need to briefly hold the pointer to the
   task/function that is currently in progress. */
static PTask* current_task = 0;
static PFunction* current_function = 0;
static stack<PBlock*> current_block_stack;

/* This is used to keep track of the extra arguments after the notifier
 * in the $setuphold and $recrem timing checks. This allows us to print
 * a warning message that the delayed signals will not be created. We
 * need to do this since not driving these signals creates real
 * simulation issues. */
static unsigned args_after_notifier;

/* The rules sometimes push attributes into a global context where
   sub-rules may grab them. This makes parser rules a little easier to
   write in some cases. */
static list<named_pexpr_t>*attributes_in_context = 0;

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
static const struct str_pair_t pull_strength = { IVL_DR_PULL,  IVL_DR_PULL };
static const struct str_pair_t str_strength = { IVL_DR_STRONG, IVL_DR_STRONG };

static list<pair<perm_string,PExpr*> >* make_port_list(char*id, PExpr*expr)
{
      list<pair<perm_string,PExpr*> >*tmp = new list<pair<perm_string,PExpr*> >;
      tmp->push_back(make_pair(lex_strings.make(id), expr));
      delete[]id;
      return tmp;
}
static list<pair<perm_string,PExpr*> >* make_port_list(list<pair<perm_string, PExpr*> >*tmp,
                                                       char*id, PExpr*expr)
{
      tmp->push_back(make_pair(lex_strings.make(id), expr));
      delete[]id;
      return tmp;
}

list<pform_range_t>* make_range_from_width(uint64_t wid)
{
      pform_range_t range;
      range.first  = new PENumber(new verinum(wid-1, integer_width));
      range.second = new PENumber(new verinum((uint64_t)0, integer_width));

      list<pform_range_t>*rlist = new list<pform_range_t>;
      rlist->push_back(range);
      return rlist;
}

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

list<pform_range_t>* copy_range(list<pform_range_t>* orig)
{
      list<pform_range_t>*copy = 0;

      if (orig)
	    copy = new list<pform_range_t> (*orig);

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
      vector<PExpr*> parms(1);
      parms[0] = arg;
      PECallFunction*tmp = new PECallFunction(tn, parms);
      return tmp;
}

static PECallFunction*make_call_function(perm_string tn, PExpr*arg1, PExpr*arg2)
{
      vector<PExpr*> parms(2);
      parms[0] = arg1;
      parms[1] = arg2;
      PECallFunction*tmp = new PECallFunction(tn, parms);
      return tmp;
}

static list<named_pexpr_t>* make_named_numbers(perm_string name, long first, long last, PExpr*val =0)
{
      list<named_pexpr_t>*lst = new list<named_pexpr_t>;
      named_pexpr_t tmp;
	// We are counting up.
      if (first <= last) {
	    for (long idx = first ; idx <= last ; idx += 1) {
		  ostringstream buf;
		  buf << name.str() << idx << ends;
		  tmp.name = lex_strings.make(buf.str());
		  tmp.parm = val;
		  val = 0;
		  lst->push_back(tmp);
	    }
	// We are counting down.
      } else {
	    for (long idx = first ; idx >= last ; idx -= 1) {
		  ostringstream buf;
		  buf << name.str() << idx << ends;
		  tmp.name = lex_strings.make(buf.str());
		  tmp.parm = val;
		  val = 0;
		  lst->push_back(tmp);
	    }
      }
      return lst;
}

static list<named_pexpr_t>* make_named_number(perm_string name, PExpr*val =0)
{
      list<named_pexpr_t>*lst = new list<named_pexpr_t>;
      named_pexpr_t tmp;
      tmp.name = name;
      tmp.parm = val;
      lst->push_back(tmp);
      return lst;
}

static long check_enum_seq_value(const YYLTYPE&loc, verinum *arg, bool zero_ok)
{
      long value = 1;
	// We can never have an undefined value in an enumeration name
	// declaration sequence.
      if (! arg->is_defined()) {
	    yyerror(loc, "error: undefined value used in enum name sequence.");
	// We can never have a negative value in an enumeration name
	// declaration sequence.
      } else if (arg->is_negative()) {
	    yyerror(loc, "error: negative value used in enum name sequence.");
      } else {
	    value = arg->as_ulong();
	      // We cannot have a zero enumeration name declaration count.
	    if (! zero_ok && (value == 0)) {
		  yyerror(loc, "error: zero count used in enum name sequence.");
		  value = 1;
	    }
      }
      return value;
}

static void current_task_set_statement(const YYLTYPE&loc, vector<Statement*>*s)
{
      if (s == 0) {
	      /* if the statement list is null, then the parser
		 detected the case that there are no statements in the
		 task. If this is System Verilog, handle it as an
		 an empty block. */
	    if (!gn_system_verilog()) {
		  yyerror(loc, "error: Support for empty tasks requires SystemVerilog.");
	    }
	    PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	    FILE_NAME(tmp, loc);
	    current_task->set_statement(tmp);
	    return;
      }

	/* The parser assures that there is a non-empty vector. */
      assert(s && !s->empty());

	/* A vector of 1 is handled as a simple statement. */
      if (s->size() == 1) {
	    current_task->set_statement((*s)[0]);
	    return;
      }

      if (!gn_system_verilog()) {
	    yyerror(loc, "error: Task body with multiple statements requires SystemVerilog.");
      }

      PBlock*tmp = new PBlock(PBlock::BL_SEQ);
      FILE_NAME(tmp, loc);
      tmp->set_statement(*s);
      current_task->set_statement(tmp);
}

static void current_function_set_statement(const YYLTYPE&loc, vector<Statement*>*s)
{
      if (s == 0) {
	      /* if the statement list is null, then the parser
		 detected the case that there are no statements in the
		 task. If this is System Verilog, handle it as an
		 an empty block. */
	    if (!gn_system_verilog()) {
		  yyerror(loc, "error: Support for empty functions requires SystemVerilog.");
	    }
	    PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	    FILE_NAME(tmp, loc);
	    current_function->set_statement(tmp);
	    return;
      }

	/* The parser assures that there is a non-empty vector. */
      assert(s && !s->empty());

	/* A vector of 1 is handled as a simple statement. */
      if (s->size() == 1) {
	    current_function->set_statement((*s)[0]);
	    return;
      }

      if (!gn_system_verilog()) {
	    yyerror(loc, "error: Function body with multiple statements requires SystemVerilog.");
      }

      PBlock*tmp = new PBlock(PBlock::BL_SEQ);
      FILE_NAME(tmp, loc);
      tmp->set_statement(*s);
      current_function->set_statement(tmp);
}

%}

%union {
      bool flag;

      char letter;
      int  int_val;

	/* text items are C strings allocated by the lexor using
	   strdup. They can be put into lists with the texts type. */
      char*text;
      list<perm_string>*perm_strings;

      list<pair<perm_string,PExpr*> >*port_list;

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

      named_number_t* named_number;
      list<named_number_t>* named_numbers;

      named_pexpr_t*named_pexpr;
      list<named_pexpr_t>*named_pexprs;
      struct parmvalue_t*parmvalue;
      list<pform_range_t>*ranges;

      PExpr*expr;
      list<PExpr*>*exprs;

      svector<PEEvent*>*event_expr;

      NetNet::Type nettype;
      PGBuiltin::Type gatetype;
      NetNet::PortType porttype;
      ivl_variable_type_t vartype;

      PWire*wire;
      svector<PWire*>*wires;

      PEventStatement*event_statement;
      Statement*statement;
      vector<Statement*>*statement_list;

      PTaskFuncArg function_type;

      net_decl_assign_t*net_decl_assign;
      enum_type_t*enum_type;

      decl_assignment_t*decl_assignment;
      list<decl_assignment_t*>*decl_assignments;

      struct_member_t*struct_member;
      list<struct_member_t*>*struct_members;
      struct_type_t*struct_type;

      data_type_t*data_type;
      class_type_t*class_type;
      real_type_t::type_t real_type;

      verinum* number;

      verireal* realtime;

      PSpecPath* specpath;
      list<index_component_t> *dimensions;
};

%token <text>   IDENTIFIER SYSTEM_IDENTIFIER STRING TIME_LITERAL
%token <data_type> TYPE_IDENTIFIER
%token <discipline> DISCIPLINE_IDENTIFIER
%token <text>   PATHPULSE_IDENTIFIER
%token <number> BASED_NUMBER DEC_NUMBER UNBASED_NUMBER
%token <realtime> REALTIME
%token K_PLUS_EQ K_MINUS_EQ K_INCR K_DECR
%token K_LE K_GE K_EG K_EQ K_NE K_CEQ K_CNE K_LP K_LS K_RS K_RSS K_SG
 /* K_CONTRIBUTE is <+, the contribution assign. */
%token K_CONTRIBUTE
%token K_PO_POS K_PO_NEG K_POW
%token K_PSTAR K_STARP K_DOTSTAR
%token K_LOR K_LAND K_NAND K_NOR K_NXOR K_TRIGGER
%token K_edge_descriptor

 /* The base tokens from 1364-1995. */
%token K_always K_and K_assign K_begin K_buf K_bufif0 K_bufif1 K_case
%token K_casex K_casez K_cmos K_deassign K_default K_defparam K_disable
%token K_edge K_else K_end K_endcase K_endfunction K_endmodule
%token K_endprimitive K_endspecify K_endtable K_endtask K_event K_for
%token K_force K_forever K_fork K_function K_highz0 K_highz1 K_if
%token K_ifnone K_initial K_inout K_input K_integer K_join K_large
%token K_macromodule K_medium K_module K_nand K_negedge K_nmos K_nor
%token K_not K_notif0 K_notif1 K_or K_output K_parameter K_pmos K_posedge
%token K_primitive K_pull0 K_pull1 K_pulldown K_pullup K_rcmos K_real
%token K_realtime K_reg K_release K_repeat K_rnmos K_rpmos K_rtran
%token K_rtranif0 K_rtranif1 K_scalared K_small K_specify K_specparam
%token K_strong0 K_strong1 K_supply0 K_supply1 K_table K_task K_time
%token K_tran K_tranif0 K_tranif1 K_tri K_tri0 K_tri1 K_triand K_trior
%token K_trireg K_vectored K_wait K_wand K_weak0 K_weak1 K_while K_wire
%token K_wor K_xnor K_xor

%token K_Shold K_Snochange K_Speriod K_Srecovery K_Ssetup K_Ssetuphold
%token K_Sskew K_Swidth

 /* Icarus specific tokens. */
%token KK_attribute K_bool K_logic

 /* The new tokens from 1364-2001. */
%token K_automatic K_endgenerate K_generate K_genvar K_localparam
%token K_noshowcancelled K_pulsestyle_onevent K_pulsestyle_ondetect
%token K_showcancelled K_signed K_unsigned

%token K_Sfullskew K_Srecrem K_Sremoval K_Stimeskew

 /* The 1364-2001 configuration tokens. */
%token K_cell K_config K_design K_endconfig K_incdir K_include K_instance
%token K_liblist K_library K_use

 /* The new tokens from 1364-2005. */
%token K_wone K_uwire

 /* The new tokens from 1800-2005. */
%token K_alias K_always_comb K_always_ff K_always_latch K_assert
%token K_assume K_before K_bind K_bins K_binsof K_bit K_break K_byte
%token K_chandle K_class K_clocking K_const K_constraint K_context
%token K_continue K_cover K_covergroup K_coverpoint K_cross K_dist K_do
%token K_endclass K_endclocking K_endgroup K_endinterface K_endpackage
%token K_endprogram K_endproperty K_endsequence K_enum K_expect K_export
%token K_extends K_extern K_final K_first_match K_foreach K_forkjoin
%token K_iff K_ignore_bins K_illegal_bins K_import K_inside K_int
 /* Icarus already has defined "logic" above! */
%token K_interface K_intersect K_join_any K_join_none K_local
%token K_longint K_matches K_modport K_new K_null K_package K_packed
%token K_priority K_program K_property K_protected K_pure K_rand K_randc
%token K_randcase K_randsequence K_ref K_return K_sequence K_shortint
%token K_shortreal K_solve K_static K_string K_struct K_super
%token K_tagged K_this K_throughout K_timeprecision K_timeunit K_type
%token K_typedef K_union K_unique K_var K_virtual K_void K_wait_order
%token K_wildcard K_with K_within
 /* Fake tokens that are passed once we have an initial token. */
%token K_timeprecision_check K_timeunit_check

 /* The new tokens from 1800-2009. */
%token K_accept_on K_checker K_endchecker K_eventually K_global K_implies
%token K_let K_nexttime K_reject_on K_restrict K_s_always K_s_eventually
%token K_s_nexttime K_s_until K_s_until_with K_strong K_sync_accept_on
%token K_sync_reject_on K_unique0 K_until K_until_with K_untyped K_weak

 /* The new tokens for Verilog-AMS 2.3. */
%token K_above K_abs K_absdelay K_abstol K_access K_acos K_acosh
 /* 1800-2005 has defined "assert" above! */
%token K_ac_stim K_aliasparam K_analog K_analysis K_asin K_asinh
%token K_atan K_atan2 K_atanh K_branch K_ceil K_connect K_connectmodule
%token K_connectrules K_continuous K_cos K_cosh K_ddt K_ddt_nature K_ddx
%token K_discipline K_discrete K_domain K_driver_update K_endconnectrules
%token K_enddiscipline K_endnature K_endparamset K_exclude K_exp
%token K_final_step K_flicker_noise K_floor K_flow K_from K_ground
%token K_hypot K_idt K_idtmod K_idt_nature K_inf K_initial_step
%token K_laplace_nd K_laplace_np K_laplace_zd K_laplace_zp
%token K_last_crossing K_limexp K_ln K_log K_max K_merged K_min K_nature
%token K_net_resolution K_noise_table K_paramset K_potential K_pow
 /* 1800-2005 has defined "string" above! */
%token K_resolveto K_sin K_sinh K_slew K_split K_sqrt K_tan K_tanh
%token K_timer K_transition K_units K_white_noise K_wreal
%token K_zi_nd K_zi_np K_zi_zd K_zi_zp

%type <flag>    from_exclude
%type <number>  number pos_neg_number
%type <flag>    signing unsigned_signed_opt signed_unsigned_opt
%type <flag>    K_automatic_opt K_packed_opt K_reg_opt K_static_opt K_virtual_opt
%type <flag>    udp_reg_opt edge_operator
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

%type <text> register_variable net_variable endname_opt class_declaration_endname_opt
%type <perm_strings> register_variable_list net_variable_list
%type <perm_strings> list_of_identifiers loop_variables
%type <port_list> list_of_port_identifiers

%type <net_decl_assign> net_decl_assign net_decl_assigns

%type <mport> port port_opt port_reference port_reference_list
%type <mport> port_declaration
%type <mports> list_of_ports module_port_list_opt list_of_port_declarations module_attribute_foreign
%type <value_range> parameter_value_range parameter_value_ranges
%type <value_range> parameter_value_ranges_opt
%type <expr> tf_port_item_expr_opt value_range_expression

%type <named_pexprs> enum_name_list enum_name
%type <enum_type> enum_data_type

%type <wires> task_item task_item_list task_item_list_opt
%type <wires> tf_port_declaration tf_port_item tf_port_list tf_port_list_opt
%type <wires> function_item function_item_list function_item_list_opt

%type <named_pexpr> port_name parameter_value_byname
%type <named_pexprs> port_name_list parameter_value_byname_list

%type <named_pexpr> attribute
%type <named_pexprs> attribute_list attribute_instance_list attribute_list_opt

%type <citem>  case_item
%type <citems> case_items

%type <gate>  gate_instance
%type <gates> gate_instance_list

%type <pform_name> hierarchy_identifier
%type <expr>  assignment_pattern expression expr_primary expr_mintypmax
%type <expr>  class_new dynamic_array_new inc_or_dec_expression inside_expression lpvalue
%type <expr>  branch_probe_expression streaming_concatenation
%type <expr>  delay_value delay_value_simple
%type <exprs> delay1 delay3 delay3_opt delay_value_list
%type <exprs> expression_list_with_nuls expression_list_proper
%type <exprs> cont_assign cont_assign_list

%type <decl_assignment> variable_decl_assignment
%type <decl_assignments> list_of_variable_decl_assignments

%type <data_type>  data_type data_type_or_implicit
%type <data_type>  class_declaration_extends_opt
%type <class_type> class_identifier
%type <struct_member>  struct_union_member
%type <struct_members> struct_union_member_list
%type <struct_type>    struct_data_type

%type <ranges> range range_opt variable_dimension
%type <ranges> dimensions_opt dimensions

%type <nettype>  net_type var_type net_type_opt
%type <gatetype> gatetype switchtype
%type <porttype> port_direction port_direction_opt
%type <vartype> primitive_type primitive_type_opt bit_logic
%type <vartype> integer_vector_type
%type <parmvalue> parameter_value_opt

%type <function_type> function_range_or_type_opt
%type <event_expr> event_expression_list
%type <event_expr> event_expression
%type <event_statement> event_control
%type <statement> statement statement_item statement_or_null
%type <statement> compressed_statement
%type <statement> loop_statement for_step jump_statement
%type <statement_list> statement_or_null_list statement_or_null_list_opt

%type <statement> analog_statement

%type <letter> spec_polarity
%type <perm_strings>  specify_path_identifiers

%type <specpath> specify_simple_path specify_simple_path_decl
%type <specpath> specify_edge_path specify_edge_path_decl

%type <real_type> non_integer_type
%type <int_val> atom2_type
%type <int_val> module_start module_end

%token K_TAND
%right K_PLUS_EQ K_MINUS_EQ K_MUL_EQ K_DIV_EQ K_MOD_EQ K_AND_EQ K_OR_EQ
%right K_XOR_EQ K_LS_EQ K_RS_EQ K_RSS_EQ
%right '?' ':' K_inside
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

assignment_pattern /* IEEE1800-2005: A.6.7.1 */
  : K_LP expression_list_proper '}'
      { PEVoid*tmp = new PEVoid;
	FILE_NAME(tmp, @1);
	yyerror(@1, "sorry: Assignment patterns (array literals) not supported.");
	$$ = tmp;
      }
  | K_LP '}'
      { PEVoid*tmp = new PEVoid;
	FILE_NAME(tmp, @1);
	yyerror(@1, "sorry: Assignment patterns (array literals) not supported.");
	$$ = tmp;
      }
  ;

class_declaration /* IEEE1800-2005: A.1.2 */
  : K_virtual_opt K_class class_identifier class_declaration_extends_opt ';'
      { pform_start_class_declaration(@2, $3);
	if ($4) {
	      yyerror(@4, "sorry: Class extends not supported yet.");
	}
      }
    class_items_opt K_endclass
      { // Process a class.
	pform_end_class_declaration();
	yyerror(@2, "sorry: Class declarations not supported yet.");
      }
    class_declaration_endname_opt
      { // Wrap up the class.
	if ($10 && $3 && $3->name != $10) {
	      yyerror(@10, "error: Class end name doesn't match class name.");
	      delete[]$10;
	}
      }
  ;

class_constraint /* IEEE1800-2005: A.1.8 */
  : constraint_prototype
  | constraint_declaration
  ;

class_identifier
  : IDENTIFIER
      { // Create a synthetic typedef for the class name so that the
	// lexor detects the name as a type.
	perm_string name = lex_strings.make($1);
	class_type_t*tmp = new class_type_t(name);
	pform_set_typedef(name, tmp);
	delete[]$1;
	$$ = tmp;
      }
  | TYPE_IDENTIFIER
      { class_type_t*tmp = dynamic_cast<class_type_t*>($1);
	if (tmp == 0) {
	      yyerror(@1, "Type name is not a predeclared class name.");
	}
	$$ = tmp;
      }
  ;

  /* The endname after a class declaration is a little tricky because
     the class name is detected by the lexor as a TYPE_IDENTIFIER if it
     does indeed match a name. */
class_declaration_endname_opt
  : ':' TYPE_IDENTIFIER
      { class_type_t*tmp = dynamic_cast<class_type_t*> ($2);
	if (tmp == 0) {
	      yyerror(@2, "error: class declaration endname is not a class name\n");
	      $$ = 0;
	} else {
	      $$ = strdupnew(tmp->name.str());
	}
      }
  | ':' IDENTIFIER
      { $$ = $2; }
  |
      { $$ = 0; }
  ;

  /* This rule implements [ extends class_type ] in the
     class_declaration. It is not a rule of its own in the LRM.

     Note that for this to be correct, the identifier after the
     extends keyword must be a class name. Therefore, match
     TYPE_IDENTIFIER instead of IDENTIFIER, and this rule will return
     a data_type. */

class_declaration_extends_opt /* IEEE1800-2005: A.1.2 */
  : K_extends TYPE_IDENTIFIER
      { $$ = $2; }
  |
      { $$ = 0; }
  ;

  /* The class_items_opt and class_items rules together implement the
     rule snippet { class_item } (zero or more class_item) of the
     class_declaration. */
class_items_opt /* IEEE1800-2005: A.1.2 */
  : class_items
  |
  ;

class_items /* IEEE1800-2005: A.1.2 */
  : class_items class_item
  | class_item
  ;

class_item /* IEEE1800-2005: A.1.8 */

    /* IEEE1800 A.1.8: class_constructor_declaration */
  : method_qualifier_opt K_function K_new '(' tf_port_list_opt ')' ';'
    function_item_list_opt
    statement_or_null_list_opt
    K_endfunction endnew_opt
      { yyerror(@3, "sorry: Class constructors not supported yet.");
	yyerrok;
      }

    /* IEEE1800 A.1.8: class_constructor_declaration with a call to
       parent constructor. Note that the implicit_class_handle must
       be K_super ("this.new" makes little sense) but that would
       cause a conflict. */
  | method_qualifier_opt K_function K_new '(' tf_port_list_opt ')' ';'
    function_item_list_opt
    attribute_list_opt implicit_class_handle '.' K_new '(' expression_list_with_nuls ')'
    statement_or_null_list_opt
    K_endfunction endnew_opt
      { yyerror(@3, "sorry: Class constructors not supported yet.");
	yyerrok;
      }

    /* Class properties... */

  | property_qualifier_opt data_type list_of_variable_decl_assignments ';'


    /* Class methods... */

  | method_qualifier_opt task_declaration

  | method_qualifier_opt function_declaration


    /* Class constraints... */

  | class_constraint

    /* Here are some error matching rules to help recover from various
       syntax errors within a class declaration. */

  | property_qualifier_opt data_type error ';'
      { yyerror(@3, "error: Errors in variable names after data type.");
	yyerrok;
      }

  | property_qualifier_opt IDENTIFIER error ';'
      { yyerror(@3, "error: %s doesn't name a type.", $2);
	yyerrok;
      }

  | method_qualifier_opt K_function K_new error K_endfunction endnew_opt
      { yyerror(@1, "error: I give up on this class constructor declaration.");
	yyerrok;
      }

  | error ';'
      { yyerror(@2, "error: invalid class item.");
	yyerrok;
      }

  ;

class_item_qualifier /* IEEE1800-2005 A.1.8 */
  : K_static
  | K_protected
  | K_local
  ;

class_new /* IEEE1800-2005 A.2.4 */
  : K_new '(' ')'
      { yyerror(@1, "sorry: class_new not implemented yet.");
	$$ = 0;
      }
  | K_new '(' expression_list_proper ')'
      { yyerror(@1, "sorry: class_new not implemented yet.");
	$$ = 0;
      }
  ;

constraint_block_item /* IEEE1800-2005 A.1.9 */
  : constraint_expression
  ;

constraint_block_item_list
  : constraint_block_item_list constraint_block_item
  | constraint_block_item
  ;

constraint_block_item_list_opt
  :
  | constraint_block_item_list
  ;

constraint_declaration /* IEEE1800-2005: A.1.9 */
  : K_static_opt K_constraint IDENTIFIER '{' constraint_block_item_list_opt '}'
      { yyerror(@2, "sorry: Constraint declarations not supported."); }

  /* Error handling rules... */

  | K_static_opt K_constraint IDENTIFIER '{' error '}'
      { yyerror(@4, "error: Errors in the constraint block item list."); }
  ;

constraint_expression /* IEEE1800-2005 A.1.9 */
  : expression ';'
  | expression K_dist '{' '}' ';'
  | expression K_TRIGGER constraint_set
  | K_if '(' expression ')' constraint_set %prec less_than_K_else
  | K_if '(' expression ')' constraint_set K_else constraint_set
  | K_foreach '(' IDENTIFIER '[' loop_variables ']' ')' constraint_set
  ;

constraint_expression_list /* */
  : constraint_expression_list constraint_expression
  | constraint_expression
  ;

constraint_prototype /* IEEE1800-2005: A.1.9 */
  : K_static_opt K_constraint IDENTIFIER ';'
      { yyerror(@2, "sorry: Constraint prototypes not supported."); }
  ;

constraint_set /* IEEE1800-2005 A.1.9 */
  : constraint_expression
  | '{' constraint_expression_list '}'
  ;

data_type /* IEEE1800-2005: A.2.2.1 */
  : integer_vector_type unsigned_signed_opt range_opt
      { ivl_variable_type_t use_vtype = $1;
	bool reg_flag = false;
	if (use_vtype == IVL_VT_NO_TYPE) {
	      use_vtype = IVL_VT_LOGIC;
	      reg_flag = true;
	}
	vector_type_t*tmp = new vector_type_t(use_vtype, $2, $3);
	tmp->reg_flag = reg_flag;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | non_integer_type
      { real_type_t*tmp = new real_type_t($1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | struct_data_type
      { $$ = $1; }
  | enum_data_type
      { $$ = $1; }
  | atom2_type signed_unsigned_opt
      { atom2_type_t*tmp = new atom2_type_t($1, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | TYPE_IDENTIFIER range_opt
      { if ($2) $$ = new array_type_t($1, $2);
	else $$ = $1;
      }
  | K_string
      { yyerror(@1, "sorry: String data type not supported.");
	$$ = 0;
      }
  ;

  /* The data_type_or_implicit rule is a little more complex then the
     rule documented in the IEEE format syntax in order to allow for
     signaling the special case that the data_type is completely
     absent. The context may need that information to decide to resort
     to left context. */

data_type_or_implicit /* IEEE1800-2005: A.2.2.1 */
  : data_type
      { $$ = $1; }
  | signing range_opt
      { vector_type_t*tmp = new vector_type_t(IVL_VT_LOGIC, $1, $2);
	tmp->implicit_flag = true;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | range
      { vector_type_t*tmp = new vector_type_t(IVL_VT_LOGIC, false, $1);
	tmp->implicit_flag = true;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  |
      { $$ = 0; }
  ;

   /* This implements the [ : INDENTIFIER ] part of the constructure
      rule documented in IEEE1800-2005: A.1.8 */
endnew_opt : ':' K_new | ;

  /* The dynamic_array_new rule is kinda like an expression, but it is
     treated differently by rules that use this "expression". Watch out! */

dynamic_array_new /* IEEE1800-2005: A.2.4 */
  : K_new '[' expression ']'
      { yyerror(@1, "sorry: Dynamic array new expression not supported.");
	$$ = 0;
      }
  | K_new '[' expression ']' '(' expression ')'
      { yyerror(@1, "sorry: Dynamic array new expression not supported.");
	$$ = 0;
      }
  ;

for_step /* IEEE1800-2005: A.6.8 */
  : lpvalue '=' expression
      { PAssign*tmp = new PAssign($1,$3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | inc_or_dec_expression
      { $$ = pform_compressed_assign_from_inc_dec(@1, $1); }
  | compressed_statement
      { $$ = $1; }
  ;


  /* The function declaration rule matches the function declaration
     header, then pushes the function scope. This causes the
     definitions in the func_body to take on the scope of the function
     instead of the module. */
function_declaration /* IEEE1800-2005: A.2.6 */
  : K_function K_automatic_opt function_range_or_type_opt IDENTIFIER ';'
      { assert(current_function == 0);
	current_function = pform_push_function_scope(@1, $4, $2);
      }
    function_item_list statement_or_null_list_opt
    K_endfunction
      { current_function->set_ports($7);
	current_function->set_return($3);
	current_function_set_statement($8? @8 : @4, $8);
	pform_pop_scope();
	current_function = 0;
      }
    endname_opt
      { // Last step: check any closing name.
	if ($11 && (strcmp($4,$11) != 0)) {
	      yyerror(@11, "error: End name doesn't match function name");
	}
	if ($11 && !gn_system_verilog()) {
	      yyerror(@11, "error: Function end names require System Verilog.");
	}
	delete[]$4;
	if ($11) delete[]$11;
      }

  | K_function K_automatic_opt function_range_or_type_opt IDENTIFIER
      { assert(current_function == 0);
	current_function = pform_push_function_scope(@1, $4, $2);
      }
    '(' tf_port_list_opt ')' ';'
    block_item_decls_opt
    statement_or_null_list_opt
    K_endfunction
      { current_function->set_ports($7);
	current_function->set_return($3);
	current_function_set_statement($11? @11 : @4, $11);
	pform_pop_scope();
	current_function = 0;
	if ($7==0 && !gn_system_verilog()) {
	      yyerror(@4, "error: Empty parenthesis syntax requires SystemVerilog.");
	}
      }
    endname_opt
      { // Last step: check any closing name.
	if ($14 && (strcmp($4,$14) != 0)) {
	      yyerror(@14, "error: End name doesn't match function name");
	}
	if ($14 && !gn_system_verilog()) {
	      yyerror(@14, "error: Function end names require System Verilog.");
	}
	delete[]$4;
	if ($14) delete[]$14;
      }

  /* Detect and recover from some errors. */

  | K_function K_automatic_opt function_range_or_type_opt IDENTIFIER error K_endfunction
      { /* */
	if (current_function) {
	      pform_pop_scope();
	      current_function = 0;
	}
	assert(current_function == 0);
	yyerror(@1, "error: Syntax error defining function.");
	yyerrok;
      }
    endname_opt
      { // Last step: check any closing name.
	if ($8 && (strcmp($4,$8) != 0)) {
	      yyerror(@4, "error: End name doesn't match function name");
	}
	if ($8 && !gn_system_verilog()) {
	      yyerror(@8, "error: Function end names require System Verilog.");
	}
	delete[]$4;
	if ($8) delete[]$8;
      }

  ;

implicit_class_handle /* IEEE1800-2005: A.8.4 */
  : K_this
  | K_super
  ;

  /* SystemVerilog adds support for the increment/decrement
     expressions, which look like a++, --a, etc. These are primaries
     but are in their own rules because they can also be
     statements. Note that the operator can only take l-value
     expressions. */

inc_or_dec_expression /* IEEE1800-2005: A.4.3 */
  : K_INCR lpvalue %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('I', $2);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | lpvalue K_INCR %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('i', $1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_DECR lpvalue %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('D', $2);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | lpvalue K_DECR %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('d', $1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

inside_expression /* IEEE1800-2005 A.8.3 */
  : expression K_inside '{' open_range_list '}'
      { yyerror(@2, "sorry: \"inside\" expressions not supported yet.");
	$$ = 0;
      }
  ;

integer_vector_type /* IEEE1800-2005: A.2.2.1 */
  : K_reg   { $$ = IVL_VT_NO_TYPE; } /* Usually a synonym for logic. */
  | K_bit   { $$ = IVL_VT_BOOL; }
  | K_logic { $$ = IVL_VT_LOGIC; }
  | K_bool  { $$ = IVL_VT_BOOL; } /* Icarus Verilog xtypes extension */
  ;

jump_statement /* IEEE1800-2005: A.6.5 */
  : K_break ';'
      { yyerror(@1, "sorry: break statements not supported.");
	$$ = 0;
      }
  | K_return ';'
      { yyerror(@1, "sorry: return statements not supported.");
	$$ = 0;
      }
  | K_return expression ';'
      { yyerror(@1, "sorry: return statements not supported.");
	$$ = 0;
      }
  ;

  /* Loop statements are kinds of statements. */

loop_statement /* IEEE1800-2005: A.6.8 */
  : K_for '(' lpvalue '=' expression ';' expression ';' for_step ')'
    statement_or_null
      { PForStatement*tmp = new PForStatement($3, $5, $7, $9, $11);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  | K_for '(' data_type IDENTIFIER '=' expression ';' expression ';' for_step ')'
    statement_or_null
      { $$ = 0;
	yyerror(@3, "sorry: for_variable_declaration not supported");
      }

  | K_forever statement_or_null
      { PForever*tmp = new PForever($2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  | K_repeat '(' expression ')' statement_or_null
      { PRepeat*tmp = new PRepeat($3, $5);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  | K_while '(' expression ')' statement_or_null
      { PWhile*tmp = new PWhile($3, $5);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  | K_foreach '(' IDENTIFIER '[' loop_variables ']' ')' statement_or_null
      { yyerror(@1, "sorry: foreach loops not supported");
	delete[]$3;
	delete $5;
	delete $8;
	$$ = 0;
      }

  /* Error forms for loop statements. */

  | K_for '(' lpvalue '=' expression ';' expression ';' error ')'
    statement_or_null
      { $$ = 0;
	yyerror(@1, "error: Error in for loop step assignment.");
      }

  | K_for '(' lpvalue '=' expression ';' error ';' for_step ')'
    statement_or_null
      { $$ = 0;
	yyerror(@1, "error: Error in for loop condition expression.");
      }

  | K_for '(' error ')' statement_or_null
      { $$ = 0;
	yyerror(@1, "error: Incomprehensible for loop.");
      }

  | K_while '(' error ')' statement_or_null
      { $$ = 0;
	yyerror(@1, "error: Error in while loop condition.");
      }

  | K_foreach '(' IDENTIFIER '[' error ']' ')' statement_or_null
      { $$ = 0;
        yyerror(@4, "error: Errors in foreach loop variables list.");
      }
  ;


/* TODO: Replace register_variable_list with list_of_variable_decl_assignments. */
list_of_variable_decl_assignments /* IEEE1800-2005 A.2.3 */
  : variable_decl_assignment
      { list<decl_assignment_t*>*tmp = new list<decl_assignment_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  | list_of_variable_decl_assignments ',' variable_decl_assignment
      { list<decl_assignment_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  ;

variable_decl_assignment /* IEEE1800-2005 A.2.3 */
  : IDENTIFIER dimensions_opt
      { decl_assignment_t*tmp = new decl_assignment_t;
	tmp->name = lex_strings.make($1);
	if ($2) {
	      tmp->index = *$2;
	      delete $2;
	}
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER '=' expression
      { decl_assignment_t*tmp = new decl_assignment_t;
	tmp->name = lex_strings.make($1);
	tmp->expr .reset($3);
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER '=' K_new '(' ')'
      { decl_assignment_t*tmp = new decl_assignment_t;
	tmp->name = lex_strings.make($1);
	yyerror("sorry: Class initialization assignment not supported here.");
	delete[]$1;
	$$ = tmp;
      }
  ;


loop_variables /* IEEE1800-2005: A.6.8 */
  : loop_variables ',' IDENTIFIER
      { list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	delete[]$3;
	$$ = tmp;
      }
  | IDENTIFIER
      { list<perm_string>*tmp = new list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  ;

method_qualifier /* IEEE1800-2005: A.1.8 */
  : K_virtual
  | class_item_qualifier
  ;

method_qualifier_opt
  : method_qualifier
  |
  ;


non_integer_type /* IEEE1800-2005: A.2.2.1 */
  : K_real { $$ = real_type_t::REAL; }
  | K_realtime { $$ = real_type_t::REAL; }
  | K_shortreal { $$ = real_type_t::SHORTREAL; }
  ;

number  : BASED_NUMBER
	     { $$ = $1; based_size = 0;}
        | DEC_NUMBER
	     { $$ = $1; based_size = 0;}
        | DEC_NUMBER BASED_NUMBER
	     { $$ = pform_verinum_with_size($1,$2, @2.text, @2.first_line);
	       based_size = 0; }
        | UNBASED_NUMBER
	     { $$ = $1; based_size = 0;}
        | DEC_NUMBER UNBASED_NUMBER
	     { yyerror(@1, "error: Unbased SystemVerilog literal cannot have "
	                   "a size.");
	       $$ = $1; based_size = 0;}
	;

open_range_list /* IEEE1800-2005 A.2.11 */
  : open_range_list ',' value_range
  | value_range
  ;

port_direction /* IEEE1800-2005 A.1.3 */
  : K_input  { $$ = NetNet::PINPUT; }
  | K_output { $$ = NetNet::POUTPUT; }
  | K_inout  { $$ = NetNet::PINOUT; }
  | K_ref
      { $$ = NetNet::PREF;
        if (!gn_system_verilog()) {
	      yyerror(@1, "error: Reference ports (ref) require SystemVerilog.");
	      $$ = NetNet::PINPUT;
	}
      }
  ;

  /* port_direction_opt is used in places where the port direction is
     optional. The default direction is selected by the context,
     which needs to notice the PIMPLICIT direction. */

port_direction_opt
  : port_direction { $$ = $1; }
  |                { $$ = NetNet::PIMPLICIT; }
  ;

  /* The property_qualifier rule is as literally described in the LRM,
     but the use is usually as { property_qualifier }, which is
     implemented bt the property_qualifier_opt rule below. */

property_qualifier /* IEEE1800-2005 A.1.8 */
  : class_item_qualifier
  | random_qualifier
  ;

property_qualifier_opt /* IEEE1800-2005 A.1.8: ... { property_qualifier } */
  : property_qualifier_list
  |
  ;

property_qualifier_list /* IEEE1800-2005 A.1.8 */
  : property_qualifier_list property_qualifier
  | property_qualifier
  ;

random_qualifier /* IEEE1800-2005 A.1.8 */
  : K_rand
  | K_randc
  ;

  /* real and realtime are exactly the same so save some code
   * with a common matching rule. */
real_or_realtime
	: K_real
	| K_realtime
	;

signing /* IEEE1800-2005: A.2.2.1 */
  : K_signed   { $$ = true; }
  | K_unsigned { $$ = false; }
  ;

statement /* IEEE1800-2005: A.6.4 */
  : attribute_list_opt statement_item
      { pform_bind_attributes($2->attributes, $1);
	$$ = $2;
      }
  ;

  /* Many places where statements are allowed can actually take a
     statement or a null statement marked with a naked semi-colon. */

statement_or_null /* IEEE1800-2005: A.6.4 */
  : statement
      { $$ = $1; }
  | attribute_list_opt ';'
      { $$ = 0; }
  ;

stream_expression
  : expression
  ;

stream_expression_list
  : stream_expression_list ',' stream_expression
  | stream_expression
  ;

stream_operator
  : K_LS
  | K_RS
  ;

streaming_concatenation /* IEEE1800-2005: A.8.1 */
  : '{' stream_operator '{' stream_expression_list '}' '}'
      { /* streaming concatenation is a SystemVerilog thing. */
	if (gn_system_verilog()) {
	      yyerror(@2, "sorry: Streaming concatenation not supported.");
	      $$ = 0;
	} else {
	      yyerror(@2, "error: Streaming concatenation requires SystemVerilog");
	      $$ = 0;
	}
      }
  ;

  /* The task declaration rule matches the task declaration
     header, then pushes the function scope. This causes the
     definitions in the task_body to take on the scope of the task
     instead of the module. */

task_declaration /* IEEE1800-2005: A.2.7 */

  : K_task K_automatic_opt IDENTIFIER ';'
      { assert(current_task == 0);
	current_task = pform_push_task_scope(@1, $3, $2);
      }
    task_item_list_opt
    statement_or_null_list_opt
    K_endtask
      { current_task->set_ports($6);
	current_task_set_statement(@3, $7);
	pform_pop_scope();
	current_task = 0;
	if ($7 && $7->size() > 1 && !gn_system_verilog()) {
	      yyerror(@7, "error: Task body with multiple statements requres SystemVerilog.");
	}
	delete $7;
      }
    endname_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// endname_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	if ($10 && (strcmp($3,$10) != 0)) {
	      yyerror(@10, "error: End name doesn't match module/program name");
	}
	if ($10 && !gn_system_verilog()) {
	      yyerror(@10, "error: Task end names require System Verilog.");
	}
	delete[]$3;
	if ($10) delete[]$10;
      }

  | K_task K_automatic_opt IDENTIFIER '('
      { assert(current_task == 0);
	current_task = pform_push_task_scope(@1, $3, $2);
      }
    tf_port_list ')' ';'
    block_item_decls_opt
    statement_or_null_list_opt
    K_endtask
      { current_task->set_ports($6);
	current_task_set_statement(@3, $10);
	pform_pop_scope();
	current_task = 0;
	if ($10) delete $10;
      }
    endname_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// endname_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	if ($13 && (strcmp($3,$13) != 0)) {
	      yyerror(@13, "error: End name doesn't match module/program name");
	}
	if ($13 && !gn_system_verilog()) {
	      yyerror(@13, "error: Task end names require System Verilog.");
	}
	delete[]$3;
	if ($13) delete[]$13;
      }

  | K_task K_automatic_opt IDENTIFIER '(' ')' ';'
      { assert(current_task == 0);
	current_task = pform_push_task_scope(@1, $3, $2);
      }
    block_item_decls_opt
    statement_or_null_list
    K_endtask
      { current_task->set_ports(0);
	current_task_set_statement(@3, $9);
	pform_pop_scope();
	current_task = 0;
	cerr << @3 << ": warning: task definition for \"" << $3
	     << "\" has an empty port declaration list!" << endl;
	if ($9->size() > 1 && !gn_system_verilog()) {
	      yyerror(@9, "error: Task body with multiple statements requres SystemVerilog.");
	}
	delete $9;
      }
    endname_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// endname_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	if ($12 && (strcmp($3,$12) != 0)) {
	      yyerror(@12, "error: End name doesn't match module/program name");
	}
	if ($12 && !gn_system_verilog()) {
	      yyerror(@12, "error: Task end names require System Verilog.");
	}
	delete[]$3;
	if ($12) delete[]$12;
      }

  | K_task K_automatic_opt IDENTIFIER error K_endtask
      {
	assert(current_task == 0);
      }
    endname_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// endname_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	if ($7 && (strcmp($3,$7) != 0)) {
	      yyerror(@7, "error: End name doesn't match module/program name");
	}
	if ($7 && !gn_system_verilog()) {
	      yyerror(@7, "error: Task end names require System Verilog.");
	}
	delete[]$3;
	if ($7) delete[]$7;
      }

  ;


tf_port_declaration /* IEEE1800-2005: A.2.7 */
  : port_direction K_reg_opt unsigned_signed_opt range_opt list_of_identifiers ';'
      { svector<PWire*>*tmp = pform_make_task_ports(@1, $1,
						$2 ? IVL_VT_LOGIC :
						     IVL_VT_NO_TYPE,
						$3, $4, $5);
	$$ = tmp;
      }

  /* When the port is an integer, infer a signed vector of the integer
     shape. Generate a range ([31:0]) to make it work. */

  | port_direction K_integer list_of_identifiers ';'
      { list<pform_range_t>*range_stub = make_range_from_width(integer_width);
	svector<PWire*>*tmp = pform_make_task_ports(@1, $1, IVL_VT_LOGIC, true,
						    range_stub, $3, true);
	$$ = tmp;
      }

  /* Ports can be time with a width of [63:0] (unsigned). */

  | port_direction K_time list_of_identifiers ';'
      { list<pform_range_t>*range_stub = make_range_from_width(64);
	svector<PWire*>*tmp = pform_make_task_ports(@1, $1, IVL_VT_LOGIC, false,
						    range_stub, $3);
	$$ = tmp;
      }

  /* Ports can be real or realtime. */

  | port_direction real_or_realtime list_of_identifiers ';'
      { svector<PWire*>*tmp = pform_make_task_ports(@1, $1, IVL_VT_REAL, false,
						    0, $3);
	$$ = tmp;
      }

  ;


  /* These rules for tf_port_item are slightly expanded from the
     strict rules in the LRM to help with LALR parsing.

     NOTE: Some of these rules should be folded into the "data_type"
     variant which uses the data_type rule to match data type
     declarations. That some rules do not use the data_type production
     is a consequence of legacy. */

tf_port_item /* IEEE1800-2005: A.2.7 */

  /* Ports can be integer with a width of [31:0]. */

  : port_direction_opt K_integer IDENTIFIER range_opt tf_port_item_expr_opt
      { list<pform_range_t>*range_stub = make_range_from_width(integer_width);
	NetNet::PortType use_port_type = $1==NetNet::PIMPLICIT? NetNet::PINPUT : $1;

	port_declaration_context.port_type = use_port_type;
	port_declaration_context.var_type = IVL_VT_LOGIC;
	port_declaration_context.sign_flag = true;
	delete port_declaration_context.range;
	port_declaration_context.range = copy_range(range_stub);
	svector<PWire*>*tmp = pform_make_task_ports(@3, use_port_type,
						    IVL_VT_LOGIC, true,
						    range_stub,
						    list_from_identifier($3), true);
	$$ = tmp;
	if ($4) {
	      yyerror(@4, "sorry: Port variable dimensions not supported yet.");
	      delete $4;
	}
	if ($5) {
	      yyerror(@5, "sorry: Port default expressions not supported yet.");
	      delete $5;
	}
      }

  /* Ports can be time with a width of [63:0] (unsigned). */

  | port_direction_opt K_time IDENTIFIER range_opt tf_port_item_expr_opt
      { list<pform_range_t>*range_stub = make_range_from_width(64);
	NetNet::PortType use_port_type = $1==NetNet::PIMPLICIT? NetNet::PINPUT : $1;

	port_declaration_context.port_type = use_port_type;
	port_declaration_context.var_type = IVL_VT_LOGIC;
	port_declaration_context.sign_flag = false;
	delete port_declaration_context.range;
	port_declaration_context.range = copy_range(range_stub);
	svector<PWire*>*tmp = pform_make_task_ports(@3, use_port_type, IVL_VT_LOGIC,
						    false, range_stub,
						    list_from_identifier($3));
	$$ = tmp;
	if ($4) {
	      yyerror(@4, "sorry: Port variable dimensions not supported yet.");
	      delete $4;
	}
	if ($5) {
	      yyerror(@5, "sorry: Port default expressions not supported yet.");
	      delete $5;
	}
      }

  | port_direction_opt data_type_or_implicit IDENTIFIER range_opt tf_port_item_expr_opt
      { svector<PWire*>*tmp;
	NetNet::PortType use_port_type = $1==NetNet::PIMPLICIT? NetNet::PINPUT : $1;
	list<perm_string>* ilist = list_from_identifier($3);

	if (($2 == 0) && ($1==NetNet::PIMPLICIT)) {
		// Detect special case this is an undecorated
		// identifier and we need to get the declaration from
		// left context.
	      if (port_declaration_context.var_type == IVL_VT_NO_TYPE) {
		    tmp = pform_make_task_ports(@3, use_port_type,
					 port_declaration_context.data_type,
					 ilist);
	      } else {
		    tmp = pform_make_task_ports(@3, use_port_type,
					 port_declaration_context.var_type,
					 port_declaration_context.sign_flag,
					 copy_range(port_declaration_context.range),
					 ilist);
	      }

	} else {
		// Otherwise, the decorations for this identifier
		// indicate the type. Save the type for any right
		// context thta may come later.
	      port_declaration_context.port_type = use_port_type;
	      port_declaration_context.var_type = IVL_VT_NO_TYPE;
	      port_declaration_context.sign_flag = false;
	      delete port_declaration_context.range;
	      port_declaration_context.range = 0;
	      if ($2 == 0) {
		    $2 = new vector_type_t(IVL_VT_LOGIC, false, 0);
		    FILE_NAME($2, @3);
	      }
	      port_declaration_context.data_type = $2;
	      tmp = pform_make_task_ports(@3, use_port_type, $2, ilist);
	}
	$$ = tmp;
	if ($4) {
	      yyerror(@4, "sorry: Port variable dimensions not supported yet.");
	      delete $4;
	}
	if ($5) {
	      yyerror(@5, "sorry: Port default expressions not supported yet.");
	      delete $5;
	}
     }

  /* Rules to match error cases... */

  | port_direction_opt data_type_or_implicit IDENTIFIER error
      { yyerror(@3, "error: Error in task/function port item after port name %s.", $3);
	yyerrok;
	$$ = 0;
      }
  ;

  /* This rule matches the [ = <expression> ] part of the tf_port_item rules. */

tf_port_item_expr_opt
  : '=' expression { $$ = $2; }
  |                { $$ = 0; }
  ;

tf_port_list /* IEEE1800-2005: A.2.7 */

  : tf_port_list ',' tf_port_item
      { svector<PWire*>*tmp;
	if ($1 && $3) {
	      tmp = new svector<PWire*>(*$1, *$3);
	      delete $1;
	      delete $3;
	} else if ($1) {
	      tmp = $1;
	} else {
	      tmp = $3;
	}
	$$ = tmp;
      }

  | tf_port_item
      { $$ = $1; }

  /* Rules to handle some errors in tf_port_list items. */

  | error ',' tf_port_item
      { yyerror(@2, "error: Syntax error in task/function port declaration.");
	$$ = $3;
      }
  | tf_port_list ','
      { yyerror(@2, "error: NULL port declarations are not allowed.");
	$$ = $1;
      }
  | tf_port_list ';'
      { yyerror(@2, "error: ';' is an invalid port declaration separator.");
	$$ = $1;
      }
  ;


value_range /* IEEE1800-2005: A.8.3 */
  : expression
      { }
  | '[' expression ':' expression ']'
      { }
  ;

variable_dimension /* IEEE1800-2005: A.2.5 */
  : '[' expression ':' expression ']'
      { list<pform_range_t> *tmp = new list<pform_range_t>;
	pform_range_t index ($2,$4);
	tmp->push_back(index);
	$$ = tmp;
      }
  | '[' expression ']'
      { // SystemVerilog canonical range
	if (generation_flag < GN_VER2005_SV) {
	      warn_count += 1;
	      cerr << @2 << ": warning: Use of SystemVerilog [size] dimension. "
		   << "Use at least -g2005-sv to remove this warning." << endl;
	}
	list<pform_range_t> *tmp = new list<pform_range_t>;
	pform_range_t index;
	index.first = new PENumber(new verinum((uint64_t)0, integer_width));
	index.second = new PEBinary('-', $2, new PENumber(new verinum((uint64_t)1, integer_width)));
	tmp->push_back(index);
	$$ = tmp;
      }
  | '[' ']'
      { list<pform_range_t> *tmp = new list<pform_range_t>;
	pform_range_t index (0,0);
	yyerror("sorry: Dynamic array ranges not supported.");
	tmp->push_back(index);
	$$ = tmp;
      }
  | '[' '$' ']'
      { // SystemVerilog queue
	list<pform_range_t> *tmp = new list<pform_range_t>;
	pform_range_t index (0,0);
	if (gn_system_verilog()) {
	      yyerror("sorry: Dynamic array ranges not supported.");
	} else {
	      yyerror("error: Queue declarations require System Verilog.");
	}
	tmp->push_back(index);
	$$ = tmp;
      }
  ;

  /* Verilog-2001 supports attribute lists, which can be attached to a
     variety of different objects. The syntax inside the (* *) is a
     comma separated list of names or names with assigned values. */
attribute_list_opt
	: attribute_instance_list
	| { $$ = 0; }
	;

attribute_instance_list
  : K_PSTAR K_STARP { $$ = 0; }
  | K_PSTAR attribute_list K_STARP { $$ = $2; }
  | attribute_instance_list K_PSTAR K_STARP { $$ = $1; }
  | attribute_instance_list K_PSTAR attribute_list K_STARP
      { list<named_pexpr_t>*tmp = $1;
	if (tmp) {
	    tmp->splice(tmp->end(), *$3);
	    delete $3;
	    $$ = tmp;
	} else $$ = $3;
      }
  ;

attribute_list
  : attribute_list ',' attribute
      { list<named_pexpr_t>*tmp = $1;
        tmp->push_back(*$3);
	delete $3;
	$$ = tmp;
      }
  | attribute
      { list<named_pexpr_t>*tmp = new list<named_pexpr_t>;
        tmp->push_back(*$1);
	delete $1;
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

  /* Integer atom declarations are simpler in that they do not have
     all the trappings of a general variable declaration. All of that
     is implicit in the "integer" of the declaration. */

  : K_integer signed_unsigned_opt register_variable_list ';'
      { pform_set_reg_integer($3, attributes_in_context);
      }

  | K_time register_variable_list ';'
      { pform_set_reg_time($2, attributes_in_context);
      }

  /* variable declarations. Note that data_type can be 0 if we are
     recovering from an error. */

  | data_type register_variable_list ';'
      { if ($1) pform_set_data_type(@1, $1, $2, attributes_in_context);
      }

  | K_reg data_type register_variable_list ';'
      { if ($2) pform_set_data_type(@2, $2, $3, attributes_in_context);
      }

  | K_event list_of_identifiers ';'
      { pform_make_events($2, @1.text, @1.first_line);
      }

  | K_parameter param_type parameter_assign_list ';'
  | K_localparam param_type localparam_assign_list ';'

  /* Blocks can have type declarations. */

  | type_declaration

  /* Recover from errors that happen within variable lists. Use the
     trailing semi-colon to resync the parser. */

  | K_integer error ';'
      { yyerror(@1, "error: syntax error in integer variable list.");
	yyerrok;
      }

  | K_time error ';'
      { yyerror(@1, "error: syntax error in time variable list.");
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

  /* Type declarations are parsed here. The rule actions call pform
     functions that add the declaration to the current lexical scope. */
type_declaration
  : K_typedef data_type IDENTIFIER ';'
      { perm_string name = lex_strings.make($3);
	pform_set_typedef(name, $2);
	delete[]$3;
      }

  /* These are forward declarations... */

  | K_typedef K_class  IDENTIFIER ';'
      { // Create a synthetic typedef for the class name so that the
	// lexor detects the name as a type.
	perm_string name = lex_strings.make($3);
	class_type_t*tmp = new class_type_t(name);
	pform_set_typedef(name, tmp);
	delete[]$3;
      }
  | K_typedef K_enum   IDENTIFIER ';'
      { yyerror(@1, "sorry: Enum forward declarations not supported yet."); }
  | K_typedef K_struct IDENTIFIER ';'
      { yyerror(@1, "sorry: Struct forward declarations not supported yet."); }
  | K_typedef K_union  IDENTIFIER ';'
      { yyerror(@1, "sorry: Union forward declarations not supported yet."); }
  | K_typedef          IDENTIFIER ';'
      { // Create a synthetic typedef for the class name so that the
	// lexor detects the name as a type.
	perm_string name = lex_strings.make($2);
	class_type_t*tmp = new class_type_t(name);
	pform_set_typedef(name, tmp);
	delete[]$2;
      }
  ;

  /* The structure for an enumeration data type is the keyword "enum",
     followed by the enumeration values in curly braces. Also allow
     for an optional base type. The default base type is "int", but it
     can be any of the integral or vector types. */

enum_data_type
  : K_enum '{' enum_name_list '}'
      { enum_type_t*enum_type = new enum_type_t;
	FILE_NAME(enum_type, @1);
	enum_type->names .reset($3);
	enum_type->base_type = IVL_VT_BOOL;
	enum_type->signed_flag = true;
	enum_type->range.reset(make_range_from_width(32));
	$$ = enum_type;
      }
  | K_enum atom2_type signed_unsigned_opt '{' enum_name_list '}'
      { enum_type_t*enum_type = new enum_type_t;
	FILE_NAME(enum_type, @1);
	enum_type->names .reset($5);
	enum_type->base_type = IVL_VT_BOOL;
	enum_type->signed_flag = $3;
	enum_type->range.reset(make_range_from_width($2));
	$$ = enum_type;
      }
  | K_enum K_integer signed_unsigned_opt '{' enum_name_list '}'
      { enum_type_t*enum_type = new enum_type_t;
	FILE_NAME(enum_type, @1);
	enum_type->names .reset($5);
	enum_type->base_type = IVL_VT_LOGIC;
	enum_type->signed_flag = $3;
	enum_type->range.reset(make_range_from_width(integer_width));
	$$ = enum_type;
      }
  | K_enum K_logic unsigned_signed_opt range '{' enum_name_list '}'
      { enum_type_t*enum_type = new enum_type_t;
	FILE_NAME(enum_type, @1);
	enum_type->names .reset($6);
	enum_type->base_type = IVL_VT_LOGIC;
	enum_type->signed_flag = $3;
	enum_type->range.reset($4);
	$$ = enum_type;
      }
  | K_enum K_reg unsigned_signed_opt range '{' enum_name_list '}'
      { enum_type_t*enum_type = new enum_type_t;
	FILE_NAME(enum_type, @1);
	enum_type->names .reset($6);
	enum_type->base_type = IVL_VT_LOGIC;
	enum_type->signed_flag = $3;
	enum_type->range.reset($4);
	$$ = enum_type;
      }
  | K_enum K_bit unsigned_signed_opt range '{' enum_name_list '}'
      { enum_type_t*enum_type = new enum_type_t;
	FILE_NAME(enum_type, @1);
	enum_type->names .reset($6);
	enum_type->base_type = IVL_VT_BOOL;
	enum_type->signed_flag = $3;
	enum_type->range.reset($4);
	$$ = enum_type;
      }
  ;

enum_name_list
  : enum_name
      { $$ = $1;
      }
  | enum_name_list ',' enum_name
      { list<named_pexpr_t>*lst = $1;
	lst->splice(lst->end(), *$3);
	delete $3;
	$$ = lst;
      }
  ;

pos_neg_number
  : number
      { $$ = $1;
      }
  | '-' number
      { verinum tmp = v_not(*($2)) + verinum(1);
	*($2) = tmp;
	$$ = $2;
      }
  ;

enum_name
  : IDENTIFIER
      { perm_string name = lex_strings.make($1);
	delete[]$1;
	$$ = make_named_number(name);
      }
  | IDENTIFIER '[' pos_neg_number ']'
      { perm_string name = lex_strings.make($1);
	long count = check_enum_seq_value(@1, $3, false);
	delete[]$1;
	$$ = make_named_numbers(name, 0, count-1);
	delete $3;
      }
  | IDENTIFIER '[' pos_neg_number ':' pos_neg_number ']'
      { perm_string name = lex_strings.make($1);
	$$ = make_named_numbers(name, check_enum_seq_value(@1, $3, true),
	                              check_enum_seq_value(@1, $5, true));
	delete[]$1;
	delete $3;
	delete $5;
      }
  | IDENTIFIER '=' expression
      { perm_string name = lex_strings.make($1);
	delete[]$1;
	$$ = make_named_number(name, $3);
      }
  | IDENTIFIER '[' pos_neg_number ']' '=' expression
      { perm_string name = lex_strings.make($1);
	long count = check_enum_seq_value(@1, $3, false);
	$$ = make_named_numbers(name, 0, count-1, $6);
	delete[]$1;
	delete $3;
      }
  | IDENTIFIER '[' pos_neg_number ':' pos_neg_number ']' '=' expression
      { perm_string name = lex_strings.make($1);
	$$ = make_named_numbers(name, check_enum_seq_value(@1, $3, true),
	                              check_enum_seq_value(@1, $5, true), $8);
	delete[]$1;
	delete $3;
	delete $5;
      }
  ;

struct_data_type
  : K_struct K_packed_opt '{' struct_union_member_list '}'
      { struct_type_t*tmp = new struct_type_t;
	FILE_NAME(tmp, @1);
	tmp->packed_flag = $2;
	tmp->members .reset($4);
	$$ = tmp;
      }
  | K_struct K_packed_opt '{' error '}'
      { yyerror(@4, "error: Errors in struct/union member list.");
	yyerrok;
	struct_type_t*tmp = new struct_type_t;
	FILE_NAME(tmp, @1);
	tmp->packed_flag = $2;
	$$ = tmp;
      }
  ;

struct_union_member_list
  : struct_union_member_list struct_union_member
      { list<struct_member_t*>*tmp = $1;
	tmp->push_back($2);
	$$ = tmp;
      }
  | struct_union_member
      { list<struct_member_t*>*tmp = new list<struct_member_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

struct_union_member
  : attribute_list_opt K_bit range_opt list_of_variable_decl_assignments ';'
      { struct_member_t*tmp = new struct_member_t;
	FILE_NAME(tmp, @2);
	tmp->type = IVL_VT_BOOL;
	tmp->range .reset($3);
	tmp->names .reset($4);
	$$ = tmp;
      }
  | attribute_list_opt K_logic range_opt list_of_variable_decl_assignments ';'
      { struct_member_t*tmp = new struct_member_t;
	FILE_NAME(tmp, @2);
	tmp->type = IVL_VT_LOGIC;
	tmp->range .reset($3);
	tmp->names .reset($4);
	$$ = tmp;
      }
  | attribute_list_opt atom2_type list_of_variable_decl_assignments ';'
      { struct_member_t*tmp = new struct_member_t;
	FILE_NAME(tmp, @2);
	tmp->type = IVL_VT_BOOL;
	tmp->range .reset(make_range_from_width($2));
	tmp->names .reset($3);
	$$ = tmp;
      }
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
		{ list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($2);
		  $$ = tmp;
		}
	| '#' '(' delay_value ')'
		{ list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	;

delay3
	: '#' delay_value_simple
		{ list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($2);
		  $$ = tmp;
		}
	| '#' '(' delay_value ')'
		{ list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($3);
		  $$ = tmp;
		}
	| '#' '(' delay_value ',' delay_value ')'
		{ list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($3);
		  tmp->push_back($5);
		  $$ = tmp;
		}
	| '#' '(' delay_value ',' delay_value ',' delay_value ')'
		{ list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($3);
		  tmp->push_back($5);
		  tmp->push_back($7);
		  $$ = tmp;
		}
	;

delay3_opt
	: delay3 { $$ = $1; }
	|        { $$ = 0; }
	;

delay_value_list
  : delay_value
      { list<PExpr*>*tmp = new list<PExpr*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  | delay_value_list ',' delay_value
      { list<PExpr*>*tmp = $1;
	tmp->push_back($3);
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
	| TIME_LITERAL
		{ int unit;

		  based_size = 0;
		  $$         = 0;
		  if ($1 == 0 || !get_time_unit($1, unit))
			yyerror(@1, "internal error: delay.");
		  else {
			double p = pow(10.0,
			               (double)(unit - pform_get_timeunit()));
			double time = atof($1) * p;

			verireal *v = new verireal(time);
			$$ = new PEFNumber(v);
			FILE_NAME($$, @1);
		  }
		}
	;

description
  : module
  | udp_primitive
  | config_declaration
  | nature_declaration
  | discipline_declaration
  | KK_attribute '(' IDENTIFIER ',' STRING ',' STRING ')'
      { perm_string tmp3 = lex_strings.make($3);
	pform_set_type_attrib(tmp3, $5, $7);
	delete[] $3;
	delete[] $5;
      }
  | K_timeunit  TIME_LITERAL ';'
      { pform_set_timeunit($2, false, false); }
  | K_timeprecision TIME_LITERAL ';'
      { pform_set_timeprecision($2, false, false); }
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

config_declaration
  : K_config IDENTIFIER ';'
    K_design lib_cell_identifiers ';'
    list_of_config_rule_statements
    K_endconfig
      { cerr << @1 << ": sorry: config declarations are not supported and "
                "will be skipped." << endl;
	delete[] $2;
      }
  ;

lib_cell_identifiers
  : /* The BNF implies this can be blank, but I'm not sure exactly what
     * this means. */
  | lib_cell_identifiers lib_cell_id
  ;

list_of_config_rule_statements
  : /* config rules are optional. */
  | list_of_config_rule_statements config_rule_statement
  ;

config_rule_statement
  : K_default K_liblist list_of_libraries ';'
  | K_instance hierarchy_identifier K_liblist list_of_libraries ';'
      { delete $2; }
  | K_instance hierarchy_identifier K_use lib_cell_id opt_config ';'
      { delete $2; }
  | K_cell lib_cell_id K_liblist list_of_libraries ';'
  | K_cell lib_cell_id K_use lib_cell_id opt_config ';'
  ;

opt_config
  : /* The use clause takse an optional :config. */
  | ':' K_config

lib_cell_id
  : IDENTIFIER
      { delete[] $1; }
  | IDENTIFIER '.' IDENTIFIER
      { delete[] $1; delete[] $3; }
  ;

list_of_libraries
  : /* A NULL library means use the parents cell library. */
  | list_of_libraries IDENTIFIER
      { delete[] $2; }

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
		  $$.str1 = IVL_DR_HiZ;
		}
	| '(' dr_strength1 ',' K_highz0 ')'
		{ $$.str0 = IVL_DR_HiZ;
		  $$.str1 = $2.str1;
		}
	| '(' K_highz1 ',' dr_strength0 ')'
		{ $$.str0 = $4.str0;
		  $$.str1 = IVL_DR_HiZ;
		}
	| '(' K_highz0 ',' dr_strength1 ')'
		{ $$.str0 = IVL_DR_HiZ;
		  $$.str1 = $4.str1;
		}
	;

drive_strength_opt
	: drive_strength { $$ = $1; }
	|                { $$.str0 = IVL_DR_STRONG; $$.str1 = IVL_DR_STRONG; }
	;

dr_strength0
	: K_supply0 { $$.str0 = IVL_DR_SUPPLY; }
	| K_strong0 { $$.str0 = IVL_DR_STRONG; }
	| K_pull0   { $$.str0 = IVL_DR_PULL; }
	| K_weak0   { $$.str0 = IVL_DR_WEAK; }
	;

dr_strength1
	: K_supply1 { $$.str1 = IVL_DR_SUPPLY; }
	| K_strong1 { $$.str1 = IVL_DR_STRONG; }
	| K_pull1   { $$.str1 = IVL_DR_PULL; }
	| K_weak1   { $$.str1 = IVL_DR_WEAK; }
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
  | inc_or_dec_expression
      { $$ = $1; }
  | inside_expression
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
	| '~' '&' expr_primary %prec UNARY_PREC
		{ yyerror(@1, "error: '~' '&'  is not a valid expression. "
			  "Please use operator '~&' instead.");
		  $$ = 0;
		}
	| '~' '|' expr_primary %prec UNARY_PREC
		{ yyerror(@1, "error: '~' '|'  is not a valid expression. "
			  "Please use operator '~|' instead.");
		  $$ = 0;
		}
	| '~' '^' expr_primary %prec UNARY_PREC
		{ yyerror(@1, "error: '~' '^'  is not a valid expression. "
			  "Please use operator '~^' instead.");
		  $$ = 0;
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
      { list<PExpr*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | expression
      { list<PExpr*>*tmp = new list<PExpr*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  |
      { list<PExpr*>*tmp = new list<PExpr*>;
        tmp->push_back(0);
	$$ = tmp;
      }
  | expression_list_with_nuls ','
      { list<PExpr*>*tmp = $1;
	tmp->push_back(0);
	$$ = tmp;
      }
	;

expression_list_proper
  : expression_list_proper ',' expression
      { list<PExpr*>*tmp = $1;
        tmp->push_back($3);
        $$ = tmp;
      }
  | expression
      { list<PExpr*>*tmp = new list<PExpr*>;
	tmp->push_back($1);
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
  | hierarchy_identifier '(' ')'
      { const vector<PExpr*> empty;
	PECallFunction*tmp = new PECallFunction(*$1, empty);
	FILE_NAME(tmp, @1);
	delete $1;
	$$ = tmp;
	if (!gn_system_verilog()) {
	      yyerror(@1, "error: Empty function argument list requires SystemVerilog.");
	}
      }
  | SYSTEM_IDENTIFIER '('  ')'
      { perm_string tn = lex_strings.make($1);
	const vector<PExpr*>empty;
	PECallFunction*tmp = new PECallFunction(tn, empty);
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
	if (!gn_system_verilog()) {
	      yyerror(@1, "error: Empty function argument list requires SystemVerilog.");
	}
      }

  | implicit_class_handle
      { yyerror(@1, "sorry: Implicit class handles (this/super) are not supported.");
	$$ = 0;
      }

  | implicit_class_handle '.' hierarchy_identifier
      { yyerror(@1, "sorry: Implicit class handles (this/super) are not supported.");
	$$ = 0;
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

  | '{' '}'
      { // This is the empty queue syntax.
	if (gn_system_verilog()) {
	      yyerror(@1, "sorry: Expty queue expressions not supported.");
	} else {
	      yyerror(@1, "error: Concatenations are not allowed to be empty.");
	}
	$$ = 0;
      }

  /* Cast expressions are primaries */

  | DEC_NUMBER '\'' '(' expression ')'
      { PExpr*base = $4;
	if (gn_system_verilog()) {
	      PECastSize*tmp = new PECastSize($1->as_ulong(), base);
	      FILE_NAME(tmp, @1);
	      delete $1;
	      $$ = tmp;
	} else {
	      yyerror(@1, "error: Size cast requires SystemVerilog.");
	      $$ = base;
	}
      }

  /* Aggregate literals are primaries. */

  | assignment_pattern
      { $$ = $1; }

  /* SystemVerilog supports streaming concatenation */
  | streaming_concatenation
      { $$ = $1; }

  | K_null
      { yyerror("sorry: null expressions not supported yet.");
	$$ = 0;
      }
  ;

  /* A function_item_list borrows the task_port_item run to match
     declarations of ports. We check later to make sure there are no
     output or inout ports actually used.

     The function_item is the same as tf_item_declaration. */
function_item_list_opt
  : function_item_list { $$ = $1; }
  |                    { $$ = 0; }
  ;

function_item_list
  : function_item
      { $$ = $1; }
  | function_item_list function_item
      { /* */
	if ($1 && $2) {
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
  : tf_port_declaration
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
	list<pform_range_t>*rng = $2;
	tmp->name = $1;
	tmp->parms = $4;
	tmp->range = rng->front();
	rng->pop_front();
	assert(rng->empty());
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
	list<pform_range_t>*rng = $2;
	tmp->name = $1;
	tmp->parms = 0;
	tmp->parms_by_name = 0;
	tmp->range = rng->front();
	rng->pop_front();
	assert(rng->empty());
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
	list<pform_range_t>*rng = $2;
	tmp->name = $1;
	tmp->parms = 0;
	tmp->parms_by_name = $4;
	tmp->range = rng->front();
	rng->pop_front();
	assert(rng->empty());
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
        ;

switchtype
	: K_nmos  { $$ = PGBuiltin::NMOS; }
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

list_of_port_identifiers
	: IDENTIFIER
                { $$ = make_port_list($1, 0); }
	| IDENTIFIER '=' expression
                { $$ = make_port_list($1, $3); }
	| list_of_port_identifiers ',' IDENTIFIER
                { $$ = make_port_list($1, $3, 0); }
	| list_of_port_identifiers ',' IDENTIFIER '=' expression
                { $$ = make_port_list($1, $3, $5); }
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
					port_declaration_context.var_type,
					port_declaration_context.sign_flag,
					port_declaration_context.data_type,
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
  : attribute_list_opt K_input net_type_opt data_type_or_implicit IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($5);
	ptmp = pform_module_port_reference(name, @2.text, @2.first_line);
	pform_module_define_port(@2, name, NetNet::PINPUT, $3, IVL_VT_NO_TYPE,
				false, $4, 0, $1);
	port_declaration_context.port_type = NetNet::PINPUT;
	port_declaration_context.port_net_type = $3;
	port_declaration_context.var_type = IVL_VT_NO_TYPE;
	port_declaration_context.sign_flag = false;
	delete port_declaration_context.range;
	port_declaration_context.range = 0;
	port_declaration_context.data_type = $4;
	delete[]$5;
	$$ = ptmp;
      }
  | attribute_list_opt
    K_input K_wreal IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($4);
	ptmp = pform_module_port_reference(name, @2.text,
					   @2.first_line);
	pform_module_define_port(@2, name, NetNet::PINPUT,
				 NetNet::WIRE, IVL_VT_REAL, true, 0, 0, $1);
	port_declaration_context.port_type = NetNet::PINPUT;
	port_declaration_context.port_net_type = NetNet::WIRE;
	port_declaration_context.var_type = IVL_VT_REAL;
	port_declaration_context.sign_flag = true;
	delete port_declaration_context.range;
	port_declaration_context.range = 0;
	port_declaration_context.data_type = 0;
	delete[]$4;
	$$ = ptmp;
      }
  | attribute_list_opt K_inout net_type_opt data_type_or_implicit IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($5);
	ptmp = pform_module_port_reference(name, @2.text, @2.first_line);
	pform_module_define_port(@2, name, NetNet::PINOUT, $3, IVL_VT_NO_TYPE,
				false, $4, 0, $1);
	port_declaration_context.port_type = NetNet::PINOUT;
	port_declaration_context.port_net_type = $3;
	port_declaration_context.var_type = IVL_VT_NO_TYPE;
	port_declaration_context.sign_flag = false;
	delete port_declaration_context.range;
	port_declaration_context.range = 0;
	port_declaration_context.data_type = $4;
	delete[]$5;
	$$ = ptmp;
      }
  | attribute_list_opt
    K_inout K_wreal IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($4);
	ptmp = pform_module_port_reference(name, @2.text,
					   @2.first_line);
	pform_module_define_port(@2, name, NetNet::PINOUT,
				 NetNet::WIRE, IVL_VT_REAL, true, 0, 0, $1);
	port_declaration_context.port_type = NetNet::PINOUT;
	port_declaration_context.port_net_type = NetNet::WIRE;
	port_declaration_context.var_type = IVL_VT_REAL;
	port_declaration_context.sign_flag = true;
	delete port_declaration_context.range;
	port_declaration_context.range = 0;
	port_declaration_context.data_type = 0;
	delete[]$4;
	$$ = ptmp;
      }
  | attribute_list_opt K_output net_type_opt data_type_or_implicit IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($5);
	NetNet::Type use_type = $3;
	if (use_type == NetNet::IMPLICIT) {
	      if (vector_type_t*dtype = dynamic_cast<vector_type_t*> ($4)) {
		    if (dtype->reg_flag)
			  use_type = NetNet::REG;
		    else if (dtype->implicit_flag)
			  use_type = NetNet::IMPLICIT;
		    else
			  use_type = NetNet::IMPLICIT_REG;
	      } else if (dynamic_cast<atom2_type_t*> ($4)) {
		    use_type = NetNet::IMPLICIT_REG;
	      }
	}
	ptmp = pform_module_port_reference(name, @2.text, @2.first_line);
	pform_module_define_port(@2, name, NetNet::POUTPUT, use_type, IVL_VT_NO_TYPE,
				false, $4, 0, $1);
	port_declaration_context.port_type = NetNet::POUTPUT;
	port_declaration_context.port_net_type = use_type;
	port_declaration_context.var_type = IVL_VT_NO_TYPE;
	port_declaration_context.sign_flag = false;
	delete port_declaration_context.range;
	port_declaration_context.range = 0;
	port_declaration_context.data_type = $4;
	delete[]$5;
	$$ = ptmp;
      }
  | attribute_list_opt
    K_output K_wreal IDENTIFIER
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($4);
	ptmp = pform_module_port_reference(name, @2.text,
					   @2.first_line);
	pform_module_define_port(@2, name, NetNet::POUTPUT,
				 NetNet::WIRE, IVL_VT_REAL, true, 0, 0, $1);
	port_declaration_context.port_type = NetNet::POUTPUT;
	port_declaration_context.port_net_type = NetNet::WIRE;
	port_declaration_context.var_type = IVL_VT_REAL;
	port_declaration_context.sign_flag = true;
	delete port_declaration_context.range;
	port_declaration_context.range = 0;
	port_declaration_context.data_type = 0;
	delete[]$4;
	$$ = ptmp;
      }
  | attribute_list_opt K_output net_type_opt data_type_or_implicit IDENTIFIER '=' expression
      { Module::port_t*ptmp;
	perm_string name = lex_strings.make($5);
	NetNet::Type use_type = $3;
	if (use_type == NetNet::IMPLICIT) {
	      if (vector_type_t*dtype = dynamic_cast<vector_type_t*> ($4)) {
		    if (dtype->reg_flag)
			  use_type = NetNet::REG;
		    else
			  use_type = NetNet::IMPLICIT_REG;
	      } else {
		    use_type = NetNet::IMPLICIT_REG;
	      }
	}
	ptmp = pform_module_port_reference(name, @2.text, @2.first_line);
	pform_module_define_port(@2, name, NetNet::POUTPUT, use_type, IVL_VT_NO_TYPE,
				false, $4, 0, $1);
	port_declaration_context.port_type = NetNet::PINOUT;
	port_declaration_context.port_net_type = use_type;
	port_declaration_context.var_type = IVL_VT_NO_TYPE;
	port_declaration_context.sign_flag = false;
	delete port_declaration_context.range;
	port_declaration_context.range = 0;
	port_declaration_context.data_type = $4;

	pform_make_reginit(@5, name, $7);

	delete[]$5;
	$$ = ptmp;
      }
  ;



net_type_opt
	: net_type { $$ = $1; }
	| { $$ = NetNet::IMPLICIT; }
	;

  /*
   * The signed_opt rule will return "true" if K_signed is present,
   * for "false" otherwise. This rule corresponds to the declaration
   * defaults for reg/bit/logic.
   *
   * The signed_unsigned_opt rule with match K_signed or K_unsigned
   * and return true or false as appropriate. The default is
   * "true". This corresponds to the declaration defaults for
   * byte/shortint/int/longint.
   */
unsigned_signed_opt
  : K_signed   { $$ = true; }
  | K_unsigned { $$ = false; }
  |            { $$ = false; }
  ;

signed_unsigned_opt
  : K_signed   { $$ = true; }
  | K_unsigned { $$ = false; }
  |            { $$ = true; }
  ;

  /*
   * In some places we can take any of the 4 2-value atom-type
   * names. All the context needs to know if that type is its width.
   */
atom2_type
  : K_byte     { $$ = 8; }
  | K_shortint { $$ = 16; }
  | K_int      { $$ = 32; }
  | K_longint  { $$ = 64; }
  ;

  /* An lpvalue is the expression that can go on the left side of a
     procedural assignment. This rule handles only procedural
     assignments. It is more limited than the general expr_primary
     rule to reflect the rules for assignment l-values. */
lpvalue
  : hierarchy_identifier
      { PEIdent*tmp = new PEIdent(*$1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete $1;
      }

  | implicit_class_handle '.' hierarchy_identifier
      { yyerror(@1, "sorry: implicit class handles (this/super) not supported.");
	PEIdent*tmp = new PEIdent(*$3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete $3;
      }

  | '{' expression_list_proper '}'
      { PEConcat*tmp = new PEConcat(*$2);
	FILE_NAME(tmp, @1);
	delete $2;
	$$ = tmp;
      }

  | streaming_concatenation
      { yyerror(@1, "sorry: streaming concatenation not supported in l-values.");
	$$ = 0;
      }
  ;


  /* Continuous assignments have a list of individual assignments. */

cont_assign
  : lpvalue '=' expression
      { list<PExpr*>*tmp = new list<PExpr*>;
	tmp->push_back($1);
	tmp->push_back($3);
	$$ = tmp;
      }
  ;

cont_assign_list
  : cont_assign_list ',' cont_assign
      { list<PExpr*>*tmp = $1;
	tmp->splice(tmp->end(), *$3);
	delete $3;
	$$ = tmp;
      }
  | cont_assign
      { $$ = $1; }
  ;

  /* We allow zero, one or two unique declarations. */
local_timeunit_prec_decl_opt
	: /* Empty */
        | local_timeunit_prec_decl
        | local_timeunit_prec_decl local_timeunit_prec_decl
	;

  /* By setting the appropriate have_time???_decl we allow only
     one declaration of each type in this module. */
local_timeunit_prec_decl
	: K_timeunit  TIME_LITERAL ';'
		{ pform_set_timeunit($2, true, false);
		  have_timeunit_decl = true;
		}
	| K_timeprecision TIME_LITERAL ';'
		{ pform_set_timeprecision($2, true, false);
		  have_timeprec_decl = true;
		}
	;

  /* This is the global structure of a module. A module in a start
     section, with optional ports, then an optional list of module
     items, and finally an end marker. */

module
  : attribute_list_opt module_start IDENTIFIER
      { pform_startmodule(@2, $3, $2==K_program, $1); }
    module_parameter_port_list_opt
    module_port_list_opt
    module_attribute_foreign ';'
      { pform_module_set_ports($6); }
    local_timeunit_prec_decl_opt
      { have_timeunit_decl = true; // Every thing past here is
	have_timeprec_decl = true; // a check!
	pform_check_timeunit_prec();
      }
    module_item_list_opt
    module_end
      { Module::UCDriveType ucd;
	  // The lexor detected `unconnected_drive directives and
	  // marked what it found in the uc_drive variable. Use that
	  // to generate a UCD flag for the module.
	switch (uc_drive) {
	    case UCD_NONE:
	    default:
	      ucd = Module::UCD_NONE;
	      break;
	    case UCD_PULL0:
	      ucd = Module::UCD_PULL0;
	      break;
	    case UCD_PULL1:
	      ucd = Module::UCD_PULL1;
	      break;
	}
	  // Check that program/endprogram and module/endmodule
	  // keywords match.
	if ($2 != $13) {
	      switch ($2) {
		  case K_module:
		    yyerror(@13, "error: module not closed by endmodule.");
		    break;
		  case K_program:
		    yyerror(@13, "error: program not closed by endprogram.");
		    break;
		  default:
		    break;
	      }
	}
	pform_endmodule($3, in_celldefine, ucd);
	delete[]$3;
	have_timeunit_decl = false; // We will allow decls again.
	have_timeprec_decl = false;
      }
    endname_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// endname_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	if ($15 && (strcmp($3,$15) != 0)) {
	      yyerror(@15, "error: End name doesn't match module/program name");
	}
	if ($15) delete[]$15;
      }
  ;

  /* Modules start with module/macromodule or program keyword, and end
     with the endmodule or endprogram keyword. The syntax for modules
     and programs is almost identical, so let semantics sort out the
     differences. */
module_start
  : K_module      { $$ = K_module; }
  | K_macromodule { $$ = K_module; }
  | K_program     { $$ = K_program; }
  ;

module_end
  : K_endmodule   { $$ = K_module; }
  | K_endprogram  { $$ = K_program; }
  ;

endname_opt
  : ':' IDENTIFIER { $$ = $2; }
  |                { $$ = 0; }
  ;

module_attribute_foreign
	: K_PSTAR IDENTIFIER K_integer IDENTIFIER '=' STRING ';' K_STARP { $$ = 0; }
	| { $$ = 0; }
	;

module_port_list_opt
  : '(' list_of_ports ')' { $$ = $2; }
  | '(' list_of_port_declarations ')' { $$ = $2; }
  |                       { $$ = 0; }
  | '(' error ')'
      { yyerror(@2, "Errors in port declarations.");
	yyerrok;
	$$ = 0;
      }
  ;

  /* Module declarations include optional ANSI style module parameter
     ports. These are simply advance ways to declare parameters, so
     that the port declarations may use them. */
module_parameter_port_list_opt
	:
        | '#' '(' module_parameter_port_list ')'
	;

module_parameter_port_list
	: K_parameter param_type parameter_assign
	| module_parameter_port_list ',' parameter_assign
	| module_parameter_port_list ',' K_parameter param_type parameter_assign
	;

module_item

  /* Modules can contain further sub-module definitions. */
  : module

  /* This rule detects net declarations that possibly include a
     primitive type, an optional vector range and signed flag. This
     also includes an optional delay set. The values are then applied
     to a list of names. If the primitive type is not specified, then
     resort to the default type LOGIC. */

  | attribute_list_opt net_type
    primitive_type_opt unsigned_signed_opt range_opt
    delay3_opt
    net_variable_list ';'

      { ivl_variable_type_t dtype = $3;
	if (dtype == IVL_VT_NO_TYPE)
	      dtype = IVL_VT_LOGIC;
	pform_makewire(@2, $5, $4, $7, $2, NetNet::NOT_A_PORT, dtype, $1);
	if ($6 != 0) {
	      yyerror(@6, "sorry: net delays not supported.");
	      delete $6;
	}
	delete $1;
      }

  | attribute_list_opt K_wreal delay3 net_variable_list ';'
      { pform_makewire(@2, 0, true, $4, NetNet::WIRE,
		       NetNet::NOT_A_PORT, IVL_VT_REAL, $1);
	    if ($3 != 0) {
		  yyerror(@3, "sorry: net delays not supported.");
		  delete $3;
	    }
	    delete $1;
      }

  | attribute_list_opt K_wreal net_variable_list ';'
      { pform_makewire(@2, 0, true, $3, NetNet::WIRE,
		       NetNet::NOT_A_PORT, IVL_VT_REAL, $1);
	    delete $1;
      }

  /* Very similar to the rule above, but this takes a list of
     net_decl_assigns, which are <name> = <expr> assignment
     declarations. */

	| attribute_list_opt net_type
          primitive_type_opt unsigned_signed_opt range_opt
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

  /* Allow struct nets. */

  | attribute_list_opt net_type struct_data_type net_variable_list ';'

      { pform_makewire(@2, $3, NetNet::NOT_A_PORT, $4, $1);
	delete $1;
      }

  | attribute_list_opt net_type struct_data_type error ';'

      { yyerror(@5, "error: Errors in net variable list.");
      }

  | attribute_list_opt net_type TYPE_IDENTIFIER net_variable_list ';'
      { yyerror(@2, "sorry: Nets of named types not supported."); }

  /* This form doesn't have the range, but does have strengths. This
     gives strength to the assignment drivers. */

	| attribute_list_opt net_type
          primitive_type_opt unsigned_signed_opt
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
	| attribute_list_opt K_wreal net_decl_assigns ';'
		{ pform_makewire(@2, 0, true, 0, str_strength, $3,
		                 NetNet::WIRE, IVL_VT_REAL);
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

	| port_direction unsigned_signed_opt range_opt delay3_opt list_of_identifiers ';'
		{ pform_set_port_type(@1, $5, $3, $2, $1);
		}

  /* The next two rules handle Verilog 2001 statements of the form:
       input wire signed [h:l] <list>;
     This creates the wire and sets the port type all at once. */

	| port_direction net_type unsigned_signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, $1, IVL_VT_NO_TYPE, 0,
		                 SR_BOTH);
		}

	| K_output var_type unsigned_signed_opt range_opt list_of_port_identifiers ';'
		{ list<pair<perm_string,PExpr*> >::const_iterator pp;
		  list<perm_string>*tmp = new list<perm_string>;
		  for (pp = $5->begin(); pp != $5->end(); ++ pp ) {
			tmp->push_back((*pp).first);
		  }
		  pform_makewire(@1, $4, $3, tmp, $2, NetNet::POUTPUT,
		                 IVL_VT_NO_TYPE, 0, SR_BOTH);
		  for (pp = $5->begin(); pp != $5->end(); ++ pp ) {
			if ((*pp).second) {
			      pform_make_reginit(@1, (*pp).first, (*pp).second);
			}
		  }
		  delete $5;
		}

	| port_direction K_wreal list_of_identifiers ';'
		{ pform_makewire(@1, 0, true, $3, NetNet::WIRE, $1,
		                 IVL_VT_REAL, 0, SR_BOTH);
		}

  /* var_type declaration (reg variables) cannot be input or output,
     because the port declaration implies an external driver, which
     cannot be attached to a reg. These rules catch that error early. */

	| K_input var_type unsigned_signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, NetNet::PINPUT,
				 IVL_VT_NO_TYPE, 0);
		  yyerror(@2, "error: reg variables cannot be inputs.");
		}

	| K_inout var_type unsigned_signed_opt range_opt list_of_identifiers ';'
		{ pform_makewire(@1, $4, $3, $5, $2, NetNet::PINOUT,
				 IVL_VT_NO_TYPE, 0);
		  yyerror(@2, "error: reg variables cannot be inouts.");
		}

	| port_direction unsigned_signed_opt range_opt delay3_opt error ';'
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
     begin/end. Careful to pass attributes to the block_item_decl. */

  | attribute_list_opt { attributes_in_context = $1; } block_item_decl
      { delete attributes_in_context;
	attributes_in_context = 0;
      }

  /* */

	| K_defparam defparam_assign_list ';'

  /* Most gate types have an optional drive strength and optional
     two/three-value delay. These rules handle the different cases.
     We check that the actual number of delays is correct later. */

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

  /* The switch type gates do not support a strength. */
	| attribute_list_opt switchtype gate_instance_list ';'
		{ pform_makegates($2, str_strength, 0, $3, $1);
		}

	| attribute_list_opt switchtype delay3 gate_instance_list ';'
		{ pform_makegates($2, str_strength, $3, $4, $1);
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

	| K_pullup '(' dr_strength1 ',' dr_strength0 ')' gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLUP, $3, 0, $7, 0);
		}

	| K_pullup '(' dr_strength0 ',' dr_strength1 ')' gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLUP, $5, 0, $7, 0);
		}

	| K_pulldown '(' dr_strength0 ')' gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLDOWN, $3, 0, $5, 0);
		}

	| K_pulldown '(' dr_strength1 ',' dr_strength0 ')' gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLDOWN, $5, 0, $7, 0);
		}

	| K_pulldown '(' dr_strength0 ',' dr_strength1 ')' gate_instance_list ';'
		{ pform_makegates(PGBuiltin::PULLDOWN, $3, 0, $7, 0);
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

  | attribute_list_opt K_always statement_item
      { PProcess*tmp = pform_make_behavior(IVL_PR_ALWAYS, $3, $1);
	FILE_NAME(tmp, @2);
      }
  | attribute_list_opt K_initial statement_item
      { PProcess*tmp = pform_make_behavior(IVL_PR_INITIAL, $3, $1);
	FILE_NAME(tmp, @2);
      }
  | attribute_list_opt K_final statement_item
      { PProcess*tmp = pform_make_behavior(IVL_PR_FINAL, $3, $1);
	FILE_NAME(tmp, @2);
      }

  | attribute_list_opt K_analog analog_statement
      { pform_make_analog_behavior(@2, IVL_PR_ALWAYS, $3); }

  | class_declaration

  | task_declaration

  | function_declaration

  /* A generate region can contain further module items. Actually, it
     is supposed to be limited to certain kinds of module items, but
     the semantic tests will check that for us. Do check that the
     generate/endgenerate regions do not nest. Generate schemes nest,
     but generate regions do not. */

  | K_generate module_item_list_opt K_endgenerate
     { // Test for bad nesting. I understand it, but it is illegal.
       if (pform_parent_generate()) {
	     cerr << @1 << ": error: Generate/endgenerate regions cannot nest." << endl;
	     cerr << @1 << ":      : Try removing optional generate/endgenerate keywords," << endl;
	     cerr << @1 << ":      : or move them to surround the parent generate scheme." << endl;
	     error_count += 1;
	}
      }

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
	if (generation_flag > GN_VER2001 && warn_anachronisms) {
	      warn_count += 1;
	      cerr << @2 << ": warning: Anachronistic use of begin/end to surround generate schemes." << endl;
	}
      }
  | K_generate K_begin ':' IDENTIFIER {
	pform_start_generate_nblock(@2, $4);
      } module_item_list_opt K_end K_endgenerate
      { /* Detect and warn about anachronistic named begin/end use */
	if (generation_flag > GN_VER2001 && warn_anachronisms) {
	      warn_count += 1;
	      cerr << @2 << ": warning: Anachronistic use of named begin/end to surround generate schemes." << endl;
	}
	pform_endgenerate();
      }

  /* 1364-2001 and later allow specparam declarations outside specify blocks. */

  | attribute_list_opt K_specparam specparam_decl ';'

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
		  delete[] $3;
		  delete[] $5;
		}
	| KK_attribute '(' error ')' ';'
		{ yyerror(@1, "error: Malformed $attribute parameter list."); }

	| K_timeunit_check  TIME_LITERAL ';'
		{ pform_set_timeunit($2, true, true); }
	| K_timeprecision_check TIME_LITERAL ';'
		{ pform_set_timeprecision($2, true, true); }
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
  | K_bool  { $$ = IVL_VT_BOOL; /* Icarus Verilog xtypes */}
  | K_bit   { $$ = IVL_VT_BOOL; /* IEEE1800 / IEEE1364-2009 */}
  | K_real  { $$ = IVL_VT_REAL; }
;

bit_logic
  : K_logic { $$ = IVL_VT_LOGIC; }
  | K_bit   { $$ = IVL_VT_BOOL; /* IEEE1800 / IEEE1364-2009 */}
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
	| K_wone    { $$ = NetNet::UNRESOLVED_WIRE;
		      cerr << @1.text << ":" << @1.first_line << ": warning: "
		              "'wone' is deprecated, please use 'uwire' "
		              "instead." << endl;
		    }
	| K_uwire   { $$ = NetNet::UNRESOLVED_WIRE; }
	;

var_type
	: K_reg { $$ = NetNet::REG; }
	;

param_type
  : 
      { param_active_range = 0;
        param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | range
      { param_active_range = $1;
        param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_signed
      { param_active_range = 0;
	param_active_signed = true;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_signed range
      { param_active_range = $2;
	param_active_signed = true;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_integer
      { param_active_range = make_range_from_width(integer_width);
	param_active_signed = true;
	param_active_type = IVL_VT_LOGIC;
      }
  | K_time
      { param_active_range = make_range_from_width(64);
	param_active_signed = false;
	param_active_type = IVL_VT_LOGIC;
      }
  | real_or_realtime
      { param_active_range = 0;
	param_active_signed = true;
	param_active_type = IVL_VT_REAL;
      }
  | atom2_type
      { param_active_range = make_range_from_width($1);
	param_active_signed = true;
	param_active_type = IVL_VT_BOOL;
      }
  ;

  /* parameter and localparam assignment lists are broken into 
     separate BNF so that I can call slightly different parameter
     handling code. localparams parse the same as parameters, they
     just behave differently when someone tries to override them. */

parameter_assign_list
  : parameter_assign
  | parameter_assign_list ',' parameter_assign
  ;

localparam_assign_list
  : localparam_assign
  | localparam_assign_list ',' localparam_assign
  ;

parameter_assign
  : IDENTIFIER '=' expression parameter_value_ranges_opt
      { PExpr*tmp = $3;
	pform_set_parameter(@1, lex_strings.make($1), param_active_type,
			    param_active_signed, param_active_range, tmp, $4);
	delete[]$1;
      }
  ;

localparam_assign
  : IDENTIFIER '=' expression
      { PExpr*tmp = $3;
	pform_set_localparam(@1, lex_strings.make($1), param_active_type,
			     param_active_signed, param_active_range, tmp);
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
		  lst->by_order = new list<PExpr*>;
		  lst->by_order->push_back(tmp);
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
      { list<named_pexpr_t>*tmp = new list<named_pexpr_t>;
	tmp->push_back(*$1);
	delete $1;
	$$ = tmp;
      }
  | parameter_value_byname_list ',' parameter_value_byname
      { list<named_pexpr_t>*tmp = $1;
	tmp->push_back(*$3);
	delete $3;
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
	| '.' IDENTIFIER
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make($2);
		  tmp->parm = new PEIdent(lex_strings.make($2), true);
		  FILE_NAME(tmp->parm, @1);
		  delete[]$2;
		  $$ = tmp;
		}
	| K_DOTSTAR
		{ named_pexpr_t*tmp = new named_pexpr_t;
		  tmp->name = lex_strings.make("*");
		  tmp->parm = 0;
		  $$ = tmp;
		}
	;

port_name_list
  : port_name_list ',' port_name
      { list<named_pexpr_t>*tmp = $1;
        tmp->push_back(*$3);
	delete $3;
	$$ = tmp;
      }
  | port_name
      { list<named_pexpr_t>*tmp = new list<named_pexpr_t>;
        tmp->push_back(*$1);
	delete $1;
	$$ = tmp;
      }
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

  /* The range is a list of variable dimensions. */
range
  : variable_dimension
      { $$ = $1; }
  | range variable_dimension
      { list<pform_range_t>*tmp = $1;
	if ($2) {
	      tmp->splice(tmp->end(), *$2);
	      delete $2;
	}
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
  : variable_dimension
      { $$ = $1; }
  | dimensions variable_dimension
      { list<pform_range_t> *tmp = $1;
	if ($2) {
	      tmp->splice(tmp->end(), *$2);
	      delete $2;
	}
	$$ = tmp;
      }
  ;

  /* This is used to express the return type of a function. This is
     not quite right, and should be replaced with a variant that uses
     the data_type rule. This will get us by for now. */
function_range_or_type_opt
  : unsigned_signed_opt range_opt
      { /* the default type is reg unsigned and no range */
	$$.type = PTF_REG;
	$$.range = $2;
	if ($1)
	      $$.type = PTF_REG_S;
      }
  | K_reg unsigned_signed_opt range_opt
      { /* the default type is reg unsigned and no range */
	$$.type = PTF_REG;
	$$.range = $3;
	if ($2)
	      $$.type = PTF_REG_S;
      }
  | bit_logic unsigned_signed_opt range_opt
      { /* the default type is bit/logic unsigned and no range */
	$$.type  = PTF_REG;
	$$.range = $3;
	if ($2)
	      $$.type = PTF_REG_S;
      }
  | K_integer  { $$.range = 0;  $$.type = PTF_INTEGER; }
  | K_real     { $$.range = 0;  $$.type = PTF_REAL; }
  | K_realtime { $$.range = 0;  $$.type = PTF_REALTIME; }
  | K_string   { $$.range = 0;  $$.type = PTF_STRING; }
  | K_time     { $$.range = 0;  $$.type = PTF_TIME; }
  | K_void     { $$.range = 0;  $$.type = PTF_VOID; }
  | atom2_type { $$.range = make_range_from_width($1); $$.type = PTF_ATOM2_S; }
  | atom2_type K_signed  { $$.range = make_range_from_width($1); $$.type = PTF_ATOM2_S; }
  | atom2_type K_unsigned { $$.range = make_range_from_width($1); $$.type = PTF_ATOM2; }
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
	pform_set_reg_idx(ident_name, $2);
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

net_variable
  : IDENTIFIER dimensions_opt
      { perm_string name = lex_strings.make($1);
	pform_makewire(@1, name, NetNet::IMPLICIT,
		       NetNet::NOT_A_PORT, IVL_VT_NO_TYPE, 0);
	pform_set_reg_idx(name, $2);
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
	: K_specparam specparam_decl ';'
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
	| K_ifnone specify_edge_path_decl ';'
		{ yyerror(@1, "Sorry: ifnone with an edge-sensitive path is "
		              "not supported.");
		  yyerrok;
		}
	| K_Sfullskew '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		  delete $9;
		}
	| K_Shold '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		}
	| K_Snochange '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		  delete $9;
		}
	| K_Speriod '(' spec_reference_event ',' delay_value
	  spec_notifier_opt ')' ';'
		{ delete $5;
		}
	| K_Srecovery '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		}
	| K_Srecrem '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		  delete $9;
		}
	| K_Sremoval '(' spec_reference_event ',' spec_reference_event
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
	| K_Sskew '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		}
	| K_Stimeskew '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value spec_notifier_opt ')' ';'
		{ delete $7;
		}
	| K_Swidth '(' spec_reference_event ',' delay_value ',' expression
	  spec_notifier_opt ')' ';'
		{ delete $5;
		  delete $7;
		}
	| K_Swidth '(' spec_reference_event ',' delay_value ')' ';'
		{ delete $5;
		}
	| K_pulsestyle_onevent specify_path_identifiers ';'
		{ delete $2;
		}
	| K_pulsestyle_ondetect specify_path_identifiers ';'
		{ delete $2;
		}
	| K_showcancelled specify_path_identifiers ';'
		{ delete $2;
		}
	| K_noshowcancelled specify_path_identifiers ';'
		{ delete $2;
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
                { list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($3);
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
                { list<PExpr*>*tmp = new list<PExpr*>;
		  tmp->push_back($3);
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
		  pform_set_specparam(@1, lex_strings.make($1),
		                      param_active_range, tmp);
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
		  pform_set_specparam(@1, lex_strings.make($1),
		                      param_active_range, tmp);
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

specparam_decl
  : specparam_list
  | range
      { param_active_range = $1; }
    specparam_list
      { param_active_range = 0; }
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
		{ args_after_notifier = 0; }
	| ','  hierarchy_identifier
		{ args_after_notifier = 0; delete $2; }
	| spec_notifier ','
		{  args_after_notifier += 1; }
	| spec_notifier ',' hierarchy_identifier
		{ args_after_notifier += 1;
		  if (args_after_notifier >= 3)  {
                    cerr << @3 << ": warning: timing checks are not supported "
		                  "and delayed signal \"" << *$3
		         << "\" will not be driven." << endl;
		  }
                  delete $3; }
  /* How do we match this path? */
	| IDENTIFIER
		{ args_after_notifier = 0; delete[]$1; }
	;


statement_item /* This is roughly statement_item in the LRM */

  /* assign and deassign statements are procedural code to do
     structural assignments, and to turn that structural assignment
     off. This is stronger than any other assign, but weaker than the
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
     name. These are handled by pushing the scope name, then matching
     the declarations. The scope is popped at the end of the block. */

  | K_begin K_end
      { PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_begin statement_or_null_list K_end
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
    statement_or_null_list_opt K_end
      { pform_pop_scope();
	assert(! current_block_stack.empty());
	PBlock*tmp = current_block_stack.top();
	current_block_stack.pop();
	if ($6) tmp->set_statement(*$6);
	delete[]$3;
	delete $6;
	$$ = tmp;
      }

  /* fork-join blocks are very similar to begin-end blocks. In fact,
     from the parser's perspective there is no real difference. All we
     need to do is remember that this is a parallel block so that the
     code generator can do the right thing. */

  | K_fork K_join
      { PBlock*tmp = new PBlock(PBlock::BL_PAR);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_fork statement_or_null_list K_join
      { PBlock*tmp = new PBlock(PBlock::BL_PAR);
	FILE_NAME(tmp, @1);
	tmp->set_statement(*$2);
	delete $2;
	$$ = tmp;
      }
  | K_fork ':' IDENTIFIER
      { PBlock*tmp = pform_push_block_scope($3, PBlock::BL_PAR);
	FILE_NAME(tmp, @1);
	current_block_stack.push(tmp);
      }
    block_item_decls_opt
    statement_or_null_list_opt K_join
      { pform_pop_scope();
        assert(! current_block_stack.empty());
	PBlock*tmp = current_block_stack.top();
	current_block_stack.pop();
	if ($6) tmp->set_statement(*$6);
	delete[]$3;
	delete $6;
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

  | loop_statement { $$ = $1; }

  | jump_statement { $$ = $1; }

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
  /* SytemVerilog adds the compressed_statement */

  | compressed_statement ';'
      { $$ = $1; }

  /* increment/decrement expressions can also be statements. When used
     as statements, we can rewrite a++ as a += 1, and so on. */

  | inc_or_dec_expression ';'
      { $$ = pform_compressed_assign_from_inc_dec(@1, $1); }

  /* */

  | delay1 statement_or_null
      { PExpr*del = $1->front();
	assert($1->size() == 1);
	delete $1;
	PDelayStatement*tmp = new PDelayStatement(del, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  | event_control statement_or_null
      { PEventStatement*tmp = $1;
	if (tmp == 0) {
	      yyerror(@1, "error: Invalid event control.");
	      $$ = 0;
	} else {
	      tmp->set_statement($2);
	      $$ = tmp;
	}
      }
  | '@' '*' statement_or_null
      { PEventStatement*tmp = new PEventStatement;
	FILE_NAME(tmp, @1);
	tmp->set_statement($3);
	$$ = tmp;
      }
  | '@' '(' '*' ')' statement_or_null
      { PEventStatement*tmp = new PEventStatement;
	FILE_NAME(tmp, @1);
	tmp->set_statement($5);
	$$ = tmp;
      }

  /* Various assignment statements */

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
	if (pform_in_program_block())
	      yyerror(@2, "Non-blocking assignments not permitted in program blocks.");
      }
  | error K_LE expression ';'
      { yyerror(@2, "Syntax in assignment statement l-value.");
	yyerrok;
	$$ = new PNoop;
	if (pform_in_program_block())
	      yyerror(@2, "Non-blocking assignments not permitted in program blocks.");
      }
  | lpvalue '=' delay1 expression ';'
      { PExpr*del = $3->front(); $3->pop_front();
	assert($3->empty());
	PAssign*tmp = new PAssign($1,del,$4);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_LE delay1 expression ';'
      { PExpr*del = $3->front(); $3->pop_front();
	assert($3->empty());
	PAssignNB*tmp = new PAssignNB($1,del,$4);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	if (pform_in_program_block())
	      yyerror(@2, "Non-blocking assignments not permitted in program blocks.");
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
	if (pform_in_program_block())
	      yyerror(@2, "Non-blocking assignments not permitted in program blocks.");
      }
  | lpvalue K_LE K_repeat '(' expression ')' event_control expression ';'
      { PAssignNB*tmp = new PAssignNB($1,$5,$7,$8);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	if (pform_in_program_block())
	      yyerror(@2, "Non-blocking assignments not permitted in program blocks.");
      }

  /* The IEEE1800 standard defines dynamic_array_new assignment as a
     different rule from regular assignment. That implies that the
     dynamic_array_new is not an expression in general, which makes
     some sense. Elaboration should make sure the lpvalue is an array name. */

  | lpvalue '=' dynamic_array_new ';'
      { PAssign*tmp = new PAssign($1,$3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  /* The class new and dynamic array new expressions are special, so
     sit in rules of their own. */

  | lpvalue '=' class_new ';'
      { PAssign*tmp = new PAssign($1,$3);
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
		{ list<PExpr*>pt;
		  PCallTask*tmp = new PCallTask(lex_strings.make($1), pt);
		  FILE_NAME(tmp,@1);
		  delete[]$1;
		  $$ = tmp;
		}

  | hierarchy_identifier '(' expression_list_with_nuls ')' ';'
      { PCallTask*tmp = new PCallTask(*$1, *$3);
	FILE_NAME(tmp, @1);
	delete $1;
	delete $3;
	$$ = tmp;
      }

  | hierarchy_identifier K_with '{' constraint_block_item_list_opt '}' ';'
      { /* ....randomize with { <constraints> } */
	if ($1 && peek_tail_name(*$1) == "randomize") {
	      if (!gn_system_verilog())
		    yyerror(@2, "error: Randomize with constraint requires SystemVerilog.");
	      else
		    yyerror(@2, "sorry: Randomize with constraint not supported.");
	} else {
	      yyerror(@2, "error: Constraint block can only be applied to randomize method.");
	}
	list<PExpr*>pt;
	PCallTask*tmp = new PCallTask(*$1, pt);
	FILE_NAME(tmp, @1);
	delete $1;
	$$ = tmp;
      }

  | implicit_class_handle '.' hierarchy_identifier '(' expression_list_with_nuls ')' ';'
      { PCallTask*tmp = new PCallTask(*$3, *$5);
	yyerror(@1, "sorry: Implicit class handle not supported in front of task names.");
	FILE_NAME(tmp, @1);
	delete $3;
	delete $5;
	$$ = tmp;
      }

  | hierarchy_identifier ';'
      { list<PExpr*>pt;
	PCallTask*tmp = new PCallTask(*$1, pt);
	FILE_NAME(tmp, @1);
	delete $1;
	$$ = tmp;
      }

  | hierarchy_identifier '(' error ')' ';'
      { yyerror(@3, "error: Syntax error in task arguments.");
	list<PExpr*>pt;
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

compressed_statement
  : lpvalue K_PLUS_EQ expression
      { PAssign*tmp = new PAssign($1, '+', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_MINUS_EQ expression
      { PAssign*tmp = new PAssign($1, '-', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_MUL_EQ expression
      { PAssign*tmp = new PAssign($1, '*', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_DIV_EQ expression
      { PAssign*tmp = new PAssign($1, '/', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_MOD_EQ expression
      { PAssign*tmp = new PAssign($1, '%', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_AND_EQ expression
      { PAssign*tmp = new PAssign($1, '&', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_OR_EQ expression
      { PAssign*tmp = new PAssign($1, '|', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_XOR_EQ expression
      { PAssign*tmp = new PAssign($1, '^', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_LS_EQ expression
      { PAssign  *tmp = new PAssign($1, 'l', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_RS_EQ expression
      { PAssign*tmp = new PAssign($1, 'r', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue K_RSS_EQ expression
      { PAssign  *tmp = new PAssign($1, 'R', $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
	;


statement_or_null_list_opt
  : statement_or_null_list
      { $$ = $1; }
  |
      { $$ = 0; }
  ;

statement_or_null_list
  : statement_or_null_list statement_or_null
      { vector<Statement*>*tmp = $1;
	tmp->push_back($2);
	$$ = tmp;
      }
  | statement_or_null
      { vector<Statement*>*tmp = new vector<Statement*>(1);
	tmp->at(0) = $1;
	$$ = tmp;
      }
  ;

analog_statement
  : branch_probe_expression K_CONTRIBUTE expression ';'
      { $$ = pform_contribution_statement(@2, $1, $3); }
  ;

  /* Task items are, other than the statement, task port items and
     other block items. */
task_item
        : block_item_decl  { $$ = new svector<PWire*>(0); }
        | tf_port_declaration   { $$ = $1; }
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

tf_port_list_opt
  : tf_port_list { $$ = $1; }
  |                     { $$ = 0; }
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
	| 'h' { $$ = 'h'; }
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

  /* Many keywords can be optional in the syntax, although their
     presence is significant. This is a fairly common pattern so
     collect those rules here. */

K_automatic_opt: K_automatic { $$ = true; } | { $$ = false;} ;
K_packed_opt   : K_packed    { $$ = true; } | { $$ = false; } ;
K_reg_opt      : K_reg       { $$ = true; } | { $$ = false; } ;
K_static_opt   : K_static    { $$ = true; } | { $$ = false; } ;
K_virtual_opt  : K_virtual   { $$ = true; } | { $$ = false; } ;
