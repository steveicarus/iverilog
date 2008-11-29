#ifndef __pform_H
#define __pform_H
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

# include  "netlist.h"
# include  "HName.h"
# include  "named.h"
# include  "Module.h"
# include  "Statement.h"
# include  "AStatement.h"
# include  "PGate.h"
# include  "PExpr.h"
# include  "PTask.h"
# include  "PUdp.h"
# include  "PWire.h"
# include  "verinum.h"
# include  "discipline.h"
# include  <iostream>
# include  <string>
# include  <list>
# include  <stdio.h>

/*
 * These classes implement the parsed form (P-form for short) of the
 * original Verilog source. the parser generates the pform for the
 * convenience of later processing steps.
 */


/*
 * Wire objects represent the named wires (of various flavor) declared
 * in the source.
 *
 * Gate objects are the functional modules that are connected together
 * by wires.
 *
 * Wires and gates, connected by joints, represent a netlist. The
 * netlist is therefore a representation of the desired circuit.
 */
class PGate;
class PExpr;
class PSpecPath;
struct vlltype;

/*
 * The min:typ:max expression s selected at parse time using the
 * enumeration. When the compiler makes a choice, it also prints a
 * warning if min_typ_max_warn > 0.
 */
extern enum MIN_TYP_MAX { MIN, TYP, MAX } min_typ_max_flag;
extern unsigned min_typ_max_warn;
PExpr* pform_select_mtm_expr(PExpr*min, PExpr*typ, PExpr*max);

/*
 * This flag is true if the lexor thinks we are in a library source
 * file.
 */
extern bool pform_library_flag;

/*
 * These type are lexical types -- that is, types that are used as
 * lexical values to decorate the parse tree during parsing. They are
 * not in any way preserved once parsing is done.
 */

/* This is information about port name information for named port
   connections. */

//typedef struct named<PExpr*> named_pexpr_t;
typedef named<PExpr*> named_pexpr_t;

struct parmvalue_t {
      svector<PExpr*>*by_order;
      svector<named_pexpr_t*>*by_name;
};

struct str_pair_t { PGate::strength_t str0, str1; };

struct net_decl_assign_t {
      perm_string name;
      PExpr*expr;
      struct net_decl_assign_t*next;
};

/* The lgate is gate instantiation information. */
struct lgate {
      lgate(int =0)
      : parms(0), parms_by_name(0), lineno(0)
      { range[0] = 0;
        range[1] = 0;
      }

      string name;
      svector<PExpr*>*parms;
      svector<named_pexpr_t*>*parms_by_name;

      PExpr*range[2];

      const char* file;
      unsigned lineno;
};

  /* Use this function to transform the parted form of the attribute
     list to the attribute map that is used later. */
extern void pform_bind_attributes(map<perm_string,PExpr*>&attributes,
				  svector<named_pexpr_t*>*attr);

  /* The lexor calls this function to change the default nettype. */
extern void pform_set_default_nettype(NetNet::Type net,
				     const char*file,
				     unsigned lineno);

/*
 * Look for the given wire in the current lexical scope. If the wire
 * (including variables of any type) cannot be found in the current
 * scope, then return 0.
 */
extern PWire* pform_get_wire_in_scope(perm_string name);

/*
 * The parser uses startmodule and endmodule together to build up a
 * module as it parses it. The startmodule tells the pform code that a
 * module has been noticed in the source file and the following events
 * are to apply to the scope of that module. The endmodule causes the
 * pform to close up and finish the named module.
 */
extern void pform_startmodule(const char*, const char*file, unsigned lineno,
			      svector<named_pexpr_t*>*attr);
extern void pform_module_set_ports(vector<Module::port_t*>*);

/* This function is used to support the port definition in a
   port_definition_list. In this case, we have everything needed to
   define the port, all in one place. */
extern void pform_module_define_port(const struct vlltype&li,
				     perm_string name,
				     NetNet::PortType,
				     NetNet::Type type,
				     bool signed_flag,
				     svector<PExpr*>*range,
				     svector<named_pexpr_t*>*attr);

extern Module::port_t* pform_module_port_reference(perm_string name,
						   const char*file,
						   unsigned lineno);
extern void pform_endmodule(const char*);

extern void pform_make_udp(perm_string name, list<perm_string>*parms,
			   svector<PWire*>*decl, list<string>*table,
			   Statement*init,
			   const char*file, unsigned lineno);

