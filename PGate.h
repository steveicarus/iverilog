#ifndef __PGate_H
#define __PGate_H
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
#ident "$Id: PGate.h,v 1.28 2004/03/08 00:47:44 steve Exp $"
#endif

# include  "svector.h"
# include  "StringHeap.h"
# include  "named.h"
# include  "LineInfo.h"
# include  "PDelays.h"
# include  <map>
# include  <string>
class PExpr;
class PUdp;
class Design;
class NetScope;
class Module;

/*
 * A PGate represents a Verilog gate. The gate has a name and other
 * properties, and a set of pins that connect to wires. It is known at
 * the time a gate is constructed how many pins the gate has.
 *
 * This pins of a gate are connected to expressions. The elaboration
 * step will need to convert expressions to a network of gates in
 * order to elaborate expression inputs, but that can easily be done.
 *
 * The PGate base class also carries the strength0 and strength1
 * strengths for those gates where the driver[s] can be described by a
 * single strength pair. There is a strength of the 0 drive, and a
 * strength of the 1 drive.
 */
class PGate : public LineInfo {
      
    public:
      enum strength_t { HIGHZ, WEAK, PULL, STRONG, SUPPLY };

      explicit PGate(perm_string name, svector<PExpr*>*pins,
		     const svector<PExpr*>*del);

      explicit PGate(perm_string name, svector<PExpr*>*pins,
		     PExpr*del);

      explicit PGate(perm_string name, svector<PExpr*>*pins);

      virtual ~PGate();

      perm_string get_name() const { return name_; }

      void eval_delays(Design*des, NetScope*scope,
		       unsigned long&rise_time,
		       unsigned long&fall_time,
		       unsigned long&decay_time) const;

      unsigned pin_count() const { return pins_? pins_->count() : 0; }
      const PExpr*pin(unsigned idx) const { return (*pins_)[idx]; }

      strength_t strength0() const;
      strength_t strength1() const;

      void strength0(strength_t);
      void strength1(strength_t);

      map<perm_string,PExpr*> attributes;

      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*sc) const;
      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

    protected:
      const svector<PExpr*>& get_pins() const { return *pins_; }

      void dump_pins(ostream&out) const;
      void dump_delays(ostream&out) const;

    private:
      perm_string name_;
      PDelays delay_;
      svector<PExpr*>*pins_;

      strength_t str0_, str1_;

    private: // not implemented
      PGate(const PGate&);
      PGate& operator= (const PGate&);
};

/* A continuous assignment has a single output and a single input. The
   input is passed directly to the output. This is different from a
   BUF because elaboration may need to turn this into a vector of
   gates. */
class PGAssign  : public PGate {

    public:
      explicit PGAssign(svector<PExpr*>*pins);
      explicit PGAssign(svector<PExpr*>*pins, svector<PExpr*>*dels);
      ~PGAssign();

      void dump(ostream&out) const;
      virtual void elaborate(Design*des, NetScope*scope) const;

    private:
};


/*
 * The Builtin class is specifically a gate with one of the builtin
 * types. The parser recognizes these types during parse. These types
 * have special properties that allow them to be treated specially.
 *
 * A PGBuiltin can be grouped into an array of devices. If this is
 * done, the msb_ and lsb_ are set to the indices of the array
 * range. Elaboration causes a gate to be created for each element of
 * the array, and a name will be generated for each gate.
 */
class PGBuiltin  : public PGate {

    public:
      enum Type { AND, NAND, OR, NOR, XOR, XNOR, BUF, BUFIF0, BUFIF1,
		  NOT, NOTIF0, NOTIF1, PULLDOWN, PULLUP, NMOS, RNMOS,
		  PMOS, RPMOS, CMOS, RCMOS, TRAN, RTRAN, TRANIF0,
		  TRANIF1, RTRANIF0, RTRANIF1 };

    public:
      explicit PGBuiltin(Type t, perm_string name,
			 svector<PExpr*>*pins,
			 svector<PExpr*>*del);
      explicit PGBuiltin(Type t, perm_string name,
			 svector<PExpr*>*pins,
			 PExpr*del);
      ~PGBuiltin();

