#ifndef __netlist_H
#define __netlist_H
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: netlist.h,v 1.156 2000/08/14 04:39:57 steve Exp $"
#endif

/*
 * The netlist types, as described in this header file, are intended
 * to be the output from elaboration of the source design. The design
 * can be passed around in this form to the various stages and design
 * processors.
 */
# include  <string>
# include  <map>
# include  "verinum.h"
# include  "LineInfo.h"
# include  "svector.h"

class Design;
class Link;
class Nexus;
class NetNode;
class NetProc;
class NetProcTop;
class NetScope;
class NetExpr;
class NetESignal;
class ostream;


struct target;
struct functor_t;

/* =========
 * A NetObj is anything that has any kind of behavior in the
 * netlist. Nodes can be gates, registers, etc. and are linked
 * together to form a design web.
 *
 * The web of nodes that makes up a circuit is held together by the
 * Link class. There is a link for each pin. All mutually connected
 * pins form a ring of links.
 *
 * A link can be INPUT, OUTPUT or PASSIVE. An input never drives the
 * signal, and PASSIVE never receives the value of the signal. Wires
 * are PASSIVE, for example.
 *
 * A NetObj also has delays specified as rise_time, fall_time and
 * decay_time. The rise and fall time are the times to transition to 1
 * or 0 values. The decay_time is the time needed to decay to a 'bz
 * value, or to decay of the net is a trireg. The exact and precise
 * interpretation of the rise/fall/decay times is typically left to
 * the target to properly interpret.
 */
class NetObj {

    public:
    public:
      explicit NetObj(const string&n, unsigned npins);
      virtual ~NetObj();

      const string& name() const { return name_; }

      unsigned pin_count() const { return npins_; }

      unsigned rise_time() const { return delay1_; }
      unsigned fall_time() const { return delay2_; }
      unsigned decay_time() const { return delay3_; }

      void rise_time(unsigned d) { delay1_ = d; }
      void fall_time(unsigned d) { delay2_ = d; }
      void decay_time(unsigned d) { delay3_ = d; }

      void set_attributes(const map<string,string>&);
      string attribute(const string&key) const;
      void attribute(const string&key, const string&value);

	// Return true if this has all the attributes in that and they
	// all have the same values.
      bool has_compat_attributes(const NetObj&that) const;

      Link&pin(unsigned idx);
      const Link&pin(unsigned idx) const;

      void dump_node_pins(ostream&, unsigned) const;
      void dump_obj_attr(ostream&, unsigned) const;

    private:
      string name_;
      Link*pins_;
      const unsigned npins_;
      unsigned delay1_;
      unsigned delay2_;
      unsigned delay3_;

      map<string,string> attributes_;
};

class Link {

      friend void connect(Link&, Link&);
      friend class NetObj;
      friend class Nexus;

    public:
      enum DIR { PASSIVE, INPUT, OUTPUT };

      enum strength_t { HIGHZ, WEAK, PULL, STRONG, SUPPLY };

      Link();
      ~Link();

	// Manipulate the link direction.
      void set_dir(DIR d);
      DIR get_dir() const;

	// A link has a drive strength for 0 and 1 values. The drive0
	// strength is for when the link has the value 0, and drive1
	// strength is for when the link has a value 1.
      void drive0(strength_t);
      void drive1(strength_t);

      strength_t drive0() const;
      strength_t drive1() const;

	// A link has an initial value that is used by the nexus to
	// figure out its initial value. Normally, only the object
	// that contains the link sets the initial value, and only the
	// attached Nexus gets it. The default link value is Vx.
      void set_init(verinum::V val);
      verinum::V get_init() const;

      void cur_link(NetObj*&net, unsigned &pin);
      void cur_link(const NetObj*&net, unsigned &pin) const;

	// Get a pointer to the nexus that represents all the links
	// connected to me.
      Nexus* nexus();
      const Nexus* nexus()const;

	// Return a pointer to the next link in the nexus.
      Link* next_nlink();
      const Link* next_nlink() const;

	// Remove this link from the set of connected pins. The
	// destructor will automatically do this if needed.
      void unlink();

	// Return true if this link is connected to anything else.
      bool is_linked() const;

	// Return true if these pins are connected.
      bool is_linked(const Link&that) const;

	// Return true if this is the same pin of the same object of
	// that link.
      bool is_equal(const Link&that) const;

	// Return information about the object that this link is
	// a part of.
      const NetObj*get_obj() const;
      NetObj*get_obj();
      unsigned get_pin() const;

	// A link of an object (sometimes called a "pin") has a
	// name. It is convenient for the name to have a string and an
	// integer part.
      void set_name(const string&, unsigned inst =0);
      const string& get_name() const;
      unsigned get_inst() const;

    private:
	// The NetNode manages these. They point back to the
	// NetNode so that following the links can get me here.
      NetObj *node_;
      unsigned pin_;

      DIR dir_;
      strength_t drive0_, drive1_;
      verinum::V init_;

	// These members name the pin of the link. If the name
	// has width, then the ninst_ member is the index of the
	// pin.
      string   name_;
      unsigned inst_;

    private:
      Link *next_;
      Nexus*nexus_;

    private: // not implemented
      Link(const Link&);
      Link& operator= (const Link&);
};


/*
 * The Nexus represents a collection of links that are joined
 * together. Each link has its own properties, this class holds the
 * properties of the group.
 *
 * The links in a nexus are grouped into a singly linked list, with
 * the nexus pointing to the first Link. Each link in turn points to
 * the next link in the nexus, with the last link pointing to 0.
 */
class Nexus {

      friend void connect(Link&, Link&);
      friend class Link;

    public:
      explicit Nexus();
      ~Nexus();

      string name() const;
      verinum::V get_init() const;

      Link*first_nlink();
      const Link* first_nlink()const;

    private:
      Link*list_;
      void unlink(Link*);
      void relink(Link*);

    private: // not implemented
      Nexus(const Nexus&);
      Nexus& operator= (const Nexus&);
};


/*
 * A NetNode is a device of some sort, where each pin has a different
 * meaning. (i.e. pin(0) is the output to an and gate.) NetNode
 * objects are listed in the nodes_ of the Design object.
 */
class NetNode  : public NetObj {

    public:
      explicit NetNode(const string&n, unsigned npins);

      virtual ~NetNode();

	// This method locates the next node that has all its pins
	// connected to the same of my own pins.
      NetNode*next_node();

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned) const;

	// This is used to scan a modifiable netlist, one node at a time.
      virtual void functor_node(Design*, functor_t*);

    private:
      friend class Design;
      NetNode*node_next_, *node_prev_;
      Design*design_;
};


/*
 * NetNet is a special kind of NetObj that doesn't really do anything,
 * but carries the properties of the wire/reg/trireg, including its
 * name. A scaler wire is a NetNet with one pin, a vector a wider
 * NetNet. NetNet objects also appear as side effects of synthesis or
 * other abstractions.
 *
 * NetNet objects have a name and exist within a scope, so the
 * constructor takes a pointer to the containing scope. The object
 * automatically adds itself to the scope.
 *
 * NetNet objects are located by searching NetScope objects.
 *
 * All the pins of a NetNet object are PASSIVE: they do not drive
 * anything and they are not a data sink, per se. The pins follow the
 * values on the nexus.
 */
class NetNet  : public NetObj, public LineInfo {

    public:
      enum Type { IMPLICIT, IMPLICIT_REG, WIRE, TRI, TRI1, SUPPLY0,
		  WAND, TRIAND, TRI0, SUPPLY1, WOR, TRIOR, REG, INTEGER };

      enum PortType { NOT_A_PORT, PIMPLICIT, PINPUT, POUTPUT, PINOUT };

      explicit NetNet(NetScope*s, const string&n, Type t, unsigned npins =1);

      explicit NetNet(NetScope*s, const string&n, Type t, long ms, long ls);

      virtual ~NetNet();

      NetScope* scope();
      const NetScope* scope() const;

      Type type() const { return type_; }
	//void type(Type t) { type_ = t; }

      PortType port_type() const { return port_type_; }
      void port_type(PortType t) { port_type_ = t; }

	/* These methods return the msb and lsb indices for the most
	   significant and least significant bits. These are signed
	   longs, and may be different from pin numbers. For example,
	   reg [1:8] has 8 bits, msb==1 and lsb==8. */
      long msb() const { return msb_; }
      long lsb() const { return lsb_; }

	/* This method converts a signed index (the type that might be
	   found in the verilog source) to a pin number. It accounts
	   for variation in the definition of the reg/wire/whatever. */
      unsigned sb_to_idx(long sb) const;

      bool local_flag() const { return local_flag_; }
      void local_flag(bool f) { local_flag_ = f; }

	/* NetESignal objects may reference this object. Keep a
	   reference count so that I keep track of them. */
      void incr_eref();
      void decr_eref();
      unsigned get_eref() const;


      virtual void dump_net(ostream&, unsigned) const;

    private:
	// The NetScope class uses this for listing signals.
      friend class NetScope;
      NetNet*sig_next_, *sig_prev_;