extern void pform_make_udp(perm_string name,
			   bool sync_flag, perm_string out_name,
			   PExpr*sync_init,
			   list<perm_string>*parms,
			   list<string>*table,
			   const char*file, unsigned lineno);

/*
 * Enter/exit name scopes. The push_scope function pushes the scope
 * name string onto the scope hierarchy. The pop pulls it off and
 * deletes it. Thus, the string pushed must be allocated.
 */
extern void pform_pop_scope();

extern PTask*pform_push_task_scope(char*name, bool is_auto);
extern PFunction*pform_push_function_scope(char*name, bool is_auto);
extern PBlock*pform_push_block_scope(char*name, PBlock::BL_TYPE tt);

extern void pform_put_behavior_in_scope(AProcess*proc);

extern verinum* pform_verinum_with_size(verinum*s, verinum*val,
					const char*file, unsigned lineno);

/*
 * This function takes the list of names as new genvars to declare in
 * the current module scope.
 */
extern void pform_genvars(const struct vlltype&li, list<perm_string>*names);

extern void pform_start_generate_for(const struct vlltype&li,
				     char*ident1,
				     PExpr*init,
				     PExpr*test,
				     char*ident2,
				     PExpr*next);
extern void pform_start_generate_if(const struct vlltype&li, PExpr*test);
extern void pform_start_generate_else(const struct vlltype&li);
extern void pform_start_generate_case(const struct vlltype&lp, PExpr*test);
extern void pform_start_generate_nblock(const struct vlltype&lp, char*name);
extern void pform_generate_case_item(const struct vlltype&lp, svector<PExpr*>*test);
extern void pform_generate_block_name(char*name);
extern void pform_endgenerate();


/*
 * The makewire functions announce to the pform code new wires. These
 * go into a module that is currently opened.
 */
extern void pform_makewire(const struct vlltype&li, perm_string name,
			   NetNet::Type type,
			   NetNet::PortType pt,
			   ivl_variable_type_t,
			   svector<named_pexpr_t*>*attr);

/* This form handles simple declarations */
extern void pform_makewire(const struct vlltype&li,
			   svector<PExpr*>*range,
			   bool signed_flag,
			   list<perm_string>*names,
			   NetNet::Type type,
			   NetNet::PortType,
			   ivl_variable_type_t,
			   svector<named_pexpr_t*>*attr,
			   PWSRType rt = SR_NET);

/* This form handles assignment declarations. */
extern void pform_makewire(const struct vlltype&li,
			   svector<PExpr*>*range,
			   bool signed_flag,
			   svector<PExpr*>*delay,
			   str_pair_t str,
			   net_decl_assign_t*assign_list,
			   NetNet::Type type,
			   ivl_variable_type_t);

extern void pform_make_reginit(const struct vlltype&li,
			       perm_string name, PExpr*expr);

  /* Look up the names of the wires, and set the port type,
     i.e. input, output or inout. If the wire does not exist, create
     it. The second form takes a single name. */
extern void pform_set_port_type(const struct vlltype&li,
				list<perm_string>*names,
				svector<PExpr*>*range,
				bool signed_flag,
				NetNet::PortType);
extern void pform_set_port_type(perm_string nm, NetNet::PortType pt,
				const char*file, unsigned lineno);

extern void pform_set_net_range(list<perm_string>*names,
				svector<PExpr*>*,
				bool signed_flag,
				ivl_variable_type_t,
				PWSRType rt = SR_NET);
extern void pform_set_reg_idx(perm_string name, PExpr*l, PExpr*r);
extern void pform_set_reg_integer(list<perm_string>*names);
extern void pform_set_reg_time(list<perm_string>*names);

  /* pform_set_attrib and pform_set_type_attrib exist to support the
     $attribute syntax, which can only set string values to
     attributes. The functions keep the value strings that are
     passed in. */
extern void pform_set_attrib(perm_string name, perm_string key,
			     char*value);
extern void pform_set_type_attrib(perm_string name, const string&key,
				  char*value);

extern LexicalScope::range_t* pform_parameter_value_range(bool exclude_flag,
						    bool low_open, PExpr*low_expr,
						    bool hig_open, PExpr*hig_expr);

extern void pform_set_parameter(const struct vlltype&loc,
				perm_string name,
				ivl_variable_type_t type,
				bool signed_flag,
				svector<PExpr*>*range,
				PExpr*expr, LexicalScope::range_t*value_range);
