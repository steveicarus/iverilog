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
#ident "$Id: Module.h,v 1.4 1999/06/15 03:44:53 steve Exp $"
#endif

# include  <list>
# include  <map>
# include  "svector.h"
# include  <string>
class PExpr;
class PGate;
class PWire;
class PProcess;
class Design;

/*
 * A module is a named container and scope. A module holds a bunch of
 * semantic quantities such as wires and gates. The module is
 * therefore the handle for grasping the described circuit.
 */

class Module {
    public:
      explicit Module(const string&name, unsigned nports)
      : ports(nports), name_(name) { }

      svector<PWire*> ports;

	/* The module has parameters that are evaluated when the
	   module is elaborated. During parsing, I put the parameters
	   into this map. */
      map<string,PExpr*>parameters;

      const string&get_name() const { return name_; }

      void add_gate(PGate*gate);
      void add_wire(PWire*wire);
      void add_behavior(PProcess*behave);

	// Find a wire by name. This is used for connecting gates to
	// existing wires, etc.
      PWire* get_wire(const string&name);

      const list<PWire*>& get_wires() const { return wires_; }
      const list<PGate*>& get_gates() const { return gates_; }
      const list<PProcess*>& get_behaviors() const { return behaviors_; }

      bool elaborate(Design*, const string&path) const;

    private:
      const string name_;

      list<PWire*> wires_;
      list<PGate*> gates_;
      list<PProcess*> behaviors_;

    private: // Not implemented
      Module(const Module&);
      Module& operator= (const Module&);
};


/*
 * $Log: Module.h,v $
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