    private:
      NetScope*scope_;
      Type   type_;
      PortType port_type_;

      long msb_, lsb_;

      bool local_flag_;
      unsigned eref_count_;
};

/*
 * This class implements the LPM_ADD_SUB component as described in the
 * EDIF LPM Version 2 1 0 standard. It is used as a structural
 * implementation of the + and - operators.
 */
class NetAddSub  : public NetNode {

    public:
      NetAddSub(const string&n, unsigned width);
      ~NetAddSub();

	// Get the width of the device (that is, the width of the
	// operands and results.)
      unsigned width() const;

      Link& pin_Aclr();
      Link& pin_Add_Sub();
      Link& pin_Clock();
      Link& pin_Cin();
      Link& pin_Cout();
      Link& pin_Overflow();

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);
      Link& pin_Result(unsigned idx);

      const Link& pin_Cout() const;
      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);
};

/*
 * This type represents the LPM_CLSHIFT device.
 */
class NetCLShift  : public NetNode {

    public:
      NetCLShift(const string&n, unsigned width, unsigned width_dist);
      ~NetCLShift();

      unsigned width() const;
      unsigned width_dist() const;

      Link& pin_Direction();
      Link& pin_Underflow();
      Link& pin_Overflow();
      Link& pin_Data(unsigned idx);
      Link& pin_Result(unsigned idx);
      Link& pin_Distance(unsigned idx);

      const Link& pin_Direction() const;
      const Link& pin_Underflow() const;
      const Link& pin_Overflow() const;
      const Link& pin_Data(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;
      const Link& pin_Distance(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
      unsigned width_dist_;
};

/*
 * This class supports the LPM_COMPARE device.
 *
 * NOTE: This is not the same as the device used to support case
 * compare. Case comparisons handle Vx and Vz values, whereas this
 * device need not.
 */
class NetCompare  : public NetNode {

    public:
      NetCompare(const string&n, unsigned width);
      ~NetCompare();

      unsigned width() const;

      Link& pin_Aclr();
      Link& pin_Clock();
      Link& pin_AGB();
      Link& pin_AGEB();
      Link& pin_AEB();
      Link& pin_ANEB();
      Link& pin_ALB();
      Link& pin_ALEB();

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);

      const Link& pin_Aclr() const;
      const Link& pin_Clock() const;
      const Link& pin_AGB() const;
      const Link& pin_AGEB() const;
      const Link& pin_AEB() const;
      const Link& pin_ANEB() const;
      const Link& pin_ALB() const;
      const Link& pin_ALEB() const;

      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;

      virtual void functor_node(Design*, functor_t*);
      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
};

/*
 * This class represents a theoretical (though not necessarily
 * practical) integer divider gate. This is not to represent any real
 * hardware, but to support the / operator in Verilog, when it shows
 * up in structural contexts.
 *
 * The operands of the operation are the DataA<i> and DataB<i> inputs,
 * and the Result<i> output reflects the value DataA/DataB.
 */

class NetDivide  : public NetNode {

    public:
      NetDivide(const string&n, unsigned width, unsigned wa, unsigned wb);
      ~NetDivide();

      unsigned width_r() const;
      unsigned width_a() const;
      unsigned width_b() const;

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);
      Link& pin_Result(unsigned idx);

      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_r_;
      unsigned width_a_;
      unsigned width_b_;
};

/*
 * This class represents an LPM_FF device. There is no literal gate
 * type in Verilog that maps, but gates of this type can be inferred.
 */
class NetFF  : public NetNode {

    public:
      NetFF(const string&n, unsigned width);
      ~NetFF();

      unsigned width() const;

      Link& pin_Clock();
      Link& pin_Enable();
      Link& pin_Aload();
      Link& pin_Aset();
      Link& pin_Aclr();
      Link& pin_Sload();
      Link& pin_Sset();
      Link& pin_Sclr();

      Link& pin_Data(unsigned);
      Link& pin_Q(unsigned);

      const Link& pin_Clock() const;
      const Link& pin_Enable() const;
      const Link& pin_Data(unsigned) const;
      const Link& pin_Q(unsigned) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);
};


/*
 * This class represents the declared memory object. The parser
 * creates one of these for each declared memory in the elaborated
 * design. A reference to one of these is handled by the NetEMemory
 * object, which is derived from NetExpr. This is not a node because
 * memory objects can only be accessed by behavioral code.
 */
class NetMemory  {

    public:
      NetMemory(NetScope*sc, const string&n, long w, long s, long e);
      ~NetMemory();

      const string&name() const { return name_; }

	// This is the width (in bits) of a single memory position.
      unsigned width() const { return width_; }

      NetScope*scope();
      const NetScope*scope() const;

	// This is the number of memory positions.
      unsigned count() const;

	// This method returns a 0 based address of a memory entry as
	// indexed by idx. The Verilog source may give index ranges
	// that are not zero based.
      unsigned index_to_address(long idx) const;

      void set_attributes(const map<string,string>&a);

      void dump(ostream&o, unsigned lm) const;

    private:
      string name_;
      unsigned width_;
      long idxh_;
      long idxl_;

      map<string,string> attributes_;

      friend class NetRamDq;
      NetRamDq* ram_list_;

      friend class NetScope;
      NetMemory*snext_, *sprev_;
      NetScope*scope_;

    private: // not implemented
      NetMemory(const NetMemory&);
      NetMemory& operator= (const NetMemory&);
};

/*
 * This class implements the LPM_MULT component as described in the
 * EDIF LPM Version 2 1 0 standard. It is used as a structural
 * implementation of the * operator. The device has inputs DataA and
 * DataB that can have independent widths, as can the result. If the
 * result is smaller then the widths of a and b together, then the
 * device drops the least significant bits of the product.
 */
class NetMult  : public NetNode {

    public:
      NetMult(const string&n, unsigned width, unsigned wa, unsigned wb,
	      unsigned width_s =0);
      ~NetMult();

	// Get the width of the device bussed inputs. There are these
	// parameterized widths:
      unsigned width_r() const; // Result
      unsigned width_a() const; // DataA
      unsigned width_b() const; // DataB
      unsigned width_s() const; // Sum (my be 0)

      Link& pin_Aclr();
      Link& pin_Clock();

      Link& pin_DataA(unsigned idx);
      Link& pin_DataB(unsigned idx);
      Link& pin_Result(unsigned idx);
      Link& pin_Sum(unsigned idx);

      const Link& pin_Aclr() const;
      const Link& pin_Clock() const;

      const Link& pin_DataA(unsigned idx) const;
      const Link& pin_DataB(unsigned idx) const;
      const Link& pin_Result(unsigned idx) const;
      const Link& pin_Sum(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_r_;
      unsigned width_a_;
      unsigned width_b_;
      unsigned width_s_;
};


/*
 * This class represents an LPM_MUX device. This device has some
 * number of Result points (the width of the device) and some number
 * of input choices. There is also a selector of some width. The
 * parameters are:
 *
 *      width  -- Width of the result and each possible Data input
 *      size   -- Number of Data input (each of width)
 *      selw   -- Width in bits of the select input
 */
class NetMux  : public NetNode {

    public:
      NetMux(const string&n, unsigned width, unsigned size, unsigned selw);
      ~NetMux();

      unsigned width() const;
      unsigned size() const;
      unsigned sel_width() const;

      Link& pin_Aclr();
      Link& pin_Clock();

      Link& pin_Result(unsigned);
      Link& pin_Data(unsigned wi, unsigned si);
      Link& pin_Sel(unsigned);

      const Link& pin_Aclr() const;
      const Link& pin_Clock() const;

      const Link& pin_Result(unsigned) const;
      const Link& pin_Data(unsigned, unsigned) const;
      const Link& pin_Sel(unsigned) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_;
      unsigned size_;
      unsigned swidth_;
};

/*
 * This device represents an LPM_RAM_DQ device. The actual content is
 * represented by a NetMemory object allocated elsewhere, but that
 * object fixes the width and size of the device. The pin count of the
 * address input is given in the constructor.
 */
class NetRamDq  : public NetNode {

    public:
      NetRamDq(const string&name, NetMemory*mem, unsigned awid);
      ~NetRamDq();

      unsigned width() const;
      unsigned awidth() const;
      unsigned size() const;
      const NetMemory*mem() const;

      Link& pin_InClock();
      Link& pin_OutClock();
      Link& pin_WE();

      Link& pin_Address(unsigned idx);
      Link& pin_Data(unsigned idx);
      Link& pin_Q(unsigned idx);

      const Link& pin_InClock() const;
      const Link& pin_OutClock() const;
      const Link& pin_WE() const;

