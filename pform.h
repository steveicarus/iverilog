#ifndef __pform_H
#define __pform_H
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: pform.h,v 1.79 2004/05/31 23:34:39 steve Exp $"
#endif

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

extern void pform_set_parameter(perm_string name,
				bool signed_flag,
				svector<PExpr*>*range,
				PExpr*expr);
extern void pform_set_localparam(perm_string name, PExpr*expr);
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

/*
 * $Log: pform.h,v $
 * Revision 1.79  2004/05/31 23:34:39  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.78  2004/05/25 19:21:07  steve
 *  More identifier lists use perm_strings.
 *
 * Revision 1.77  2004/03/08 00:10:30  steve
 *  Verilog2001 new style port declartions for primitives.
 *
 * Revision 1.76  2004/02/20 18:53:35  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.75  2004/02/20 06:22:58  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.74  2004/02/18 17:11:57  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.73  2003/07/04 03:57:19  steve
 *  Allow attributes on Verilog 2001 port declarations.
 *
 * Revision 1.72  2003/06/20 00:53:19  steve
 *  Module attributes from the parser
 *  through to elaborated form.
 *
 * Revision 1.71  2003/06/13 00:27:09  steve
 *  Task/functions can have signed ports.
 *
 * Revision 1.70  2003/04/28 17:50:57  steve
 *  More 2001 port declaration support.
 *
 * Revision 1.69  2003/04/14 03:39:15  steve
 *  Break sized constants into a size token
 *  and a based numeric constant.
 *
 * Revision 1.68  2003/02/27 06:45:11  steve
 *  specparams as far as pform.
 *
 * Revision 1.67  2003/02/02 19:02:40  steve
 *  Add support for signed ports and nets.
 *
 * Revision 1.66  2003/01/30 16:23:08  steve
 *  Spelling fixes.
 *
 * Revision 1.65  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.64  2002/09/01 03:01:48  steve
 *  Properly cast signedness of parameters with ranges.
 *
 * Revision 1.63  2002/08/19 02:39:17  steve
 *  Support parameters with defined ranges.
 *
 * Revision 1.62  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.61  2002/06/06 18:57:18  steve
 *  Use standard name for iostream.
 *
 * Revision 1.60  2002/05/26 01:39:02  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.59  2002/05/24 04:36:23  steve
 *  Verilog 2001 attriubtes on nets/wires.
 *
 * Revision 1.58  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.57  2002/05/20 02:06:01  steve
 *  Add ranges and signed to port list declarations.
 *
 * Revision 1.56  2002/05/19 23:37:28  steve
 *  Parse port_declaration_lists from the 2001 Standard.
 *
 * Revision 1.55  2002/01/12 04:03:39  steve
 *  Drive strengths for continuous assignments.
 *
 * Revision 1.54  2001/12/07 05:03:13  steve
 *  Support integer for function return value.
 *
 * Revision 1.53  2001/12/03 04:47:15  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.52  2001/11/29 17:37:51  steve
 *  Properly parse net_decl assignments with delays.
 *
 * Revision 1.51  2001/11/10 02:08:49  steve
 *  Coerse input to inout when assigned to.
 *
 * Revision 1.50  2001/10/31 03:11:15  steve
 *  detect module ports not declared within the module.
 *
 * Revision 1.49  2001/10/21 01:55:25  steve
 *  Error messages for missing UDP port declarations.
 *
 * Revision 1.48  2001/10/21 00:42:48  steve
 *  Module types in pform are char* instead of string.
 *
 * Revision 1.47  2001/10/20 23:02:40  steve
 *  Add automatic module libraries.
 *
 * Revision 1.46  2001/10/20 05:21:51  steve
 *  Scope/module names are char* instead of string.
 *
 * Revision 1.45  2000/12/11 00:31:43  steve
 *  Add support for signed reg variables,
 *  simulate in t-vvm signed comparisons.
 *
 * Revision 1.44  2000/11/30 17:31:42  steve
 *  Change LineInfo to store const C strings.
 *
 * Revision 1.43  2000/10/31 17:49:02  steve
 *  Support time variables.
 *
 * Revision 1.42  2000/10/31 17:00:05  steve
 *  Remove C++ string from variable lists.
 *
 * Revision 1.41  2000/07/29 17:58:21  steve
 *  Introduce min:typ:max support.
 *
 * Revision 1.40  2000/05/08 05:30:20  steve
 *  Deliver gate output strengths to the netlist.
 *
 * Revision 1.39  2000/05/06 15:41:57  steve
 *  Carry assignment strength to pform.
 *
 * Revision 1.38  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 * Revision 1.37  2000/03/12 17:09:41  steve
 *  Support localparam.
 *
 * Revision 1.36  2000/03/08 04:36:54  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.35  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.34  2000/01/10 22:16:24  steve
 *  minor type syntax fix for stubborn C++ compilers.
 *
 * Revision 1.33  2000/01/09 05:50:49  steve
 *  Support named parameter override lists.
 *
 * Revision 1.32  1999/12/30 19:06:14  steve
 *  Support reg initial assignment syntax.
 *
 * Revision 1.31  1999/09/10 05:02:09  steve
 *  Handle integers at task parameters.
 *
 * Revision 1.30  1999/08/27 15:08:37  steve
 *  continuous assignment lists.
 *
 * Revision 1.29  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 * Revision 1.28  1999/08/23 16:48:39  steve
 *  Parameter overrides support from Peter Monta
 *  AND and XOR support wide expressions.
 *
 * Revision 1.27  1999/08/03 04:14:49  steve
 *  Parse into pform arbitrarily complex module
 *  port declarations.
 *
 * Revision 1.26  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.25  1999/07/31 19:14:47  steve
 *  Add functions up to elaboration (Ed Carter)
 *
 * Revision 1.24  1999/07/24 02:11:20  steve
 *  Elaborate task input ports.
 *
 * Revision 1.23  1999/07/10 01:03:18  steve
 *  remove string from lexical phase.
 *
 * Revision 1.22  1999/07/03 02:12:52  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.21  1999/06/24 04:24:18  steve
 *  Handle expression widths for EEE and NEE operators,
 *  add named blocks and scope handling,
 *  add registers declared in named blocks.
 *
 * Revision 1.20  1999/06/15 03:44:53  steve
 *  Get rid of the STL vector template.
 *
 * Revision 1.19  1999/06/12 20:35:27  steve
 *  parse more verilog.
 *
 * Revision 1.18  1999/06/06 20:45:39  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.17  1999/06/02 15:38:46  steve
 *  Line information with nets.
 *
 * Revision 1.16  1999/05/29 02:36:17  steve
 *  module parameter bind by name.
 *
 * Revision 1.15  1999/05/20 04:31:45  steve
 *  Much expression parsing work,
 *  mark continuous assigns with source line info,
 *  replace some assertion failures with Sorry messages.
 *
 * Revision 1.14  1999/05/16 05:08:42  steve
 *  Redo constant expression detection to happen
 *  after parsing.
 *
 *  Parse more operators and expressions.
 *
 * Revision 1.13  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.12  1999/05/07 04:26:49  steve
 *  Parse more complex continuous assign lvalues.
 *
 * Revision 1.11  1999/05/06 04:37:17  steve
 *  Get rid of list<lgate> types.
 *
 * Revision 1.10  1999/05/06 04:09:28  steve
 *  Parse more constant expressions.
 *
 * Revision 1.9  1999/04/19 01:59:37  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.8  1999/02/21 17:01:57  steve
 *  Add support for module parameters.
 *
 * Revision 1.7  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 * Revision 1.6  1999/01/25 05:45:56  steve
 *  Add the LineInfo class to carry the source file
 *  location of things. PGate, Statement and PProcess.
 *
 *  elaborate handles module parameter mismatches,
 *  missing or incorrect lvalues for procedural
 *  assignment, and errors are propogated to the
 *  top of the elaboration call tree.
 *
 *  Attach line numbers to processes, gates and
 *  assignment statements.
 *
 * Revision 1.5  1998/12/09 04:02:47  steve
 *  Support the include directive.
 *
 * Revision 1.4  1998/12/01 00:42:14  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.3  1998/11/25 02:35:53  steve
 *  Parse UDP primitives all the way to pform.
 *
 * Revision 1.2  1998/11/23 00:20:23  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.1  1998/11/03 23:29:04  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
