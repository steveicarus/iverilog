#ifndef __Module_H
#define __Module_H
/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: Module.h,v 1.9 1999/08/23 16:48:39 steve Exp $"
#endif

# include  <list>
# include  <map>
# include  "svector.h"
# include  <string>
class PExpr;
class PGate;
class PTask;
class PFunction;
class PWire;
class PProcess;
class Design;

/*
 * A module is a named container and scope. A module holds a bunch of
 * semantic quantities such as wires and gates. The module is
 * therefore the handle for grasping the described circuit.
 */

class Module {

	/* The module ports are in general a vector of port_t
	   objects. Each port has a name and an ordered list of
	   wires. The name is the means that the outside uses to
	   access the port, the wires are the internal connections to
	   the port. */
    public:
      struct port_t {
	    string name;
	    svector<PWire*>wires;

	    port_t(int c=0) : wires(c) { }
      };

    public:
      explicit Module(const string&name, const svector<port_t*>*);


	/* The module has parameters that are evaluated when the
	   module is elaborated. During parsing, I put the parameters
	   into this map. */
      map<string,PExpr*>parameters;

        /* Parameters may be overridden at instantiation time;
           the overrides do not contain explicit parameter names,
           but rather refer to parameters in the order they
           appear in the instantiated module.  Therefore a
           list of names in module-order is needed to pass from
           a parameter-index to its name. */
      list<string> param_names;

      const string&get_name() const { return name_; }

      void add_gate(PGate*gate);
      void add_wire(PWire*wire);
      void add_behavior(PProcess*behave);
      void add_task(const string&name, PTask*def);
      void add_function(const string&name, PFunction*def);

      unsigned port_count() const;
      const svector<PWire*>& get_port(unsigned idx) const;
      unsigned find_port(const string&) const;

	// Find a wire by name. This is used for connecting gates to
	// existing wires, etc.
      PWire* get_wire(const string&name);

      const list<PWire*>& get_wires() const { return wires_; }
      const list<PGate*>& get_gates() const { return gates_; }
      const list<PProcess*>& get_behaviors() const { return behaviors_; }

      void dump(ostream&out) const;
      bool elaborate(Design*, const string&path, svector<PExpr*>*overrides_) const;

    private:
      const string name_;

      svector<port_t*> ports_;
      list<PWire*> wires_;
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