      const Link& pin_Address(unsigned idx) const;
      const Link& pin_Data(unsigned idx) const;
      const Link& pin_Q(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

	// Use this method to absorb other NetRamDq objects that are
	// connected to the same memory, and have compatible pin
	// connections.
      void absorb_partners();

	// Use this method to count the partners (including myself)
	// that are ports to the attached memory.
      unsigned count_partners() const;

    private:
      NetMemory*mem_;
      NetRamDq*next_;
      unsigned awidth_;

};

/* =========
 * There are cases where expressions need to be represented. The
 * NetExpr class is the root of a heirarchy that serves that purpose.
 *
 * The expr_width() is the width of the expression, that accounts
 * for the widths of the sub-expressions I might have. It is up to the
 * derived classes to properly set the expr width, if need be. The
 * set_width() method is used to compel an expression to have a
 * certain width, and is used particulary when the expression is an
 * rvalue in an assignment statement.
 */
class NetExpr  : public LineInfo {
    public:
      explicit NetExpr(unsigned w =0);
      virtual ~NetExpr() =0;

      virtual void expr_scan(struct expr_scan_t*) const =0;
      virtual void dump(ostream&) const;

	// How wide am I?
      unsigned expr_width() const { return width_; }

	// Coerce the expression to have a specific width. If the
	// coersion works, then return true. Otherwise, return false.
      virtual bool set_width(unsigned);

	// This method evaluates the expression and returns an
	// equivilent expression that is reduced as far as compile
	// time knows how. Essentially, this is designed to fold
	// constants.
      virtual NetExpr*eval_tree();

	// Make a duplicate of myself, and subexpressions if I have
	// any. This is a deep copy operation.
      virtual NetExpr*dup_expr() const =0;

	// Return a version of myself that is structural. This is used
	// for converting expressions to gates.
      virtual NetNet*synthesize(Design*);


    protected:
      void expr_width(unsigned w) { width_ = w; }

    private:
      unsigned width_;

    private: // not implemented
      NetExpr(const NetExpr&);
      NetExpr& operator=(const NetExpr&);
};

/*
 * The expression constant is slightly special, and is sometimes
 * returned from other classes that can be evaluated at compile
 * time. This class represents constant values in expressions.
 */
class NetEConst  : public NetExpr {

    public:
      explicit NetEConst(const verinum&val);
      ~NetEConst();

      const verinum&value() const { return value_; }

      virtual bool set_width(unsigned w);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetEConst* dup_expr() const;
      virtual NetNet*synthesize(Design*);

    private:
      verinum value_;
};

/*
 * The NetTmp object is a network that is only used momentarily by
 * elaboration to carry links around. A completed netlist should not
 * have any of these within. This is a kind of wire, so it is NetNet
 * type. The constructor for this class also marks the NetNet as
 * local, so that it is not likely to suppress a real symbol.
 */
class NetTmp  : public NetNet {

    public:
      explicit NetTmp(NetScope*s, const string&name, unsigned npins =1);

};

/*
 * The NetBUFZ is a magic device that represents the continuous
 * assign, with the output being the target register and the input
 * the logic that feeds it. The netlist preserves the directional
 * nature of that assignment with the BUFZ. The target may elide it if
 * that makes sense for the technology.
 */
class NetBUFZ  : public NetNode {

    public:
      explicit NetBUFZ(const string&n);
      ~NetBUFZ();

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
};

/*
 * This node is used to represent case equality in combinational
 * logic. Although this is not normally synthesizeable, it makes sense
 * to support an abstract gate that can compare x and z.
 *
 * This pins are assigned as:
 *
 *     0   -- Output (always returns 0 or 1)
 *     1   -- Input
 *     2   -- Input
 */
class NetCaseCmp  : public NetNode {

    public:
      explicit NetCaseCmp(const string&n);
      ~NetCaseCmp();

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
};

/*
 * This class represents instances of the LPM_CONSTANT device. The
 * node has only outputs and a constant value. The width is available
 * by getting the pin_count(), and the value bits are available one at
 * a time. There is no meaning to the aggregation of bits to form a
 * wide NetConst object, although some targets may have an easier time
 * detecting interesting constructs if they are combined.
 */
class NetConst  : public NetNode {

    public:
      explicit NetConst(const string&n, verinum::V v);
      explicit NetConst(const string&n, const verinum&val);
      ~NetConst();

      verinum::V value(unsigned idx) const;

      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      verinum::V*value_;
};

/*
 * This class represents all manner of logic gates. Pin 0 is OUTPUT and
 * all the remaining pins are INPUT. The BUFIF[01] gates have the
 * more specific pinout as follows:
 *
 *     bufif<N>
 *       0  -- output
 *       1  -- input data
 *       2  -- enable
 */
class NetLogic  : public NetNode {

    public:
      enum TYPE { AND, BUF, BUFIF0, BUFIF1, NAND, NOR, NOT, NOTIF0,
		  NOTIF1, OR, XNOR, XOR };

      explicit NetLogic(const string&n, unsigned pins, TYPE t);

      TYPE type() const { return type_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);

    private:
      const TYPE type_;
};

/*
 * The UDP is a User Defined Primitive from the Verilog source. Do not
 * expand it out any further then this in the netlist, as this can be
 * used to represent target device primitives.
 *
 * The UDP can be combinational or sequential. The sequential UDP
 * includes the current output in the truth table, and supports edges,
 * whereas the combinational does not and is entirely level sensitive.
 * In any case, pin 0 is an output, and all the remaining pins are
 * inputs.
 *
 * The sequential truth table is canonically represented as a finite state
 * machine with the current state representing the inputs and the
 * current output, and the next state carrying the new output value to
 * use. All the outgoing transitions from a state represent a single
 * edge.
 *
 * Set_table takes as input a string with one letter per pin. The
 * parser translates the written sequences to one of these. The
 * valid characters are:
 *
 *      0, 1, x  -- The levels
 *      r   -- (01)
 *      R   -- (x1)
 *      f   -- (10)
 *      F   -- (x0)
 *      P   -- (0x)
 *      N   -- (1x)
 *
 * It also takes one of the following glob letters to represent more
 * then one item.
 *
 *      p   -- 01, 0x or x1
 *      n   -- 10, 1x or x0
 *      ?   -- 0, 1, or x
 *      *   -- any edge
 *      +   -- 01 or x1
 *      _   -- 10 or x0  (Note that this is not the output '-'.)
 *      %   -- 0x or 1x
 *
 * SEQUENTIAL
 * These objects have a single bit of memory. The logic table includes
 * an entry for the current value, and allows edges on the inputs. In
 * canonical form, inly then entries that generate 0, 1 or - (no change)
 * are listed.
 *
 * COMBINATIONAL
 * The logic table is a map between the input levels and the
 * output. Each input pin can have the value 0, 1 or x and the output
 * can have the values 0 or 1. If the input matches nothing, the
 * output is x. In canonical form, only the entries that generate 0 or
 * 1 are listed.
 *
 */
class NetUDP  : public NetNode {

    public:
      explicit NetUDP(const string&n, unsigned pins, bool sequ =false);

      virtual bool emit_node( struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;

	/* return false if the entry conflicts with an existing
	   entry. In any case, the new output overrides. */
      bool set_table(const string&input, char output);
      void cleanup_table();

	/* Return the next output from the passed state. Each letter
	   of the input string represents the pin of the same
	   position. */
      char table_lookup(const string&from, char to, unsigned pin) const;

      void set_initial(char);
      char get_initial() const { return init_; }

      bool is_sequential() const { return sequential_; }

    private:
      bool sequential_;
      char init_;

      struct state_t_;
      struct pin_t_ {
	    state_t_*zer;
	    state_t_*one;
	    state_t_*xxx;

	    explicit pin_t_() : zer(0), one(0), xxx(0) { }
      };

      struct state_t_ {
	    char out;
	    pin_t_*pins;

	    state_t_(unsigned n) : out(0), pins(new pin_t_[n]) {}
	    ~state_t_() { delete[]pins; }
      };

      typedef map<string,state_t_*> FSM_;
      FSM_ fsm_;
      bool set_sequ_(const string&in, char out);
      bool sequ_glob_(string, char out);

      state_t_*find_state_(const string&);

	// A combinational primitive is more simply represented as a
	// simple map of input signals to a single output.
      typedef map<string,char> CM_;
      CM_ cm_;

      void dump_sequ_(ostream&o, unsigned ind) const;
      void dump_comb_(ostream&o, unsigned ind) const;
};

class NetUDP_COMB  : public NetNode {

    public:
      explicit NetUDP_COMB(const string&n, unsigned pins);

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;

	/* append a new truth table row. */
      void set_table(const string&input, char output);

	/* After the primitive is built up, this method is called to
	   clean up redundancies, and possibly optimize the table. */
      void cleanup_table();

	/* Use these methods to scan the truth table of the
	   device. "first" returns the first item in the table, and
	   "next" returns the next item in the table. The method will
	   return false when the scan is done. */
      bool first(string&inp, char&out) const;
      bool next(string&inp, char&out) const;

    private:

	// A combinational primitive is more simply represented as a
	// simple map of input signals to a single output.
      map<string,char> cm_;

      mutable map<string,char>::const_iterator idx_;
};



/* =========
 * A process is a behavioral-model description. A process is a
 * statement that may be compound. the various statement types may
 * refer to places in a netlist (by pointing to nodes) but is not
 * linked into the netlist. However, elaborating a process may cause
 * special nodes to be created to handle things like events.
 */
class NetProc : public LineInfo {

