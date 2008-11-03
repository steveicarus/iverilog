#ifndef __Module_H
#define __Module_H
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


# include  <list>
# include  <map>
# include  <vector>
# include  <utility>
# include  "StringHeap.h"
# include  "HName.h"
# include  "named.h"
# include  "PScope.h"
# include  "LineInfo.h"
# include  "netlist.h"
# include  "pform_types.h"
class PExpr;
class PEIdent;
class PGate;
class PGenerate;
class PSpecPath;
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

class Module : public PScope, public LineInfo {

	/* The module ports are in general a vector of port_t
	   objects. Each port has a name and an ordered list of
	   wires. The name is the means that the outside uses to
	   access the port, the wires are the internal connections to
	   the port. */
    public:
      struct port_t {
	    perm_string name;
	    vector<PEIdent*> expr;
      };

    public:
	/* The name passed here is the module name, not the instance
	   name. This make must be a permallocated string. */
      explicit Module(perm_string name);
      ~Module();

	/* Initially false. This is set to true if the module has been
	   declared as a library module. This makes the module
	   ineligible for being chosen as an implicit root. It has no
	   other effect. */
      bool library_flag;

      NetNet::Type default_nettype;

	/* specparams are simpler then other params, in that they have
	   no type information. They are merely constant
	   expressions. */
      map<perm_string,PExpr*>specparams;

	/* The module also has defparam assignments which don't create
	   new parameters within the module, but may be used to set
	   values within this module (when instantiated) or in other
	   instantiated modules. */
      typedef pair<pform_name_t,PExpr*> named_expr_t;
      list<named_expr_t>defparms;

        /* Parameters may be overridden at instantiation time;
           the overrides do not contain explicit parameter names,
           but rather refer to parameters in the order they
           appear in the instantiated module.  Therefore a
           list of names in module-order is needed to pass from
           a parameter-index to its name. */
      list<perm_string> param_names;

	/* This is an array of port descriptors, which is in turn a
	   named array of PEident pointers. */
      vector<port_t*> ports;

      map<perm_string,PExpr*> attributes;

	/* These are the timescale for this module. The default is
	   set by the `timescale directive. */
      int time_unit, time_precision;

	/* Task definitions within this module */
      map<perm_string,PTask*> tasks;
      map<perm_string,PFunction*> funcs;

	/* The module has a list of genvars that may be used in
	   various generate schemes. */
      map<perm_string,LineInfo*> genvars;

	/* the module has a list of generate schemes that appear in
	   the module definition. These are used at elaboration time. */
      list<PGenerate*> generate_schemes;

      list<PSpecPath*> specify_paths;

	// The mod_name() is the name of the module type.
      perm_string mod_name() const { return pscope_name(); }

      void add_gate(PGate*gate);

      unsigned port_count() const;
      const vector<PEIdent*>& get_port(unsigned idx) const;
      unsigned find_port(const char*name) const;

      PGate* get_gate(perm_string name);

      const list<PGate*>& get_gates() const;

      void dump(ostream&out) const;
      bool elaborate(Design*, NetScope*scope) const;

      typedef map<perm_string,NetExpr*> replace_t;
      bool elaborate_scope(Design*, NetScope*scope, const replace_t&rep);

      bool elaborate_sig(Design*, NetScope*scope) const;

    private:
      list<PGate*> gates_;

      static void elaborate_parm_item_(perm_string name, const param_expr_t&cur,
				       Design*des, NetScope*scope);

    private: // Not implemented
      Module(const Module&);
      Module& operator= (const Module&);
};

#endif
