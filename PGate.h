#ifndef __PGate_H
#define __PGate_H
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
#ident "$Id: PGate.h,v 1.1 1998/11/03 23:28:54 steve Exp $"
#endif

# include  <vector>
class PExpr;
class Design;

/*
 * A PGate represents a Verilog gate. The gate has a name and other
 * properties, and a set of pins that connect to wires. It is known at
 * the time a gate is constructed how many pins the gate has.
 *
 * This pins of a gate are connected to expressions. The elaboration
 * step will need to convert expressions to a network of gates in
 * order to elaborate expression inputs, but that can easily be done.
 */
class PGate {
      
    public:
      explicit PGate(const string&name, const vector<PExpr*>&pins, long del)
      : name_(name), delay_(del), pins_(pins) { }

      virtual ~PGate() { }

      const string& get_name() const { return name_; }

      long get_delay() const { return delay_; }

      unsigned pin_count() const { return pins_.size(); }
      const PExpr*pin(unsigned idx) const { return pins_[idx]; }

      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*des, const string&path) const;

    protected:
      void dump_pins(ostream&out) const;

    private:
      const string name_;
      const unsigned long delay_;
      const vector<PExpr*> pins_;

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
      explicit PGAssign(const vector<PExpr*>&pins)
      : PGate("", pins, 0) { assert(pins.size() == 2); }

      void dump(ostream&out) const;
      virtual void elaborate(Design*des, const string&path) const;

    private:
};


/* The Builtin class is specifically a gate with one of the builtin
   types. The parser recognizes these types during parse. These types
   have special properties that allow them to be treated specially. */
class PGBuiltin  : public PGate {

    public:
      enum Type { AND, NAND, OR, NOR, XOR, XNOR, BUF, BUFIF0, BUFIF1,
		  NOT, NOTIF0, NOTIF1, PULLDOWN, PULLUP, NMOS, RNMOS,
		  PMOS, RPMOS, CMOS, RCMOS, TRAN, RTRAN, TRANIF0,
		  TRANIF1, RTRANIF0, RTRANIF1 };

    public:
      explicit PGBuiltin(Type t, const string&name,
			 const vector<PExpr*>&pins, long del = 0)
      : PGate(name, pins, del), type_(t)
      {  }

      Type type() const { return type_; }

      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*, const string&path) const;

    private:
      Type type_;
};

/*
 * This kind of gate is an instantiation of a module. The stored type
 * is the name of a module definition somewhere in the pform.
 */
class PGModule  : public PGate {

    public:
      explicit PGModule(const string&type, const string&name,
			const vector<PExpr*>&pins)
      : PGate(name, pins, 0), type_(type) { }

      virtual void dump(ostream&out) const;
      virtual void elaborate(Design*, const string&path) const;

    private:
      string type_;
};

/*
 * $Log: PGate.h,v $
 * Revision 1.1  1998/11/03 23:28:54  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