    public:
      explicit NetProc();
      virtual ~NetProc();

	// This method is called to emit the statement to the
	// target. The target returns true if OK, false for errors.
      virtual bool emit_proc(struct target_t*) const;

	// This method is called by functors that want to scan a
	// process in search of matchable patterns.
      virtual int match_proc(struct proc_match_t*);

      virtual void dump(ostream&, unsigned ind) const;

    private:
      friend class NetBlock;
      NetProc*next_;

    private: // not implemented
      NetProc(const NetProc&);
      NetProc& operator= (const NetProc&);
};

/*
 * This is a procedural assignment. The lval is a register, and the
 * assignment happens when the code is executed by the design. The
 * node part of the NetAssign has as many pins as the width of the
 * lvalue object and represents the elaborated lvalue. Thus, this
 * appears as a procedural statement AND a structural node. The
 * LineInfo is the location of the assignment statement in the source.
 *
 * NOTE: The elaborator will make an effort to match the width of the
 * r-value to the with of the assign node, but targets and functions
 * should know that this is not a guarantee.
 */

class NetAssign_ : public NetProc, public NetNode {

    public:

	// This is the (procedural) value that is to be assigned when
	// the assignment is executed.
      NetExpr*rval();
      const NetExpr*rval() const;

	// If this expression exists, then only a single bit is to be
	// set from the rval, and the value of this expression selects
	// the pin that gets the value.
      const NetExpr*bmux() const;

      void set_rval(NetExpr*);

    protected:
      NetAssign_(const string&n, unsigned w);
      virtual ~NetAssign_() =0;

      void set_bmux(NetExpr*);

    private:
      NetExpr*rval_;
      NetExpr*bmux_;
};

class NetAssign  : public NetAssign_ {
    public:
      explicit NetAssign(const string&, Design*des, unsigned w, NetExpr*rv);
      explicit NetAssign(const string&, Design*des, unsigned w,
			 NetExpr*mux, NetExpr*rv);
      ~NetAssign();

      virtual bool emit_proc(struct target_t*) const;
      virtual bool emit_node(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
};

/*
 * ... and this is a non-blocking version of above.
 */
class NetAssignNB  : public NetAssign_ {
    public:
      explicit NetAssignNB(const string&, Design*des, unsigned w, NetExpr*rv);
      explicit NetAssignNB(const string&, Design*des, unsigned w,
			   NetExpr*mux, NetExpr*rv);
      ~NetAssignNB();


      virtual bool emit_proc(struct target_t*) const;
      virtual bool emit_node(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
};

/*
 * Assignment to memory is handled separately because memory is
 * not a node. There are blocking and non-blocking variants, just like
 * regular assign, and the NetAssignMem_ base class takes care of all
 * the common stuff.
 */
class NetAssignMem_ : public NetProc {

    public:
      explicit NetAssignMem_(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMem_();

      NetMemory*memory() { return mem_; }
      NetExpr*index()    { return index_; }
      NetExpr*rval()     { return rval_; }

      const NetMemory*memory()const { return mem_; }
      const NetExpr*index()const    { return index_; }
      const NetExpr*rval()const     { return rval_; }

    private:
      NetMemory*mem_;
      NetExpr* index_;
      NetExpr* rval_;
};

class NetAssignMem : public NetAssignMem_ {

    public:
      explicit NetAssignMem(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMem();

      virtual int match_proc(struct proc_match_t*);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
};

class NetAssignMemNB : public NetAssignMem_ {

    public:
      explicit NetAssignMemNB(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMemNB();

      virtual int match_proc(struct proc_match_t*);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
};

/*
 * A block is stuff like begin-end blocks, that contain an ordered
 * list of NetProc statements.
 *
 * NOTE: The emit method calls the target->proc_block function but
 * does not recurse. It is up to the target-supplied proc_block
 * function to call emit_recurse.
 */
class NetBlock  : public NetProc {

    public:
      enum Type { SEQU, PARA };

      NetBlock(Type t) : type_(t), last_(0) { }
      ~NetBlock();

      const Type type() const { return type_; }

      void append(NetProc*);

      const NetProc*proc_first() const;
      const NetProc*proc_next(const NetProc*cur) const;

	// This version of emit_recurse scans all the statements of
	// the begin-end block sequentially. It is typically of use
	// for sequential blocks.
      void emit_recurse(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
      const Type type_;

      NetProc*last_;
};

/*
 * A CASE statement in the verilog source leads, eventually, to one of
 * these. This is different from a simple conditional because of the
 * way the comparisons are performed. Also, it is likely that the
 * target may be able to optimize differently.
 *
 * Case cane be one of three types:
 *    EQ  -- All bits must exactly match
 *    EQZ -- z bits are don't care
 *    EQX -- x and z bits are don't care.
 */
class NetCase  : public NetProc {

    public:
      enum TYPE { EQ, EQX, EQZ };
      NetCase(TYPE c, NetExpr*ex, unsigned cnt);
      ~NetCase();

      void set_case(unsigned idx, NetExpr*ex, NetProc*st);

      TYPE type() const;
      const NetExpr*expr() const { return expr_; }
      unsigned nitems() const { return nitems_; }

      const NetExpr*expr(unsigned idx) const { return items_[idx].guard;}
      const NetProc*stat(unsigned idx) const { return items_[idx].statement; }

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:

      TYPE type_;

      struct Item {
	    NetExpr*guard;
	    NetProc*statement;
      };

      NetExpr* expr_;
      unsigned nitems_;
      Item*items_;
};

/*
 * The cassign statement causes the r-val net to be forced onto the
 * l-val reg when it is executed. The code generator is expected to
 * know what that means. All the expressions are structural and behave
 * like nets.
 *
 * This class is a NetProc because it it turned on by procedural
 * behavior. However, it is also a NetNode because it connects to
 * nets, and when activated follows the net values.
 */
class NetCAssign  : public NetProc, public NetNode {

    public:
      explicit NetCAssign(const string&n, NetNet*l);
      ~NetCAssign();

      const Link& lval_pin(unsigned) const;

      virtual void dump(ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      NetNet*lval_;

    private: // not implemented
      NetCAssign(const NetCAssign&);
      NetCAssign& operator= (const NetCAssign&);
};


/* A condit represents a conditional. It has an expression to test,
   and a pair of statements to select from. */
class NetCondit  : public NetProc {

    public:
      explicit NetCondit(NetExpr*ex, NetProc*i, NetProc*e);
      ~NetCondit();

      const NetExpr*expr() const;
      NetExpr*expr();

      NetProc* if_clause();
      NetProc* else_clause();

	// Replace the condition expression.
      void set_expr(NetExpr*ex);

      void emit_recurse_if(struct target_t*) const;
      void emit_recurse_else(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr* expr_;
      NetProc*if_;
      NetProc*else_;
};

/*
 * The procedural deassign statement (the opposite of assign) releases
 * any assign expressions attached to the bits of the reg. The
 * lval is the expression of the "deassign <expr>;" statement with the
 * expr elaborated to a net.
 */
class NetDeassign : public NetProc {

    public:
      explicit NetDeassign(NetNet*l);
      ~NetDeassign();

      const NetNet*lval() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetNet*lval_;

    private: // not implemented
      NetDeassign(const NetDeassign&);
      NetDeassign& operator= (const NetDeassign&);
};

/*
 * This node represents the behavioral disable statement. The Verilog
 * source that produces it looks like:
 *
 *          disable <scope>;
 *
 * Where the scope is a named block or a task. It cannot be a module
 * instance scope because module instances cannot be disabled.
 */
class NetDisable  : public NetProc {

    public:
      explicit NetDisable(NetScope*tgt);
      ~NetDisable();

      const NetScope*target() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetScope*target_;

    private: // not implemented
      NetDisable(const NetDisable&);
      NetDisable& operator= (const NetDisable&);
};

/*
 * A NetEvent is an object that represents an event object, that is
 * objects declared like so in Verilog:
 *
 *        event foo;
 *
 * Once an object of this type exists, behavioral code can wait on the
 * event or trigger the event. Event waits refer to this object, as do
 * the event trigger statements.
 *
 * The NetEvWait class represents a thread wait for an event. When
 * this statement is executed, it starts waiting on the
 * event. Conceptually, it puts itself on the event list for the
 * referenced event. When the event is triggered, the wit ends its
 * block and starts the associated statement.
 *
 * The NetEvTrig class represents trigger statements. Executing this
 * statement causes the referenced event to be triggered, which it
 * turn awakens the waiting threads. Each NetEvTrig object references
 * exactly one event object.
 *
 * The NetEvProbe class is the structural equivilent of the NetEvTrig,
 * in that it is a node and watches bit values that it receives. It
 * checks for edges then if appropriate triggers the associated
 * NetEvent. Each NetEvProbe references exactly one event object, and
 * the NetEvent objects have a list of NetEvProbe objects that
 * reference it.
 */
class NetEvent : public LineInfo {

