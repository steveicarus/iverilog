#ifndef __Module_H
#define __Module_H
/*
 * Copyright (c) 1998-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: Module.h,v 1.37 2004/06/13 04:56:53 steve Exp $"
#endif

# include  <list>
# include  <map>
# include  "svector.h"
# include  "StringHeap.h"
# include  "HName.h"
# include  "named.h"
# include  "LineInfo.h"
# include  "netlist.h"
class PData;
class PEvent;
class PExpr;
class PEIdent;
class PGate;
class PTask;
class PFunction;
class PWire;
class PProcess;
class Design;
class NetScope;

/*
 * A module is a named container and scope. A module holds a bunch of
 * semantic quantities such as wires and gates. The module is
 * therefore the handle for grasping the described circuit.
 */

class Module : public LineInfo {

	/* The module ports are in general a vector of port_t
	   objects. Each port has a name and an ordered list of
	   wires. The name is the means that the outside uses to
	   access the port, the wires are the internal connections to
	   the port. */
    public:
      struct port_t {
	    perm_string name;
	    svector<PEIdent*> expr;
      };

    public:
	/* The name passed here is the module name, not the instance
	   name. This make must be a permallocated string. */
      explicit Module(perm_string name);
      ~Module();

      NetNet::Type default_nettype;

	/* The module has parameters that are evaluated when the
	   module is elaborated. During parsing, I put the parameters
	   into this map. */
      struct param_expr_t {
	    PExpr*expr;
	    PExpr*msb;
	    PExpr*lsb;
	    bool signed_flag;
      };
      map<perm_string,param_expr_t>parameters;
      map<perm_string,param_expr_t>localparams;


	/* specparams are simpler then other params, in that they have
	   no type information. They are merely constant
	   expressions. */
      map<perm_string,PExpr*>specparams;

	/* The module also has defparam assignments which don't create
	   new parameters within the module, but may be used to set
	   values within this module (when instantiated) or in other
	   instantiated modules. */
      map<hname_t,PExpr*>defparms;

        /* Parameters may be overridden at instantiation time;
           the overrides do not contain explicit parameter names,
           but rather refer to parameters in the order they
           appear in the instantiated module.  Therefore a
           list of names in module-order is needed to pass from
           a parameter-index to its name. */
      list<perm_string> param_names;

	/* This is an array of port descriptors, which is in turn a
	   named array of PEident pointers. */
      svector<port_t*> ports;

	/* Keep a table of named events declared in the module. */
      map<perm_string,PEvent*>events;

	/* Keep a table of datum variables declared in the module. */
      map<hname_t,PData*>datum;

      map<perm_string,PExpr*> attributes;

	/* These are the timescale for this module. The default is
	   set by the `timescale directive. */
      int time_unit, time_precision;

      perm_string mod_name() const { return name_; }

      void add_gate(PGate*gate);

	// The add_wire method adds a wire by name, but only if the
	// wire name doesn't already exist. Either way, the result is
	// the existing wire or the pointer passed in.
      PWire* add_wire(PWire*wire);

      void add_behavior(PProcess*behave);
      void add_task(perm_string name, PTask*def);
      void add_function(perm_string name, PFunction*def);

      unsigned port_count() const;
      const svector<PEIdent*>& get_port(unsigned idx) const;
      unsigned find_port(const char*name) const;

	// Find a wire by name. This is used for connecting gates to
	// existing wires, etc.
      PWire* get_wire(const hname_t&name) const;
      PGate* get_gate(perm_string name);

      const map<hname_t,PWire*>& get_wires() const;
      const list<PGate*>& get_gates() const;
      const list<PProcess*>& get_behaviors() const;

      void dump(ostream&out) const;
      bool elaborate(Design*, NetScope*scope) const;

      bool elaborate_scope(Design*, NetScope*scope) const;

      bool elaborate_sig(Design*, NetScope*scope) const;

    private:
      perm_string name_;

      map<hname_t,PWire*> wires_;
      list<PGate*> gates_;
      list<PProcess*> behaviors_;
      map<perm_string,PTask*> tasks_;
      map<perm_string,PFunction*> funcs_;

    private: // Not implemented
      Module(const Module&);
      Module& operator= (const Module&);
};


/*
 * $Log: Module.h,v $
 * Revision 1.37  2004/06/13 04:56:53  steve
 *  Add support for the default_nettype directive.
 *
 * Revision 1.36  2004/05/25 19:21:06  steve
 *  More identifier lists use perm_strings.
 *
 * Revision 1.35  2004/02/20 18:53:33  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.34  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.33  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.32  2003/06/20 00:53:19  steve
 *  Module attributes from the parser
 *  through to elaborated form.
 *
 * Revision 1.31  2003/06/13 19:10:45  steve
 *  Properly manage real variables in subscopes.
 *
 * Revision 1.30  2003/03/06 04:37:12  steve
 *  lex_strings.add module names earlier.
 *
 * Revision 1.29  2003/02/27 06:45:11  steve
 *  specparams as far as pform.
 *
 * Revision 1.28  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.27  2002/08/19 02:39:16  steve
 *  Support parameters with defined ranges.
 *
 * Revision 1.26  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.25  2002/05/19 23:37:28  steve
 *  Parse port_declaration_lists from the 2001 Standard.
 */
#endif