extern void pform_set_localparam(const struct vlltype&loc,
				 perm_string name,
				 ivl_variable_type_t type,
				 bool signed_flag,
				 svector<PExpr*>*range,
				 PExpr*expr);
extern void pform_set_defparam(const pform_name_t&name, PExpr*expr);

/*
 * Functions related to specify blocks.
 */
extern void pform_set_specparam(perm_string name, PExpr*expr);

extern PSpecPath*pform_make_specify_path(const struct vlltype&li,
					 list<perm_string>*src, char pol,
					 bool full_flag, list<perm_string>*dst);
extern PSpecPath*pform_make_specify_edge_path(const struct vlltype&li,
					 int edge_flag, /*posedge==true */
					 list<perm_string>*src, char pol,
					 bool full_flag, list<perm_string>*dst,
					 PExpr*data_source_expression);
extern PSpecPath*pform_assign_path_delay(PSpecPath*obj, svector<PExpr*>*delays);

extern void pform_module_specify_path(PSpecPath*obj);

/*
 * pform_make_behavior creates processes that are declared with always
 * or initial items.
 */
extern PProcess*  pform_make_behavior(ivl_process_type_t, Statement*,
				      svector<named_pexpr_t*>*attr);

extern svector<PWire*>* pform_make_udp_input_ports(list<perm_string>*);

extern void pform_make_events(list<perm_string>*names,
			      const char*file, unsigned lineno);
/*
 * Make real datum objects.
 */
extern void pform_make_reals(list<perm_string>*names,
			     const char*file, unsigned lineno);

/*
 * The makegate function creates a new gate (which need not have a
 * name) and connects it to the specified wires.
 */
extern void pform_makegates(PGBuiltin::Type type,
			    struct str_pair_t str,
			    svector<PExpr*>*delay,
			    svector<lgate>*gates,
			    svector<named_pexpr_t*>*attr);

extern void pform_make_modgates(perm_string type,
				struct parmvalue_t*overrides,
				svector<lgate>*gates);

/* Make a continuous assignment node, with optional bit- or part- select. */
extern void pform_make_pgassign_list(svector<PExpr*>*alist,
				     svector<PExpr*>*del,
				     struct str_pair_t str,
				     const char* fn, unsigned lineno);

/* Given a port type and a list of names, make a list of wires that
   can be used as task port information. */
extern svector<PWire*>*pform_make_task_ports(NetNet::PortType pt,
					     ivl_variable_type_t vtype,
					     bool signed_flag,
					     svector<PExpr*>*range,
					     list<perm_string>*names,
					     const char* file,
					     unsigned lineno);


/*
 * These are functions that the outside-the-parser code uses the do
 * interesting things to the Verilog. The parse function reads and
 * parses the source file and places all the modules it finds into the
 * mod list. The dump function dumps a module to the output stream.
 */
extern void pform_dump(ostream&out, Module*mod);

/*
 * Used to report the original module location when a nested module
 * (missing endmodule) is found by the parser.
 */
extern void pform_error_nested_modules();

/* ** pform_discipline.cc
 * Functions for handling the parse of natures and disciplines. These
 * functions are in pform_disciplines.cc
 */

extern void pform_start_nature(const char*name);
extern void pform_end_nature(const struct vlltype&loc);

extern void pform_nature_access(const struct vlltype&loc, const char*name);

extern void pform_start_discipline(const char*name);
extern void pform_end_discipline(const struct vlltype&loc);

extern void pform_discipline_domain(const struct vlltype&loc, ivl_dis_domain_t use_domain);
extern void pform_discipline_potential(const struct vlltype&loc, const char*name);
extern void pform_discipline_flow(const struct vlltype&loc, const char*name);

extern void pform_attach_discipline(const struct vlltype&loc,
				    ivl_discipline_t discipline, list<perm_string>*names);

extern void pform_dump(ostream&out, const ivl_nature_s*);
extern void pform_dump(ostream&out, const ivl_discipline_s*);

/* ** pform_analog.cc
*/
extern void pform_make_analog_behavior(const struct vlltype&loc,
				       ivl_process_type_t type, Statement*st);

extern AContrib*pform_contribution_statement(const struct vlltype&loc,
					     PExpr*lval, PExpr*rval);

extern PExpr* pform_make_branch_probe_expression(const struct vlltype&loc,
						 char*name, char*n1, char*n2);

extern PExpr* pform_make_branch_probe_expression(const struct vlltype&loc,
						 char*name, char*branch);
#endif