      friend class NetScope;
      friend class NetEvProbe;
      friend class NetEvTrig;
      friend class NetEvWait;

    public:
      explicit NetEvent (const string&n);
      ~NetEvent();

      string name() const;
      string full_name() const;

	// Get information about probes connected to me.
      unsigned nprobe() const;
      NetEvProbe* probe(unsigned);

	// Return the number of NetEvWait nodes that reference me.
      unsigned nwait() const;

      unsigned ntrig() const;

      NetScope* scope();
      const NetScope* scope() const;

	// Locate the first event that matches my behavior and
	// monitors the same signals.
      NetEvent* find_similar_event();

	// This method replaces pointers to me with pointers to
	// that. It is typically used to replace similar events
	// located by the find_similar_event method.
      void replace_event(NetEvent*that);

    private:
      string name_;

	// The NetScope class uses these to list the events.
      NetScope*scope_;
      NetEvent*snext_;

	// Use these methods to list the probes attached to me.
      NetEvProbe*probes_;

	// Use these methods to list the triggers attached to me.
      NetEvTrig* trig_;

	// Use This member to count references by NetEvWait objects.
      unsigned waitref_;
      struct wcell_ {
	    NetEvWait*obj;
	    struct wcell_*next;
      };
      struct wcell_ *wlist_;

    private: // not implemented
      NetEvent(const NetEvent&);
      NetEvent& operator= (const NetEvent&);
};

class NetEvTrig  : public NetProc {

      friend class NetEvent;

    public:
      explicit NetEvTrig(NetEvent*tgt);
      ~NetEvTrig();

      const NetEvent*event() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetEvent*event_;
	// This is used to place me in the NetEvents lists of triggers.
      NetEvTrig*enext_;
};

class NetEvWait  : public NetProc {

    public:
      explicit NetEvWait(NetProc*st);
      ~NetEvWait();

      void add_event(NetEvent*tgt);
      void replace_event(NetEvent*orig, NetEvent*repl);

      unsigned nevents() const;
      const NetEvent*event(unsigned) const;
      NetEvent*event(unsigned);

      NetProc*statement();

      virtual bool emit_proc(struct target_t*) const;
      bool emit_recurse(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetProc*statement_;

      unsigned nevents_;
      NetEvent**events_;
};

class NetEvProbe  : public NetNode {

      friend class NetEvent;

    public:
      enum edge_t { ANYEDGE, POSEDGE, NEGEDGE };

      explicit NetEvProbe(const string&n, NetEvent*tgt,
			  edge_t t, unsigned p);
      ~NetEvProbe();

      edge_t edge() const;
      NetEvent* event();
      const NetEvent* event() const;

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      NetEvent*event_;
      edge_t edge_;
	// The NetEvent class uses this to list me.
      NetEvProbe*enext_;
};

/*
 * The force statement causes the r-val net to be forced onto the
 * l-val net when it is executed. The code generator is expected to
 * know what that means. All the expressions are structural and behave
 * like nets.
 *
 * This class is a NetProc because it it turned on by procedural
 * behavior. However, it is also a NetNode because it connects to
 * nets, and when activated follows the net values.
 */
class NetForce  : public NetProc, public NetNode {

    public:
      explicit NetForce(const string&n, NetNet*l);
      ~NetForce();

      const Link& lval_pin(unsigned) const;

      virtual void dump(ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      NetNet*lval_;
};

/*
 * A forever statement is executed over and over again forever. Or
 * until its block is disabled.
 */
class NetForever : public NetProc {

    public:
      explicit NetForever(NetProc*s);
      ~NetForever();

      void emit_recurse(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetProc*statement_;
};

/*
 * A funciton definition is elaborated just like a task, though by now
 * it is certain that the first parameter (a phantom parameter) is the
 * output and all the remaining parameters are the inputs. This makes
 * for easy code generation in targets that support behavioral descriptions.
 */
class NetFuncDef {

    public:
      NetFuncDef(NetScope*, const svector<NetNet*>&po);
      ~NetFuncDef();

      void set_proc(NetProc*st);

      const string name() const;
      const NetProc*proc() const;
      NetScope*scope();

      unsigned port_count() const;
      const NetNet*port(unsigned idx) const;

      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetScope*scope_;
      NetProc*statement_;
      svector<NetNet*>ports_;
};

/*
 * This class represents delay statements of the form:
 *
 *     #<expr> <statement>
 *
 * Where the statement may be null. The delay is evaluated at
 * elaboration time to make a constant unsigned long that is the delay
 * in simulation ticks.
 *
 * If the delay expression is non-constant, construct the NetPDelay
 * object with a NetExpr* instead of the d value, and use th expr()
 * method to get the expression. If expr() returns 0, use the delay()
 * method to get the constant delay.
 */
class NetPDelay  : public NetProc {

    public:
      NetPDelay(unsigned long d, NetProc*st);
      NetPDelay(NetExpr* d, NetProc*st);
      ~NetPDelay();

      unsigned long delay() const;
      const NetExpr*expr() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

      bool emit_proc_recurse(struct target_t*) const;

    private:
      unsigned long delay_;
      NetExpr*expr_;
      NetProc*statement_;
};

/*
 * A repeat statement is executed some fixed number of times.
 */
class NetRepeat : public NetProc {

    public:
      explicit NetRepeat(NetExpr*e, NetProc*s);
      ~NetRepeat();

      const NetExpr*expr() const;
      void emit_recurse(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr*expr_;
      NetProc*statement_;
};

/*
 * The procedural release statement (the opposite of force) releases
 * any force expressions attached to the bits of the wire or reg. The
 * lval is the expression of the "release <expr>;" statement with the
 * expr elaborated to a net.
 */
class NetRelease : public NetProc {

    public:
      explicit NetRelease(NetNet*l);
      ~NetRelease();

      const NetNet*lval() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetNet*lval_;
};


/*
 * The NetSTask class is a call to a system task. These kinds of tasks
 * are generally handled very simply in the target. They certainly are
 * handled differently from user defined tasks because ivl knows all
 * about the user defined tasks.
 */
class NetSTask  : public NetProc {

    public:
      NetSTask(const string&na, const svector<NetExpr*>&);
      ~NetSTask();

      const string& name() const { return name_; }

      unsigned nparms() const { return parms_.count(); }

      const NetExpr* parm(unsigned idx) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      string name_;
      svector<NetExpr*>parms_;
};

/*
 * This class represents an elaborated class definition. NetUTask
 * classes may refer to objects of this type to get the meaning of the
 * defined task.
 *
 * The task also introduces a scope, and the parameters are actually
 * reg objects in the new scope. The task is called by the calling
 * thread assigning (blocking assignment) to the in and inout
 * paramters, then invoking the thread, and finally assigning out the
 * output and inout variables. The variables accessable as ports are
 * also elaborated and accessible as ordinary reg objects.
 */
class NetTaskDef {

    public:
      NetTaskDef(const string&n, const svector<NetNet*>&po);
      ~NetTaskDef();

      void set_proc(NetProc*p);

      const string& name() const;
      const NetProc*proc() const;

      unsigned port_count() const;
      NetNet*port(unsigned idx);

      void dump(ostream&, unsigned) const;

    private:
      string name_;
      NetProc*proc_;
      svector<NetNet*>ports_;

    private: // not implemented
      NetTaskDef(const NetTaskDef&);
      NetTaskDef& operator= (const NetTaskDef&);
};

/*
 * This node represents a function call in an expression. The object
 * contains a pointer to the function definition, which is used to
 * locate the value register and input expressions.
 *
 * The NetNet parameter to the constructor is the *register* NetNet
 * that receives the result of the function, and the NetExpr list is
 * the paraneters passed to the function.
 */
class NetEUFunc  : public NetExpr {

    public:
      NetEUFunc(NetFuncDef*, NetESignal*, svector<NetExpr*>&);
      ~NetEUFunc();

      const string name() const;

      const NetESignal*result() const;
      unsigned parm_count() const;
      const NetExpr* parm(unsigned idx) const;

      const NetFuncDef* definition() const;

      virtual bool set_width(unsigned);
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEUFunc*dup_expr() const;

    private:
      NetFuncDef*func_;
      NetESignal*result_;
      svector<NetExpr*> parms_;

    private: // not implemented
      NetEUFunc(const NetEUFunc&);
      NetEUFunc& operator= (const NetEUFunc&);
};

/*
 * A call to a user defined task is elaborated into this object. This
 * contains a pointer to the elaborated task definition, but is a
 * NetProc object so that it can be linked into statements.
 */
class NetUTask  : public NetProc {

    public:
      NetUTask(NetTaskDef*);
      ~NetUTask();

      const string& name() const { return task_->name(); }

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetTaskDef*task_;
};

/*
 * The while statement is a condition that is tested in the front of
 * each iteration, and a statement (a NetProc) that is executed as
 * long as the condition is true.
 */
class NetWhile  : public NetProc {

    public:
      NetWhile(NetExpr*c, NetProc*p)
      : cond_(c), proc_(p) { }

      const NetExpr*expr() const { return cond_; }

