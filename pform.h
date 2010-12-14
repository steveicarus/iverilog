#ifndef __pform_H
#define __pform_H
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

# include  "netlist.h"
# include  "HName.h"
# include  "named.h"
# include  "Module.h"
# include  "Statement.h"
# include  "PGate.h"
# include  "PExpr.h"
# include  "PTask.h"
# include  "PUdp.h"
# include  "PWire.h"
# include  "verinum.h"
# include  <iostream>
# include  <string>
# include  <list>
# include  <stdio.h>
# include  <cstring>
# include  <cstdlib>

/*
 * These classes implement the parsed form (P-form for short) of the
 * original verilog source. the parser generates the pform for the
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
 * These type are lexical types -- that is, types that are used as
 * lexical values to decorate the parse tree during parsing. They are
 * not in any way preserved once parsing is done.
 */

/* This is information about port name information for named port
   connections. */

typedef struct named<PExpr*> named_pexpr_t;

struct parmvalue_t {
      svector<PExpr*>*by_order;
      svector<named_pexpr_t*>*by_name;
};

struct str_pair_t { PGate::strength_t str0, str1; };

struct net_decl_assign_t {
      char*name;
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

  /* The lexor calls this function to change the default nettype. */
extern void pform_set_default_nettype(NetNet::Type net,
				     const char*file,
				     unsigned lineno);

/*
 * The parser uses startmodule and endmodule together to build up a
 * module as it parses it. The startmodule tells the pform code that a
 * module has been noticed in the source file and the following events
 * are to apply to the scope of that module. The endmodule causes the
 * pform to close up and finish the named module.
 */
extern void pform_startmodule(const char*, const char*file, unsigned lineno,
			      svector<named_pexpr_t*>*attr);
extern void pform_module_set_ports(svector<Module::port_t*>*);

/* This function is used to support the port definition in a
   port_definition_list. In this case, we have everything needed to
   define the port, all in one place. */
extern void pform_module_define_port(const struct vlltype&li,
				     const char*name,
				     NetNet::PortType,
				     NetNet::Type type,
				     bool signed_flag,
				     svector<PExpr*>*range,
				     svector<named_pexpr_t*>*attr);

extern Module::port_t* pform_module_port_reference(char*name,
						   const char*file,
						   unsigned lineno);
extern void pform_endmodule(const char*);

extern void pform_make_udp(perm_string name, list<string>*parms,
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
extern void pform_push_scope(char*name);
extern void pform_pop_scope();


extern verinum* pform_verinum_with_size(verinum*s, verinum*val,
					const char*file, unsigned loneno);

/*
 * The makewire functions announce to the pform code new wires. These
 * go into a module that is currently opened.
 */
extern void pform_makewire(const struct vlltype&li, const char*name,
			   NetNet::Type type, NetNet::PortType pt,
			   svector<named_pexpr_t*>*attr);

extern void pform_makewire(const struct vlltype&li,
			   svector<PExpr*>*range,
			   bool signed_flag,
			   list<perm_string>*names,
			   NetNet::Type type,
			   NetNet::PortType,
			   svector<named_pexpr_t*>*attr);
extern void pform_makewire(const struct vlltype&li,
			   svector<PExpr*>*range,
			   bool signed_flag,
			   svector<PExpr*>*delay,
			   str_pair_t str,
			   net_decl_assign_t*assign_list,
			   NetNet::Type type);
extern void pform_make_reginit(const struct vlltype&li,
			       const char*name, PExpr*expr);

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
				svector<PExpr*>*, bool);
extern void pform_set_reg_idx(const char*name, PExpr*l, PExpr*r);
extern void pform_set_reg_integer(list<perm_string>*names);
extern void pform_set_reg_time(list<perm_string>*names);
extern void pform_set_task(perm_string name, PTask*);
extern void pform_set_function(perm_string name, PFunction*);

  /* pform_set_attrib and pform_set_type_attrib exist to support the
     $attribute syntax, which can only set string values to
     attributes. The functions keep the value strings that are
     passed in. */
extern void pform_set_attrib(perm_string name, perm_string key,
			     char*value);
extern void pform_set_type_attrib(perm_string name, const string&key,
				  char*value);

extern void pform_attach_attributes(Statement*obj, svector<named_pexpr_t*>*attr);

extern void pform_set_parameter(perm_string name,
				bool signed_flag,
				svector<PExpr*>*range,
				PExpr*expr);
extern void pform_set_localparam(perm_string name,
				bool signed_flag,
				svector<PExpr*>*range,
				 PExpr*expr);
extern void pform_set_defparam(const hname_t&name, PExpr*expr);

/*
 * Functions related to specify blocks.
 */
extern void pform_set_specparam(perm_string name, PExpr*expr);
extern void pform_make_specify_path(list<perm_string>*src, char pol,
				    bool full_flag, list<perm_string>*dst);

/*
 * pform_make_behavior creates processes that are declared with always
 * or initial items.
 */
extern PProcess*  pform_make_behavior(PProcess::Type, Statement*,
				      svector<named_pexpr_t*>*attr);

extern svector<PWire*>* pform_make_udp_input_ports(list<perm_string>*);

extern bool pform_expression_is_constant(const PExpr*);

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
extern PGAssign* pform_make_pgassign(PExpr*lval, PExpr*rval,
				     svector<PExpr*>*delays,
				     struct str_pair_t str);
extern void pform_make_pgassign_list(svector<PExpr*>*alist,
				     svector<PExpr*>*del,
				     struct str_pair_t str,
				     const char* fn, unsigned lineno);

/* Given a port type and a list of names, make a list of wires that
   can be used as task port information. */
extern svector<PWire*>*pform_make_task_ports(NetNet::PortType pt,
					     bool signed_flag,
					     svector<PExpr*>*range,
					     list<perm_string>*names,
					     const char* file,
					     unsigned lineno);


/*
 * These are functions that the outside-the-parser code uses the do
 * interesting things to the verilog. The parse function reads and
 * parses the source file and places all the modules it finds into the
 * mod list. The dump function dumps a module to the output stream.
 */
extern void pform_dump(ostream&out, Module*mod);

#endif
