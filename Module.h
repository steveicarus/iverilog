#ifndef __Module_H
#define __Module_H
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
#ident "$Id: Module.h,v 1.28 2003/01/26 21:15:58 steve Exp $"
#endif

# include  <list>
# include  <map>
# include  "svector.h"
# include  "HName.h"
# include  "named.h"
# include  "LineInfo.h"
# include  <string>
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
	    string name;
	    svector<PEIdent*> expr;
      };

    public:
      explicit Module(const char*name);
      ~Module();


	/* The module has parameters that are evaluated when the
	   module is elaborated. During parsing, I put the parameters
	   into this map. */
      struct param_expr_t {
	    PExpr*expr;
	    PExpr*msb;
	    PExpr*lsb;
	    bool signed_flag;
      };
      map<string,param_expr_t>parameters;
      map<string,param_expr_t>localparams;

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
      list<string> param_names;

	/* This is an array of port descriptors, which is in turn a
	   named array of PEident pointers. */
      svector<port_t*> ports;

	/* Keep a table of named events declared in the module. */
      map<string,PEvent*>events;

	/* Keep a table of datum variables declared in the module. */
      map<string,PData*>datum;

	/* These are the timescale for this module. The default is
	   set by the `timescale directive. */
      int time_unit, time_precision;

      const char*mod_name() const { return name_; }

      void add_gate(PGate*gate);

	// The add_wire method adds a wire by name, but only if the
	// wire name doesn't already exist. Either way, the result is
	// the existing wire or the pointer passed in.
      PWire* add_wire(PWire*wire);

      void add_behavior(PProcess*behave);
      void add_task(const string&name, PTask*def);
      void add_function(const string&name, PFunction*def);

      unsigned port_count() const;
      const svector<PEIdent*>& get_port(unsigned idx) const;
      unsigned find_port(const string&) const;

	// Find a wire by name. This is used for connecting gates to
	// existing wires, etc.
      PWire* get_wire(const hname_t&name) const;
      PGate* get_gate(const string&name);

      const map<hname_t,PWire*>& get_wires() const;
      const list<PGate*>& get_gates() const;
      const list<PProcess*>& get_behaviors() const;

      void dump(ostream&out) const;
      bool elaborate(Design*, NetScope*scope) const;

      bool elaborate_scope(Design*, NetScope*scope) const;

      bool elaborate_sig(Design*, NetScope*scope) const;

    private:
      char* name_;

      map<hname_t,PWire*> wires_;
      list<PGate*> gates_;
      list<PProcess*> behaviors_;
      map<string,PTask*> tasks_;
      map<string,PFunction*> funcs_;

    private: // Not implemented
      Module(const Module&);
      Module& operator= (const Module&);
};


/*
 * $Log: Module.h,v $
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
 *
 * Revision 1.24  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.23  2001/10/31 03:11:15  steve
 *  detect module ports not declared within the module.
 *
 * Revision 1.22  2001/10/20 05:21:51  steve
 *  Scope/module names are char* instead of string.
 *
 * Revision 1.21  2000/11/05 06:05:59  steve
 *  Handle connectsion to internally unconnected modules (PR#38)
 *
 * Revision 1.20  2000/07/22 22:09:03  steve
 *  Parse and elaborate timescale to scopes.
 *
 * Revision 1.19  2000/05/16 04:05:15  steve
 *  Module ports are really special PEIdent
 *  expressions, because a name can be used
 *  many places in the port list.
 *
 * Revision 1.18  2000/05/02 16:27:38  steve
 *  Move signal elaboration to a seperate pass.
 *
 * Revision 1.17  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 * Revision 1.16  2000/03/12 17:09:40  steve
 *  Support localparam.
 *
 * Revision 1.15  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.14  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.13  2000/01/09 20:37:57  steve
 *  Careful with wires connected to multiple ports.
 *
 * Revision 1.12  2000/01/09 05:50:48  steve
 *  Support named parameter override lists.
 *
 * Revision 1.11  1999/12/11 05:45:41  steve
 *  Fix support for attaching attributes to primitive gates.
 *
 * Revision 1.10  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 * Revision 1.9  1999/08/23 16:48:39  steve
 *  Parameter overrides support from Peter Monta
 *  AND and XOR support wide expressions.
 *
 * Revision 1.8  1999/08/04 02:13:02  steve
 *  Elaborate module ports that are concatenations of
 *  module signals.
 *
 * Revision 1.7  1999/08/03 04:14:49  steve
 *  Parse into pform arbitrarily complex module
 *  port declarations.
 *
 * Revision 1.6  1999/07/31 19:14:47  steve
 *  Add functions up to elaboration (Ed Carter)
 *
 * Revision 1.5  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.4  1999/06/15 03:44:53  steve
 *  Get rid of the STL vector template.
 *
 * Revision 1.3  1999/02/21 17:01:57  steve
 *  Add support for module parameters.
 *
 * Revision 1.2  1999/01/25 05:45:56  steve
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
 * Revision 1.1  1998/11/03 23:28:52  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