      void emit_proc_recurse(struct target_t*) const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr* cond_;
      NetProc*proc_;
};


/*
 * The is the top of any process. It carries the type (initial or
 * always) and a pointer to the statement, probably a block, that
 * makes up the process.
 */
class NetProcTop  : public LineInfo {

    public:
      enum Type { KINITIAL, KALWAYS };

      NetProcTop(Type t, class NetProc*st);
      ~NetProcTop();

      Type type() const { return type_; }
      NetProc*statement();
      const NetProc*statement() const;

      void dump(ostream&, unsigned ind) const;
      bool emit(struct target_t*tgt) const;

    private:
      const Type type_;
      NetProc*const statement_;

      friend class Design;
      NetProcTop*next_;
};

/*
 * This class represents a binary operator, with the left and right
 * operands and a single character for the operator. The operator
 * values are:
 *
 *   ^  -- Bit-wise exclusive OR
 *   +  -- Arithmetic add
 *   -  -- Arithmetic minus
 *   *  -- Arithmetic multiply
 *   /  -- Arithmetic divide
 *   %  -- Arithmetic modulus
 *   &  -- Bit-wise AND
 *   |  -- Bit-wise OR
 *   <  -- Less then
 *   >  -- Greater then
 *   e  -- Logical equality (==)
 *   E  -- Case equality (===)
 *   L  -- Less or equal
 *   G  -- Greater or equal
 *   n  -- Logical inequality (!=)
 *   N  -- Case inequality (!==)
 *   a  -- Logical AND (&&)
 *   o  -- Logical OR (||)
 *   O  -- Bit-wise NOR
 *   l  -- Left shift (<<)
 *   r  -- Right shift (>>)
 *   X  -- Bitwise exclusive NOR (~^)
 */
class NetEBinary  : public NetExpr {

    public:
      NetEBinary(char op, NetExpr*l, NetExpr*r);
      ~NetEBinary();

      const NetExpr*left() const { return left_; }
      const NetExpr*right() const { return right_; }

      char op() const { return op_; }

      virtual bool set_width(unsigned w);

      virtual NetEBinary* dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    protected:
      char op_;
      NetExpr* left_;
      NetExpr* right_;

      virtual void eval_sub_tree_();
};

/*
 * The addition operators have slightly more complex width
 * calculations because there is the optional carry bit that can be
 * used. The operators covered by this class are:
 *   +  -- Arithmetic add
 *   -  -- Arithmetic minus
 */
class NetEBAdd : public NetEBinary {

    public:
      NetEBAdd(char op, NetExpr*l, NetExpr*r);
      ~NetEBAdd();

      virtual bool set_width(unsigned w);
      virtual NetEBAdd* dup_expr() const;
      virtual NetEConst* eval_tree();
      virtual NetNet* synthesize(Design*);
};

/*
 * This class represents the integer division operators.
 *   /  -- Divide
 *   %  -- Modulus
 */
class NetEBDiv : public NetEBinary {

    public:
      NetEBDiv(char op, NetExpr*l, NetExpr*r);
      ~NetEBDiv();

      virtual bool set_width(unsigned w);
      virtual NetEBDiv* dup_expr() const;
      virtual NetEConst* eval_tree();
      virtual NetNet* synthesize(Design*);
};

/*
 * The bitwise binary operators are represented by this class. This is
 * a specialization of the binary operator, so is derived from
 * NetEBinary. The particular constraints on these operators are that
 * operand and result widths match exactly, and each bit slice of the
 * operation can be represented by a simple gate. The operators
 * covered by this class are:
 *
 *   ^  -- Bit-wise exclusive OR
 *   &  -- Bit-wise AND
 *   |  -- Bit-wise OR
 *   O  -- Bit-wise NOR
 *   X  -- Bit-wise XNOR (~^)
 */
class NetEBBits : public NetEBinary {

    public:
      NetEBBits(char op, NetExpr*l, NetExpr*r);
      ~NetEBBits();

      virtual bool set_width(unsigned w);
      virtual NetEBBits* dup_expr() const;

      virtual NetNet* synthesize(Design*);
};

/*
 * The binary comparison operators are handled by this class. This
 * this case the bit width of the expression is 1 bit, and the
 * operands take their natural widths. The supported operators are:
 *
 *   <  -- Less then
 *   >  -- Greater then
 *   e  -- Logical equality (==)
 *   E  -- Case equality (===)
 *   L  -- Less or equal (<=)
 *   G  -- Greater or equal (>=)
 *   n  -- Logical inequality (!=)
 *   N  -- Case inequality (!==)
 */
class NetEBComp : public NetEBinary {

    public:
      NetEBComp(char op, NetExpr*l, NetExpr*r);
      ~NetEBComp();

      virtual bool set_width(unsigned w);
      virtual NetEBComp* dup_expr() const;
      virtual NetEConst* eval_tree();

      virtual NetNet* synthesize(Design*);

    private:
      NetEConst*eval_eqeq_();
      NetEConst*eval_leeq_();

};

/*
 * The binary logical operators are those that return boolean
 * results. The supported operators are:
 *
 *   a  -- Logical AND (&&)
 */
class NetEBLogic : public NetEBinary {

    public:
      NetEBLogic(char op, NetExpr*l, NetExpr*r);
      ~NetEBLogic();

      virtual bool set_width(unsigned w);
      virtual NetEBLogic* dup_expr() const;
      virtual NetEConst* eval_tree();

    private:
};


/*
 * Support the binary multiplication (*) operator.
 */
class NetEBMult : public NetEBinary {

    public:
      NetEBMult(char op, NetExpr*l, NetExpr*r);
      ~NetEBMult();

      virtual bool set_width(unsigned w);
      virtual NetEBMult* dup_expr() const;
      virtual NetEConst* eval_tree();

    private:
};


/*
 * The binary logical operators are those that return boolean
 * results. The supported operators are:
 *
 *   l  -- left shift (<<)
 *   r  -- right shift (>>)
 */
class NetEBShift : public NetEBinary {

    public:
      NetEBShift(char op, NetExpr*l, NetExpr*r);
      ~NetEBShift();

      virtual bool set_width(unsigned w);
      virtual NetEBShift* dup_expr() const;
      virtual NetEConst* eval_tree();

    private:
};


/*
 * This expression node supports the concat expression. This is an
 * operator that just glues the results of many expressions into a
 * single value.
 *
 * Note that the class stores the parameter expressions in source code
 * order. That is, the parm(0) is placed in the most significant
 * position of the result.
 */
class NetEConcat  : public NetExpr {

    public:
      NetEConcat(unsigned cnt, unsigned repeat =1);
      ~NetEConcat();

	// Manipulate the parameters.
      void set(unsigned idx, NetExpr*e);

      unsigned repeat() const { return repeat_; }
      unsigned nparms() const { return parms_.count() ; }
      NetExpr* parm(unsigned idx) const { return parms_[idx]; }

      virtual bool set_width(unsigned w);
      virtual NetEConcat* dup_expr() const;
      virtual NetExpr* eval_tree();
      virtual NetNet*synthesize(Design*);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      svector<NetExpr*>parms_;
      unsigned repeat_;
};

/*
 * This clas is a placeholder for a parameter expression. When
 * parameters are first created, an instance of this object is used to
 * hold the place where the parameter exression goes. Then, when the
 * parameters are resolved, these objects are removed.
 *
 * If the parameter object is created with a path and name, then the
 * object represents a reference to a parameter that is known to exist.
 */
class NetEParam  : public NetExpr {
    public:
      NetEParam();
      NetEParam(class Design*des, NetScope*scope, const string&name);
      ~NetEParam();

      virtual bool set_width(unsigned w);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetExpr* eval_tree();
      virtual NetEParam* dup_expr() const;

      virtual void dump(ostream&) const;

    private:
      Design*des_;
      NetScope*scope_;
      string name_;
};


/*
 * This class is a special (and magical) expression node type that
 * represents scope names. These can only be found as parameters to
 * NetSTask objects.
 */
class NetEScope  : public NetExpr {

    public:
      NetEScope(NetScope*);
      ~NetEScope();

      const NetScope* scope() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEScope* dup_expr() const;

      virtual void dump(ostream&os) const;

    private:
      NetScope*scope_;
};

/*
 * This node represents a system function call in an expression. The
 * object contains the name of the system function, which the backend
 * uses to do VPI matching.
 */
class NetESFunc  : public NetExpr {

    public:
      NetESFunc(const string&name, unsigned width, unsigned nprms);
      ~NetESFunc();

      const string& name() const;

      unsigned nparms() const;
      void parm(unsigned idx, NetExpr*expr);
      NetExpr* parm(unsigned idx);
      const NetExpr* parm(unsigned idx) const;

      virtual bool set_width(unsigned);
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetESFunc*dup_expr() const;

    private:
      string name_;
      unsigned nparms_;
      NetExpr**parms_;

    private: // not implemented
      NetESFunc(const NetESFunc&);
      NetESFunc& operator= (const NetESFunc&);
};

/*
 * This class represents the ternary (?:) operator. It has 3
 * expressions, one of which is a condition used to select which of
 * the other two expressions is the result.
 */
class NetETernary  : public NetExpr {

