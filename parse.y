
%{
/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2013 / Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <climits>
# include  <cstdarg>
# include  "parse_misc.h"
# include  "compiler.h"
# include  "pform.h"
# include  "Statement.h"
# include  "PSpec.h"
# include  "PTimingCheck.h"
# include  "PPackage.h"
# include  <stack>
# include  <cstring>
# include  <sstream>
# include  <memory>

using namespace std;

class PSpecPath;

extern void lex_end_table();

static data_type_t* param_data_type = 0;
static bool param_is_local = false;
static bool param_is_type = false;
static bool in_gen_region = false;
static std::list<pform_range_t>* specparam_active_range = 0;

/* Port declaration lists use this structure for context. */
static struct {
      NetNet::Type port_net_type;
      NetNet::PortType port_type;
      data_type_t* data_type;
} port_declaration_context = {NetNet::NONE, NetNet::NOT_A_PORT, 0};

/* Modport port declaration lists use this structure for context. */
enum modport_port_type_t { MP_NONE, MP_SIMPLE, MP_TF, MP_CLOCKING };
static struct {
      modport_port_type_t type;
      union {
	    NetNet::PortType direction;
	    bool is_import;
      };
} last_modport_port = { MP_NONE, {NetNet::NOT_A_PORT}};

/* The task and function rules need to briefly hold the pointer to the
   task/function that is currently in progress. */
static PTask* current_task = 0;
static PFunction* current_function = 0;
static stack<PBlock*> current_block_stack;

/* The variable declaration rules need to know if a lifetime has been
   specified. */
static LexicalScope::lifetime_t var_lifetime;

static void check_in_gen_region(const struct vlltype &loc)
{
      if (in_gen_region) {
	    cerr << loc << ": error: generate/endgenerate regions cannot nest." << endl;
	    error_count += 1;
      }
      in_gen_region = true;
}

static pform_name_t* pform_create_this(void)
{
      name_component_t name (perm_string::literal(THIS_TOKEN));
      pform_name_t*res = new pform_name_t;
      res->push_back(name);
      return res;
}

static pform_name_t* pform_create_super(void)
{
      name_component_t name (perm_string::literal(SUPER_TOKEN));
      pform_name_t*res = new pform_name_t;
      res->push_back(name);
      return res;
}

/* The rules sometimes push attributes into a global context where
   sub-rules may grab them. This makes parser rules a little easier to
   write in some cases. */
static std::list<named_pexpr_t>*attributes_in_context = 0;

/* Later version of bison (including 1.35) will not compile in stack
   extension if the output is compiled with C++ and either the YYSTYPE
   or YYLTYPE are provided by the source code. However, I can get the
   old behavior back by defining these symbols. */
# define YYSTYPE_IS_TRIVIAL 1
# define YYLTYPE_IS_TRIVIAL 1

/* Recent version of bison expect that the user supply a
   YYLLOC_DEFAULT macro that makes up a yylloc value from existing
   values. I need to supply an explicit version to account for the
   text field, that otherwise won't be copied.

   The YYLLOC_DEFAULT blends the file range for the tokens of Rhs
   rule, which has N tokens.
*/
# define YYLLOC_DEFAULT(Current, Rhs, N)  do {				\
      if (N) {							        \
	    (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	    (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	    (Current).last_line    = YYRHSLOC (Rhs, N).last_line;	\
	    (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	    (Current).lexical_pos  = YYRHSLOC (Rhs, 1).lexical_pos;	\
	    (Current).text         = YYRHSLOC (Rhs, 1).text;		\
      } else {								\
	    (Current).first_line   = YYRHSLOC (Rhs, 0).last_line;	\
	    (Current).first_column = YYRHSLOC (Rhs, 0).last_column;	\
	    (Current).last_line    = YYRHSLOC (Rhs, 0).last_line;	\
	    (Current).last_column  = YYRHSLOC (Rhs, 0).last_column;	\
	    (Current).lexical_pos  = YYRHSLOC (Rhs, 0).lexical_pos;	\
	    (Current).text         = YYRHSLOC (Rhs, 0).text;		\
      }									\
   } while (0)

/*
 * These are some common strength pairs that are used as defaults when
 * the user is not otherwise specific.
 */
static const struct str_pair_t pull_strength = { IVL_DR_PULL,  IVL_DR_PULL };
static const struct str_pair_t str_strength = { IVL_DR_STRONG, IVL_DR_STRONG };

static std::list<pform_port_t>* make_port_list(char*id, unsigned idn,
					       std::list<pform_range_t>*udims,
					       PExpr*expr)
{
      std::list<pform_port_t>*tmp = new std::list<pform_port_t>;
      pform_ident_t tmp_name = { lex_strings.make(id), idn };
      tmp->push_back(pform_port_t(tmp_name, udims, expr));
      delete[]id;
      return tmp;
}
static std::list<pform_port_t>* make_port_list(list<pform_port_t>*tmp,
					       char*id, unsigned idn,
					       std::list<pform_range_t>*udims,
					       PExpr*expr)
{
      pform_ident_t tmp_name = { lex_strings.make(id), idn };
      tmp->push_back(pform_port_t(tmp_name, udims, expr));
      delete[]id;
      return tmp;
}

static std::list<pform_ident_t>* list_from_identifier(char*id, unsigned idn)
{
      std::list<pform_ident_t>*tmp = new std::list<pform_ident_t>;
      tmp->push_back({ lex_strings.make(id), idn });
      delete[]id;
      return tmp;
}

static std::list<pform_ident_t>* list_from_identifier(list<pform_ident_t>*tmp,
                                                      char*id, unsigned idn)
{
      tmp->push_back({ lex_strings.make(id), idn });
      delete[]id;
      return tmp;
}

template <class T> void append(vector<T>&out, const std::vector<T>&in)
{
      for (size_t idx = 0 ; idx < in.size() ; idx += 1)
	    out.push_back(in[idx]);
}

/*
 * The parser parses an empty argument list as an argument list with an single
 * empty argument. Fix this up here and replace it with an empty list.
 */
static void argument_list_fixup(list<named_pexpr_t> *lst)
{
      if (lst->size() == 1 && lst->front().name.nil() && !lst->front().parm)
	    lst->clear();
}

/*
 * This is a shorthand for making a PECallFunction that takes a single
 * arg. This is used by some of the code that detects built-ins.
 */
static PECallFunction*make_call_function(perm_string tn, PExpr*arg)
{
      std::vector<named_pexpr_t> parms(1);
      parms[0].parm = arg;
      parms[0].set_line(*arg);
      PECallFunction*tmp = new PECallFunction(tn, parms);
      return tmp;
}

static PECallFunction*make_call_function(perm_string tn, PExpr*arg1, PExpr*arg2)
{
      std::vector<named_pexpr_t> parms(2);
      parms[0].parm = arg1;
      parms[0].set_line(*arg1);
      parms[1].parm = arg2;
      parms[1].set_line(*arg2);
      PECallFunction*tmp = new PECallFunction(tn, parms);
      return tmp;
}

static std::list<named_pexpr_t>* make_named_numbers(const struct vlltype &loc,
						    perm_string name,
						    long first, long last,
						    PExpr *val = nullptr)
{
      std::list<named_pexpr_t>*lst = new std::list<named_pexpr_t>;
      named_pexpr_t tmp;
	// We are counting up.
      if (first <= last) {
	    for (long idx = first ; idx <= last ; idx += 1) {
		  ostringstream buf;
		  buf << name.str() << idx << ends;
		  tmp.name = lex_strings.make(buf.str());
		  tmp.parm = val;
		  FILE_NAME(&tmp, loc);
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
		  FILE_NAME(&tmp, loc);
		  val = 0;
		  lst->push_back(tmp);
	    }
      }
      return lst;
}

static std::list<named_pexpr_t>* make_named_number(const struct vlltype &loc,
						   perm_string name,
						   PExpr *val = nullptr)
{
      std::list<named_pexpr_t>*lst = new std::list<named_pexpr_t>;
      named_pexpr_t tmp;
      tmp.name = name;
      tmp.parm = val;
      FILE_NAME(&tmp, loc);
      lst->push_back(tmp);
      return lst;
}

static long check_enum_seq_value(const YYLTYPE&loc, verinum *arg, bool zero_ok)
{
      long value = 1;
	// We can never have an undefined value in an enumeration name
	// declaration sequence.
      if (! arg->is_defined()) {
	    yyerror(loc, "error: Undefined value used in enum name sequence.");
	// We can never have a negative value in an enumeration name
	// declaration sequence.
      } else if (arg->is_negative()) {
	    yyerror(loc, "error: Negative value used in enum name sequence.");
      } else {
	    value = arg->as_ulong();
	      // We cannot have a zero enumeration name declaration count.
	    if (! zero_ok && (value == 0)) {
		  yyerror(loc, "error: Zero count used in enum name sequence.");
		  value = 1;
	    }
      }
      return value;
}

static void check_end_label(const struct vlltype&loc, const char *type,
			    const char *begin, const char *end)
{
      if (!end)
	    return;

      if (!begin)
	    yyerror(loc, "error: Unnamed %s must not have end label.", type);
      else if (strcmp(begin, end) != 0)
	    yyerror(loc, "error: %s end label `%s` doesn't match %s name"
	                 " `%s`.", type, end, type, begin);

      if (!gn_system_verilog())
	    yyerror(loc, "error: %s end label requires SystemVerilog.", type);

      delete[] end;
}

static void check_for_loop(const struct vlltype&loc, PExpr*init,
			   PExpr*cond, Statement*step)
{
      if (generation_flag >= GN_VER2012)
	    return;

      if (!init)
	    yyerror(loc, "error: null for-loop initialization requires "
                         "SystemVerilog 2012 or later.");
      if (!cond)
	    yyerror(loc, "error: null for-loop termination requires "
                         "SystemVerilog 2012 or later.");
      if (!step)
	    yyerror(loc, "error: null for-loop step requires "
                         "SystemVerilog 2012 or later.");
}

static void current_task_set_statement(const YYLTYPE&loc, std::vector<Statement*>*s)
{
      if (s == 0) {
	      /* if the statement list is null, then the parser
		 detected the case that there are no statements in the
		 task. If this is SystemVerilog, handle it as an
		 an empty block. */
	    pform_requires_sv(loc, "Task body with no statements");

	    PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	    FILE_NAME(tmp, loc);
	    current_task->set_statement(tmp);
	    return;
      }
      assert(s);

        /* An empty vector represents one or more null statements. Handle
           this as a simple null statement. */
      if (s->empty())
            return;

	/* A vector of 1 is handled as a simple statement. */
      if (s->size() == 1) {
	    current_task->set_statement((*s)[0]);
	    return;
      }

      pform_requires_sv(loc, "Task body with multiple statements");

      PBlock*tmp = new PBlock(PBlock::BL_SEQ);
      FILE_NAME(tmp, loc);
      tmp->set_statement(*s);
      current_task->set_statement(tmp);
}

static void current_function_set_statement(const YYLTYPE&loc, std::vector<Statement*>*s)
{
      if (s == 0) {
	      /* if the statement list is null, then the parser
		 detected the case that there are no statements in the
		 task. If this is SystemVerilog, handle it as an
		 an empty block. */
	    pform_requires_sv(loc, "Function body with no statements");

	    PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	    FILE_NAME(tmp, loc);
	    current_function->set_statement(tmp);
	    return;
      }
      assert(s);

        /* An empty vector represents one or more null statements. Handle
           this as a simple null statement. */
      if (s->empty())
            return;

	/* A vector of 1 is handled as a simple statement. */
      if (s->size() == 1) {
	    current_function->set_statement((*s)[0]);
	    return;
      }

      pform_requires_sv(loc, "Function body with multiple statements");

      PBlock*tmp = new PBlock(PBlock::BL_SEQ);
      FILE_NAME(tmp, loc);
      tmp->set_statement(*s);
      current_function->set_statement(tmp);
}

static void port_declaration_context_init(void)
{
      port_declaration_context.port_type = NetNet::PINOUT;
      port_declaration_context.port_net_type = NetNet::IMPLICIT;
      port_declaration_context.data_type = nullptr;
}

Module::port_t *module_declare_port(const YYLTYPE&loc, char *id,
			            NetNet::PortType port_type,
				    NetNet::Type net_type,
				    data_type_t *data_type,
				    std::list<pform_range_t> *unpacked_dims,
				    PExpr *default_value,
				    std::list<named_pexpr_t> *attributes)
{
      pform_ident_t name = { lex_strings.make(id), loc.lexical_pos };
      delete[] id;

      Module::port_t *port = pform_module_port_reference(loc, name.first);

      switch (port_type) {
	  case NetNet::PINOUT:
	    if (default_value)
		  yyerror(loc, "error: Default port value not allowed for inout ports.");
	    if (unpacked_dims) {
		  yyerror(loc, "sorry: Inout ports with unpacked dimensions are not supported.");
		  delete unpacked_dims;
		  unpacked_dims = nullptr;
	    }
	    break;
	  case NetNet::PINPUT:
	    if (default_value) {
		  pform_requires_sv(loc, "Input port default value");
		  port->default_value = default_value;
	    }
	    break;
	  case NetNet::POUTPUT:
	    if (default_value)
		  pform_make_var_init(loc, name, default_value);

	      // Output types without an implicit net type but with a data type
	      // are variables. Unlike the other port types, which are nets in
	      // that case.
	    if (net_type == NetNet::IMPLICIT) {
		  if (vector_type_t*dtype = dynamic_cast<vector_type_t*> (data_type)) {
			if (!dtype->implicit_flag)
			      net_type = NetNet::IMPLICIT_REG;
		  } else if (data_type) {
			net_type = NetNet::IMPLICIT_REG;
		  }
	    }
	    break;
	  default:
	    break;
      }

      pform_module_define_port(loc, name, port_type, net_type, data_type,
			       unpacked_dims, attributes);

      port_declaration_context.port_type = port_type;
      port_declaration_context.port_net_type = net_type;
      port_declaration_context.data_type = data_type;

      return port;
}

%}

%union {
      bool flag;

      char letter;
      int  int_val;

      enum atom_type_t::type_code atom_type;

	/* text items are C strings allocated by the lexor using
	   strdup. They can be put into lists with the texts type. */
      char*text;
      std::list<perm_string>*perm_strings;

      std::list<pform_ident_t>*identifiers;

      std::list<pform_port_t>*port_list;

      std::vector<pform_tf_port_t>* tf_ports;

      pform_name_t*pform_name;

      ivl_discipline_t discipline;

      hname_t*hier;

      std::list<std::string>*strings;

      struct str_pair_t drive;

      PCase::Item*citem;
      std::vector<PCase::Item*>*citems;

      lgate*gate;
      std::vector<lgate>*gates;

      Module::port_t *mport;
      LexicalScope::range_t* value_range;
      std::vector<Module::port_t*>*mports;

      std::list<PLet::let_port_t*>*let_port_lst;
      PLet::let_port_t*let_port_itm;

      named_pexpr_t*named_pexpr;
      std::list<named_pexpr_t>*named_pexprs;
      struct parmvalue_t*parmvalue;
      std::list<pform_range_t>*ranges;

      PExpr*expr;
      std::list<PExpr*>*exprs;

      PEEvent*event_expr;
      std::vector<PEEvent*>*event_exprs;

      ivl_case_quality_t case_quality;
      NetNet::Type nettype;
      PGBuiltin::Type gatetype;
      NetNet::PortType porttype;
      ivl_variable_type_t vartype;
      PBlock::BL_TYPE join_keyword;

      PWire*wire;
      std::vector<PWire*>*wires;

      PCallTask *subroutine_call;

      PEventStatement*event_statement;
      Statement*statement;
      std::vector<Statement*>*statement_list;

      decl_assignment_t*decl_assignment;
      std::list<decl_assignment_t*>*decl_assignments;

      struct_member_t*struct_member;
      std::list<struct_member_t*>*struct_members;
      struct_type_t*struct_type;

      data_type_t*data_type;
      class_type_t*class_type;
      real_type_t::type_t real_type;
      property_qualifier_t property_qualifier;
      PPackage*package;

      struct {
	    char*text;
	    typedef_t*type;
      } type_identifier;

      struct {
	    data_type_t*type;
	    std::list<named_pexpr_t> *args;
      } class_declaration_extends;

      struct {
	    char*text;
	    PExpr*expr;
      } genvar_iter;

      struct {
	    bool packed_flag;
	    bool signed_flag;
      } packed_signing;

      verinum* number;

      verireal* realtime;

      PSpecPath* specpath;
      std::list<index_component_t> *dimensions;

      PTimingCheck::event_t* timing_check_event;
      PTimingCheck::optional_args_t* spec_optional_args;

      LexicalScope::lifetime_t lifetime;

      enum typedef_t::basic_type typedef_basic_type;
};

%token <text>      IDENTIFIER SYSTEM_IDENTIFIER STRING TIME_LITERAL
%token <type_identifier> TYPE_IDENTIFIER
%token <package>   PACKAGE_IDENTIFIER
%token <discipline> DISCIPLINE_IDENTIFIER
%token <text>   PATHPULSE_IDENTIFIER
%token <number> BASED_NUMBER DEC_NUMBER UNBASED_NUMBER
%token <realtime> REALTIME
%token K_PLUS_EQ K_MINUS_EQ K_INCR K_DECR
%token K_LE K_GE K_EG K_EQ K_NE K_CEQ K_CNE K_WEQ K_WNE K_LP K_LS K_RS K_RSS K_SG
 /* K_CONTRIBUTE is <+, the contribution assign. */
%token K_CONTRIBUTE
%token K_PO_POS K_PO_NEG K_POW
%token K_PSTAR K_STARP K_DOTSTAR
%token K_LOR K_LAND K_NAND K_NOR K_NXOR K_TRIGGER K_NB_TRIGGER K_LEQUIV
%token K_SCOPE_RES
%token K_edge_descriptor

%token K_CONSTRAINT_IMPL

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

 /* The new tokens from 1800-2009. */
%token K_accept_on K_checker K_endchecker K_eventually K_global K_implies
%token K_let K_nexttime K_reject_on K_restrict K_s_always K_s_eventually
%token K_s_nexttime K_s_until K_s_until_with K_strong K_sync_accept_on
%token K_sync_reject_on K_unique0 K_until K_until_with K_untyped K_weak

 /* The new tokens from 1800-2012. */
%token K_implements K_interconnect K_nettype K_soft

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

%type <flag>    from_exclude block_item_decls_opt
%type <number>  number pos_neg_number
%type <flag>    signing unsigned_signed_opt signed_unsigned_opt
%type <flag>    import_export
%type <flag>    K_genvar_opt K_static_opt K_virtual_opt K_const_opt
%type <flag>    udp_reg_opt edge_operator
%type <drive>   drive_strength drive_strength_opt dr_strength0 dr_strength1
%type <letter>  udp_input_sym udp_output_sym
%type <text>    udp_input_list udp_sequ_entry udp_comb_entry
%type <identifiers> udp_input_declaration_list
%type <strings> udp_entry_list udp_comb_entry_list udp_sequ_entry_list
%type <strings> udp_body
%type <identifiers> udp_port_list
%type <wires>   udp_port_decl udp_port_decls
%type <statement> udp_initial udp_init_opt

%type <wire> net_variable
%type <wires> net_variable_list

%type <text> event_variable label_opt class_declaration_endlabel_opt
%type <text> block_identifier_opt
%type <text> identifier_name
%type <identifiers> event_variable_list
%type <identifiers> list_of_identifiers
%type <perm_strings> loop_variables
%type <port_list> list_of_port_identifiers list_of_variable_port_identifiers

%type <decl_assignments> net_decl_assigns
%type <decl_assignment> net_decl_assign

%type <mport> port port_opt port_reference port_reference_list
%type <mport> port_declaration
%type <mports> list_of_ports module_port_list_opt list_of_port_declarations module_attribute_foreign
%type <value_range> parameter_value_range parameter_value_ranges
%type <value_range> parameter_value_ranges_opt
%type <expr> value_range_expression

%type <named_pexprs> enum_name_list enum_name
%type <data_type> enum_data_type enum_base_type

%type <tf_ports> tf_item_declaration tf_item_list tf_item_list_opt
%type <tf_ports> tf_port_declaration tf_port_item tf_port_item_list
%type <tf_ports> tf_port_list tf_port_list_opt tf_port_list_parens_opt

%type <named_pexpr> named_expression named_expression_opt port_name
%type <named_pexprs> port_name_list parameter_value_byname_list
%type <exprs> port_conn_expression_list_with_nuls

%type <named_pexpr> attribute
%type <named_pexprs> attribute_list attribute_instance_list attribute_list_opt

%type <named_pexpr> argument
%type <named_pexprs> argument_list
%type <named_pexprs> argument_list_parens argument_list_parens_opt

%type <citem>  case_item
%type <citems> case_items

%type <gate>  gate_instance
%type <gates> gate_instance_list
%type <let_port_lst> let_port_list_opt let_port_list
%type <let_port_itm> let_port_item

%type <pform_name> hierarchy_identifier implicit_class_handle class_hierarchy_identifier
%type <pform_name> spec_notifier_opt spec_notifier
%type <timing_check_event> spec_reference_event
%type <spec_optional_args> setuphold_opt_args recrem_opt_args setuphold_recrem_opt_notifier
%type <spec_optional_args> setuphold_recrem_opt_timestamp_cond setuphold_recrem_opt_timecheck_cond
%type <spec_optional_args> setuphold_recrem_opt_delayed_reference setuphold_recrem_opt_delayed_data
%type <spec_optional_args> timeskew_opt_args fullskew_opt_args
%type <spec_optional_args> timeskew_fullskew_opt_notifier timeskew_fullskew_opt_event_based_flag
%type <spec_optional_args> timeskew_fullskew_opt_remain_active_flag

%type <expr>  assignment_pattern expression expression_opt expr_mintypmax
%type <expr>  expr_primary_or_typename expr_primary
%type <expr>  class_new dynamic_array_new
%type <expr>  var_decl_initializer_opt initializer_opt
%type <expr>  inc_or_dec_expression inside_expression lpvalue
%type <expr>  branch_probe_expression streaming_concatenation
%type <expr>  delay_value delay_value_simple
%type <exprs> delay1 delay3 delay3_opt delay_value_list
%type <exprs> expression_list_with_nuls expression_list_proper
%type <exprs> cont_assign cont_assign_list

%type <decl_assignment> variable_decl_assignment
%type <decl_assignments> list_of_variable_decl_assignments

%type <data_type>  data_type data_type_opt data_type_or_implicit data_type_or_implicit_or_void
%type <data_type>  data_type_or_implicit_no_opt
%type <data_type>  simple_type_or_string let_formal_type
%type <data_type>  packed_array_data_type
%type <data_type>  ps_type_identifier
%type <data_type>  simple_packed_type
%type <data_type>  class_scope
%type <struct_member>  struct_union_member
%type <struct_members> struct_union_member_list
%type <struct_type>    struct_data_type
%type <packed_signing> packed_signing

%type <class_declaration_extends> class_declaration_extends_opt

%type <property_qualifier> class_item_qualifier property_qualifier
%type <property_qualifier> class_item_qualifier_list property_qualifier_list
%type <property_qualifier> class_item_qualifier_opt property_qualifier_opt
%type <property_qualifier> random_qualifier

%type <ranges> variable_dimension
%type <ranges> dimensions_opt dimensions

%type <nettype>  net_type net_type_opt net_type_or_var net_type_or_var_opt
%type <gatetype> gatetype switchtype
%type <porttype> port_direction port_direction_opt
%type <vartype> integer_vector_type
%type <parmvalue> parameter_value_opt

%type <event_exprs> event_expression_list
%type <event_expr> event_expression
%type <event_statement> event_control
%type <statement> statement statement_item statement_or_null
%type <statement> compressed_statement
%type <statement> loop_statement for_step for_step_opt jump_statement
%type <statement> concurrent_assertion_statement
%type <statement> deferred_immediate_assertion_statement
%type <statement> simple_immediate_assertion_statement
%type <statement> procedural_assertion_statement
%type <statement_list> statement_or_null_list statement_or_null_list_opt

%type <statement> analog_statement

%type <subroutine_call> subroutine_call

%type <join_keyword> join_keyword

%type <letter> spec_polarity
%type <perm_strings>  specify_path_identifiers

%type <specpath> specify_simple_path specify_simple_path_decl
%type <specpath> specify_edge_path specify_edge_path_decl

%type <real_type> non_integer_type
%type <int_val> assert_or_assume
%type <int_val> deferred_mode
%type <atom_type> atom_type
%type <int_val> module_start module_end

%type <lifetime> lifetime lifetime_opt

%type <case_quality> unique_priority

%type <genvar_iter> genvar_iteration

%type <package> package_scope

%type <letter> compressed_operator

%type <typedef_basic_type> typedef_basic_type

%token K_TAND
%nonassoc K_PLUS_EQ K_MINUS_EQ K_MUL_EQ K_DIV_EQ K_MOD_EQ K_AND_EQ K_OR_EQ
%nonassoc K_XOR_EQ K_LS_EQ K_RS_EQ K_RSS_EQ K_NB_TRIGGER
%right K_TRIGGER K_LEQUIV
%right '?' ':' K_inside
%left K_LOR
%left K_LAND
%left '|'
%left '^' K_NXOR K_NOR
%left '&' K_NAND
%left K_EQ K_NE K_CEQ K_CNE K_WEQ K_WNE
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

 /* to resolve timeunits declaration/redeclaration ambiguity */
%nonassoc no_timeunits_declaration
%nonassoc one_timeunits_declaration
%nonassoc K_timeunit K_timeprecision

%%


  /* IEEE1800-2005: A.1.2 */
  /* source_text ::= [ timeunits_declaration ] { description } */
source_text
  : timeunits_declaration_opt
      { pform_set_scope_timescale(yyloc); }
    description_list
  | /* empty */
  ;

assert_or_assume
  : K_assert
      { $$ = 1; } /* IEEE1800-2012: Table 20-7 */
  | K_assume
      { $$ = 4; } /* IEEE1800-2012: Table 20-7 */
  ;

assertion_item /* IEEE1800-2012: A.6.10 */
  : concurrent_assertion_item
  | deferred_immediate_assertion_item
  ;

assignment_pattern /* IEEE1800-2005: A.6.7.1 */
  : K_LP expression_list_proper '}'
      { PEAssignPattern*tmp = new PEAssignPattern(*$2);
	FILE_NAME(tmp, @1);
	delete $2;
	$$ = tmp;
      }
  | K_LP '}'
      { PEAssignPattern*tmp = new PEAssignPattern;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

  /* Some rules have a ... [ block_identifier ':' ] ... part. This
     implements it in a LALR way. */
block_identifier_opt /* */
  : IDENTIFIER ':'
      { $$ = $1; }
  |
      { $$ = 0; }
  ;

class_declaration /* IEEE1800-2005: A.1.2 */
  : K_virtual_opt K_class lifetime_opt identifier_name class_declaration_extends_opt ';'
      { /* Up to 1800-2017 the grammar in the LRM allowed an optional lifetime
	 * qualifier for class declarations. But the LRM never specified what
	 * this qualifier should do. Starting with 1800-2023 the qualifier has
	 * been removed from the grammar. Allow it for backwards compatibility,
	 * but print a warning.
	 */
	if ($3 != LexicalScope::INHERITED) {
	      cerr << @1 << ": warning: Class lifetime qualifier is deprecated "
			    "and has no effect." << endl;
	      warn_count += 1;
	}
	perm_string name = lex_strings.make($4);
	class_type_t *class_type= new class_type_t(name);
	FILE_NAME(class_type, @4);
	pform_set_typedef(@4, name, class_type, nullptr);
	pform_start_class_declaration(@2, class_type, $5.type, $5.args, $1);
      }
    class_items_opt K_endclass
      { // Process a class.
	pform_end_class_declaration(@9);
      }
    class_declaration_endlabel_opt
      { // Wrap up the class.
	check_end_label(@11, "class", $4, $11);
	delete[] $4;
      }
  ;

class_constraint /* IEEE1800-2005: A.1.8 */
  : constraint_prototype
  | constraint_declaration
  ;

  // This is used in places where a new type can be declared or an existig type
  // is referenced. E.g. typedefs.
identifier_name
  : IDENTIFIER { $$ = $1; }
  | TYPE_IDENTIFIER { $$ = $1.text; }
  ;

  /* The endlabel after a class declaration is a little tricky because
     the class name is detected by the lexor as a TYPE_IDENTIFIER if it
     does indeed match a name. */
class_declaration_endlabel_opt
  : ':' identifier_name { $$ = $2; }
  | { $$ = 0; }
  ;

  /* This rule implements [ extends class_type ] in the
     class_declaration. It is not a rule of its own in the LRM.

     Note that for this to be correct, the identifier after the
     extends keyword must be a class name. Therefore, match
     TYPE_IDENTIFIER instead of IDENTIFIER, and this rule will return
     a data_type. */

class_declaration_extends_opt /* IEEE1800-2005: A.1.2 */
  : K_extends ps_type_identifier argument_list_parens_opt
      { $$.type = $2;
	$$.args = $3;
      }
  |
      { $$ = {nullptr, nullptr};
      }
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
  : method_qualifier_opt K_function K_new
      { assert(current_function==0);
	current_function = pform_push_constructor_scope(@3);
      }
    tf_port_list_parens_opt ';'
    block_item_decls_opt
    statement_or_null_list_opt
    K_endfunction endnew_opt
      { current_function->set_ports($5);
	pform_set_constructor_return(current_function);
	pform_set_this_class(@3, current_function);
	current_function_set_statement(@3, $8);
	pform_pop_scope();
	current_function = 0;
      }

    /* IEEE1800-2017: A.1.9 Class items: Class properties... */

  | property_qualifier_opt data_type list_of_variable_decl_assignments ';'
      { pform_class_property(@2, $1, $2, $3); }

  | K_const class_item_qualifier_opt data_type list_of_variable_decl_assignments ';'
      { pform_class_property(@1, $2 | property_qualifier_t::make_const(), $3, $4); }

    /* IEEEE1800-2017: A.1.9 Class items: class_item ::= { property_qualifier} data_declaration */

    /* TODO: Restrict the access based on the property qualifier. */
  | property_qualifier_opt type_declaration

    /* IEEE1800-1017: A.1.9 Class items: Class methods... */

  | method_qualifier_opt task_declaration
      { /* The task_declaration rule puts this into the class */ }

  | method_qualifier_opt function_declaration
      { /* The function_declaration rule puts this into the class */ }

    /* External class method definitions... */

  | K_extern method_qualifier_opt K_function K_new tf_port_list_parens_opt ';'
      { yyerror(@1, "sorry: External constructors are not yet supported."); }
  | K_extern method_qualifier_opt K_function data_type_or_implicit_or_void
    IDENTIFIER tf_port_list_parens_opt ';'
      { yyerror(@1, "sorry: External methods are not yet supported.");
	delete[] $5;
      }
  | K_extern method_qualifier_opt K_task IDENTIFIER tf_port_list_parens_opt ';'
      { yyerror(@1, "sorry: External methods are not yet supported.");
	delete[] $4;
      }

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

  | parameter_declaration

    /* Empty class item */
  | ';'

  | error ';'
      { yyerror(@2, "error: Invalid class item.");
	yyerrok;
      }

  ;

class_item_qualifier /* IEEE1800-2005 A.1.8 */
  : K_static     { $$ = property_qualifier_t::make_static(); }
  | K_protected  { $$ = property_qualifier_t::make_protected(); }
  | K_local      { $$ = property_qualifier_t::make_local(); }
  ;

class_item_qualifier_list
  : class_item_qualifier_list class_item_qualifier { $$ = $1 | $2; }
  | class_item_qualifier { $$ = $1; }
  ;

class_item_qualifier_opt
  : class_item_qualifier_list { $$ = $1; }
  | { $$ = property_qualifier_t::make_none(); }
  ;

class_scope
  : ps_type_identifier K_SCOPE_RES { $$ = $1; }

class_new /* IEEE1800-2005 A.2.4 */
  : K_new argument_list_parens_opt
      { PENewClass*tmp = new PENewClass(*$2);
	FILE_NAME(tmp, @1);
	delete $2;
	$$ = tmp;
      }
    // This can't be a class_scope_opt because it will lead to shift/reduce
    // conflicts with array_new
  | class_scope K_new argument_list_parens_opt
      { PENewClass *new_expr = new PENewClass(*$3, $1);
	FILE_NAME(new_expr, @2);
	delete $3;
	$$ = new_expr;
      }
  | K_new hierarchy_identifier
      { PEIdent*tmpi = new PEIdent(*$2, @2.lexical_pos);
	FILE_NAME(tmpi, @2);
	PENewCopy*tmp = new PENewCopy(tmpi);
	FILE_NAME(tmp, @1);
	delete $2;
	$$ = tmp;
      }
  ;

  /* The concurrent_assertion_item pulls together the
     concurrent_assertion_statement and checker_instantiation rules. */

concurrent_assertion_item /* IEEE1800-2012 A.2.10 */
  : block_identifier_opt concurrent_assertion_statement
      { delete $1;
	delete $2;
      }
  ;

concurrent_assertion_statement /* IEEE1800-2012 A.2.10 */
  : assert_or_assume K_property '(' property_spec ')' statement_or_null %prec less_than_K_else
      { /* */
	if (gn_unsupported_assertions_flag) {
	      yyerror(@1, "sorry: concurrent_assertion_item not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
        $$ = 0;
      }
  | assert_or_assume K_property '(' property_spec ')' K_else statement_or_null
      { /* */
	if (gn_unsupported_assertions_flag) {
	      yyerror(@1, "sorry: concurrent_assertion_item not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
        $$ = 0;
      }
  | assert_or_assume K_property '(' property_spec ')' statement_or_null K_else statement_or_null
      { /* */
	if (gn_unsupported_assertions_flag) {
	      yyerror(@1, "sorry: concurrent_assertion_item not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
        $$ = 0;
      }
  | K_cover K_property '(' property_spec ')' statement_or_null
      { /* */
	if (gn_unsupported_assertions_flag) {
	      yyerror(@1, "sorry: concurrent_assertion_item not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
        $$ = 0;
      }
      /* For now, cheat, and use property_spec for the sequence specification.
         They are syntactically identical. */
  | K_cover K_sequence '(' property_spec ')' statement_or_null
      { /* */
	if (gn_unsupported_assertions_flag) {
	      yyerror(@1, "sorry: concurrent_assertion_item not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
        $$ = 0;
      }
  | K_restrict K_property '(' property_spec ')' ';'
      { /* */
	if (gn_unsupported_assertions_flag) {
	      yyerror(@2, "sorry: concurrent_assertion_item not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
        $$ = 0;
      }
  | assert_or_assume K_property '(' error ')' statement_or_null %prec less_than_K_else
      { yyerrok;
        yyerror(@2, "error: Error in property_spec of concurrent assertion item.");
        $$ = 0;
      }
  | assert_or_assume K_property '(' error ')' K_else statement_or_null
      { yyerrok;
        yyerror(@2, "error: Error in property_spec of concurrent assertion item.");
        $$ = 0;
      }
  | assert_or_assume K_property '(' error ')' statement_or_null K_else statement_or_null
      { yyerrok;
        yyerror(@2, "error: Error in property_spec of concurrent assertion item.");
        $$ = 0;
      }
  | K_cover K_property '(' error ')' statement_or_null
      { yyerrok;
        yyerror(@2, "error: Error in property_spec of concurrent assertion item.");
        $$ = 0;
      }
  | K_cover K_sequence '(' error ')' statement_or_null
      { yyerrok;
        yyerror(@2, "error: Error in property_spec of concurrent assertion item.");
        $$ = 0;
      }
  | K_restrict K_property '(' error ')' ';'
      { yyerrok;
        yyerror(@2, "error: Error in property_spec of concurrent assertion item.");
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
  | expression constraint_trigger
  | K_if '(' expression ')' constraint_set %prec less_than_K_else
  | K_if '(' expression ')' constraint_set K_else constraint_set
  | K_foreach '(' IDENTIFIER '[' loop_variables ']' ')' constraint_set
  ;

constraint_trigger
  : K_CONSTRAINT_IMPL '{' constraint_expression_list '}'
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

data_declaration /* IEEE1800-2005: A.2.1.3 */
   : attribute_list_opt K_const_opt data_type list_of_variable_decl_assignments ';'
      { data_type_t *data_type = $3;
	if (!data_type) {
	      data_type = new vector_type_t(IVL_VT_LOGIC, false, 0);
	      FILE_NAME(data_type, @3);
	}
	pform_makewire(@3, 0, str_strength, $4, NetNet::IMPLICIT_REG, data_type,
		       $1, $2);
      }
  | attribute_list_opt K_const_opt K_var data_type_or_implicit list_of_variable_decl_assignments ';'
      { data_type_t *data_type = $4;
	if (!data_type) {
	      data_type = new vector_type_t(IVL_VT_LOGIC, false, 0);
	      FILE_NAME(data_type, @3);
	}
	pform_make_var(@3, $5, data_type, $1, $2);
      }
  | attribute_list_opt K_event event_variable_list ';'
      { if ($3) pform_make_events(@2, $3);
      }
  | attribute_list_opt package_import_declaration
  ;

package_scope
  : PACKAGE_IDENTIFIER K_SCOPE_RES
      { lex_in_package_scope($1);
        $$ = $1;
      }
  ;

ps_type_identifier /* IEEE1800-2017: A.9.3 */
 : TYPE_IDENTIFIER
      { pform_set_type_referenced(@1, $1.text);
	delete[]$1.text;
	$$ = new typeref_t($1.type);
	FILE_NAME($$, @1);
      }
  | package_scope TYPE_IDENTIFIER
      { lex_in_package_scope(0);
	$$ = new typeref_t($2.type, $1);
	FILE_NAME($$, @2);
	delete[] $2.text;
      }
  ;

/* Data types that can have packed dimensions directly attached to it */
packed_array_data_type /* IEEE1800-2005: A.2.2.1 */
  : enum_data_type
      { $$ = $1; }
  | struct_data_type
      { if (!$1->packed_flag) {
	      yyerror(@1, "sorry: Unpacked structs not supported.");
        }
	$$ = $1;
      }
  | ps_type_identifier
  ;

simple_packed_type /* Integer and vector types */
  : integer_vector_type unsigned_signed_opt dimensions_opt
      { vector_type_t*tmp = new vector_type_t($1, $2, $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | atom_type signed_unsigned_opt
      { atom_type_t*tmp = new atom_type_t($1, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_time unsigned_signed_opt
      { atom_type_t*tmp = new atom_type_t(atom_type_t::TIME, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

data_type /* IEEE1800-2005: A.2.2.1 */
  : simple_packed_type
      { $$ = $1;
      }
  | non_integer_type
      { real_type_t*tmp = new real_type_t($1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | packed_array_data_type dimensions_opt
      { if ($2) {
	      parray_type_t*tmp = new parray_type_t($1, $2);
	      FILE_NAME(tmp, @1);
	      $$ = tmp;
        } else {
	      $$ = $1;
        }
      }
  | K_string
      { string_type_t*tmp = new string_type_t;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

/* Data type or nothing, but not implicit */
data_type_opt
  : data_type { $$ = $1; }
  | { $$ = 0; }

  /* The data_type_or_implicit rule is a little more complex then the
     rule documented in the IEEE format syntax in order to allow for
     signaling the special case that the data_type is completely
     absent. The context may need that information to decide to resort
     to left context. */

scalar_vector_opt /*IEEE1800-2005: optional support for packed array */
  : K_vectored
      { /* Ignore */ }
  | K_scalared
      { /* Ignore */ }
  |
      { /* Ignore */ }
  ;

data_type_or_implicit /* IEEE1800-2005: A.2.2.1 */
  : data_type_or_implicit_no_opt
  | { $$ = nullptr; }

data_type_or_implicit_no_opt
  : data_type
      { $$ = $1; }
  | signing dimensions_opt
      { vector_type_t*tmp = new vector_type_t(IVL_VT_LOGIC, $1, $2);
	tmp->implicit_flag = true;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | scalar_vector_opt dimensions
      { vector_type_t*tmp = new vector_type_t(IVL_VT_LOGIC, false, $2);
	tmp->implicit_flag = true;
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;


data_type_or_implicit_or_void
  : data_type_or_implicit
      { $$ = $1; }
  | K_void
      { void_type_t*tmp = new void_type_t;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

deferred_immediate_assertion_item /* IEEE1800-2012: A.6.10 */
  : block_identifier_opt deferred_immediate_assertion_statement
      { delete $1;
	delete $2;
      }
  ;

deferred_immediate_assertion_statement /* IEEE1800-2012 A.6.10 */
  : assert_or_assume deferred_mode '(' expression ')' statement_or_null %prec less_than_K_else
      {
	if (gn_unsupported_assertions_flag) {
	      yyerror(@1, "sorry: Deferred assertions are not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
	delete $4;
	delete $6;
	$$ = 0;
      }
  | assert_or_assume deferred_mode '(' expression ')' K_else statement_or_null
      {
	if (gn_unsupported_assertions_flag) {
	      yyerror(@1, "sorry: Deferred assertions are not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
	delete $4;
	delete $7;
	$$ = 0;
      }
  | assert_or_assume deferred_mode '(' expression ')' statement_or_null K_else statement_or_null
      {
	if (gn_unsupported_assertions_flag) {
	      yyerror(@1, "sorry: Deferred assertions are not supported."
		      " Try -gno-assertions or -gsupported-assertions"
		      " to turn this message off.");
	}
	delete $4;
	delete $6;
	delete $8;
	$$ = 0;
      }
  | K_cover deferred_mode '(' expression ')' statement_or_null
      {
	  /* Coverage collection is not currently supported. */
	delete $4;
	delete $6;
	$$ = 0;
      }
  | assert_or_assume deferred_mode '(' error ')' statement_or_null %prec less_than_K_else
      { yyerror(@1, "error: Malformed conditional expression.");
	$$ = $6;
      }
  | assert_or_assume deferred_mode '(' error ')' K_else statement_or_null
      { yyerror(@1, "error: Malformed conditional expression.");
	$$ = $7;
      }
  | assert_or_assume deferred_mode '(' error ')' statement_or_null K_else statement_or_null
      { yyerror(@1, "error: Malformed conditional expression.");
	$$ = $6;
      }
  | K_cover deferred_mode '(' error ')' statement_or_null
      { yyerror(@1, "error: Malformed conditional expression.");
	$$ = $6;
      }
  ;

deferred_mode
  : '#' DEC_NUMBER
      { if (!$2->is_zero()) {
	      yyerror(@2, "error: Delay value must be zero for deferred assertion.");
	}
        delete $2;
	$$ = 0; }
  | K_final
      { $$ = 1; }
  ;

  /* NOTE: The "module" rule of the description combines the
     module_declaration, program_declaration, and interface_declaration
     rules from the standard description. */

description /* IEEE1800-2005: A.1.2 */
  : module
  | udp_primitive
  | config_declaration
  | nature_declaration
  | package_declaration
  | discipline_declaration
  | package_item
  | KK_attribute '(' IDENTIFIER ',' STRING ',' STRING ')'
      { perm_string tmp3 = lex_strings.make($3);
	pform_set_type_attrib(tmp3, $5, $7);
	delete[] $3;
	delete[] $5;
      }
  | ';'
      { }
  ;

description_list
  : description
  | description_list description
  ;


   /* This implements the [ : IDENTIFIER ] part of the constructor
      rule documented in IEEE1800-2005: A.1.8 */
endnew_opt : ':' K_new | ;

  /* The dynamic_array_new rule is kinda like an expression, but it is
     treated differently by rules that use this "expression". Watch out! */

dynamic_array_new /* IEEE1800-2005: A.2.4 */
  : K_new '[' expression ']'
      { $$ = new PENewArray($3, 0);
	FILE_NAME($$, @1);
      }
  | K_new '[' expression ']' '(' expression ')'
      { $$ = new PENewArray($3, $6);
	FILE_NAME($$, @1);
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

for_step_opt
  : for_step { $$ = $1; }
  | { $$ = nullptr; }
  ;

  /* The function declaration rule matches the function declaration
     header, then pushes the function scope. This causes the
     definitions in the func_body to take on the scope of the function
     instead of the module. */
function_declaration /* IEEE1800-2005: A.2.6 */
  : K_function lifetime_opt data_type_or_implicit_or_void IDENTIFIER ';'
      { assert(current_function == 0);
	current_function = pform_push_function_scope(@1, $4, $2);
      }
    tf_item_list_opt
    statement_or_null_list_opt
    K_endfunction
      { current_function->set_ports($7);
	current_function->set_return($3);
	current_function_set_statement($8? @8 : @4, $8);
	pform_set_this_class(@4, current_function);
	pform_pop_scope();
	current_function = 0;
      }
    label_opt
      { // Last step: check any closing name.
	check_end_label(@11, "function", $4, $11);
	delete[]$4;
      }

  | K_function lifetime_opt data_type_or_implicit_or_void IDENTIFIER
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
	pform_set_this_class(@4, current_function);
	pform_pop_scope();
	current_function = 0;
	if ($7 == 0) {
	      pform_requires_sv(@4, "Functions with no ports");
	}
      }
    label_opt
      { // Last step: check any closing name.
	check_end_label(@14, "function", $4, $14);
	delete[]$4;
      }

  /* Detect and recover from some errors. */

  | K_function lifetime_opt data_type_or_implicit_or_void IDENTIFIER error K_endfunction
      { /* */
	if (current_function) {
	      pform_pop_scope();
	      current_function = 0;
	}
	assert(current_function == 0);
	yyerror(@1, "error: Syntax error defining function.");
	yyerrok;
      }
    label_opt
      { // Last step: check any closing name.
	check_end_label(@8, "function", $4, $8);
	delete[]$4;
      }

  ;

genvar_iteration /* IEEE1800-2012: A.4.2 */
  : IDENTIFIER '=' expression
      { $$.text = $1;
        $$.expr = $3;
      }
  | IDENTIFIER compressed_operator expression
      { $$.text = $1;
        $$.expr = pform_genvar_compressed(@1, $1, $2, $3);;
      }
  | IDENTIFIER K_INCR
      { $$.text = $1;
        $$.expr = pform_genvar_inc_dec(@1, $1, true);
      }
  | IDENTIFIER K_DECR
      { $$.text = $1;
        $$.expr = pform_genvar_inc_dec(@1, $1, false);
      }
  | K_INCR IDENTIFIER
      { $$.text = $2;
        $$.expr = pform_genvar_inc_dec(@1, $2, true);
      }
  | K_DECR IDENTIFIER
      { $$.text = $2;
        $$.expr = pform_genvar_inc_dec(@1, $2, false);
      }
  ;

import_export /* IEEE1800-2012: A.2.9 */
  : K_import { $$ = true; }
  | K_export { $$ = false; }
  ;

implicit_class_handle /* IEEE1800-2005: A.8.4 */
  : K_this '.' { $$ = pform_create_this(); }
  | K_super '.' { $$ = pform_create_super(); }
  | K_this '.' K_super '.' { $$ = pform_create_super(); }
  ;

/* `this` or `super` followed by an identifier */
class_hierarchy_identifier
  : implicit_class_handle hierarchy_identifier
      { $1->splice($1->end(), *$2);
	delete $2;
	$$ = $1;
      }
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
  : K_reg   { $$ = IVL_VT_LOGIC; } /* A synonym for logic. */
  | K_bit   { $$ = IVL_VT_BOOL; }
  | K_logic { $$ = IVL_VT_LOGIC; }
  | K_bool  { $$ = IVL_VT_BOOL; } /* Icarus Verilog xtypes extension */
  ;

join_keyword /* IEEE1800-2005: A.6.3 */
  : K_join
      { $$ = PBlock::BL_PAR; }
  | K_join_none
      { $$ = PBlock::BL_JOIN_NONE; }
  | K_join_any
      { $$ = PBlock::BL_JOIN_ANY; }
  ;

jump_statement /* IEEE1800-2005: A.6.5 */
  : K_break ';'
      { PBreak*tmp = new PBreak;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_continue ';'
      { PContinue*tmp = new PContinue;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_return ';'
      { PReturn*tmp = new PReturn(0);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_return expression ';'
      { PReturn*tmp = new PReturn($2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

lifetime /* IEEE1800-2005: A.2.1.3 */
  : K_automatic { $$ = LexicalScope::AUTOMATIC; }
  | K_static    { $$ = LexicalScope::STATIC; }
  ;

lifetime_opt /* IEEE1800-2005: A.2.1.3 */
  : lifetime { $$ = $1; }
  |          { $$ = LexicalScope::INHERITED; }
  ;

  /* Loop statements are kinds of statements. */

loop_statement /* IEEE1800-2005: A.6.8 */
  : K_for '(' lpvalue '=' expression ';' expression_opt ';' for_step_opt ')'
    statement_or_null
      { check_for_loop(@1, $5, $7, $9);
	PForStatement*tmp = new PForStatement($3, $5, $7, $9, $11);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

      // The initialization statement is optional.
  | K_for '(' ';' expression_opt ';' for_step_opt ')'
    statement_or_null
      { check_for_loop(@1, nullptr, $4, $6);
	PForStatement*tmp = new PForStatement(nullptr, nullptr, $4, $6, $8);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

      // Handle for_variable_declaration syntax by wrapping the for(...)
      // statement in a synthetic named block. We can name the block
      // after the variable that we are creating, that identifier is
      // safe in the controlling scope.
  | K_for '(' K_var_opt data_type IDENTIFIER '=' expression ';' expression_opt ';' for_step_opt ')'
      { static unsigned for_counter = 0;
	char for_block_name [64];
	snprintf(for_block_name, sizeof for_block_name, "$ivl_for_loop%u", for_counter);
	for_counter += 1;
	PBlock*tmp = pform_push_block_scope(@1, for_block_name, PBlock::BL_SEQ);
	current_block_stack.push(tmp);

	list<decl_assignment_t*>assign_list;
	decl_assignment_t*tmp_assign = new decl_assignment_t;
	tmp_assign->name = { lex_strings.make($5), @5.lexical_pos };
	assign_list.push_back(tmp_assign);
	pform_make_var(@5, &assign_list, $4);
      }
    statement_or_null
      { pform_name_t tmp_hident;
	tmp_hident.push_back(name_component_t(lex_strings.make($5)));

	PEIdent*tmp_ident = pform_new_ident(@5, tmp_hident);
	FILE_NAME(tmp_ident, @5);

	check_for_loop(@1, $7, $9, $11);
	PForStatement*tmp_for = new PForStatement(tmp_ident, $7, $9, $11, $14);
	FILE_NAME(tmp_for, @1);

	pform_pop_scope();
	vector<Statement*>tmp_for_list (1);
	tmp_for_list[0] = tmp_for;
	PBlock*tmp_blk = current_block_stack.top();
	current_block_stack.pop();
	tmp_blk->set_statement(tmp_for_list);
	$$ = tmp_blk;
	delete[]$5;
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

  | K_do statement_or_null K_while '(' expression ')' ';'
      { PDoWhile*tmp = new PDoWhile($5, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

      // When matching a foreach loop, implicitly create a named block
      // to hold the definitions for the index variables.
  | K_foreach '(' IDENTIFIER '[' loop_variables ']' ')'
      { static unsigned foreach_counter = 0;
	char for_block_name[64];
	snprintf(for_block_name, sizeof for_block_name, "$ivl_foreach%u", foreach_counter);
	foreach_counter += 1;

	PBlock*tmp = pform_push_block_scope(@1, for_block_name, PBlock::BL_SEQ);
	current_block_stack.push(tmp);

	pform_make_foreach_declarations(@1, $5);
      }
    statement_or_null
      { PForeach*tmp_for = pform_make_foreach(@1, $3, $5, $9);

	pform_pop_scope();
	vector<Statement*>tmp_for_list(1);
	tmp_for_list[0] = tmp_for;
	PBlock*tmp_blk = current_block_stack.top();
	current_block_stack.pop();
	tmp_blk->set_statement(tmp_for_list);
	$$ = tmp_blk;
      }

  /* Error forms for loop statements. */

  | K_for '(' lpvalue '=' expression ';' expression_opt ';' error ')'
    statement_or_null
      { $$ = 0;
	yyerror(@1, "error: Error in for loop step assignment.");
      }

  | K_for '(' lpvalue '=' expression ';' error ';' for_step_opt ')'
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

  | K_do statement_or_null K_while '(' error ')' ';'
      { $$ = 0;
	yyerror(@1, "error: Error in do/while loop condition.");
      }

  | K_foreach '(' IDENTIFIER '[' error ']' ')' statement_or_null
      { $$ = 0;
        yyerror(@4, "error: Errors in foreach loop variables list.");
      }
  ;


list_of_variable_decl_assignments /* IEEE1800-2005 A.2.3 */
  : variable_decl_assignment
      { std::list<decl_assignment_t*>*tmp = new std::list<decl_assignment_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  | list_of_variable_decl_assignments ',' variable_decl_assignment
      { std::list<decl_assignment_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  ;

initializer_opt
 : '=' expression { $$ = $2; }
 | { $$ = nullptr; }
 ;

var_decl_initializer_opt
 : initializer_opt
 | '=' class_new { $$ = $2; }
 | '=' dynamic_array_new { $$ = $2; }
 ;

variable_decl_assignment /* IEEE1800-2005 A.2.3 */
  : IDENTIFIER dimensions_opt var_decl_initializer_opt
      { if ($3 && pform_peek_scope()->var_init_needs_explicit_lifetime()
	    && (var_lifetime == LexicalScope::INHERITED)) {
	      cerr << @1 << ": warning: Static variable initialization requires "
			    "explicit lifetime in this context." << endl;
	      warn_count += 1;
	}

	decl_assignment_t*tmp = new decl_assignment_t;
	tmp->name = { lex_strings.make($1), @1.lexical_pos };
	if ($2) {
	      tmp->index = *$2;
	      delete $2;
	}
	tmp->expr.reset($3);
	delete[]$1;
	$$ = tmp;
      }
  ;


loop_variables /* IEEE1800-2005: A.6.8 */
  : loop_variables ',' IDENTIFIER
      { std::list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	delete[]$3;
	$$ = tmp;
      }
  | loop_variables ','
      { std::list<perm_string>*tmp = $1;
	tmp->push_back(perm_string());
	$$ = tmp;
      }
  | IDENTIFIER
      { std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  |
      { std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(perm_string());
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

modport_declaration /* IEEE1800-2012: A.2.9 */
  : K_modport
      { if (!pform_in_interface())
	      yyerror(@1, "error: modport declarations are only allowed "
			  "in interfaces.");
      }
    modport_item_list ';'

modport_item_list
  : modport_item
  | modport_item_list ',' modport_item
  ;

modport_item
  : IDENTIFIER
      { pform_start_modport_item(@1, $1); }
    '(' modport_ports_list ')'
      { pform_end_modport_item(@1); }
  ;

  /* The modport_ports_list is a LALR(2) grammar. When the parser sees a
     ',' it needs to look ahead to the next token to decide whether it is
     a continuation of the preceding modport_ports_declaration, or the
     start of a new modport_ports_declaration. bison only supports LALR(1),
     so we have to handcraft a mini parser for this part of the syntax.
     last_modport_port holds the state for this mini parser.*/

modport_ports_list
  : modport_ports_declaration
  | modport_ports_list ',' modport_ports_declaration
  | modport_ports_list ',' named_expression
      { if (last_modport_port.type == MP_SIMPLE) {
	      pform_add_modport_port(@3, last_modport_port.direction,
				     $3->name, $3->parm);
	} else {
	      yyerror(@3, "error: modport expression not allowed here.");
	}
	delete $3;
      }
  | modport_ports_list ',' modport_tf_port
      { if (last_modport_port.type != MP_TF)
	      yyerror(@3, "error: task/function declaration not allowed here.");
      }
  | modport_ports_list ',' IDENTIFIER
      { if (last_modport_port.type == MP_SIMPLE) {
	      pform_add_modport_port(@3, last_modport_port.direction,
				     lex_strings.make($3), 0);
	} else if (last_modport_port.type != MP_TF) {
	      yyerror(@3, "error: List of identifiers not allowed here.");
	}
	delete[] $3;
      }
  | modport_ports_list ','
      { yyerror(@2, "error: Superfluous comma in port declaration list."); }
  ;

modport_ports_declaration
  : attribute_list_opt port_direction IDENTIFIER
      { last_modport_port.type = MP_SIMPLE;
	last_modport_port.direction = $2;
	pform_add_modport_port(@3, $2, lex_strings.make($3), 0);
	delete[] $3;
	delete $1;
      }
  | attribute_list_opt port_direction named_expression
      { last_modport_port.type = MP_SIMPLE;
	last_modport_port.direction = $2;
	pform_add_modport_port(@3, $2, $3->name, $3->parm);
	delete $3;
	delete $1;
      }
  | attribute_list_opt import_export IDENTIFIER
      { last_modport_port.type = MP_TF;
	last_modport_port.is_import = $2;
	yyerror(@3, "sorry: modport task/function ports are not yet supported.");
	delete[] $3;
	delete $1;
      }
  | attribute_list_opt import_export modport_tf_port
      { last_modport_port.type = MP_TF;
	last_modport_port.is_import = $2;
	yyerror(@3, "sorry: modport task/function ports are not yet supported.");
	delete $1;
      }
  | attribute_list_opt K_clocking IDENTIFIER
      { last_modport_port.type = MP_CLOCKING;
	last_modport_port.direction = NetNet::NOT_A_PORT;
	yyerror(@3, "sorry: modport clocking declaration is not yet supported.");
	delete[] $3;
	delete $1;
      }
  ;

modport_tf_port
  : K_task IDENTIFIER tf_port_list_parens_opt
  | K_function data_type_or_implicit_or_void IDENTIFIER tf_port_list_parens_opt
  ;

non_integer_type /* IEEE1800-2005: A.2.2.1 */
  : K_real { $$ = real_type_t::REAL; }
  | K_realtime { $$ = real_type_t::REAL; }
  | K_shortreal { $$ = real_type_t::SHORTREAL; }
  ;

number
  : BASED_NUMBER
      { $$ = $1; based_size = 0;}
  | DEC_NUMBER
      { $$ = $1; based_size = 0;}
  | DEC_NUMBER BASED_NUMBER
      { $$ = pform_verinum_with_size($1,$2, @2.text, @2.first_line);
	based_size = 0; }
  | UNBASED_NUMBER
      { $$ = $1; based_size = 0;}
  | DEC_NUMBER UNBASED_NUMBER
      { yyerror(@1, "error: Unbased SystemVerilog literal cannot have a size.");
	$$ = $1; based_size = 0;}
  ;

open_range_list /* IEEE1800-2005 A.2.11 */
  : open_range_list ',' value_range
  | value_range
  ;

package_declaration /* IEEE1800-2005 A.1.2 */
  : K_package lifetime_opt IDENTIFIER ';'
      { pform_start_package_declaration(@1, $3, $2); }
    timeunits_declaration_opt
      { pform_set_scope_timescale(@1); }
    package_item_list_opt
    K_endpackage label_opt
      { pform_end_package_declaration(@1);
	check_end_label(@10, "package", $3, $10);
	delete[]$3;
      }
  ;

module_package_import_list_opt
  :
  | package_import_list
  ;

package_import_list
  : package_import_declaration
  | package_import_list package_import_declaration
  ;

package_import_declaration /* IEEE1800-2005 A.2.1.3 */
  : K_import package_import_item_list ';'
      { }
  ;

package_import_item
  : package_scope IDENTIFIER
      { lex_in_package_scope(0);
	pform_package_import(@1, $1, $2);
	delete[]$2;
      }
  | package_scope TYPE_IDENTIFIER
      { lex_in_package_scope(0);
	pform_package_import(@1, $1, $2.text);
	delete[]$2.text;
      }
  | package_scope '*'
      { lex_in_package_scope(0);
        pform_package_import(@1, $1, 0);
      }
  ;

package_import_item_list
  : package_import_item_list',' package_import_item
  | package_import_item
  ;

package_export_declaration /* IEEE1800-2017 A.2.1.3 */
  : K_export package_export_item_list ';'
  | K_export '*' K_SCOPE_RES '*' ';' { pform_package_export(@$, nullptr, nullptr); }
  ;

package_export_item
  : PACKAGE_IDENTIFIER K_SCOPE_RES IDENTIFIER
      { pform_package_export(@2, $1, $3);
	delete[] $3;
      }
  | PACKAGE_IDENTIFIER K_SCOPE_RES TYPE_IDENTIFIER
      { pform_package_export(@2, $1, $3.text);
	delete[] $3.text;
      }
  | PACKAGE_IDENTIFIER K_SCOPE_RES '*'
      { pform_package_export(@2, $1, nullptr);
      }
  ;

package_export_item_list
  : package_export_item_list ',' package_export_item
  | package_export_item
  ;

package_item /* IEEE1800-2005 A.1.10 */
  : timeunits_declaration
  | parameter_declaration
  | type_declaration
  | function_declaration
  | task_declaration
  | data_declaration
  | class_declaration
  | package_export_declaration
  ;

package_item_list
  : package_item_list package_item
  | package_item
  ;

package_item_list_opt : package_item_list | ;

port_direction /* IEEE1800-2005 A.1.3 */
  : K_input  { $$ = NetNet::PINPUT; }
  | K_output { $$ = NetNet::POUTPUT; }
  | K_inout  { $$ = NetNet::PINOUT; }
  | K_ref
      { $$ = NetNet::PREF;

	if (!pform_requires_sv(@1, "Reference port (ref)")) {
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

procedural_assertion_statement /* IEEE1800-2012 A.6.10 */
  : block_identifier_opt concurrent_assertion_statement
      { $$ = $2; }
  | block_identifier_opt simple_immediate_assertion_statement
      { $$ = $2; }
  | block_identifier_opt deferred_immediate_assertion_statement
      { $$ = $2; }
  ;

property_expr /* IEEE1800-2012 A.2.10 */
  : expression
  ;

  /* The property_qualifier rule is as literally described in the LRM,
     but the use is usually as { property_qualifier }, which is
     implemented by the property_qualifier_opt rule below. */

property_qualifier /* IEEE1800-2005 A.1.8 */
  : class_item_qualifier
  | random_qualifier
  ;

property_qualifier_opt /* IEEE1800-2005 A.1.8: ... { property_qualifier } */
  : property_qualifier_list { $$ = $1; }
  | { $$ = property_qualifier_t::make_none(); }
  ;

property_qualifier_list /* IEEE1800-2005 A.1.8 */
  : property_qualifier_list property_qualifier { $$ = $1 | $2; }
  | property_qualifier { $$ = $1; }
  ;

  /* The property_spec rule uses some helper rules to implement this
     rule from the LRM:
     [ clocking_event ] [ disable iff ( expression_or_dist ) ] property_expr
     This does it is a YACC friendly way. */

property_spec /* IEEE1800-2012 A.2.10 */
  : clocking_event_opt property_spec_disable_iff_opt property_expr
  ;

property_spec_disable_iff_opt /* */
  : K_disable K_iff '(' expression ')'
  |
  ;

random_qualifier /* IEEE1800-2005 A.1.8 */
  : K_rand { $$ = property_qualifier_t::make_rand(); }
  | K_randc { $$ = property_qualifier_t::make_randc(); }
  ;

signing /* IEEE1800-2005: A.2.2.1 */
  : K_signed   { $$ = true; }
  | K_unsigned { $$ = false; }
  ;

simple_immediate_assertion_statement /* IEEE1800-2012 A.6.10 */
  : assert_or_assume '(' expression ')' statement_or_null %prec less_than_K_else
      {
	if (gn_supported_assertions_flag) {
	      std::list<named_pexpr_t> arg_list;
	      PCallTask*tmp1 = new PCallTask(lex_strings.make("$error"), arg_list);
	      FILE_NAME(tmp1, @1);
	      PCondit*tmp2 = new PCondit($3, $5, tmp1);
	      FILE_NAME(tmp2, @1);
	      $$ = tmp2;
	} else {
	      delete $3;
	      delete $5;
	      $$ = 0;
	}
      }
  | assert_or_assume '(' expression ')' K_else statement_or_null
      {
	if (gn_supported_assertions_flag) {
	      PCondit*tmp = new PCondit($3, 0, $6);
	      FILE_NAME(tmp, @1);
	      $$ = tmp;
	} else {
	      delete $3;
	      delete $6;
	      $$ = 0;
	}
      }
  | assert_or_assume '(' expression ')' statement_or_null K_else statement_or_null
      {
	if (gn_supported_assertions_flag) {
	      PCondit*tmp = new PCondit($3, $5, $7);
	      FILE_NAME(tmp, @1);
	      $$ = tmp;
	} else {
	      delete $3;
	      delete $5;
	      delete $7;
	      $$ = 0;
	}
      }
  | K_cover '(' expression ')' statement_or_null
      {
	  /* Coverage collection is not currently supported. */
	delete $3;
	delete $5;
	$$ = 0;
      }
  | assert_or_assume '(' error ')' statement_or_null %prec less_than_K_else
      { yyerror(@1, "error: Malformed conditional expression.");
	$$ = $5;
      }
  | assert_or_assume '(' error ')' K_else statement_or_null
      { yyerror(@1, "error: Malformed conditional expression.");
	$$ = $6;
      }
  | assert_or_assume '(' error ')' statement_or_null K_else statement_or_null
      { yyerror(@1, "error: Malformed conditional expression.");
	$$ = $5;
      }
  | K_cover '(' error ')' statement_or_null
      { yyerror(@1, "error: Malformed conditional expression.");
	$$ = $5;
      }
  ;

simple_type_or_string /* IEEE1800-2005: A.2.2.1 */
  : integer_vector_type
      { vector_type_t*tmp = new vector_type_t($1, false, 0);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | non_integer_type
      { real_type_t*tmp = new real_type_t($1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | atom_type
      { atom_type_t*tmp = new atom_type_t($1, true);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_time
      { atom_type_t*tmp = new atom_type_t(atom_type_t::TIME, false);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_string
      { string_type_t*tmp = new string_type_t;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | ps_type_identifier
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
	if (pform_requires_sv(@2, "Streaming concatenation")) {
	      yyerror(@2, "sorry: Streaming concatenation not supported.");
	      $$ = 0;
	} else {
	      $$ = 0;
	}
      }
  ;

  /* The task declaration rule matches the task declaration
     header, then pushes the function scope. This causes the
     definitions in the task_body to take on the scope of the task
     instead of the module. */

task_declaration /* IEEE1800-2005: A.2.7 */

  : K_task lifetime_opt IDENTIFIER ';'
      { assert(current_task == 0);
	current_task = pform_push_task_scope(@1, $3, $2);
      }
    tf_item_list_opt
    statement_or_null_list_opt
    K_endtask
      { current_task->set_ports($6);
	current_task_set_statement(@3, $7);
	pform_set_this_class(@3, current_task);
	pform_pop_scope();
	current_task = 0;
	if ($7 && $7->size() > 1) {
	      pform_requires_sv(@7, "Task body with multiple statements");
	}
	delete $7;
      }
    label_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// label_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	check_end_label(@10, "task", $3, $10);
	delete[]$3;
      }

  | K_task lifetime_opt IDENTIFIER '('
      { assert(current_task == 0);
	current_task = pform_push_task_scope(@1, $3, $2);
      }
    tf_port_list_opt ')' ';'
    block_item_decls_opt
    statement_or_null_list_opt
    K_endtask
      { current_task->set_ports($6);
	current_task_set_statement(@3, $10);
	pform_set_this_class(@3, current_task);
	pform_pop_scope();
	if (generation_flag < GN_VER2005 && $6 == 0) {
	      cerr << @3 << ": warning: task definition for \"" << $3
		   << "\" has an empty port declaration list!" << endl;
	}
	current_task = 0;
	if ($10) delete $10;
      }
    label_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// label_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	check_end_label(@13, "task", $3, $13);
	delete[]$3;
      }

  | K_task lifetime_opt IDENTIFIER error K_endtask
      {
	if (current_task) {
	      pform_pop_scope();
	      current_task = 0;
	}
      }
    label_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// label_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	check_end_label(@7, "task", $3, $7);
	delete[]$3;
      }

  ;


tf_port_declaration /* IEEE1800-2005: A.2.7 */
  : port_direction K_var_opt data_type_or_implicit list_of_port_identifiers ';'
      { $$ = pform_make_task_ports(@1, $1, $3, $4, true);
      }
  ;


  /* These rules for tf_port_item are slightly expanded from the
     strict rules in the LRM to help with LALR parsing.

     NOTE: Some of these rules should be folded into the "data_type"
     variant which uses the data_type rule to match data type
     declarations. That some rules do not use the data_type production
     is a consequence of legacy. */

tf_port_item /* IEEE1800-2005: A.2.7 */

  : port_direction_opt K_var_opt data_type_or_implicit IDENTIFIER dimensions_opt initializer_opt
      { std::vector<pform_tf_port_t>*tmp;
	NetNet::PortType use_port_type = $1;
        if ((use_port_type == NetNet::PIMPLICIT) && (gn_system_verilog() || ($3 == 0)))
              use_port_type = port_declaration_context.port_type;
	list<pform_port_t>* port_list = make_port_list($4, @4.lexical_pos, $5, 0);

	if (use_port_type == NetNet::PIMPLICIT) {
	      yyerror(@1, "error: Missing task/function port direction.");
	      use_port_type = NetNet::PINPUT; // for error recovery
	}
	if (($3 == 0) && ($1==NetNet::PIMPLICIT)) {
		// Detect special case this is an undecorated
		// identifier and we need to get the declaration from
		// left context.
	      if ($5 != 0) {
		    yyerror(@5, "internal error: How can there be an unpacked range here?\n");
	      }
	      tmp = pform_make_task_ports(@4, use_port_type,
					  port_declaration_context.data_type,
					  port_list);

	} else {
		// Otherwise, the decorations for this identifier
		// indicate the type. Save the type for any right
		// context that may come later.
	      port_declaration_context.port_type = use_port_type;
	      if ($3 == 0) {
		    $3 = new vector_type_t(IVL_VT_LOGIC, false, 0);
		    FILE_NAME($3, @4);
	      }
	      port_declaration_context.data_type = $3;
	      tmp = pform_make_task_ports(@3, use_port_type, $3, port_list);
	}

	$$ = tmp;
	if ($6) {
	      pform_requires_sv(@6, "Task/function default argument");
	      assert(tmp->size()==1);
	      tmp->front().defe = $6;
	}
      }

  /* Rules to match error cases... */

  | port_direction_opt K_var_opt data_type_or_implicit IDENTIFIER error
      { yyerror(@3, "error: Error in task/function port item after port name %s.", $4);
	yyerrok;
	$$ = 0;
      }
  ;

tf_port_list /* IEEE1800-2005: A.2.7 */
  :   { port_declaration_context.port_type = gn_system_verilog() ? NetNet::PINPUT : NetNet::PIMPLICIT;
	port_declaration_context.data_type = 0;
      }
    tf_port_item_list
      { $$ = $2; }
  ;

tf_port_item_list
  : tf_port_item_list ',' tf_port_item
      { std::vector<pform_tf_port_t>*tmp;
	if ($1 && $3) {
	      size_t s1 = $1->size();
	      tmp = $1;
	      tmp->resize(tmp->size()+$3->size());
	      for (size_t idx = 0 ; idx < $3->size() ; idx += 1)
		    tmp->at(s1+idx) = $3->at(idx);
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
  | tf_port_item_list ','
      { yyerror(@2, "error: Superfluous comma in port declaration list.");
	$$ = $1;
      }
  | tf_port_item_list ';'
      { yyerror(@2, "error: ';' is an invalid port declaration separator.");
	$$ = $1;
      }
  ;

timeunits_declaration /* IEEE1800-2005: A.1.2 */
  : K_timeunit TIME_LITERAL ';'
      { pform_set_timeunit($2, allow_timeunit_decl); }
  | K_timeunit TIME_LITERAL '/' TIME_LITERAL ';'
      { bool initial_decl = allow_timeunit_decl && allow_timeprec_decl;
        pform_set_timeunit($2, initial_decl);
        pform_set_timeprec($4, initial_decl);
      }
  | K_timeprecision TIME_LITERAL ';'
      { pform_set_timeprec($2, allow_timeprec_decl); }
  ;

  /* Allow zero, one, or two declarations. The second declaration might
     be a repeat declaration, but the pform functions take care of that. */
timeunits_declaration_opt
  : /* empty */           %prec no_timeunits_declaration
  | timeunits_declaration %prec one_timeunits_declaration
  | timeunits_declaration timeunits_declaration
  ;

value_range /* IEEE1800-2005: A.8.3 */
  : expression
      { }
  | '[' expression ':' expression ']'
      { }
  ;

variable_dimension /* IEEE1800-2005: A.2.5 */
  : '[' expression ':' expression ']'
      { std::list<pform_range_t> *tmp = new std::list<pform_range_t>;
	pform_range_t index ($2,$4);
	tmp->push_back(index);
	$$ = tmp;
      }
  | '[' expression ']'
      { // SystemVerilog canonical range
	if (!gn_system_verilog()) {
	      warn_count += 1;
	      cerr << @2 << ": warning: Use of SystemVerilog [size] dimension. "
		   << "Use at least -g2005-sv to remove this warning." << endl;
	}
	list<pform_range_t> *tmp = new std::list<pform_range_t>;
	pform_range_t index ($2,0);
	tmp->push_back(index);
	$$ = tmp;
      }
  | '[' ']'
      { std::list<pform_range_t> *tmp = new std::list<pform_range_t>;
	pform_range_t index (0,0);
	pform_requires_sv(@$, "Dynamic array declaration");
	tmp->push_back(index);
	$$ = tmp;
      }
  | '[' '$' ']'
      { // SystemVerilog queue
	list<pform_range_t> *tmp = new std::list<pform_range_t>;
	pform_range_t index (new PENull,0);
	pform_requires_sv(@$, "Queue declaration");
	tmp->push_back(index);
	$$ = tmp;
      }
  | '[' '$' ':' expression ']'
      { // SystemVerilog queue with a max size
	list<pform_range_t> *tmp = new std::list<pform_range_t>;
	pform_range_t index (new PENull,$4);
	pform_requires_sv(@$, "Queue declaration");
	tmp->push_back(index);
	$$ = tmp;
      }
  ;

variable_lifetime_opt
  : lifetime
      { if (pform_requires_sv(@1, "Overriding default variable lifetime") &&
	    $1 != pform_peek_scope()->default_lifetime) {
	      yyerror(@1, "sorry: Overriding the default variable lifetime "
			  "is not yet supported.");
	}
	var_lifetime = $1;
      }
  |
  ;

  /* Verilog-2001 supports attribute lists, which can be attached to a
     variety of different objects. The syntax inside the (* *) is a
     comma separated list of names or names with assigned values. */
attribute_list_opt
  : attribute_instance_list
      { $$ = $1; }
  |
      { $$ = 0; }
  ;

attribute_instance_list
  : K_PSTAR K_STARP { $$ = 0; }
  | K_PSTAR attribute_list K_STARP { $$ = $2; }
  | attribute_instance_list K_PSTAR K_STARP { $$ = $1; }
  | attribute_instance_list K_PSTAR attribute_list K_STARP
      { std::list<named_pexpr_t>*tmp = $1;
	if (tmp) {
	    tmp->splice(tmp->end(), *$3);
	    delete $3;
	    $$ = tmp;
	} else $$ = $3;
      }
  ;

attribute_list
  : attribute_list ',' attribute
      { std::list<named_pexpr_t>*tmp = $1;
        tmp->push_back(*$3);
	delete $3;
	$$ = tmp;
      }
  | attribute
      { std::list<named_pexpr_t>*tmp = new std::list<named_pexpr_t>;
        tmp->push_back(*$1);
	delete $1;
	$$ = tmp;
      }
  ;


attribute
  : IDENTIFIER initializer_opt
      { named_pexpr_t*tmp = new named_pexpr_t;
	FILE_NAME(tmp, @$);
	tmp->name = lex_strings.make($1);
	tmp->parm = $2;
	delete[]$1;
	$$ = tmp;
      }
  ;


  /* The block_item_decl is used in function definitions, task
     definitions, module definitions and named blocks. Wherever a new
     scope is entered, the source may declare new registers and
     integers. This rule matches those declarations. The containing
     rule has presumably set up the scope. */

block_item_decl

  /* variable declarations. Note that data_type can be 0 if we are
     recovering from an error. */

  : K_const_opt K_var variable_lifetime_opt data_type_or_implicit list_of_variable_decl_assignments ';'
      { data_type_t *data_type = $4;
	if (!data_type) {
	      data_type = new vector_type_t(IVL_VT_LOGIC, false, 0);
	      FILE_NAME(data_type, @2);
	}
	pform_make_var(@2, $5, data_type, attributes_in_context, $1);
	var_lifetime = LexicalScope::INHERITED;
      }

  | K_const_opt variable_lifetime_opt data_type list_of_variable_decl_assignments ';'
      { if ($3) pform_make_var(@3, $4, $3, attributes_in_context, $1);
	var_lifetime = LexicalScope::INHERITED;
      }

  /* The extra `reg` is not valid (System)Verilog, this is a iverilog extension. */
  | K_const_opt variable_lifetime_opt K_reg data_type list_of_variable_decl_assignments ';'
      { if ($4) pform_make_var(@4, $5, $4, attributes_in_context, $1);
	var_lifetime = LexicalScope::INHERITED;
      }

  | K_event event_variable_list ';'
      { if ($2) pform_make_events(@1, $2);
      }

  | parameter_declaration

  /* Blocks can have type declarations. */

  | type_declaration

  /* Blocks can have imports. */

  | package_import_declaration

  /* Recover from errors that happen within variable lists. Use the
     trailing semi-colon to resync the parser. */

  | K_const_opt K_var variable_lifetime_opt data_type_or_implicit error ';'
      { yyerror(@1, "error: Syntax error in variable list.");
	yyerrok;
      }
  | K_const_opt variable_lifetime_opt data_type error ';'
      { yyerror(@1, "error: Syntax error in variable list.");
	yyerrok;
      }
  | K_event error ';'
      { yyerror(@1, "error: Syntax error in event variable list.");
	yyerrok;
      }

  | parameter error ';'
      { yyerror(@1, "error: Syntax error in parameter list.");
	yyerrok;
      }
  | localparam error ';'
      { yyerror(@1, "error: Syntax error localparam list.");
	yyerrok;
      }
  ;

block_item_decls
  : block_item_decl
  | block_item_decls block_item_decl
  ;

block_item_decls_opt
  : block_item_decls { $$ = true; }
  | { $$ = false; }
  ;

  /* We need to handle K_enum separately because
   * `typedef enum <TYPE_IDENTIFIER>` can either be the start of a enum forward
   * declaration or a enum type declaration with a type identifier as its base
   * type. And this abmiguity can not be resolved if we reduce the K_enum to
   * typedef_basic_type. */
typedef_basic_type
  : K_struct { $$ = typedef_t::STRUCT; }
  | K_union { $$ = typedef_t::UNION; }
  | K_class { $$ = typedef_t::CLASS; }
  ;

  /* Type declarations are parsed here. The rule actions call pform
     functions that add the declaration to the current lexical scope. */
type_declaration
  : K_typedef data_type identifier_name dimensions_opt ';'
      { perm_string name = lex_strings.make($3);
	pform_set_typedef(@3, name, $2, $4);
	delete[]$3;
      }

  /* These are forward declarations... */

  | K_typedef identifier_name ';'
      { perm_string name = lex_strings.make($2);
	pform_forward_typedef(@2, name, typedef_t::ANY);
	delete[]$2;
      }
  | K_typedef typedef_basic_type identifier_name ';'
      { perm_string name = lex_strings.make($3);
	pform_forward_typedef(@3, name, $2);
	delete[]$3;
      }
  | K_typedef K_enum identifier_name ';'
      { perm_string name = lex_strings.make($3);
	pform_forward_typedef(@3, name, typedef_t::ENUM);
	delete[]$3;
      }
  | K_typedef error ';'
      { yyerror(@2, "error: Syntax error in typedef clause.");
	yyerrok;
      }

  ;

  /* The structure for an enumeration data type is the keyword "enum",
     followed by the enumeration values in curly braces. Also allow
     for an optional base type. The default base type is "int", but it
     can be any of the integral or vector types. */

enum_base_type /* IEEE 1800-2012 A.2.2.1 */
  : simple_packed_type
      { $$ = $1;
      }
  | ps_type_identifier dimensions_opt
      { if ($2) {
	      $$ = new parray_type_t($1, $2);
	      FILE_NAME($$, @1);
        } else {
	      $$ = $1;
        }
      }
  |
      { $$ = new atom_type_t(atom_type_t::INT, true);
        FILE_NAME($$, @0);
      }
  ;

enum_data_type /* IEEE 1800-2012 A.2.2.1 */
  : K_enum enum_base_type '{' enum_name_list '}'
      { enum_type_t*enum_type = new enum_type_t($2);
	FILE_NAME(enum_type, @1);
	enum_type->names.reset($4);
	pform_put_enum_type_in_scope(enum_type);
	$$ = enum_type;
      }
  ;

enum_name_list
  : enum_name
      { $$ = $1;
      }
  | enum_name_list ',' enum_name
      { std::list<named_pexpr_t>*lst = $1;
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
      { verinum tmp = -(*($2));
	*($2) = tmp;
	$$ = $2;
      }
  ;

enum_name
  : IDENTIFIER initializer_opt
      { perm_string name = lex_strings.make($1);
	delete[]$1;
	$$ = make_named_number(@$, name, $2);
      }
  | IDENTIFIER '[' pos_neg_number ']' initializer_opt
      { perm_string name = lex_strings.make($1);
	long count = check_enum_seq_value(@1, $3, false);
	$$ = make_named_numbers(@$, name, 0, count-1, $5);
	delete[]$1;
	delete $3;
      }
  | IDENTIFIER '[' pos_neg_number ':' pos_neg_number ']' initializer_opt
      { perm_string name = lex_strings.make($1);
	$$ = make_named_numbers(@$, name, check_enum_seq_value(@1, $3, true),
	                                  check_enum_seq_value(@1, $5, true), $7);
	delete[]$1;
	delete $3;
	delete $5;
      }
  ;

/* `signed` and `unsigned` are only valid if preceded by `packed` */
packed_signing /* IEEE 1800-2012 A.2.2.1 */
  : K_packed unsigned_signed_opt
      { $$.packed_flag = true;
        $$.signed_flag = $2;
      }
  |
      { $$.packed_flag = false;
        $$.signed_flag = false;
      }
  ;

struct_data_type /* IEEE 1800-2012 A.2.2.1 */
  : K_struct packed_signing '{' struct_union_member_list '}'
      { struct_type_t*tmp = new struct_type_t;
	FILE_NAME(tmp, @1);
	tmp->packed_flag = $2.packed_flag;
	tmp->signed_flag = $2.signed_flag;
	tmp->union_flag = false;
	tmp->members .reset($4);
	$$ = tmp;
      }
  | K_union packed_signing '{' struct_union_member_list '}'
      { struct_type_t*tmp = new struct_type_t;
	FILE_NAME(tmp, @1);
	tmp->packed_flag = $2.packed_flag;
	tmp->signed_flag = $2.signed_flag;
	tmp->union_flag = true;
	tmp->members .reset($4);
	$$ = tmp;
      }
  | K_struct packed_signing '{' error '}'
      { yyerror(@3, "error: Errors in struct member list.");
	yyerrok;
	struct_type_t*tmp = new struct_type_t;
	FILE_NAME(tmp, @1);
	tmp->packed_flag = $2.packed_flag;
	tmp->signed_flag = $2.signed_flag;
	tmp->union_flag = false;
	$$ = tmp;
      }
  | K_union packed_signing '{' error '}'
      { yyerror(@3, "error: Errors in union member list.");
	yyerrok;
	struct_type_t*tmp = new struct_type_t;
	FILE_NAME(tmp, @1);
	tmp->packed_flag = $2.packed_flag;
	tmp->signed_flag = $2.signed_flag;
	tmp->union_flag = true;
	$$ = tmp;
      }
  ;

  /* This is an implementation of the rule snippet:
       struct_union_member { struct_union_member }
     that is used in the rule matching struct and union types
     in IEEE 1800-2012 A.2.2.1. */
struct_union_member_list
  : struct_union_member_list struct_union_member
      { std::list<struct_member_t*>*tmp = $1;
	if ($2) tmp->push_back($2);
	$$ = tmp;
      }
  | struct_union_member
      { std::list<struct_member_t*>*tmp = new std::list<struct_member_t*>;
	if ($1) tmp->push_back($1);
	$$ = tmp;
      }
  ;

struct_union_member /* IEEE 1800-2012 A.2.2.1 */
  : attribute_list_opt data_type list_of_variable_decl_assignments ';'
      { struct_member_t*tmp = new struct_member_t;
	FILE_NAME(tmp, @2);
	tmp->type  .reset($2);
	tmp->names .reset($3);
	$$ = tmp;
      }
  | error ';'
      { yyerror(@2, "error: Error in struct/union member.");
	yyerrok;
	$$ = 0;
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
      { $1->push_back($2);
	$$ = $1;
      }
  | case_item
      { $$ = new std::vector<PCase::Item*>(1, $1);
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
  | dimensions defparam_assign
      { yyerror(@1, "error: defparam may not include a range.");
	delete $1;
      }
  | defparam_assign_list ',' defparam_assign
  ;

delay1
  : '#' delay_value_simple
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($2);
	$$ = tmp;
      }
  | '#' '(' delay_value ')'
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($3);
	$$ = tmp;
      }
  ;

delay3
  : '#' delay_value_simple
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($2);
	$$ = tmp;
      }
  | '#' '(' delay_value ')'
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($3);
	$$ = tmp;
      }
  | '#' '(' delay_value ',' delay_value ')'
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($3);
	tmp->push_back($5);
	$$ = tmp;
      }
  | '#' '(' delay_value ',' delay_value ',' delay_value ')'
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
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
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  | delay_value_list ',' delay_value
      { std::list<PExpr*>*tmp = $1;
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
	      yyerror(@1, "internal error: decimal delay.");
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
	      yyerror(@1, "internal error: real time delay.");
	      $$ = 0;
	} else {
	      $$ = new PEFNumber(tmp);
	      FILE_NAME($$, @1);
	}
      }
  | IDENTIFIER
      { PEIdent*tmp = new PEIdent(lex_strings.make($1), @1.lexical_pos);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete[]$1;
      }
  | TIME_LITERAL
      { int unit;

	based_size = 0;
	$$         = 0;
	if ($1 == 0 || !get_time_unit($1, unit))
	      yyerror(@1, "internal error: time literal delay.");
	else {
#ifdef __FreeBSD__
		// Using raw pow() in FreeBSD gives a value that is off by one and this causes
		// rounding issues later, so for now use powl() to get the correct result.
	      long double ldp = powl(10.0, (long double)(unit - pform_get_timeunit()));
	      double p = (double) ldp;
#else
	      double p = pow(10.0, (double)(unit - pform_get_timeunit()));
#endif
	      double time = atof($1) * p;

	      verireal *v = new verireal(time);
	      $$ = new PEFNumber(v);
	      FILE_NAME($$, @1);
	}
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
  : /* The use clause takes an optional :config. */
  | ':' K_config
  ;

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
  : drive_strength
      { $$ = $1; }
  |
      { $$.str0 = IVL_DR_STRONG; $$.str1 = IVL_DR_STRONG; }
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

clocking_event_opt /* */
  : event_control
  |
  ;

event_control /* A.K.A. clocking_event */
  : '@' hierarchy_identifier
      { PEIdent*tmpi = pform_new_ident(@2, *$2);
	FILE_NAME(tmpi, @2);
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
      { $$ = new std::vector<PEEvent*>(1, $1);
      }
  | event_expression_list K_or event_expression
      { $1->push_back($3);
	$$ = $1;
      }
  | event_expression_list ',' event_expression
      { $1->push_back($3);
	$$ = $1;
      }
  ;

event_expression
  : K_posedge expression
      { PEEvent*tmp = new PEEvent(PEEvent::POSEDGE, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_negedge expression
      { PEEvent*tmp = new PEEvent(PEEvent::NEGEDGE, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_edge expression
      { PEEvent*tmp = new PEEvent(PEEvent::EDGE, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	pform_requires_sv(@1, "Edge event");
      }
  | expression
      { PEEvent*tmp = new PEEvent(PEEvent::ANYEDGE, $1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
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
  : expr_primary_or_typename
      { $$ = $1; }
  | inc_or_dec_expression
      { $$ = $1; }
  | inside_expression
      { $$ = $1; }
  | '+' attribute_list_opt expr_primary %prec UNARY_PREC
      { $$ = $3; }
  | '-' attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('-', $3);
	FILE_NAME(tmp, @3);
	$$ = tmp;
      }
  | '~' attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('~', $3);
	FILE_NAME(tmp, @3);
	$$ = tmp;
      }
  | '&' attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('&', $3);
	FILE_NAME(tmp, @3);
	$$ = tmp;
      }
  | '!' attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('!', $3);
	FILE_NAME(tmp, @3);
	$$ = tmp;
      }
  | '|' attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('|', $3);
	FILE_NAME(tmp, @3);
	$$ = tmp;
      }
  | '^' attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('^', $3);
	FILE_NAME(tmp, @3);
	$$ = tmp;
      }
  | '~' '&' attribute_list_opt expr_primary %prec UNARY_PREC
      { yyerror(@1, "error: '~' '&'  is not a valid expression. "
		"Please use operator '~&' instead.");
	$$ = 0;
      }
  | '~' '|' attribute_list_opt expr_primary %prec UNARY_PREC
      { yyerror(@1, "error: '~' '|'  is not a valid expression. "
		"Please use operator '~|' instead.");
	$$ = 0;
      }
  | '~' '^' attribute_list_opt expr_primary %prec UNARY_PREC
      { yyerror(@1, "error: '~' '^'  is not a valid expression. "
		"Please use operator '~^' instead.");
	$$ = 0;
      }
  | K_NAND attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('A', $3);
	FILE_NAME(tmp, @3);
	$$ = tmp;
      }
  | K_NOR attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('N', $3);
	FILE_NAME(tmp, @3);
	$$ = tmp;
      }
  | K_NXOR attribute_list_opt expr_primary %prec UNARY_PREC
      { PEUnary*tmp = new PEUnary('X', $3);
	FILE_NAME(tmp, @3);
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
  | expression '^' attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('^', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_POW attribute_list_opt expression
      { PEBinary*tmp = new PEBPower('p', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '*' attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('*', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '/' attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('/', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '%' attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('%', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '+' attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('+', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '-' attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('-', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '&' attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('&', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '|' attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('|', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_NAND attribute_list_opt expression
      { if (gn_icarus_misc_flag) {
	      PEBinary*tmp = new PEBinary('A', $1, $4);
	      FILE_NAME(tmp, @2);
	      $$ = tmp;
	} else {
	      yyerror(@2, "error: The binary NAND operator "
			  "is an Icarus Verilog extension. "
			  "Use -gicarus-misc to enable it.");
	      $$ = 0;
	}
      }
  | expression K_NOR attribute_list_opt expression
      { if (gn_icarus_misc_flag) {
	      PEBinary*tmp = new PEBinary('O', $1, $4);
	      FILE_NAME(tmp, @2);
	      $$ = tmp;
	} else {
	      yyerror(@2, "error: The binary NOR operator "
			  "is an Icarus Verilog extension. "
			  "Use -gicarus-misc to enable it.");
	      $$ = 0;
	}
      }
  | expression K_NXOR attribute_list_opt expression
      { PEBinary*tmp = new PEBinary('X', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '<' attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('<', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '>' attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('>', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_LS attribute_list_opt expression
      { PEBinary*tmp = new PEBShift('l', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_RS attribute_list_opt expression
      { PEBinary*tmp = new PEBShift('r', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_RSS attribute_list_opt expression
      { PEBinary*tmp = new PEBShift('R', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_EQ attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('e', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_CEQ attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('E', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_WEQ attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('w', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_LE attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('L', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_GE attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('G', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_NE attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('n', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_CNE attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('N', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_WNE attribute_list_opt expression
      { PEBinary*tmp = new PEBComp('W', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_LOR attribute_list_opt expression
      { PEBinary*tmp = new PEBLogic('o', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_LAND attribute_list_opt expression
      { PEBinary*tmp = new PEBLogic('a', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression K_TRIGGER attribute_list_opt expression
      { PEBinary*tmp = new PEBLogic('q', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }

  | expression K_LEQUIV attribute_list_opt expression
      { PEBinary*tmp = new PEBLogic('Q', $1, $4);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | expression '?' attribute_list_opt expression ':' expression
      { PETernary*tmp = new PETernary($1, $4, $6);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_opt
  : expression { $$ = $1; }
  | { $$ = nullptr; }
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
	      cerr << $$->get_fileline() << ": warning: Choosing ";
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
      { std::list<PExpr*>*tmp = $1;
	if (tmp->empty()) tmp->push_back(0);
	tmp->push_back($3);
	$$ = tmp;
      }
  | expression
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  |
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	$$ = tmp;
      }
  | expression_list_with_nuls ','
      { std::list<PExpr*>*tmp = $1;
	if (tmp->empty()) tmp->push_back(0);
	tmp->push_back(0);
	$$ = tmp;
      }
  ;

argument
  : expression
      { named_pexpr_t *tmp = new named_pexpr_t;
	FILE_NAME(tmp, @$);
	tmp->name = perm_string();
	tmp->parm = $1;
	$$ = tmp;
      }
  | named_expression_opt
      { $$ = $1;
      }
  |
      { named_pexpr_t *tmp = new named_pexpr_t;
	tmp->name = perm_string();
	tmp->parm = nullptr;
	$$ = tmp;
      }
  ;

argument_list
 : argument
      { std::list<named_pexpr_t> *expr = new std::list<named_pexpr_t>;
	expr->push_back(*$1);
	delete $1;
	$$ = expr;
      }
 | argument_list ',' argument
      { $1->push_back(*$3);
	delete $3;
	$$ = $1;
      }
 ;

  /* An argument list enclosed in parenthesis. The parser will parse '()' as a
   * argument list with an single empty item. We fix this up once the list
   * parsing is done by replacing it with the empty list.
   */
argument_list_parens
  : '(' argument_list ')'
      { argument_list_fixup($2);
	$$ = $2; }
  ;

  /* A task or function can be invoked with the task/function name followed by
   * an argument list in parenthesis or with just the task/function name by
   * itself. When an argument list is used it might be empty. */
argument_list_parens_opt
  : argument_list_parens
      { $$ = $1; }
  |
      { $$ = new std::list<named_pexpr_t>; }
  ;

expression_list_proper
  : expression_list_proper ',' expression
      { std::list<PExpr*>*tmp = $1;
        tmp->push_back($3);
        $$ = tmp;
      }
  | expression
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

expr_primary_or_typename
  : expr_primary

  /* There are a few special cases (notably $bits argument) where the
     expression may be a type name. Let the elaborator sort this out. */
  | data_type
      { PETypename*tmp = new PETypename($1);
	FILE_NAME(tmp, @1);
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
  | TIME_LITERAL
      { int unit;

        based_size = 0;
        $$         = 0;
        if ($1 == 0 || !get_time_unit($1, unit))
              yyerror(@1, "internal error: time literal.");
        else {
#ifdef __FreeBSD__
                // Using raw pow() in FreeBSD gives a value that is off by one and this causes
                // rounding issues below, so for now use powl() to get the correct result.
              long double ldp = powl(10.0, (double)(unit - pform_get_timeunit()));
              double p = (double) ldp;
#else
              double p = pow(10.0, (double)(unit - pform_get_timeunit()));
#endif
              double time = atof($1) * p;
              // The time value needs to be rounded at the correct digit
              // since this is a normal real value and not a delay that
              // will be rounded later. This style of rounding is not safe
              // for all real values!
              int rdigit = pform_get_timeunit() - pform_get_timeprec();
              assert(rdigit >= 0);
              double scale = pow(10.0, (double)rdigit);
              time = round(time*scale)/scale;

              verireal *v = new verireal(time);
              $$ = new PEFNumber(v);
              FILE_NAME($$, @1);
        }
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
      { PEIdent*tmp = pform_new_ident(@1, *$1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete $1;
      }
  /* These are array methods that cannot be matched with the above rule */
  | hierarchy_identifier '.' K_and
      { pform_name_t * nm = $1;
	nm->push_back(name_component_t(lex_strings.make("and")));
	PEIdent*tmp = pform_new_ident(@1, *nm);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete nm;
      }
  | hierarchy_identifier '.' K_or
      { pform_name_t * nm = $1;
	nm->push_back(name_component_t(lex_strings.make("or")));
	PEIdent*tmp = pform_new_ident(@1, *nm);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete nm;
      }
  | hierarchy_identifier '.' K_unique
      { pform_name_t * nm = $1;
	nm->push_back(name_component_t(lex_strings.make("unique")));
	PEIdent*tmp = pform_new_ident(@1, *nm);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete nm;
      }
  | hierarchy_identifier '.' K_xor
      { pform_name_t * nm = $1;
	nm->push_back(name_component_t(lex_strings.make("xor")));
	PEIdent*tmp = pform_new_ident(@1, *nm);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete nm;
      }

  | package_scope hierarchy_identifier
      { lex_in_package_scope(0);
	$$ = pform_package_ident(@2, $1, $2);
	delete $2;
      }

  /* An identifier followed by an expression list in parentheses is a
     function call. If a system identifier, then a system function
     call. It can also be a call to a class method (function). */

  | hierarchy_identifier attribute_list_opt argument_list_parens
      { PECallFunction*tmp = pform_make_call_function(@1, *$1, *$3);
	delete $1;
	delete $2;
	delete $3;
	$$ = tmp;
      }
  | class_hierarchy_identifier argument_list_parens
      { PECallFunction*tmp = pform_make_call_function(@1, *$1, *$2);
	delete $1;
	delete $2;
	$$ = tmp;
      }
  | SYSTEM_IDENTIFIER argument_list_parens
      { perm_string tn = lex_strings.make($1);
	PECallFunction *tmp = new PECallFunction(tn, *$2);
	if ($2->empty())
	      pform_requires_sv(@1, "Empty function argument list");
	FILE_NAME(tmp, @1);
	delete[]$1;
	delete $2;
	$$ = tmp;
      }
  | package_scope hierarchy_identifier { lex_in_package_scope(0); } argument_list_parens
      { PECallFunction*tmp = new PECallFunction($1, *$2, *$4);
	FILE_NAME(tmp, @2);
	delete $2;
	delete $4;
	$$ = tmp;
      }
  | K_this
      { PEIdent*tmp = new PEIdent(perm_string::literal(THIS_TOKEN), UINT_MAX);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }

  | class_hierarchy_identifier
      { PEIdent*tmp = new PEIdent(*$1, @1.lexical_pos);
	FILE_NAME(tmp, @1);
	delete $1;
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

  | '{' '}'
      { // This is the empty queue syntax.
	if (gn_system_verilog()) {
	      std::list<PExpr*> empty_list;
	      PEConcat*tmp = new PEConcat(empty_list);
	      FILE_NAME(tmp, @1);
	      $$ = tmp;
	} else {
	      yyerror(@1, "error: Concatenations are not allowed to be empty.");
	      $$ = 0;
	}
      }

  /* Cast expressions are primaries */

  | expr_primary '\'' '(' expression ')'
      { PExpr*base = $4;
	if (pform_requires_sv(@1, "Size cast")) {
	      PECastSize*tmp = new PECastSize($1, base);
	      FILE_NAME(tmp, @1);
	      $$ = tmp;
	} else {
	      $$ = base;
	}
      }

  | simple_type_or_string '\'' '(' expression ')'
      { PExpr*base = $4;
	if (pform_requires_sv(@1, "Type cast")) {
	      PECastType*tmp = new PECastType($1, base);
	      FILE_NAME(tmp, @1);
	      $$ = tmp;
	} else {
	      $$ = base;
	}
      }
  | signing '\'' '(' expression ')'
      { PExpr*base = $4;
	if (pform_requires_sv(@1, "Signing cast")) {
	      PECastSign*tmp = new PECastSign($1, base);
	      FILE_NAME(tmp, @1);
	      $$ = tmp;
	} else {
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
      { PENull*tmp = new PENull;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

  /* A tf_item_list is shared between functions and tasks to match
     declarations of ports. We check later to make sure there are no
     output or inout ports actually used for functions. */
tf_item_list_opt /* IEEE1800-2017: A.2.7 */
  : tf_item_list
      { $$ = $1; }
  |
      { $$ = 0; }
  ;

tf_item_list /* IEEE1800-2017: A.2.7 */
  : tf_item_declaration
      { $$ = $1; }
  | tf_item_list tf_item_declaration
      { if ($1 && $2) {
	      std::vector<pform_tf_port_t>*tmp = $1;
	      size_t s1 = tmp->size();
	      tmp->resize(s1 + $2->size());
	      for (size_t idx = 0 ; idx < $2->size() ; idx += 1)
		    tmp->at(s1+idx) = $2->at(idx);
	      delete $2;
	      $$ = tmp;
	} else if ($1) {
	      $$ = $1;
	} else {
	      $$ = $2;
	}
      }
 ;

tf_item_declaration /* IEEE1800-2017: A.2.7 */
  : tf_port_declaration { $$ = $1; }
  | block_item_decl     { $$ = 0; }
  ;

  /* A gate_instance is a module instantiation or a built in part
     type. In any case, the gate has a set of connections to ports. */
gate_instance
  : IDENTIFIER '(' port_conn_expression_list_with_nuls ')'
      { lgate*tmp = new lgate;
	tmp->name = $1;
	tmp->parms = $3;
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }

  | IDENTIFIER dimensions '(' port_conn_expression_list_with_nuls ')'
      { lgate*tmp = new lgate;
	tmp->name = $1;
	tmp->parms = $4;
	tmp->ranges = $2;
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }

  | '(' port_conn_expression_list_with_nuls ')'
      { lgate*tmp = new lgate;
	tmp->name = "";
	tmp->parms = $2;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  /* Degenerate modules can have no ports. */

  | IDENTIFIER dimensions
      { lgate*tmp = new lgate;
	tmp->name = $1;
	tmp->parms = 0;
	tmp->parms_by_name = 0;
	tmp->ranges = $2;
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }

  /* Modules can also take ports by port-name expressions. */

  | IDENTIFIER '(' port_name_list ')'
      { lgate*tmp = new lgate;
	tmp->name = $1;
	tmp->parms = 0;
	tmp->parms_by_name = $3;
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }

  | IDENTIFIER dimensions '(' port_name_list ')'
      { lgate*tmp = new lgate;
	tmp->name = $1;
	tmp->parms = 0;
	tmp->parms_by_name = $4;
	tmp->ranges = $2;
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }

  | IDENTIFIER '(' error ')'
      { lgate*tmp = new lgate;
	tmp->name = $1;
	tmp->parms = 0;
	tmp->parms_by_name = 0;
	FILE_NAME(tmp, @1);
	yyerror(@2, "error: Syntax error in instance port "
	        "expression(s).");
	delete[]$1;
	$$ = tmp;
      }

  | IDENTIFIER dimensions '(' error ')'
      { lgate*tmp = new lgate;
	tmp->name = $1;
	tmp->parms = 0;
	tmp->parms_by_name = 0;
	tmp->ranges = $2;
	FILE_NAME(tmp, @1);
	yyerror(@3, "error: Syntax error in instance port "
	        "expression(s).");
	delete[]$1;
	$$ = tmp;
      }
  ;

gate_instance_list
  : gate_instance_list ',' gate_instance
      { $1->push_back(*$3);
	delete $3;
	$$ = $1;
      }
  | gate_instance
      { $$ = new std::vector<lgate>(1, *$1);
	delete $1;
      }
  ;

gatetype
  : K_and    { $$ = PGBuiltin::AND; }
  | K_nand   { $$ = PGBuiltin::NAND; }
  | K_or     { $$ = PGBuiltin::OR; }
  | K_nor    { $$ = PGBuiltin::NOR; }
  | K_xor    { $$ = PGBuiltin::XOR; }
  | K_xnor   { $$ = PGBuiltin::XNOR; }
  | K_buf    { $$ = PGBuiltin::BUF; }
  | K_bufif0 { $$ = PGBuiltin::BUFIF0; }
  | K_bufif1 { $$ = PGBuiltin::BUFIF1; }
  | K_not    { $$ = PGBuiltin::NOT; }
  | K_notif0 { $$ = PGBuiltin::NOTIF0; }
  | K_notif1 { $$ = PGBuiltin::NOTIF1; }
  ;

switchtype
  : K_nmos     { $$ = PGBuiltin::NMOS; }
  | K_rnmos    { $$ = PGBuiltin::RNMOS; }
  | K_pmos     { $$ = PGBuiltin::PMOS; }
  | K_rpmos    { $$ = PGBuiltin::RPMOS; }
  | K_cmos     { $$ = PGBuiltin::CMOS; }
  | K_rcmos    { $$ = PGBuiltin::RCMOS; }
  | K_tran     { $$ = PGBuiltin::TRAN; }
  | K_rtran    { $$ = PGBuiltin::RTRAN; }
  | K_tranif0  { $$ = PGBuiltin::TRANIF0; }
  | K_tranif1  { $$ = PGBuiltin::TRANIF1; }
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
  | hierarchy_identifier '[' '$' ']'
      { pform_requires_sv(@3, "Last element expression ($)");
        pform_name_t * tmp = $1;
	name_component_t&tail = tmp->back();
	index_component_t itmp;
	itmp.sel = index_component_t::SEL_BIT_LAST;
	itmp.msb = 0;
	itmp.lsb = 0;
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
      { $$ = list_from_identifier($1, @1.lexical_pos); }
  | list_of_identifiers ',' IDENTIFIER
      { $$ = list_from_identifier($1, $3, @3.lexical_pos); }
  ;

list_of_port_identifiers
  : IDENTIFIER dimensions_opt
      { $$ = make_port_list($1, @1.lexical_pos, $2, 0); }
  | list_of_port_identifiers ',' IDENTIFIER dimensions_opt
      { $$ = make_port_list($1, $3, @3.lexical_pos, $4, 0); }
  ;

list_of_variable_port_identifiers
  : IDENTIFIER dimensions_opt initializer_opt
      { $$ = make_port_list($1, @1.lexical_pos, $2, $3); }
  | list_of_variable_port_identifiers ',' IDENTIFIER dimensions_opt initializer_opt
      { $$ = make_port_list($1, $3, @3.lexical_pos, $4, $5); }
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
      { std::vector<Module::port_t*>*tmp = new std::vector<Module::port_t*>(1);
	(*tmp)[0] = $1;
	$$ = tmp;
      }
  | list_of_ports ',' port_opt
      { std::vector<Module::port_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  ;

list_of_port_declarations
  : port_declaration
      { std::vector<Module::port_t*>*tmp = new std::vector<Module::port_t*>(1);
	(*tmp)[0] = $1;
	$$ = tmp;
      }
  | list_of_port_declarations ',' port_declaration
      { std::vector<Module::port_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | list_of_port_declarations ',' attribute_list_opt IDENTIFIER dimensions_opt initializer_opt
      { std::vector<Module::port_t*> *ports = $1;

	Module::port_t* port;
	port = module_declare_port(@4, $4, port_declaration_context.port_type,
				   port_declaration_context.port_net_type,
				   port_declaration_context.data_type,
				   $5, $6, $3);
	ports->push_back(port);
	$$ = ports;
      }
  | list_of_port_declarations ','
      { yyerror(@2, "error: Superfluous comma in port declaration list."); }
  | list_of_port_declarations ';'
      { yyerror(@2, "error: ';' is an invalid port declaration separator."); }
  ;

  // All of port direction, port kind and data type are optional, but at least
  // one has to be specified, so we need multiple rules.
port_declaration
  : attribute_list_opt port_direction net_type_or_var_opt data_type_or_implicit IDENTIFIER dimensions_opt initializer_opt
      { $$ = module_declare_port(@5, $5, $2, $3, $4, $6, $7, $1);
      }
  | attribute_list_opt net_type_or_var data_type_or_implicit IDENTIFIER dimensions_opt initializer_opt
      { pform_requires_sv(@4, "Partial ANSI port declaration");
	$$ = module_declare_port(@4, $4, port_declaration_context.port_type,
			         $2, $3, $5, $6, $1);
      }
  | attribute_list_opt data_type_or_implicit_no_opt IDENTIFIER dimensions_opt initializer_opt
      { pform_requires_sv(@3, "Partial ANSI port declaration");
	$$ = module_declare_port(@3, $3, port_declaration_context.port_type,
			         NetNet::IMPLICIT, $2, $4, $5, $1);
      }
  | attribute_list_opt port_direction K_wreal IDENTIFIER
      { real_type_t*real_type = new real_type_t(real_type_t::REAL);
	FILE_NAME(real_type, @3);
	$$ = module_declare_port(@4, $4, $2, NetNet::WIRE,
				 real_type, nullptr, nullptr, $1);
      }
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
atom_type
  : K_byte     { $$ = atom_type_t::BYTE; }
  | K_shortint { $$ = atom_type_t::SHORTINT; }
  | K_int      { $$ = atom_type_t::INT; }
  | K_longint  { $$ = atom_type_t::LONGINT; }
  | K_integer  { $$ = atom_type_t::INTEGER; }
  ;

  /* An lpvalue is the expression that can go on the left side of a
     procedural assignment. This rule handles only procedural
     assignments. It is more limited than the general expr_primary
     rule to reflect the rules for assignment l-values. */
lpvalue
  : hierarchy_identifier
      { PEIdent*tmp = pform_new_ident(@1, *$1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	delete $1;
      }

  | class_hierarchy_identifier
      { PEIdent*tmp = new PEIdent(*$1, @1.lexical_pos);
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

  | streaming_concatenation
      { yyerror(@1, "sorry: Streaming concatenation not supported in l-values.");
	$$ = 0;
      }
  ;


  /* Continuous assignments have a list of individual assignments. */

cont_assign
  : lpvalue '=' expression
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($1);
	tmp->push_back($3);
	$$ = tmp;
      }
  ;

cont_assign_list
  : cont_assign_list ',' cont_assign
      { std::list<PExpr*>*tmp = $1;
	tmp->splice(tmp->end(), *$3);
	delete $3;
	$$ = tmp;
      }
  | cont_assign
      { $$ = $1; }
  ;

  /* This is the global structure of a module. A module is a start
     section, with optional ports, then an optional list of module
     items, and finally an end marker. */

module
  : attribute_list_opt module_start lifetime_opt IDENTIFIER
      { pform_startmodule(@2, $4, $2==K_program, $2==K_interface, $3, $1);
        port_declaration_context_init(); }
    module_package_import_list_opt
    module_parameter_port_list_opt
    module_port_list_opt
    module_attribute_foreign ';'
      { pform_module_set_ports($8); }
    timeunits_declaration_opt
      { pform_set_scope_timescale(@2); }
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
	if ($2 != $15) {
	      switch ($2) {
		  case K_module:
		    yyerror(@15, "error: module not closed by endmodule.");
		    break;
		  case K_program:
		    yyerror(@15, "error: program not closed by endprogram.");
		    break;
		  case K_interface:
		    yyerror(@15, "error: interface not closed by endinterface.");
		    break;
		  default:
		    break;
	      }
	}
	pform_endmodule($4, in_celldefine, ucd);
      }
    label_opt
      { // Last step: check any closing name. This is done late so
	// that the parser can look ahead to detect the present
	// label_opt but still have the pform_endmodule() called
	// early enough that the lexor can know we are outside the
	// module.
	switch ($2) {
	    case K_module:
	      check_end_label(@17, "module", $4, $17);
	      break;
	    case K_program:
	      check_end_label(@17, "program", $4, $17);
	      break;
	    case K_interface:
	      check_end_label(@17, "interface", $4, $17);
	      break;
	    default:
	      break;
	}
	delete[]$4;
      }
  ;

  /* Modules start with a module/macromodule, program, or interface
     keyword, and end with a endmodule, endprogram, or endinterface
     keyword. The syntax for modules programs, and interfaces is
     almost identical, so let semantics sort out the differences. */
module_start
  : K_module
      { pform_error_in_generate(@1, "module declaration");
        $$ = K_module;
      }
  | K_macromodule
      { pform_error_in_generate(@1, "module declaration");
        $$ = K_module;
      }
  | K_program
      { pform_error_in_generate(@1, "program declaration");
        $$ = K_program;
      }
  | K_interface
      { pform_error_in_generate(@1, "interface declaration");
        $$ = K_interface;
      }
  ;

module_end
  : K_endmodule    { $$ = K_module; }
  | K_endprogram   { $$ = K_program; }
  | K_endinterface { $$ = K_interface; }
  ;

label_opt
  : ':' IDENTIFIER { $$ = $2; }
  |                { $$ = 0; }
  ;

module_attribute_foreign
  : K_PSTAR IDENTIFIER K_integer IDENTIFIER '=' STRING ';' K_STARP { $$ = 0; }
  | { $$ = 0; }
  ;

module_port_list_opt
  : '(' list_of_ports ')'
      { $$ = $2; }
  | '(' list_of_port_declarations ')'
      { $$ = $2; }
  |
      { $$ = 0; }
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
  | '#' '('
      { pform_start_parameter_port_list(); }
    module_parameter_port_list
      { pform_end_parameter_port_list(); }
    ')'
  ;

type_param
  : K_type { param_is_type = true; }
  ;

module_parameter
  : parameter param_type parameter_assign
  | localparam param_type parameter_assign
      { pform_requires_sv(@1, "Local parameter in module parameter port list");
      }
  ;

module_parameter_port_list
  : module_parameter
  | data_type_opt
      { param_data_type = $1;
        param_is_local = false;
        param_is_type = false;
      }
    parameter_assign
      { pform_requires_sv(@3, "Omitting initial `parameter` in parameter port "
			      "list");
      }
  | type_param
      { param_is_local = false; }
    parameter_assign
  | module_parameter_port_list ',' module_parameter
  | module_parameter_port_list ',' data_type_opt
      { if ($3) {
	      pform_requires_sv(@3, "Omitting `parameter`/`localparam` before "
				    "data type in parameter port list");
	      param_data_type = $3;
	      param_is_type = false;
        }
      }
    parameter_assign
  | module_parameter_port_list ',' type_param parameter_assign
  ;

module_item

  /* Modules can contain further sub-module definitions. */
  : module

  | attribute_list_opt net_type data_type_or_implicit delay3_opt net_variable_list ';'

      { data_type_t*data_type = $3;
        pform_check_net_data_type(@2, $2, $3);
	if (data_type == 0) {
	      data_type = new vector_type_t(IVL_VT_LOGIC, false, 0);
	      FILE_NAME(data_type, @2);
	}
	pform_set_data_type(@2, data_type, $5, $2, $1);
	if ($4 != 0) {
	      yyerror(@2, "sorry: Net delays not supported.");
	      delete $4;
	}
	delete $1;
      }

  | attribute_list_opt K_wreal delay3 net_variable_list ';'
      { real_type_t*tmpt = new real_type_t(real_type_t::REAL);
	pform_set_data_type(@2, tmpt, $4, NetNet::WIRE, $1);
	if ($3 != 0) {
	      yyerror(@3, "sorry: Net delays not supported.");
	      delete $3;
	}
	delete $1;
      }

  | attribute_list_opt K_wreal net_variable_list ';'
      { real_type_t*tmpt = new real_type_t(real_type_t::REAL);
	pform_set_data_type(@2, tmpt, $3, NetNet::WIRE, $1);
	delete $1;
      }

  /* Very similar to the rule above, but this takes a list of
     net_decl_assigns, which are <name> = <expr> assignment
     declarations. */

  | attribute_list_opt net_type data_type_or_implicit delay3_opt net_decl_assigns ';'
      { data_type_t*data_type = $3;
        pform_check_net_data_type(@2, $2, $3);
	if (data_type == 0) {
	      data_type = new vector_type_t(IVL_VT_LOGIC, false, 0);
	      FILE_NAME(data_type, @2);
	}
	pform_makewire(@2, $4, str_strength, $5, $2, data_type, $1);
	delete $1;
      }

  /* This form doesn't have the range, but does have strengths. This
     gives strength to the assignment drivers. */

  | attribute_list_opt net_type data_type_or_implicit drive_strength net_decl_assigns ';'
      { data_type_t*data_type = $3;
        pform_check_net_data_type(@2, $2, $3);
	if (data_type == 0) {
	      data_type = new vector_type_t(IVL_VT_LOGIC, false, 0);
	      FILE_NAME(data_type, @2);
	}
	pform_makewire(@2, 0, $4, $5, $2, data_type, $1);
	delete $1;
      }

  | attribute_list_opt K_wreal net_decl_assigns ';'
      { real_type_t*data_type = new real_type_t(real_type_t::REAL);
	pform_makewire(@2, 0, str_strength, $3, NetNet::WIRE, data_type, $1);
	delete $1;
      }

  | K_trireg charge_strength_opt dimensions_opt delay3_opt list_of_identifiers ';'
      { yyerror(@1, "sorry: trireg nets not supported.");
	delete $3;
	delete $4;
      }


  /* The next two rules handle port declarations that include a net type, e.g.
       input wire signed [h:l] <list>;
     This creates the wire and sets the port type all at once. */

  | attribute_list_opt port_direction net_type_or_var data_type_or_implicit list_of_port_identifiers ';'
      { pform_module_define_port(@2, $5, $2, $3, $4, $1); }

  | attribute_list_opt port_direction K_wreal list_of_port_identifiers ';'
      { real_type_t*real_type = new real_type_t(real_type_t::REAL);
	pform_module_define_port(@2, $4, $2, NetNet::WIRE, real_type, $1);
      }

  /* The next three rules handle port declarations that include a variable
     type, e.g.
       output reg signed [h:l] <list>;
     and also handle incomplete port declarations, e.g.
       input signed [h:l] <list>;
   */
  | attribute_list_opt K_inout data_type_or_implicit list_of_port_identifiers ';'
      { NetNet::Type use_type = $3 ? NetNet::IMPLICIT : NetNet::NONE;
	if (vector_type_t*dtype = dynamic_cast<vector_type_t*> ($3)) {
	      if (dtype->implicit_flag)
		    use_type = NetNet::NONE;
	}
	if (use_type == NetNet::NONE)
	      pform_set_port_type(@2, $4, NetNet::PINOUT, $3, $1);
	else
	      pform_module_define_port(@2, $4, NetNet::PINOUT, use_type, $3, $1);
      }

  | attribute_list_opt K_input data_type_or_implicit list_of_port_identifiers ';'
      { NetNet::Type use_type = $3 ? NetNet::IMPLICIT : NetNet::NONE;
	if (vector_type_t*dtype = dynamic_cast<vector_type_t*> ($3)) {
	      if (dtype->implicit_flag)
		    use_type = NetNet::NONE;
	}
	if (use_type == NetNet::NONE)
	      pform_set_port_type(@2, $4, NetNet::PINPUT, $3, $1);
	else
	      pform_module_define_port(@2, $4, NetNet::PINPUT, use_type, $3, $1);
      }

  | attribute_list_opt K_output data_type_or_implicit list_of_variable_port_identifiers ';'
      { NetNet::Type use_type = $3 ? NetNet::IMPLICIT : NetNet::NONE;
	if (vector_type_t*dtype = dynamic_cast<vector_type_t*> ($3)) {
	      if (dtype->implicit_flag)
		    use_type = NetNet::NONE;
	      else
		    use_type = NetNet::IMPLICIT_REG;

		// The SystemVerilog types that can show up as
		// output ports are implicitly (on the inside)
		// variables because "reg" is not valid syntax
		// here.
	} else if ($3) {
	      use_type = NetNet::IMPLICIT_REG;
	}
	if (use_type == NetNet::NONE)
	      pform_set_port_type(@2, $4, NetNet::POUTPUT, $3, $1);
	else
	      pform_module_define_port(@2, $4, NetNet::POUTPUT, use_type, $3, $1);
      }

  | attribute_list_opt port_direction net_type_or_var data_type_or_implicit error ';'
      { yyerror(@2, "error: Invalid variable list in port declaration.");
	if ($1) delete $1;
	if ($4) delete $4;
	yyerrok;
      }

  | attribute_list_opt K_inout data_type_or_implicit error ';'
      { yyerror(@2, "error: Invalid variable list in port declaration.");
	if ($1) delete $1;
	if ($3) delete $3;
	yyerrok;
      }

  | attribute_list_opt K_input data_type_or_implicit error ';'
      { yyerror(@2, "error: Invalid variable list in port declaration.");
	if ($1) delete $1;
	if ($3) delete $3;
	yyerrok;
      }

  | attribute_list_opt K_output data_type_or_implicit error ';'
      { yyerror(@2, "error: Invalid variable list in port declaration.");
	if ($1) delete $1;
	if ($3) delete $3;
	yyerrok;
      }

  | K_let IDENTIFIER let_port_list_opt '=' expression ';'
      { perm_string tmp2 = lex_strings.make($2);
        pform_make_let(@1, tmp2, $3, $5);
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

  | K_defparam
      { if (pform_in_interface())
	      yyerror(@1, "error: Parameter overrides are not allowed "
			  "in interfaces.");
      }
    defparam_assign_list ';'

  /* Most gate types have an optional drive strength and optional
     two/three-value delay. These rules handle the different cases.
     We check that the actual number of delays is correct later. */

  | attribute_list_opt gatetype gate_instance_list ';'
      { pform_makegates(@2, $2, str_strength, 0, $3, $1); }

  | attribute_list_opt gatetype delay3 gate_instance_list ';'
      { pform_makegates(@2, $2, str_strength, $3, $4, $1); }

  | attribute_list_opt gatetype drive_strength gate_instance_list ';'
      { pform_makegates(@2, $2, $3, 0, $4, $1); }

  | attribute_list_opt gatetype drive_strength delay3 gate_instance_list ';'
      { pform_makegates(@2, $2, $3, $4, $5, $1); }

  /* The switch type gates do not support a strength. */
  | attribute_list_opt switchtype gate_instance_list ';'
      { pform_makegates(@2, $2, str_strength, 0, $3, $1); }

  | attribute_list_opt switchtype delay3 gate_instance_list ';'
      { pform_makegates(@2, $2, str_strength, $3, $4, $1); }

  /* Pullup and pulldown devices cannot have delays, and their
     strengths are limited. */

  | K_pullup gate_instance_list ';'
      { pform_makegates(@1, PGBuiltin::PULLUP, pull_strength, 0, $2, 0); }
  | K_pulldown gate_instance_list ';'
      { pform_makegates(@1, PGBuiltin::PULLDOWN, pull_strength, 0, $2, 0); }

  | K_pullup '(' dr_strength1 ')' gate_instance_list ';'
      { pform_makegates(@1, PGBuiltin::PULLUP, $3, 0, $5, 0); }

  | K_pullup '(' dr_strength1 ',' dr_strength0 ')' gate_instance_list ';'
      { pform_makegates(@1, PGBuiltin::PULLUP, $3, 0, $7, 0); }

  | K_pullup '(' dr_strength0 ',' dr_strength1 ')' gate_instance_list ';'
      { pform_makegates(@1, PGBuiltin::PULLUP, $5, 0, $7, 0); }

  | K_pulldown '(' dr_strength0 ')' gate_instance_list ';'
      { pform_makegates(@1, PGBuiltin::PULLDOWN, $3, 0, $5, 0); }

  | K_pulldown '(' dr_strength1 ',' dr_strength0 ')' gate_instance_list ';'
      { pform_makegates(@1, PGBuiltin::PULLDOWN, $5, 0, $7, 0); }

  | K_pulldown '(' dr_strength0 ',' dr_strength1 ')' gate_instance_list ';'
      { pform_makegates(@1, PGBuiltin::PULLDOWN, $3, 0, $7, 0); }

  /* This rule handles instantiations of modules and user defined
     primitives. These devices to not have delay lists or strengths,
     but then can have parameter lists. */

  | attribute_list_opt
	  IDENTIFIER parameter_value_opt gate_instance_list ';'
      { perm_string tmp1 = lex_strings.make($2);
		  pform_make_modgates(@2, tmp1, $3, $4, $1);
		  delete[]$2;
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
      { pform_make_pgassign_list(@1, $4, $3, $2); }

  /* Always and initial items are behavioral processes. */

  | attribute_list_opt K_always statement_item
      { PProcess*tmp = pform_make_behavior(IVL_PR_ALWAYS, $3, $1);
	FILE_NAME(tmp, @2);
      }
  | attribute_list_opt K_always_comb statement_item
      { PProcess*tmp = pform_make_behavior(IVL_PR_ALWAYS_COMB, $3, $1);
	FILE_NAME(tmp, @2);
      }
  | attribute_list_opt K_always_ff statement_item
      { PProcess*tmp = pform_make_behavior(IVL_PR_ALWAYS_FF, $3, $1);
	FILE_NAME(tmp, @2);
      }
  | attribute_list_opt K_always_latch statement_item
      { PProcess*tmp = pform_make_behavior(IVL_PR_ALWAYS_LATCH, $3, $1);
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

  | attribute_list_opt assertion_item

  | timeunits_declaration
      { pform_error_in_generate(@1, "timeunit declaration"); }

  | class_declaration

  | task_declaration

  | function_declaration

  /* A generate region can contain further module items. Actually, it
     is supposed to be limited to certain kinds of module items, but
     the semantic tests will check that for us. Do check that the
     generate/endgenerate regions do not nest. Generate schemes nest,
     but generate regions do not. */

  | K_generate { check_in_gen_region(@1); } generate_item_list_opt K_endgenerate { in_gen_region = false; }

  | K_genvar list_of_identifiers ';'
      { pform_genvars(@1, $2); }

  | K_for '(' K_genvar_opt IDENTIFIER '=' expression ';'
              expression ';'
              genvar_iteration ')'
      { pform_start_generate_for(@2, $3, $4, $6, $8, $10.text, $10.expr); }
    generate_block
      { pform_endgenerate(false); }

  | generate_if
    generate_block
    K_else
      { pform_start_generate_else(@1); }
    generate_block
      { pform_endgenerate(true); }

  | generate_if
    generate_block %prec less_than_K_else
      { pform_endgenerate(true); }

  | K_case '(' expression ')'
      { pform_start_generate_case(@1, $3); }
    generate_case_items
    K_endcase
      { pform_endgenerate(true); }

  /* Elaboration system tasks. */
  | SYSTEM_IDENTIFIER argument_list_parens_opt ';'
      { pform_make_elab_task(@1, lex_strings.make($1), *$2);
	delete[]$1;
	delete $2;
      }

  | modport_declaration

  /* 1364-2001 and later allow specparam declarations outside specify blocks. */

  | attribute_list_opt K_specparam
      { if (pform_in_interface())
	      yyerror(@2, "error: specparam declarations are not allowed "
			  "in interfaces.");
        pform_error_in_generate(@2, "specparam declaration");
      }
    specparam_decl ';'

  /* specify blocks are parsed but ignored. */

  | K_specify
      { if (pform_in_interface())
	      yyerror(@1, "error: specify blocks are not allowed "
			  "in interfaces.");
        pform_error_in_generate(@1, "specify block");
      }

    specify_item_list_opt K_endspecify

  | K_specify error K_endspecify
      { yyerror(@1, "error: Syntax error in specify block");
	yyerrok;
      }

  /* These rules match various errors that the user can type into
     module items. These rules try to catch them at a point where a
     reasonable error message can be produced. */

  | error ';'
      { yyerror(@2, "error: Invalid module item.");
	yyerrok;
      }

  | K_assign error '=' expression ';'
      { yyerror(@1, "error: Syntax error in left side of "
	            "continuous assignment.");
	yyerrok;
      }

  | K_assign error ';'
      { yyerror(@1, "error: Syntax error in continuous assignment");
	yyerrok;
      }

  | K_function error K_endfunction label_opt
      { yyerror(@1, "error: I give up on this function definition.");
	if ($4) {
	    pform_requires_sv(@4, "Function end label");
	    delete[]$4;
	}
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

  | ';'
      { }

  ;

let_port_list_opt
  : '(' let_port_list ')'
      { $$ = $2; }
  | '(' ')'
      { $$ = 0; }
  |
      { $$ = 0; }
  ;

let_port_list
  : let_port_item
      { std::list<PLet::let_port_t*>*tmp = new std::list<PLet::let_port_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  | let_port_list ',' let_port_item
      { std::list<PLet::let_port_t*>*tmp = $1;
        tmp->push_back($3);
        $$ = tmp;
      }
  ;

  // FIXME: What about the attributes?
let_port_item
  : attribute_list_opt let_formal_type IDENTIFIER dimensions_opt initializer_opt
      { perm_string tmp3 = lex_strings.make($3);
        $$ = pform_make_let_port($2, tmp3, $4, $5);
      }
  ;

let_formal_type
  : data_type_or_implicit
      { $$ = $1; }
  | K_untyped
      { $$ = 0; }
  ;

module_item_list
  : module_item_list module_item
  | module_item
  ;

module_item_list_opt
  : module_item_list
  |
  ;

generate_if
  : K_if '(' expression ')'
      { pform_start_generate_if(@1, $3); }
  ;

generate_case_items
  : generate_case_items generate_case_item
  | generate_case_item
  ;

generate_case_item
  : expression_list_proper ':'
      { pform_generate_case_item(@1, $1); }
    generate_block
      { pform_endgenerate(false); }
  | K_default ':'
      { pform_generate_case_item(@1, 0); }
    generate_block
      { pform_endgenerate(false); }
  ;

generate_item
  : module_item
  /* Handle some anachronistic syntax cases. */
  | K_begin generate_item_list_opt K_end
      { /* Detect and warn about anachronistic begin/end use */
	if (generation_flag > GN_VER2001 && warn_anachronisms) {
	      warn_count += 1;
	      cerr << @1 << ": warning: Anachronistic use of begin/end to surround generate schemes." << endl;
	}
      }
  | K_begin ':' IDENTIFIER
      { pform_start_generate_nblock(@1, $3); }
    generate_item_list_opt K_end
      { /* Detect and warn about anachronistic named begin/end use */
	if (generation_flag > GN_VER2001 && warn_anachronisms) {
	      warn_count += 1;
	      cerr << @1 << ": warning: Anachronistic use of named begin/end to surround generate schemes." << endl;
	}
	pform_endgenerate(false);
      }
  ;

generate_item_list
  : generate_item_list generate_item
  | generate_item
  ;

generate_item_list_opt
  :   { pform_generate_single_item = false; }
    generate_item_list
  |
  ;

  /* A generate block is the thing within a generate scheme. It may be
     a single module item, an anonymous block of module items, or a
     named module item. In all cases, the meat is in the module items
     inside, and the processing is done by the module_item rules. We
     only need to take note here of the scope name, if any. */

generate_block
  :   { pform_generate_single_item = true; }
    module_item
      { pform_generate_single_item = false; }
  | K_begin label_opt generate_item_list_opt K_end label_opt
      { if ($2)
	    pform_generate_block_name($2);
	check_end_label(@5, "block", $2, $5);
	delete[]$2;
      }
  ;

  /* A net declaration assignment allows the programmer to combine the
     net declaration and the continuous assignment into a single
     statement.

     Note that the continuous assignment statement is generated as a
     side effect, and all I pass up is the name of the l-value. */

net_decl_assign
  : IDENTIFIER '=' expression
      { decl_assignment_t*tmp = new decl_assignment_t;
	tmp->name = { lex_strings.make($1), @1.lexical_pos };
	tmp->expr.reset($3);
	delete[]$1;
	$$ = tmp;
      }
  ;

net_decl_assigns
  : net_decl_assigns ',' net_decl_assign
      { std::list<decl_assignment_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | net_decl_assign
      { std::list<decl_assignment_t*>*tmp = new std::list<decl_assignment_t*>;
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
  | K_wone    { $$ = NetNet::UNRESOLVED_WIRE;
		cerr << @1.text << ":" << @1.first_line << ": warning: "
		        "'wone' is deprecated, please use 'uwire' "
		        "instead." << endl;
	      }
  | K_uwire   { $$ = NetNet::UNRESOLVED_WIRE; }
  ;

net_type_opt
  : net_type { $$ = $1; }
  |          { $$ = NetNet::IMPLICIT; }
  ;

net_type_or_var
  : net_type { $$ = $1; }
  | K_var    { $$ = NetNet::REG; }

net_type_or_var_opt
  : net_type_opt { $$ = $1; }
  | K_var        { $$ = NetNet::REG; }
  ;

  /* The param_type rule is just the data_type_or_implicit rule wrapped
     with an assignment to para_data_type with the figured data type.
     This is used by parameter_assign, which is found to the right of
     the param_type in various rules. */

param_type
  : data_type_or_implicit
      { param_is_type = false;
        param_data_type = $1;
      }
  | type_param

parameter
  : K_parameter
      { param_is_local = false; }
  ;

localparam
  : K_localparam
      { param_is_local = true; }
  ;

parameter_declaration
  : parameter_or_localparam param_type parameter_assign_list ';'

parameter_or_localparam
  : parameter
  | localparam
  ;

  /* parameter and localparam assignment lists are broken into
     separate BNF so that I can call slightly different parameter
     handling code. localparams parse the same as parameters, they
     just behave differently when someone tries to override them. */

parameter_assign_list
  : parameter_assign
  | parameter_assign_list ',' parameter_assign
  ;

parameter_assign
  : IDENTIFIER dimensions_opt initializer_opt parameter_value_ranges_opt
      { pform_set_parameter(@1, lex_strings.make($1), param_is_local,
			    param_is_type, param_data_type, $2, $3, $4);
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
  | K_inf      { $$ = 0; }
  | '+' K_inf  { $$ = 0; }
  | '-' K_inf  { $$ = 0; }
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
     one value, so I'll accept simple numbers here. This also catches
     the case of a UDP with a single delay value, so we need to accept
     real values as well as decimal ones.

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
	lst->by_order = new std::list<PExpr*>;
	lst->by_order->push_back(tmp);
	lst->by_name = 0;
	$$ = lst;
	based_size = 0;
      }
  | '#' REALTIME
      { assert($2);
	PEFNumber*tmp = new PEFNumber($2);
	FILE_NAME(tmp, @1);

	struct parmvalue_t*lst = new struct parmvalue_t;
	lst->by_order = new std::list<PExpr*>;
	lst->by_order->push_back(tmp);
	lst->by_name = 0;
	$$ = lst;
      }
  | '#' error
      { yyerror(@1, "error: Syntax error in parameter value assignment list.");
	$$ = 0;
      }
  |
      { $$ = 0; }
  ;

named_expression
  : '.' IDENTIFIER '(' expression ')'
      { named_pexpr_t*tmp = new named_pexpr_t;
	FILE_NAME(tmp, @$);
	tmp->name = lex_strings.make($2);
	tmp->parm = $4;
	delete[]$2;
	$$ = tmp;
      }

named_expression_opt
  : named_expression
  | '.' IDENTIFIER '(' ')'
      { named_pexpr_t*tmp = new named_pexpr_t;
	FILE_NAME(tmp, @$);
	tmp->name = lex_strings.make($2);
	tmp->parm = 0;
	delete[]$2;
	$$ = tmp;
      }
  ;

parameter_value_byname_list
  : named_expression_opt
      { std::list<named_pexpr_t>*tmp = new std::list<named_pexpr_t>;
	tmp->push_back(*$1);
	delete $1;
	$$ = tmp;
      }
  | parameter_value_byname_list ',' named_expression_opt
      { std::list<named_pexpr_t>*tmp = $1;
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
  |      { $$ = 0; }
  ;

  /* The port_name rule is used with a module is being *instantiated*,
     and not when it is being declared. See the port rule if you are
     looking for the ports of a module declaration. */

port_name
  : attribute_list_opt named_expression_opt
      { delete $1;
	$$ = $2;
      }
  | attribute_list_opt '.' IDENTIFIER '(' error ')'
      { yyerror(@3, "error: Invalid port connection expression.");
	named_pexpr_t*tmp = new named_pexpr_t;
	FILE_NAME(tmp, @$);
	tmp->name = lex_strings.make($3);
	tmp->parm = 0;
	delete[]$3;
	delete $1;
	$$ = tmp;
      }
  | attribute_list_opt '.' IDENTIFIER
      { pform_requires_sv(@3, "Implicit named port connections");
	named_pexpr_t*tmp = new named_pexpr_t;
	FILE_NAME(tmp, @$);
	tmp->name = lex_strings.make($3);
	tmp->parm = new PEIdent(lex_strings.make($3), @3.lexical_pos, true);
	FILE_NAME(tmp->parm, @3);
	delete[]$3;
	delete $1;
	$$ = tmp;
      }
  | K_DOTSTAR
      { named_pexpr_t*tmp = new named_pexpr_t;
	FILE_NAME(tmp, @$);
	tmp->name = lex_strings.make("*");
	tmp->parm = 0;
	$$ = tmp;
      }
  ;

port_name_list
  : port_name_list ',' port_name
      { std::list<named_pexpr_t>*tmp = $1;
        tmp->push_back(*$3);
	delete $3;
	$$ = tmp;
      }
  | port_name
      { std::list<named_pexpr_t>*tmp = new std::list<named_pexpr_t>;
        tmp->push_back(*$1);
	delete $1;
	$$ = tmp;
      }
  ;

port_conn_expression_list_with_nuls
  : port_conn_expression_list_with_nuls ',' attribute_list_opt expression
      { std::list<PExpr*>*tmp = $1;
	tmp->push_back($4);
	delete $3;
	$$ = tmp;
      }
  | attribute_list_opt expression
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($2);
	delete $1;
	$$ = tmp;
      }
  |
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
        tmp->push_back(0);
	$$ = tmp;
      }
  | port_conn_expression_list_with_nuls ','
      { std::list<PExpr*>*tmp = $1;
	tmp->push_back(0);
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
	ptmp = pform_module_port_reference(@1, name);
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

	PEIdent*wtmp = new PEIdent(pname, @1.lexical_pos);
	FILE_NAME(wtmp, @1);

	Module::port_t*ptmp = new Module::port_t;
	ptmp->name = perm_string();
	ptmp->expr.push_back(wtmp);
	ptmp->default_value = 0;

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

	PEIdent*tmp = new PEIdent(pname, @1.lexical_pos);
	FILE_NAME(tmp, @1);

	Module::port_t*ptmp = new Module::port_t;
	ptmp->name = perm_string();
	ptmp->expr.push_back(tmp);
	ptmp->default_value = 0;
	delete[]$1;
	$$ = ptmp;
      }
  | IDENTIFIER '[' error ']'
      { yyerror(@1, "error: Invalid port bit select");
	Module::port_t*ptmp = new Module::port_t;
	PEIdent*wtmp = new PEIdent(lex_strings.make($1), @1.lexical_pos);
	FILE_NAME(wtmp, @1);
	ptmp->name = lex_strings.make($1);
	ptmp->expr.push_back(wtmp);
	ptmp->default_value = 0;
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
dimensions_opt
  :            { $$ = 0; }
  | dimensions { $$ = $1; }
  ;

dimensions
  : variable_dimension
      { $$ = $1; }
  | dimensions variable_dimension
      { std::list<pform_range_t> *tmp = $1;
	if ($2) {
	      tmp->splice(tmp->end(), *$2);
	      delete $2;
	}
	$$ = tmp;
      }
  ;

net_variable
  : IDENTIFIER dimensions_opt
      { pform_ident_t name = { lex_strings.make($1), @1.lexical_pos };
	$$ = pform_makewire(@1, name, NetNet::IMPLICIT, $2);
	delete [] $1;
      }
  ;

net_variable_list
  : net_variable
      { std::vector<PWire*> *tmp = new std::vector<PWire*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  | net_variable_list ',' net_variable
      { $1->push_back($3);
	$$ = $1;
      }
  ;

event_variable
  : IDENTIFIER dimensions_opt
      { if ($2) {
	      yyerror(@2, "sorry: event arrays are not supported.");
	      delete $2;
	}
	$$ = $1;
      }
  ;

event_variable_list
  : event_variable
      { $$ = list_from_identifier($1, @1.lexical_pos); }
  | event_variable_list ',' event_variable
      { $$ = list_from_identifier($1, $3, @3.lexical_pos); }
  ;

specify_item
  : K_specparam specparam_decl ';'
  | specify_simple_path_decl ';'
      { pform_module_specify_path($1); }
  | specify_edge_path_decl ';'
      { pform_module_specify_path($1); }
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
      { yywarn(@1, "sorry: ifnone with an edge-sensitive path is not supported.");
	yyerrok;
      }
  | K_Sfullskew '(' spec_reference_event ',' spec_reference_event
    ',' delay_value ',' delay_value fullskew_opt_args ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // spec_reference_event
	delete $7; // delay_value
	delete $9; // delay_value

	delete $10->notifier;
	delete $10->event_based_flag;
	delete $10->remain_active_flag;

	delete $10; // fullskew_opt_args
      }
  | K_Shold '(' spec_reference_event ',' spec_reference_event
    ',' delay_value spec_notifier_opt ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // spec_reference_event
	delete $7; // delay_value
	delete $8; // spec_notifier_opt
      }
  | K_Snochange '(' spec_reference_event ',' spec_reference_event
	  ',' delay_value ',' delay_value spec_notifier_opt ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // spec_reference_event
	delete $7; // delay_value
	delete $9; // delay_value
	delete $10; // spec_notifier_opt
      }
  | K_Speriod '(' spec_reference_event ',' delay_value
    spec_notifier_opt ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // delay_value
	delete $6; // spec_notifier_opt
      }
  | K_Srecovery '(' spec_reference_event ',' spec_reference_event
    ',' delay_value spec_notifier_opt ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // spec_reference_event
	delete $7; // delay_value
	delete $8; // spec_notifier_opt
      }
  | K_Srecrem '(' spec_reference_event ',' spec_reference_event
    ',' expr_mintypmax ',' expr_mintypmax recrem_opt_args ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported. ";
	if ($10->delayed_reference != nullptr || $10->delayed_data != nullptr)
	{
		cerr << "Delayed reference and data signals become copies of the"
		<< " original reference and data signals." << endl;
	}
	else
	{
		cerr << endl;
	}

	PRecRem*recrem = pform_make_recrem(@1, $3, $5, $7, $9, $10);
	pform_module_timing_check(recrem);

	delete $10; // setuphold_recrem_opt_notifier
      }
  | K_Sremoval '(' spec_reference_event ',' spec_reference_event
    ',' delay_value spec_notifier_opt ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // spec_reference_event
	delete $7; // delay_value
	delete $8; // spec_notifier_opt
      }
  | K_Ssetup '(' spec_reference_event ',' spec_reference_event
    ',' delay_value spec_notifier_opt ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // spec_reference_event
	delete $7; // delay_value
	delete $8; // spec_notifier_opt
      }
  | K_Ssetuphold '(' spec_reference_event ',' spec_reference_event
    ',' expr_mintypmax ',' expr_mintypmax setuphold_opt_args ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported. ";
	if ($10->delayed_reference != nullptr || $10->delayed_data != nullptr)
	{
		cerr << "Delayed reference and data signals become copies of the"
		<< " original reference and data signals." << endl;
	}
	else
	{
		cerr << endl;
	}

	PSetupHold*setuphold = pform_make_setuphold(@1, $3, $5, $7, $9, $10);
	pform_module_timing_check(setuphold);

	delete $10; // setuphold_recrem_opt_notifier
      }
  | K_Sskew '(' spec_reference_event ',' spec_reference_event
    ',' delay_value spec_notifier_opt ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // spec_reference_event
	delete $7; // delay_value
	delete $8; // spec_notifier_opt
      }
  | K_Stimeskew '(' spec_reference_event ',' spec_reference_event
    ',' delay_value timeskew_opt_args ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // spec_reference_event
	delete $7; // delay_value

	delete $8->notifier;
	delete $8->event_based_flag;
	delete $8->remain_active_flag;

	delete $8; // timeskew_opt_args
      }
  | K_Swidth '(' spec_reference_event ',' delay_value ',' expression
    spec_notifier_opt ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // delay_value
	delete $7; // expression
	delete $8;
      }
  | K_Swidth '(' spec_reference_event ',' delay_value ')' ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $3; // spec_reference_event
	delete $5; // delay_value
      }
  | K_pulsestyle_onevent specify_path_identifiers ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $2; // specify_path_identifiers
      }
  | K_pulsestyle_ondetect specify_path_identifiers ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $2; // specify_path_identifiers
      }
  | K_showcancelled specify_path_identifiers ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $2; // specify_path_identifiers
      }
  | K_noshowcancelled specify_path_identifiers ';'
      {
	cerr << @3 << ": warning: Timing checks are not supported." << endl;
	delete $2; // specify_path_identifiers
      }
  ;

specify_item_list
  : specify_item
  | specify_item_list specify_item
  ;

specify_item_list_opt
  : /* empty */
      {  }
  | specify_item_list
      {  }

specify_edge_path_decl
  : specify_edge_path '=' '(' delay_value_list ')'
      { $$ = pform_assign_path_delay($1, $4); }
  | specify_edge_path '=' delay_value_simple
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
	tmp->push_back($3);
	$$ = pform_assign_path_delay($1, tmp);
      }
  ;

edge_operator
  : K_posedge { $$ = true; }
  | K_negedge { $$ = false; }
  ;

specify_edge_path
  : '('               specify_path_identifiers spec_polarity
    K_EG '(' specify_path_identifiers polarity_operator expression ')' ')'
      { int edge_flag = 0;
	$$ = pform_make_specify_edge_path(@1, edge_flag, $2, $3, false, $6, $8);
      }
  | '(' edge_operator specify_path_identifiers spec_polarity
    K_EG '(' specify_path_identifiers polarity_operator expression ')' ')'
      { int edge_flag = $2? 1 : -1;
	$$ = pform_make_specify_edge_path(@1, edge_flag, $3, $4, false, $7, $9);
      }
  | '('               specify_path_identifiers spec_polarity
    K_SG  '(' specify_path_identifiers polarity_operator expression ')' ')'
      { int edge_flag = 0;
	$$ = pform_make_specify_edge_path(@1, edge_flag, $2, $3, true, $6, $8);
      }
  | '(' edge_operator specify_path_identifiers spec_polarity
    K_SG '(' specify_path_identifiers polarity_operator expression ')' ')'
      { int edge_flag = $2? 1 : -1;
	$$ = pform_make_specify_edge_path(@1, edge_flag, $3, $4, true, $7, $9);
      }
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
      { std::list<PExpr*>*tmp = new std::list<PExpr*>;
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
  : '(' specify_path_identifiers spec_polarity K_EG specify_path_identifiers ')'
      { $$ = pform_make_specify_path(@1, $2, $3, false, $5); }
  | '(' specify_path_identifiers spec_polarity K_SG specify_path_identifiers ')'
      { $$ = pform_make_specify_path(@1, $2, $3, true, $5); }
  | '(' error ')'
      { yyerror(@1, "Invalid simple path");
	yyerrok;
      }
  ;

specify_path_identifiers
  : IDENTIFIER
      { std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	$$ = tmp;
	delete[]$1;
      }
  | IDENTIFIER '[' expr_primary ']'
      { if (gn_specify_blocks_flag) {
	      yywarn(@4, "warning: Bit selects are not currently supported "
			 "in path declarations. The declaration "
			 "will be applied to the whole vector.");
	}
	std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	$$ = tmp;
	delete[]$1;
      }
  | IDENTIFIER '[' expr_primary polarity_operator expr_primary ']'
      { if (gn_specify_blocks_flag) {
	      yywarn(@4, "warning: Part selects are not currently supported "
			 "in path declarations. The declaration "
			 "will be applied to the whole vector.");
	}
	std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	$$ = tmp;
	delete[]$1;
      }
  | specify_path_identifiers ',' IDENTIFIER
      { std::list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	$$ = tmp;
	delete[]$3;
      }
  | specify_path_identifiers ',' IDENTIFIER '[' expr_primary ']'
      { if (gn_specify_blocks_flag) {
	      yywarn(@4, "warning: Bit selects are not currently supported "
			 "in path declarations. The declaration "
			 "will be applied to the whole vector.");
	}
	std::list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	$$ = tmp;
	delete[]$3;
      }
  | specify_path_identifiers ',' IDENTIFIER '[' expr_primary polarity_operator expr_primary ']'
      { if (gn_specify_blocks_flag) {
	      yywarn(@4, "warning: Part selects are not currently supported "
			 "in path declarations. The declaration "
			 "will be applied to the whole vector.");
	}
	std::list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	$$ = tmp;
	delete[]$3;
      }
  ;

specparam
  : IDENTIFIER '=' expr_mintypmax
      { pform_set_specparam(@1, lex_strings.make($1), specparam_active_range, $3);
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
  | dimensions
      { specparam_active_range = $1; }
    specparam_list
      { specparam_active_range = 0; }
  ;

spec_polarity
  : '+'  { $$ = '+'; }
  | '-'  { $$ = '-'; }
  |      { $$ = 0;   }
  ;

// TODO spec_controlled_reference_event
spec_reference_event
  : hierarchy_identifier
      { PTimingCheck::event_t* event = new PTimingCheck::event_t;
	event->name = *$1;
	event->posedge = false;
	event->negedge = false;
	event->condition = nullptr;
	delete $1;
	$$ = event;
      }
  | hierarchy_identifier K_TAND expression
      { PTimingCheck::event_t* event = new PTimingCheck::event_t;
	event->name = *$1;
	event->posedge = false;
	event->negedge = false;
	event->condition = std::unique_ptr<PExpr>($3);
	delete $1;
	$$ = event;
      }
  | K_posedge hierarchy_identifier
      { PTimingCheck::event_t* event = new PTimingCheck::event_t;
	event->name = *$2;
	event->posedge = true;
	event->negedge = false;
	event->condition = nullptr;
	delete $2;
	$$ = event;
      }
  | K_negedge hierarchy_identifier
      { PTimingCheck::event_t* event = new PTimingCheck::event_t;
	event->name = *$2;
	event->posedge = false;
	event->negedge = true;
	event->condition = nullptr;
	delete $2;
	$$ = event;
      }
  | K_posedge hierarchy_identifier K_TAND expression
      { PTimingCheck::event_t* event = new PTimingCheck::event_t;
	event->name = *$2;
	event->posedge = true;
	event->negedge = false;
	event->condition = std::unique_ptr<PExpr>($4);
	delete $2;
	$$ = event;
      }
  | K_negedge hierarchy_identifier K_TAND expression
      { PTimingCheck::event_t* event = new PTimingCheck::event_t;
	event->name = *$2;
	event->posedge = false;
	event->negedge = true;
	event->condition = std::unique_ptr<PExpr>($4);
	delete $2;
	$$ = event;
      }
  | K_edge '[' edge_descriptor_list ']' hierarchy_identifier
      { PTimingCheck::event_t* event = new PTimingCheck::event_t;
	event->name = *$5;
	event->posedge = false;
	event->negedge = false;
	// TODO add edge descriptors
	event->condition = nullptr;
	delete $5;
	$$ = event;
      }
  | K_edge '[' edge_descriptor_list ']' hierarchy_identifier K_TAND expression
      { PTimingCheck::event_t* event = new PTimingCheck::event_t;
	event->name = *$5;
	event->posedge = false;
	event->negedge = false;
	// TODO add edge descriptors
	event->condition = std::unique_ptr<PExpr>($7);
	delete $5;
	$$ = event;
      }
  ;

  /* The edge_descriptor is detected by the lexor as the various
     2-letter edge sequences that are supported here. For now, we
     don't care what they are, because we do not yet support specify
     edge events. */
edge_descriptor_list
  : edge_descriptor_list ',' K_edge_descriptor
  | K_edge_descriptor
  ;

setuphold_opt_args
  : setuphold_recrem_opt_notifier
    { $$ = $1; }
  |
    { $$ = new PTimingCheck::optional_args_t; }
  ;

recrem_opt_args
  : setuphold_recrem_opt_notifier
    { $$ = $1; }
  |
    { $$ = new PTimingCheck::optional_args_t; }
  ;

  /* The following rules are used for the optional arguments
     in $recrem and $setuphold */
setuphold_recrem_opt_notifier
  : ',' // Empty and end of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        $$ = args;
      }
  | ',' hierarchy_identifier // End of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        args->notifier = $2;
        $$ = args;
      }
  | ',' setuphold_recrem_opt_timestamp_cond // Empty
      { $$ = $2; }
  | ',' hierarchy_identifier setuphold_recrem_opt_timestamp_cond
        {
          $$ = $3;
          $$->notifier = $2;
        }
  ;

setuphold_recrem_opt_timestamp_cond
  : ',' // Empty and end of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        $$ = args;
      }
  | ',' expression // End of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        args->timestamp_cond = $2;
        $$ = args;
      }
  | ',' setuphold_recrem_opt_timecheck_cond // Empty
      { $$ = $2; }
  | ',' expression setuphold_recrem_opt_timecheck_cond
        {
          $$ = $3;
          $$->timestamp_cond = $2;
        }
  ;

setuphold_recrem_opt_timecheck_cond
  : ',' // Empty and end of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        $$ = args;
      }
  | ',' expression // End of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        args->timecheck_cond = $2;
        $$ = args;
      }
  | ',' setuphold_recrem_opt_delayed_reference // Empty
      { $$ = $2; }
  | ',' expression setuphold_recrem_opt_delayed_reference
        {
          $$ = $3;
          $$->timecheck_cond = $2;
        }
  ;

setuphold_recrem_opt_delayed_reference
  : ',' // Empty and end of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        $$ = args;
      }
  | ',' hierarchy_identifier // End of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        args->delayed_reference = $2;
        $$ = args;
      }
  | ',' setuphold_recrem_opt_delayed_data // Empty
      { $$ = $2; }
  | ',' hierarchy_identifier setuphold_recrem_opt_delayed_data
        {
          $$ = $3;
          $$->delayed_reference = $2;
        }
  ;

setuphold_recrem_opt_delayed_data
  : ',' // Empty and end of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        $$ = args;
      }
  | ',' hierarchy_identifier // End of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        args->delayed_data = $2;
        $$ = args;
      }
  ;

timeskew_opt_args
  : timeskew_fullskew_opt_notifier
    { $$ = $1; }
  |
    { $$ = new PTimingCheck::optional_args_t; }
  ;

fullskew_opt_args
  : timeskew_fullskew_opt_notifier
    { $$ = $1; }
  |
    { $$ = new PTimingCheck::optional_args_t; }
  ;

  /* The following rules are used for the optional arguments
     in $timeskew and $fullskew */
timeskew_fullskew_opt_notifier
  : ',' // Empty and end of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        $$ = args;
      }
  | ',' hierarchy_identifier // End of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        args->notifier = $2;
        $$ = args;
      }
  | ',' timeskew_fullskew_opt_event_based_flag // Empty
      { $$ = $2; }
  | ',' hierarchy_identifier timeskew_fullskew_opt_event_based_flag
        {
          $$ = $3;
          $$->notifier = $2;
        }
  ;

timeskew_fullskew_opt_event_based_flag
  : ',' // Empty and end of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        $$ = args;
      }
  | ',' expression // End of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        args->event_based_flag = $2;
        $$ = args;
      }
  | ',' timeskew_fullskew_opt_remain_active_flag // Empty
      { $$ = $2; }
  | ',' expression timeskew_fullskew_opt_remain_active_flag
        {
          $$ = $3;
          $$->event_based_flag = $2;
        }
  ;

timeskew_fullskew_opt_remain_active_flag
  : ',' // Empty and end of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        $$ = args;
      }
  | ',' expression // End of list
      {
        PTimingCheck::optional_args_t* args = new PTimingCheck::optional_args_t;
        args->remain_active_flag = $2;
        $$ = args;
      }
  ;

spec_notifier_opt
  : /* empty */
      { $$ = nullptr; }
  | spec_notifier
      { $$ = $1; }
  ;

spec_notifier
  : ','
      { $$ = nullptr; }
  | ','  hierarchy_identifier
      { $$ = $2; }
  ;

subroutine_call
  : hierarchy_identifier argument_list_parens_opt
      { PCallTask*tmp = pform_make_call_task(@1, *$1, *$2);
	delete $1;
	delete $2;
	$$ = tmp;
      }
  | class_hierarchy_identifier argument_list_parens_opt
      { PCallTask*tmp = new PCallTask(*$1, *$2);
	FILE_NAME(tmp, @1);
	delete $1;
	delete $2;
	$$ = tmp;
      }
  | SYSTEM_IDENTIFIER argument_list_parens_opt
      { PCallTask*tmp = new PCallTask(lex_strings.make($1), *$2);
	FILE_NAME(tmp,@1);
	delete[]$1;
	delete $2;
	$$ = tmp;
      }
  | hierarchy_identifier '(' error ')'
      { yyerror(@3, "error: Syntax error in task arguments.");
	std::list<named_pexpr_t> pt;
	PCallTask*tmp = pform_make_call_task(@1, *$1, pt);
	delete $1;
	$$ = tmp;
      }
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

  /* In SystemVerilog an unnamed block can contain variable declarations. */
  | K_begin label_opt
      { PBlock*tmp = pform_push_block_scope(@1, $2, PBlock::BL_SEQ);
	current_block_stack.push(tmp);
      }
    block_item_decls_opt
      {
        if (!$2) {
	      if ($4) {
		    pform_block_decls_requires_sv();
	      } else {
		    /* If there are no declarations in the scope then just delete it. */
		    pform_pop_scope();
		    assert(! current_block_stack.empty());
		    PBlock*tmp = current_block_stack.top();
		    current_block_stack.pop();
		    delete tmp;
	      }
	}
      }
    statement_or_null_list_opt K_end label_opt
      { PBlock*tmp;
	if ($2 || $4) {
	      pform_pop_scope();
	      assert(! current_block_stack.empty());
	      tmp = current_block_stack.top();
	      current_block_stack.pop();
	} else {
	      tmp = new PBlock(PBlock::BL_SEQ);
	      FILE_NAME(tmp, @1);
	}
	if ($6) tmp->set_statement(*$6);
	delete $6;
	check_end_label(@8, "block", $2, $8);
	delete[]$2;
	$$ = tmp;
      }

  /* fork-join blocks are very similar to begin-end blocks. In fact,
     from the parser's perspective there is no real difference. All we
     need to do is remember that this is a parallel block so that the
     code generator can do the right thing. */

  /* In SystemVerilog an unnamed block can contain variable declarations. */
  | K_fork label_opt
      { PBlock*tmp = pform_push_block_scope(@1, $2, PBlock::BL_PAR);
	current_block_stack.push(tmp);
      }
    block_item_decls_opt
      {
        if (!$2) {
	      if ($4) {
		    pform_requires_sv(@4, "Variable declaration in unnamed block");
	      } else {
		    /* If there are no declarations in the scope then just delete it. */
		    pform_pop_scope();
		    assert(! current_block_stack.empty());
		    PBlock*tmp = current_block_stack.top();
		    current_block_stack.pop();
		    delete tmp;
	      }
	}
      }
    statement_or_null_list_opt join_keyword label_opt
      { PBlock*tmp;
	if ($2 || $4) {
	      pform_pop_scope();
	      assert(! current_block_stack.empty());
	      tmp = current_block_stack.top();
	      current_block_stack.pop();
	      tmp->set_join_type($7);
	} else {
	      tmp = new PBlock($7);
	      FILE_NAME(tmp, @1);
	}
	if ($6) tmp->set_statement(*$6);
	delete $6;
	check_end_label(@8, "fork", $2, $8);
	delete[]$2;
	$$ = tmp;
      }

  | K_disable hierarchy_identifier ';'
      { PDisable*tmp = new PDisable(*$2);
	FILE_NAME(tmp, @1);
	delete $2;
	$$ = tmp;
      }
  | K_disable K_fork ';'
      { pform_name_t tmp_name;
	PDisable*tmp = new PDisable(tmp_name);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_TRIGGER hierarchy_identifier ';'
      { PTrigger*tmp = pform_new_trigger(@2, 0, *$2, @2.lexical_pos);
	delete $2;
	$$ = tmp;
      }
  | K_TRIGGER package_scope hierarchy_identifier
      { lex_in_package_scope(0);
	PTrigger*tmp = pform_new_trigger(@3, $2, *$3, @3.lexical_pos);
	delete $3;
	$$ = tmp;
      }
    /* FIXME: Does this need support for package resolution like above? */
  | K_NB_TRIGGER hierarchy_identifier ';'
      { PNBTrigger*tmp = pform_new_nb_trigger(@2, 0, *$2, @2.lexical_pos);
	delete $2;
	$$ = tmp;
      }
  | K_NB_TRIGGER delay1 hierarchy_identifier ';'
      { PNBTrigger*tmp = pform_new_nb_trigger(@3, $2, *$3, @3.lexical_pos);
	delete $3;
	$$ = tmp;
      }
  | K_NB_TRIGGER event_control hierarchy_identifier ';'
      { PNBTrigger*tmp = pform_new_nb_trigger(@3, 0, *$3, @3.lexical_pos);
	delete $3;
	$$ = tmp;
        yywarn(@1, "sorry: ->> with event control is not currently supported.");
      }
  | K_NB_TRIGGER K_repeat '(' expression ')' event_control hierarchy_identifier ';'
      { PNBTrigger*tmp = pform_new_nb_trigger(@7, 0, *$7, @7.lexical_pos);
	delete $7;
	$$ = tmp;
        yywarn(@1, "sorry: ->> with repeat event control is not currently supported.");
      }

  | procedural_assertion_statement
      { $$ = $1; }

  | loop_statement
      { $$ = $1; }

  | jump_statement
      { $$ = $1; }

  | unique_priority K_case '(' expression ')' case_items K_endcase
      { PCase*tmp = new PCase($1, NetCase::EQ, $4, $6);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | unique_priority K_casex '(' expression ')' case_items K_endcase
      { PCase*tmp = new PCase($1, NetCase::EQX, $4, $6);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | unique_priority K_casez '(' expression ')' case_items K_endcase
      { PCase*tmp = new PCase($1, NetCase::EQZ, $4, $6);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | unique_priority K_case '(' expression ')' error K_endcase
      { yyerrok; }
  | unique_priority K_casex '(' expression ')' error K_endcase
      { yyerrok; }
  | unique_priority K_casez '(' expression ')' error K_endcase
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
  /* SystemVerilog adds the compressed_statement */

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
      }
  | error K_LE expression ';'
      { yyerror(@2, "Syntax in assignment statement l-value.");
	yyerrok;
	$$ = new PNoop;
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
      }
  | lpvalue '=' event_control expression ';'
      { PAssign*tmp = new PAssign($1,0,$3,$4);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | lpvalue '=' K_repeat '(' expression ')' event_control expression ';'
      { PAssign*tmp = new PAssign($1,$5,$7,$8);
	FILE_NAME(tmp,@1);
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
  | K_wait K_fork ';'
      { PEventStatement*tmp = new PEventStatement((PEEvent*)0);
	FILE_NAME(tmp,@1);
	$$ = tmp;
      }
  | K_void '\'' '(' subroutine_call ')' ';'
      { $4->void_cast();
	$$ = $4;
      }

  | subroutine_call ';'
      { $$ = $1;
      }

  | hierarchy_identifier K_with '{' constraint_block_item_list_opt '}' ';'
      { /* ....randomize with { <constraints> } */
	if ($1 && peek_tail_name(*$1) == "randomize") {
	      if (pform_requires_sv(@2, "Randomize with constraint"))
		    yyerror(@2, "sorry: Randomize with constraint not supported.");
	} else {
	      yyerror(@2, "error: Constraint block can only be applied to randomize method.");
	}
	list<named_pexpr_t> pt;
	PCallTask*tmp = new PCallTask(*$1, pt);
	FILE_NAME(tmp, @1);
	delete $1;
	$$ = tmp;
      }

    /* IEEE1800 A.1.8: class_constructor_declaration with a call to
       parent constructor. Note that the implicit_class_handle must
       be K_super ("this.new" makes little sense) but that would
       cause a conflict. Anyhow, this statement must be in the
       beginning of a constructor, but let the elaborator figure that
       out. */

  | implicit_class_handle K_new argument_list_parens_opt ';'
      { PChainConstructor*tmp = new PChainConstructor(*$3);
	FILE_NAME(tmp, @3);
	if (peek_head_name(*$1) == THIS_TOKEN) {
	      yyerror(@1, "error: this.new is invalid syntax. Did you mean super.new?");
	}
	delete $1;
	$$ = tmp;
      }
  | error ';'
      { yyerror(@2, "error: Malformed statement");
	yyerrok;
	$$ = new PNoop;
      }

  ;

compressed_operator
  : K_PLUS_EQ  { $$ = '+'; }
  | K_MINUS_EQ { $$ = '-'; }
  | K_MUL_EQ   { $$ = '*'; }
  | K_DIV_EQ   { $$ = '/'; }
  | K_MOD_EQ   { $$ = '%'; }
  | K_AND_EQ   { $$ = '&'; }
  | K_OR_EQ    { $$ = '|'; }
  | K_XOR_EQ   { $$ = '^'; }
  | K_LS_EQ    { $$ = 'l'; }
  | K_RS_EQ    { $$ = 'r'; }
  | K_RSS_EQ   { $$ = 'R'; }
  ;

compressed_statement
  : lpvalue compressed_operator expression
      { PAssign*tmp = new PAssign($1, $2, $3);
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
      { std::vector<Statement*>*tmp = $1;
	if ($2) tmp->push_back($2);
	$$ = tmp;
      }
  | statement_or_null
      { std::vector<Statement*>*tmp = new std::vector<Statement*>(0);
	if ($1) tmp->push_back($1);
	$$ = tmp;
      }
  ;

analog_statement
  : branch_probe_expression K_CONTRIBUTE expression ';'
      { $$ = pform_contribution_statement(@2, $1, $3); }
  ;

tf_port_list_opt
  : tf_port_list { $$ = $1; }
  |              { $$ = 0; }
  ;

  /* A task or function prototype can be declared with the task/function name
   * followed by a port list in parenthesis or or just the task/function name by
   * itself. When a port list is used it might be empty. */
tf_port_list_parens_opt
  : '(' tf_port_list_opt ')' { $$ = $2; }
  |                          { $$ = 0; }

  /* Note that the lexor notices the "table" keyword and starts
     the UDPTABLE state. It needs to happen there so that all the
     characters in the table are interpreted in that mode. It is still
     up to this rule to take us out of the UDPTABLE state. */
udp_body
  : K_table udp_entry_list K_endtable
      { lex_end_table();
	$$ = $2;
      }
  | K_table K_endtable
      { lex_end_table();
	yyerror(@1, "error: Empty UDP table.");
	$$ = 0;
      }
  | K_table error K_endtable
      { lex_end_table();
	yyerror(@2, "errors in UDP table");
	yyerrok;
	$$ = 0;
      }
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
      { std::list<string>*tmp = new std::list<string>;
	tmp->push_back($1);
	delete[]$1;
	$$ = tmp;
      }
  | udp_comb_entry_list udp_comb_entry
      { std::list<string>*tmp = $1;
	tmp->push_back($2);
	delete[]$2;
	$$ = tmp;
      }
  ;

udp_sequ_entry_list
  : udp_sequ_entry
      { std::list<string>*tmp = new std::list<string>;
	tmp->push_back($1);
	delete[]$1;
	$$ = tmp;
      }
  | udp_sequ_entry_list udp_sequ_entry
      { std::list<string>*tmp = $1;
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
	PEIdent*itmp = new PEIdent(lex_strings.make($2), @2.lexical_pos);
	PAssign*atmp = new PAssign(itmp, etmp);
	FILE_NAME(atmp, @2);
	delete[]$2;
	$$ = atmp;
      }
  ;

udp_init_opt
  : udp_initial { $$ = $1; }
  |             { $$ = 0; }
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
  | DEC_NUMBER
        { yyerror(@1, "internal error: Input digits parse as decimal number!");
          $$ = '0';
        }
  ;

udp_output_sym
  : '0' { $$ = '0'; }
  | '1' { $$ = '1'; }
  | 'x' { $$ = 'x'; }
  | '-' { $$ = '-'; }
  | DEC_NUMBER
        { yyerror(@1, "internal error: Output digits parse as decimal number!");
          $$ = '0';
        }
  ;

  /* Port declarations create wires for the inputs and the output. The
     makes for these ports are scoped within the UDP, so there is no
     hierarchy involved. */
udp_port_decl
  : K_input list_of_identifiers ';'
      { $$ = pform_make_udp_input_ports($2); }
  | K_output IDENTIFIER ';'
      { perm_string pname = lex_strings.make($2);
	PWire*pp = new PWire(pname, @2.lexical_pos, NetNet::IMPLICIT, NetNet::POUTPUT);
	vector<PWire*>*tmp = new std::vector<PWire*>(1);
	(*tmp)[0] = pp;
	$$ = tmp;
	delete[]$2;
      }
  | K_reg IDENTIFIER ';'
      { perm_string pname = lex_strings.make($2);
	PWire*pp = new PWire(pname, @2.lexical_pos, NetNet::REG, NetNet::PIMPLICIT);
	vector<PWire*>*tmp = new std::vector<PWire*>(1);
	(*tmp)[0] = pp;
	$$ = tmp;
	delete[]$2;
      }
  | K_output K_reg IDENTIFIER ';'
      { perm_string pname = lex_strings.make($3);
	PWire*pp = new PWire(pname, @3.lexical_pos, NetNet::REG, NetNet::POUTPUT);
	vector<PWire*>*tmp = new std::vector<PWire*>(1);
	(*tmp)[0] = pp;
	$$ = tmp;
	delete[]$3;
      }
    ;

udp_port_decls
  : udp_port_decl
      { $$ = $1; }
  | udp_port_decls udp_port_decl
      { std::vector<PWire*>*tmp = $1;
	size_t s1 = $1->size();
	tmp->resize(s1+$2->size());
	for (size_t idx = 0 ; idx < $2->size() ; idx += 1)
	      tmp->at(s1+idx) = $2->at(idx);
	$$ = tmp;
	delete $2;
      }
  ;

udp_port_list
  : IDENTIFIER
      { $$ = list_from_identifier($1, @1.lexical_pos); }
  | udp_port_list ',' IDENTIFIER
      { $$ = list_from_identifier($1, $3, @3.lexical_pos); }
  ;

udp_reg_opt
  : K_reg  { $$ = true; }
  |        { $$ = false; };

udp_input_declaration_list
  : K_input IDENTIFIER
      { $$ = list_from_identifier($2, @2.lexical_pos); }
  | udp_input_declaration_list ',' K_input IDENTIFIER
      { $$ = list_from_identifier($1, $4, @4.lexical_pos); }
  ;

udp_primitive
        /* This is the syntax for primitives that uses the IEEE1364-1995
	   format. The ports are simply names in the port list, and the
	   declarations are in the body. */

  : K_primitive IDENTIFIER '(' udp_port_list ')' ';'
    udp_port_decls
    udp_init_opt
    udp_body
    K_endprimitive label_opt
      { perm_string tmp2 = lex_strings.make($2);
	pform_make_udp(@2, tmp2, $4, $7, $9, $8);
	check_end_label(@11, "primitive", $2, $11);
	delete[]$2;
      }

        /* This is the syntax for IEEE1364-2001 format definitions. The port
	   names and declarations are all in the parameter list. */

  | K_primitive IDENTIFIER
    '(' K_output udp_reg_opt IDENTIFIER initializer_opt ','
    udp_input_declaration_list ')' ';'
    udp_body
    K_endprimitive label_opt
      { perm_string tmp2 = lex_strings.make($2);
	pform_ident_t tmp6 = { lex_strings.make($6) , @6.lexical_pos };
	pform_make_udp(@2, tmp2, $5, tmp6, $7, $9, $12);
	check_end_label(@14, "primitive", $2, $14);
	delete[]$2;
	delete[]$6;
      }
  ;

unique_priority
  :             { $$ = IVL_CASE_QUALITY_BASIC; }
  | K_unique    { $$ = IVL_CASE_QUALITY_UNIQUE; }
  | K_unique0   { $$ = IVL_CASE_QUALITY_UNIQUE0; }
  | K_priority  { $$ = IVL_CASE_QUALITY_PRIORITY; }
  ;

  /* Many keywords can be optional in the syntax, although their
     presence is significant. This is a fairly common pattern so
     collect those rules here. */

K_const_opt
 : K_const { $$ = true; }
 |         { $$ = false; }
 ;

K_genvar_opt
 : K_genvar { $$ = true; }
 |          { $$ = false; }
 ;

K_static_opt
 : K_static { $$ = true; }
 |          { $$ = false; }
 ;

K_virtual_opt
  : K_virtual { $$ = true; }
  |           { $$ = false; }
  ;

K_var_opt
  : K_var
  |
  ;