      Type type() const { return type_; }
      void set_range(PExpr*msb, PExpr*lsb);

      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*, NetScope*scope) const;

    private:
      Type type_;

      PExpr*msb_;
      PExpr*lsb_;
};

/*
 * This kind of gate is an instantiation of a module. The stored type
 * is the name of a module definition somewhere in the pform. This
 * type also handles UDP devices, because it is generally not known at
 * parse time whether a name belongs to a module or a UDP.
 */
class PGModule  : public PGate {

    public:
	// The name is the *instance* name of the gate.

	// If the binding of ports is by position, this constructor
	// builds everything all at once.
      explicit PGModule(perm_string type, perm_string name,
			svector<PExpr*>*pins);

	// If the binding of ports is by name, this constructor takes
	// the bindings and stores them for later elaboration.
      explicit PGModule(perm_string type, perm_string name,
			named<PExpr*>*pins, unsigned npins);


      ~PGModule();

	// Parameter overrides can come as an ordered list, or a set
	// of named expressions.
      void set_parameters(svector<PExpr*>*o);
      void set_parameters(named<PExpr*>*pa, unsigned npa);

	// Modules can be instantiated in ranges. The parser uses this
	// method to pass the range to the pform.
      void set_range(PExpr*msb, PExpr*lsb);

      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*sc) const;
      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

	// This returns the module name of this module. It is a
	// permallocated string.
      perm_string get_type();

    private:
      perm_string type_;
      svector<PExpr*>*overrides_;
      named<PExpr*>*pins_;
      unsigned npins_;

	// These members support parameter override by name
      named<PExpr*>*parms_;
      unsigned nparms_;

	// Arrays of modules are give if these are set.
      PExpr*msb_;
      PExpr*lsb_;

      void elaborate_mod_(Design*, Module*mod, NetScope*scope) const;
      void elaborate_udp_(Design*, PUdp  *udp, NetScope*scope) const;
      void elaborate_scope_mod_(Design*des, Module*mod, NetScope*sc) const;
      bool elaborate_sig_mod_(Design*des, NetScope*scope, Module*mod) const;
};

/*
 * $Log: PGate.h,v $
 * Revision 1.28  2004/03/08 00:47:44  steve
 *  primitive ports can bind bi name.
 *
 * Revision 1.27  2004/02/20 18:53:33  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.26  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.25  2003/03/06 04:37:12  steve
 *  lex_strings.add module names earlier.
 *
 * Revision 1.24  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.23  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.22  2001/11/22 06:20:59  steve
 *  Use NetScope instead of string for scope path.
 *
 * Revision 1.21  2001/10/21 00:42:47  steve
 *  Module types in pform are char* instead of string.
 *
 * Revision 1.20  2001/10/19 01:55:32  steve
 *  Method to get the type_ member
 *
 * Revision 1.19  2001/04/22 23:09:45  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.18  2000/05/06 15:41:56  steve
 *  Carry assignment strength to pform.
 *
 * Revision 1.17  2000/05/02 16:27:38  steve
 *  Move signal elaboration to a seperate pass.
 *
 * Revision 1.16  2000/03/29 04:37:10  steve
 *  New and improved combinational primitives.
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
 * Revision 1.13  2000/02/18 05:15:02  steve
 *  Catch module instantiation arrays.
 *
 * Revision 1.12  2000/01/09 05:50:48  steve
 *  Support named parameter override lists.
 *
 * Revision 1.11  1999/12/11 05:45:41  steve
 *  Fix support for attaching attributes to primitive gates.
 *
 * Revision 1.10  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 * Revision 1.9  1999/08/23 16:48:39  steve
 *  Parameter overrides support from Peter Monta
 *  AND and XOR support wide expressions.
 *
 * Revision 1.8  1999/08/01 21:18:55  steve
 *  elaborate rise/fall/decay for continuous assign.
 *
 * Revision 1.7  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.6  1999/05/29 02:36:17  steve
 *  module parameter bind by name.
 *
 * Revision 1.5  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.4  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 * Revision 1.3  1999/01/25 05:45:56  steve
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
 * Revision 1.2  1998/12/01 00:42:13  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.1  1998/11/03 23:28:54  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