    public:
      NetETernary(NetExpr*c, NetExpr*t, NetExpr*f);
      ~NetETernary();

      virtual bool set_width(unsigned w);

      const NetExpr*cond_expr() const;
      const NetExpr*true_expr() const;
      const NetExpr*false_expr() const;

      virtual NetETernary* dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;
      virtual NetNet*synthesize(Design*);

    private:
      NetExpr*cond_;
      NetExpr*true_val_;
      NetExpr*false_val_;
};

/*
 * This class represents a unary operator, with the single operand
 * and a single character for the operator. The operator values are:
 *
 *   ~  -- Bit-wise negation
 *   !  -- Logical negation
 *   &  -- Reduction AND
 *   |  -- Reduction OR
 *   ^  -- Reduction XOR
 *   +  --
 *   -  --
 *   A  -- Reduction NAND (~&)
 *   N  -- Reduction NOR (~|)
 *   X  -- Reduction NXOR (~^ or ^~)
 */
class NetEUnary  : public NetExpr {

    public:
      NetEUnary(char op, NetExpr*ex);
      ~NetEUnary();

      char op() const { return op_; }
      const NetExpr* expr() const { return expr_; }

      virtual bool set_width(unsigned w);

      virtual NetEUnary* dup_expr() const;
      virtual NetEConst* eval_tree();

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    protected:
      char op_;
      NetExpr* expr_;
};

class NetEUBits : public NetEUnary {

    public:
      NetEUBits(char op, NetExpr*ex);
      ~NetEUBits();

      virtual NetNet* synthesize(Design*);

};

/* System identifiers are represented here. */
class NetEIdent  : public NetExpr {

    public:
      NetEIdent(const string&n, unsigned w)
      : NetExpr(w), name_(n) { }

      const string& name() const { return name_; }

      NetEIdent* dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      string name_;
};

/*
 * A reference to a memory is represented by this expression. If the
 * index is not supplied, then the node is only valid in certain
 * specific contexts.
 */
class NetEMemory  : public NetExpr {

    public:
      NetEMemory(NetMemory*mem, NetExpr*idx =0);
      virtual ~NetEMemory();

      const string& name () const { return mem_->name(); }
      const NetExpr* index() const { return idx_; }

      virtual bool set_width(unsigned);

      virtual NetEMemory*dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      NetMemory*mem_;
      NetExpr* idx_;
};

/*
 * When a signal shows up in an expression, this type represents
 * it. From this the expression can get any kind of access to the
 * structural signal.
 *
 * A signal shows up as a node in the netlist so that structural
 * activity can invoke the expression.
 */
class NetESignal  : public NetExpr {

    public:
      NetESignal(NetNet*n);
      ~NetESignal();

      const string& name() const;
      virtual bool set_width(unsigned);

      virtual NetESignal* dup_expr() const;
      NetNet* synthesize(Design*des);

	// These methods actually reference the properties of the
	// NetNet object that I point to.
      unsigned pin_count() const;
      Link& pin(unsigned idx);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      NetNet*net_;
};

/*
 * An expression that takes a portion of a signal is represented as
 * one of these. For example, ``foo[x+5]'' is a signal and x+5 is an
 * expression to select a single bit from that signal. I can't just
 * make a new NetESignal node connected to the single net because the
 * expression may vary during execution, so the structure is not known
 * at compile (elaboration) time.
 */
class NetESubSignal  : public NetExpr {

    public:
      NetESubSignal(NetESignal*sig, NetExpr*ex);
      ~NetESubSignal();

      const string&name() const { return sig_->name(); }
      const NetExpr*index() const { return idx_; }

      virtual bool set_width(unsigned);

      NetESubSignal* dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
	// For now, only support single-bit selects of a signal.
      NetESignal*sig_;
      NetExpr* idx_;
};


/*
 * This object type is used to contain a logical scope within a
 * design. The scope doesn't represent any executable hardware, but is
 * just a handle that netlist processors can use to grab at the design.
 */
class NetScope {

    public:
      enum TYPE { MODULE, TASK, FUNC, BEGIN_END, FORK_JOIN };
      NetScope(const string&root);
      NetScope(NetScope*up, const string&name, TYPE t);
      ~NetScope();

	/* Parameters exist within a scope, and these methods allow
	   one to manipulate the set. In these cases, the name is the
	   *simple* name of the paramter, the heirarchy is implicit in
	   the scope. The return value from set_parameter is the
	   previous expression, if there was one. */

      NetExpr* set_parameter(const string&name, NetExpr*val);
      NetExpr* set_localparam(const string&name, NetExpr*val);
      const NetExpr*get_parameter(const string&name) const;

	/* These methods set or access events that live in this
	   scope. */

      void add_event(NetEvent*);
      void rem_event(NetEvent*);
      NetEvent*find_event(const string&name);


	/* These methods manage signals. The add_ and rem_signal
	   methods are used by the NetNet objects to make themselves
	   available to the scope, and the find_signal method can be
	   used to locate signals within a scope. */

      void add_signal(NetNet*);
      void rem_signal(NetNet*);

      NetNet* find_signal(const string&name);


	/* ... and these methods manage memory the same way as signals
	   are managed above. */

      void add_memory(NetMemory*);
      void rem_memory(NetMemory*);

      NetMemory* find_memory(const string&name);


	/* The parent and child() methods allow users of NetScope
	   objects to locate nearby scopes. */
      NetScope* parent();
      NetScope* child(const string&name);
      const NetScope* parent() const;
      const NetScope* child(const string&name) const;

      TYPE type() const;

      void set_task_def(NetTaskDef*);
      void set_func_def(NetFuncDef*);

      NetTaskDef* task_def();
      NetFuncDef* func_def();

      const NetTaskDef* task_def() const;
      const NetFuncDef* func_def() const;

	/* Scopes have their own time units and time precision. The
	   unit and precision are given as power of 10, i.e. -3 is
	   units of milliseconds.

	   If a NetScope is created with a parent scope, the new scope
	   will initially inherit the unit and precision of the
	   parent scope. */

      void time_unit(int);
      void time_precision(int);

      int time_unit() const;
      int time_precision() const;

	/* The name of the scope is the fully qualified hierarchical
	   name, whereas the basename is just my name within my parent
	   scope. */
      string basename() const;
      string name() const;

      void run_defparams(class Design*);
      void evaluate_parameters(class Design*);

	/* This method generates a non-hierarchical name that is
	   guaranteed to be unique within this scope. */
      string local_symbol();

      void dump(ostream&) const;
      void emit_scope(struct target_t*tgt) const;
      void emit_defs(struct target_t*tgt) const;

	/* This method runs the functor on me. Recurse through the
	   children of this node as well. */
      void run_functor(Design*des, functor_t*fun);


	/* This member is used during elaboration to pass defparam
	   assignments from the scope pass to the parameter evaluation
	   step. After that, it is not used. */

      map<string,NetExpr*>defparams;

    private:
      TYPE type_;
      string name_;

      signed char time_unit_, time_prec_;

      map<string,NetExpr*>parameters_;
      map<string,NetExpr*>localparams_;

      NetEvent *events_;
      NetNet   *signals_;
      NetMemory*memories_;

      union {
	    NetTaskDef*task_;
	    NetFuncDef*func_;
      };

      NetScope*up_;
      NetScope*sib_;
      NetScope*sub_;

      unsigned lcounter_;
};

/*
 * This class contains an entire design. It includes processes and a
 * netlist, and can be passed around from function to function.
 */
class Design {

    public:
      Design();
      ~Design();


	/* The flags are a generic way of accepting command line
	   parameters/flags and passing them to the processing steps
	   that deal with the design. The compilation driver sets the
	   entire flags map after elaboration is done. Subsequent
	   steps can then use the get_flag() function to get the value
	   of an interesting key. */

      void set_flags(const map<string,string>&f) { flags_ = f; }

      string get_flag(const string&key) const;

      NetScope* make_root_scope(const string&name);
      NetScope* find_root_scope();


	/* Attempt to set the precision to the specified value. If the
	   precision is already more precise, the keep the precise
	   setting. This is intended to hold the simulation precision
	   for use throughout the entire design. */

      void set_precision(int val);
      int  get_precision() const;

	/* This function takes a delay value and a scope, and returns
	   the delay value scaled to the precision of the design. */
      unsigned long scale_to_precision(unsigned long, const NetScope*)const;

	/* look up a scope. If no starting scope is passed, then the
	   path name string is taken as an absolute scope
	   name. Otherwise, the scope is located starting at the
	   passed scope and working up if needed. */
      NetScope* find_scope(const string&path) const;
      NetScope* find_scope(NetScope*, const string&path) const;

	// PARAMETERS

	/* This method searches for a parameter, starting in the given
	   scope. This method handles the upward searches that the
	   NetScope class itself does not support. */
      const NetExpr*find_parameter(const NetScope*, const string&name) const;

      void run_defparams();
      void evaluate_parameters();

