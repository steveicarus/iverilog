#ifndef __PGate_H
#define __PGate_H
/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PGate.h,v 1.9 1999/08/23 16:48:39 steve Exp $"
#endif

# include  "svector.h"
# include  "LineInfo.h"
class PExpr;
class PUdp;
class Design;
class Module;

/*
 * A PGate represents a Verilog gate. The gate has a name and other
 * properties, and a set of pins that connect to wires. It is known at
 * the time a gate is constructed how many pins the gate has.
 *
 * This pins of a gate are connected to expressions. The elaboration
 * step will need to convert expressions to a network of gates in
 * order to elaborate expression inputs, but that can easily be done.
 */
class PGate : public LineInfo {
      
    public:
      explicit PGate(const string&name, svector<PExpr*>*pins,
		     const svector<PExpr*>*del);

      explicit PGate(const string&name, svector<PExpr*>*pins,
		     PExpr*del);

      explicit PGate(const string&name, svector<PExpr*>*pins);

      virtual ~PGate();

      const string& get_name() const { return name_; }

      void eval_delays(Design*des, const string&path,
		       unsigned long&rise_time,
		       unsigned long&fall_time,
		       unsigned long&decay_time) const;

      unsigned pin_count() const { return pins_? pins_->count() : 0; }
      const PExpr*pin(unsigned idx) const { return (*pins_)[idx]; }

      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*des, const string&path) const;

    protected:
      const svector<PExpr*>* get_pins() const { return pins_; }

      void dump_pins(ostream&out) const;
      void dump_delays(ostream&out) const;

    private:
      const string name_;
      PExpr* delay_[3];
      svector<PExpr*>*pins_;

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
      virtual void elaborate(Design*des, const string&path) const;

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
      explicit PGBuiltin(Type t, const string&name,
			 svector<PExpr*>*pins,
			 svector<PExpr*>*del);
      explicit PGBuiltin(Type t, const string&name,
			 svector<PExpr*>*pins,
			 PExpr*del);
      ~PGBuiltin();

      Type type() const { return type_; }
      void set_range(PExpr*msb, PExpr*lsb);

      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*, const string&path) const;

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
	// If the binding of ports is by position, this constructor
	// builds everything all at once.
      explicit PGModule(const string&type, const string&name,
			svector<PExpr*>*overrides, svector<PExpr*>*pins)
      : PGate(name, pins), type_(type), overrides_(overrides), pins_(0), npins_(0) { }

	// If the binding of ports is by name, this constructor takes
	// the bindings and stores them for later elaboration.
      struct bind_t {
	    string name;
	    PExpr* parm;
      };
      explicit PGModule(const string&type, const string&name,
			svector<PExpr*>*overrides, bind_t*pins, unsigned npins)
      : PGate(name, 0), type_(type), overrides_(overrides), pins_(pins), npins_(npins) { }


      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*, const string&path) const;

    private:
      string type_;
      svector<PExpr*>*overrides_;
      bind_t*pins_;
      unsigned npins_;

      void elaborate_mod_(Design*, Module*mod, const string&path) const;
      void elaborate_udp_(Design*, PUdp  *udp, const string&path) const;
};

/*
 * $Log: PGate.h,v $
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