	/* This method locates a signal, starting at a given
	   scope. The name parameter may be partially hierarchical, so
	   this method, unlike the NetScope::find_signal method,
	   handles global name binding. */

      NetNet*find_signal(NetScope*scope, const string&name);

	// Memories
      NetMemory* find_memory(NetScope*scope, const string&name);

	/* This is a more general lookup that finds the named signal
	   or memory, whichever is first in the search path. */
      void find_symbol(NetScope*,const string&key,
		       NetNet*&sig, NetMemory*&mem);

	// Functions
      NetFuncDef* find_function(const string&path, const string&key);
      NetFuncDef* find_function(const string&path);

	// Tasks
      NetTaskDef* find_task(const string&path, const string&name);
      NetTaskDef* find_task(const string&key);

	// NODES
      void add_node(NetNode*);
      void del_node(NetNode*);

	// PROCESSES
      void add_process(NetProcTop*);
      void delete_process(NetProcTop*);

	// Iterate over the design...
      void dump(ostream&) const;
      void functor(struct functor_t*);
      bool emit(struct target_t*) const;

	// This is incremented by elaboration when an error is
	// detected. It prevents code being emitted.
      unsigned errors;

    public:
      string local_symbol(const string&path);

    private:
	// Keep a tree of scopes. The NetScope class handles the wide
	// tree and per-hop searches for me.
      NetScope*root_scope_;

	// List the nodes in the design
      NetNode*nodes_;

	// List the processes in the design.
      NetProcTop*procs_;
      NetProcTop*procs_idx_;

      map<string,string> flags_;

      int des_precision_;

      unsigned lcounter_;

    private: // not implemented
      Design(const Design&);
      Design& operator= (const Design&);
};


/* =======
 */

inline bool operator == (const Link&l, const Link&r)
{ return l.is_equal(r); }

inline bool operator != (const Link&l, const Link&r)
{ return ! l.is_equal(r); }

/* Connect the pins of two nodes together. Either may already be
   connected to other things, connect is transitive. */
extern void connect(Link&, Link&);

/* Return true if l and r are connected. */
inline bool connected(const Link&l, const Link&r)
{ return l.is_linked(r); }

/* return the number of links in the ring that are of the specified
   type. */
extern unsigned count_inputs(const Link&pin);
extern unsigned count_outputs(const Link&pin);
extern unsigned count_signals(const Link&pin);

/* Find the next link that is an output into the nexus. */
extern Link* find_next_output(Link*lnk);

/* Find the signal connected to the given node pin. There should
   always be exactly one signal. The bidx parameter get filled with
   the signal index of the Net, in case it is a vector. */
const NetNet* find_link_signal(const NetObj*net, unsigned pin,
			       unsigned&bidx);

inline ostream& operator << (ostream&o, const NetExpr&exp)
{ exp.dump(o); return o; }

extern ostream& operator << (ostream&, NetNet::Type);

/*
 * $Log: netlist.h,v $
 * Revision 1.156  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.155  2000/08/09 03:43:45  steve
 *  Move all file manipulation out of target class.
 *
 * Revision 1.154  2000/08/08 01:50:42  steve
 *  target methods need not take a file stream.
 *
 * Revision 1.153  2000/08/02 14:48:15  steve
 *  cleanup.
 *
 * Revision 1.152  2000/08/01 02:48:42  steve
 *  Support <= in synthesis of DFF and ram devices.
 *
 * Revision 1.151  2000/07/30 18:25:44  steve
 *  Rearrange task and function elaboration so that the
 *  NetTaskDef and NetFuncDef functions are created during
 *  signal enaboration, and carry these objects in the
 *  NetScope class instead of the extra, useless map in
 *  the Design class.
 *
 * Revision 1.150  2000/07/29 16:21:08  steve
 *  Report code generation errors through proc_delay.
 *
 * Revision 1.149  2000/07/27 05:13:44  steve
 *  Support elaboration of disable statements.
 *
 * Revision 1.148  2000/07/22 22:09:04  steve
 *  Parse and elaborate timescale to scopes.
 *
 * Revision 1.147  2000/07/16 04:56:08  steve
 *  Handle some edge cases during node scans.
 *
 * Revision 1.146  2000/07/15 05:13:44  steve
 *  Detect muxing Vz as a bufufN.
 *
 * Revision 1.145  2000/07/14 06:12:57  steve
 *  Move inital value handling from NetNet to Nexus
 *  objects. This allows better propogation of inital
 *  values.
 *
 *  Clean up constant propagation  a bit to account
 *  for regs that are not really values.
 *
 * Revision 1.144  2000/07/07 04:53:54  steve
 *  Add support for non-constant delays in delay statements,
 *  Support evaluating ! in constant expressions, and
 *  move some code from netlist.cc to net_proc.cc.
 *
 * Revision 1.143  2000/06/25 19:59:42  steve
 *  Redesign Links to include the Nexus class that
 *  carries properties of the connected set of links.
 *
 * Revision 1.142  2000/06/24 22:55:20  steve
 *  Get rid of useless next_link method.
 *
 * Revision 1.141  2000/06/13 03:24:48  steve
 *  Index in memory assign should be a NetExpr.
 *
 * Revision 1.140  2000/05/31 02:26:49  steve
 *  Globally merge redundant event objects.
 *
 * Revision 1.139  2000/05/27 19:33:23  steve
 *  Merge similar probes within a module.
 *
 * Revision 1.138  2000/05/11 23:37:27  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.137  2000/05/07 18:20:07  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.136  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.135  2000/05/04 03:37:58  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.134  2000/05/02 03:13:31  steve
 *  Move memories to the NetScope object.
 *
 * Revision 1.133  2000/05/02 00:58:12  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.132  2000/04/28 18:43:23  steve
 *  integer division in expressions properly get width.
 *
 * Revision 1.131  2000/04/28 16:50:53  steve
 *  Catch memory word parameters to tasks.
 *
 * Revision 1.130  2000/04/23 21:17:31  steve
 *  Better comments about bufif devices.
 *
 * Revision 1.129  2000/04/23 21:15:07  steve
 *  Emit code for the bufif devices.
 *
 * Revision 1.128  2000/04/23 03:45:24  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.127  2000/04/22 04:20:19  steve
 *  Add support for force assignment.
 *
 * Revision 1.126  2000/04/20 00:28:03  steve
 *  Catch some simple identity compareoptimizations.
 *
 * Revision 1.125  2000/04/18 04:50:20  steve
 *  Clean up unneeded NetEvent objects.
 *
 * Revision 1.124  2000/04/18 01:02:54  steve
 *  Minor cleanup of NetTaskDef.
 *
 * Revision 1.123  2000/04/16 23:32:19  steve
 *  Synthesis of comparator in expressions.
 *
 *  Connect the NetEvent and related classes
 *  together better.
 *
 * Revision 1.122  2000/04/15 19:51:30  steve
 *  fork-join support in vvm.
 *
 * Revision 1.121  2000/04/12 20:02:53  steve
 *  Finally remove the NetNEvent and NetPEvent classes,
 *  Get synthesis working with the NetEvWait class,
 *  and get started supporting multiple events in a
 *  wait in vvm.
 *
 * Revision 1.120  2000/04/12 04:23:58  steve
 *  Named events really should be expressed with PEIdent
 *  objects in the pform,
 *
 *  Handle named events within the mix of net events
 *  and edges. As a unified lot they get caught together.
 *  wait statements are broken into more complex statements
 *  that include a conditional.
 *
 *  Do not generate NetPEvent or NetNEvent objects in
 *  elaboration. NetEvent, NetEvWait and NetEvProbe
 *  take over those functions in the netlist.
 *
 * Revision 1.119  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.118  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
 * Revision 1.117  2000/04/02 04:26:06  steve
 *  Remove the useless sref template.
 *
 * Revision 1.116  2000/04/01 21:40:22  steve
 *  Add support for integer division.
 *
 * Revision 1.115  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 * Revision 1.114  2000/03/12 17:09:41  steve
 *  Support localparam.
 *
 * Revision 1.113  2000/03/11 03:25:52  steve
 *  Locate scopes in statements.
 *
 * Revision 1.112  2000/03/10 06:20:48  steve
 *  Handle defparam to partial hierarchical names.
 *
 * Revision 1.111  2000/03/08 04:36:54  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.110  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.109  2000/02/13 04:35:43  steve
 *  Include some block matching from Larry.
 *
 * Revision 1.108  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.107  2000/01/10 01:35:24  steve
 *  Elaborate parameters afer binding of overrides.
 *
 * Revision 1.106  2000/01/01 06:18:00  steve
 *  Handle synthesis of concatenation.
 *
 * Revision 1.105  1999/12/30 04:19:12  steve
 *  Propogate constant 0 in low bits of adders.
 *
 * Revision 1.104  1999/12/17 06:18:16  steve
 *  Rewrite the cprop functor to use the functor_t interface.
 *
 * Revision 1.103  1999/12/17 03:38:46  steve
 *  NetConst can now hold wide constants.
 */
#endif
