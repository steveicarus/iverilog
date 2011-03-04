#ifndef __netlist_H
#define __netlist_H
/*
 * Copyright (c) 1998-2010 Stephen Williams (steve@icarus.com)
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

/*
 * The netlist types, as described in this header file, are intended
 * to be the output from elaboration of the source design. The design
 * can be passed around in this form to the various stages and design
 * processors.
 */
# include  <string>
# include  <map>
# include  <list>
# include  "verinum.h"
# include  "verireal.h"
# include  "StringHeap.h"
# include  "HName.h"
# include  "LineInfo.h"
# include  "svector.h"
# include  "Attrib.h"

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif

class Design;
class Link;
class Nexus;
class NetMemory;
class NetNode;
class NetProc;
class NetProcTop;
class NetRelease;
class NetScope;
class NetVariable;
class NetEvProbe;
class NetExpr;
class NetESignal;
class NetEVariable;
class NetFF;
class NetFuncDef;

class NetRamDq;
class NetEvTrig;
class NetEvWait;

struct target;
struct functor_t;

struct sync_accounting_cell {
      NetProc*proc;
      NetFF*ff;
      unsigned pin;
};

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
class NetObj  : public Attrib, public virtual LineInfo {

    public:
	// The name of the object must be a permallocated string. A
	// lex_strings string, for example.
      explicit NetObj(NetScope*s, perm_string n, unsigned npins);
      virtual ~NetObj();

      NetScope* scope();
      const NetScope* scope() const;

      perm_string name() const { return name_; }

      unsigned pin_count() const { return npins_; }

      unsigned rise_time() const { return delay1_; }
      unsigned fall_time() const { return delay2_; }
      unsigned decay_time() const { return delay3_; }

      void rise_time(unsigned d) { delay1_ = d; }
      void fall_time(unsigned d) { delay2_ = d; }
      void decay_time(unsigned d) { delay3_ = d; }

      Link&pin(unsigned idx);
      const Link&pin(unsigned idx) const;

      void dump_node_pins(ostream&, unsigned) const;
      void dump_obj_attr(ostream&, unsigned) const;

    private:
      NetScope*scope_;
      perm_string name_;
      Link*pins_;
      const unsigned npins_;
      unsigned delay1_;
      unsigned delay2_;
      unsigned delay3_;
};

class Link {

      friend void connect(Link&, Link&);
      friend void connect(Nexus*, Link&);
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
      void set_name(perm_string, unsigned inst =0);
      perm_string get_name() const;
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
	// has width, then the inst_ member is the index of the
	// pin.
      perm_string name_;
      unsigned    inst_;

    private:
      Link *next_, *prev_;
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
 *
 * The t_cookie() is a void* that targets can use to store information
 * in a Nexus. ivl guarantees that the t_cookie will be 0 when the
 * target is invoked.
 */
class Nexus {

      friend void connect(Link&, Link&);
      friend void connect(Nexus*, Link&);
      friend class Link;

    public:
      explicit Nexus();
      ~Nexus();

      const char* name() const;
      verinum::V get_init() const;

      Link*first_nlink();
      const Link* first_nlink()const;

	/* Return the number of drivers, or 0 if undriven. */
      int is_driven() const;

	/* This method returns true if all the possible drivers of
	   this nexus are constant. It will also return true if there
	   are no drivers at all. */
      bool drivers_constant() const;

	/* Given the nexus has constant drivers, this method returns
	   the value that has been driven. */
      verinum::V driven_value() const;

      void* t_cookie() const;
      void* t_cookie(void*) const;

    private:
      Link*list_;
      int list_len_;
      void unlink(Link*);
      void relink(Link*);

      mutable char* name_; /* Cache the calculated name for the Nexus. */
      mutable void* t_cookie_;

      enum VALUE { NO_GUESS, V0, V1, Vx, Vz, VAR };
      mutable VALUE driven_;

    private: // not implemented
      Nexus(const Nexus&);
      Nexus& operator= (const Nexus&);
};

class NexusSet {

    public:
      ~NexusSet();
      NexusSet();

      unsigned count() const;

	// Add the nexus to the set, if it is not already present.
      void add(Nexus*that);
      void add(const NexusSet&that);

	// Remove the nexus from the set, if it is present.
      void rem(Nexus*that);
      void rem(const NexusSet&that);

      Nexus* operator[] (unsigned idx) const;

	// Return true if this set contains every nexus in that set.
      bool contains(const NexusSet&that) const;

	// Return true if this set contains any nexus in that set.
      bool intersect(const NexusSet&that) const;

    private:
      Nexus**items_;
      unsigned*index_;
      unsigned nitems_;

      unsigned bsearch_(Nexus*that) const;

    private: // not implemented
      NexusSet(const NexusSet&);
      NexusSet& operator= (const NexusSet&);
};

/*
 * A NetNode is a device of some sort, where each pin has a different
 * meaning. (i.e., pin(0) is the output to an and gate.) NetNode
 * objects are listed in the nodes_ of the Design object.
 */
class NetNode  : public NetObj {

    public:
	// The name parameter must be a permallocated string.
      explicit NetNode(NetScope*s, perm_string n, unsigned npins);

      virtual ~NetNode();

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
 * name. A scalar wire is a NetNet with one pin, a vector a wider
 * NetNet. NetNet objects also appear as side effects of synthesis or
 * other abstractions.
 *
 * Note that INTEGER types are an alias for a ``reg signed [31:0]''.
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
class NetNet  : public NetObj {

    public:
      enum Type { NONE, IMPLICIT, IMPLICIT_REG, INTEGER, WIRE, TRI, TRI1,
		  SUPPLY0, SUPPLY1, WAND, TRIAND, TRI0, WOR, TRIOR, REG };

      enum PortType { NOT_A_PORT, PIMPLICIT, PINPUT, POUTPUT, PINOUT };

      explicit NetNet(NetScope*s, perm_string n, Type t, unsigned npins =1);

      explicit NetNet(NetScope*s, perm_string n, Type t, long ms, long ls);

      virtual ~NetNet();

      Type type() const;
      void type(Type t);

      PortType port_type() const;
      void port_type(PortType t);

	/* If a NetNet is signed, then its value is to be treated as
	   signed. Otherwise, it is unsigned. */
      bool get_signed() const;
      void set_signed(bool);

	/* Used to maintain original type of net since integers are
	   implemented as 'reg signed [31:0]' in Icarus */
      bool get_isint() const;
      void set_isint(bool);

	/* These methods return the msb and lsb indices for the most
	   significant and least significant bits. These are signed
	   longs, and may be different from pin numbers. For example,
	   reg [1:8] has 8 bits, msb==1 and lsb==8. */
      long msb() const;
      long lsb() const;

	/* This method converts a signed index (the type that might be
	   found in the verilog source) to a pin number. It accounts
	   for variation in the definition of the reg/wire/whatever. */
      unsigned sb_to_idx(long sb) const;

	/* This method checks that the signed index is valid for this
	   signal. If it is, the above sb_to_idx can be used to get
	   the pin# from the index. */
      bool sb_is_valid(long sb) const;

      bool local_flag() const { return local_flag_; }
      void local_flag(bool f) { local_flag_ = f; }

	/* NetESignal objects may reference this object. Keep a
	   reference count so that I keep track of them. */
      void incr_eref();
      void decr_eref();
      unsigned peek_eref() const;

	/* Assignment statements count their lrefs here. */
      void incr_lref();
      void decr_lref();
      unsigned peek_lref() const;

      unsigned get_refs() const;

	/* This may be and explode of a memory. */
      void mref(NetMemory*ref);
      NetMemory*mref();
      const NetMemory*mref() const;

      virtual void dump_net(ostream&, unsigned) const;

    private:
	// The NetScope class uses this for listing signals.
      friend class NetScope;
      NetNet*sig_next_, *sig_prev_;

	// Keep a list of release statements that reference me. I may
	// need to know this in order to fix them up when I am
	// deleted.
      friend class NetRelease;
      NetRelease*release_list_;

    private:
      Type   type_;
      PortType port_type_;
      bool signed_;
      bool isint_;		// original type of integer

      long msb_, lsb_;

      bool local_flag_;
      unsigned eref_count_;
      unsigned lref_count_;
      NetMemory*mref_;
};

/*
 * This class implements the LPM_ADD_SUB component as described in the
 * EDIF LPM Version 2 1 0 standard. It is used as a structural
 * implementation of the + and - operators.
 */
class NetAddSub  : public NetNode {

    public:
      NetAddSub(NetScope*s, perm_string n, unsigned width);
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
      NetCLShift(NetScope*s, perm_string n, unsigned width,
		 unsigned width_dist, bool right_flag, bool signed_flag);
      ~NetCLShift();

      unsigned width() const;
      unsigned width_dist() const;

      bool right_flag() const;
      bool signed_flag() const;

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
      bool right_flag_;
      bool signed_flag_;
};

/*
 * This class supports the LPM_COMPARE device.
 *
 * The width of the device is the width of the inputs. If one of the
 * inputs is narrower than the other, it is up to the generator to
 * make sure all the data pins are properly driven.
 *
 * NOTE: This is not the same as the device used to support case
 * compare. Case comparisons handle Vx and Vz values, whereas this
 * device need not.
 */
class NetCompare  : public NetNode {

    public:
      NetCompare(NetScope*scope, perm_string n, unsigned width);
      ~NetCompare();

      unsigned width() const;

      bool get_signed() const;
      void set_signed(bool);

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
      bool signed_flag_;
};

/*
 * A decoder takes an address input and activates (high) the single
 * Q bit that is addressed. This can be used, for example, to
 * generate an enable for a FF from an array of FFs.
 */
class NetDecode  : public NetNode {

    public:
      NetDecode(NetScope*s, perm_string name, NetFF*mem,
		unsigned awid, unsigned word_width);
      ~NetDecode();

	// This is the width of the word. The width of the NetFF mem
	// is an even multiple of this.
      unsigned width() const;
	// This is the width of the address. The address value for the
	// base of a word is the address * width().
      unsigned awidth() const;

      const NetFF*ff() const;

      Link& pin_Address(unsigned idx);

      const Link& pin_Address(unsigned idx) const;
      const Link& pin_Q(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
      NetFF* ff_;

    private:
      void make_pins_(unsigned awid);
};

/*
 * The NetDemux is similar to the NetDecode, except that it is
 * combinational. The inputs are an address, Data, and WriteData.
 * The Q output is the same as the Data input, except for the bit that
 * is addressed by the address input, which gets WriteData instead.
 */
class NetDemux  : public NetNode {

    public:
      NetDemux(NetScope*s, perm_string name,
	       unsigned bus_width, unsigned address_width,
	       unsigned size);
      ~NetDemux();

	// This is the width of the bus that passes through the
	// device. The address addresses into this width.
      unsigned width() const;
	// This is the width of the address. The address value for the
	// base of a word is the address * width().
      unsigned awidth() const;
	// This is the number of words in the width that can be
	// addressed. This implies (by division) the width of a word.
      unsigned size() const;

      Link& pin_Address(unsigned idx);
      Link& pin_Data(unsigned idx);
      Link& pin_Q(unsigned idx);
      Link& pin_WriteData(unsigned idx);

      const Link& pin_Address(unsigned idx) const;
      const Link& pin_Data(unsigned idx) const;
      const Link& pin_Q(unsigned idx) const;
      const Link& pin_WriteData(unsigned idx) const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_, awidth_, size_;

    private:
      void make_pins_(unsigned wid, unsigned awid);
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
      NetDivide(NetScope*scope, perm_string n,
		unsigned width, unsigned wa, unsigned wb);
      ~NetDivide();

      unsigned width_r() const;
      unsigned width_a() const;
      unsigned width_b() const;

      void set_signed(bool);
      bool get_signed() const;

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

      bool signed_flag_;
};

/*
 * This class represents a theoretical (though not necessarily
 * practical) integer modulo gate. This is not to represent any real
 * hardware, but to support the % operator in Verilog, when it shows
 * up in structural contexts.
 *
 * The operands of the operation are the DataA<i> and DataB<i> inputs,
 * and the Result<i> output reflects the value DataA%DataB.
 */

class NetModulo  : public NetNode {

    public:
      NetModulo(NetScope*s, perm_string n,
		unsigned width, unsigned wa, unsigned wb);
      ~NetModulo();

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
      NetFF(NetScope*s, perm_string n, unsigned width);
      ~NetFF();

      unsigned width() const;

      Link& pin_Clock();
      Link& pin_Enable();
      Link& pin_Aset();
      Link& pin_Aclr();
      Link& pin_Sset();
      Link& pin_Sclr();
      Link& pin_Data(unsigned);
      Link& pin_Q(unsigned);

      const Link& pin_Clock() const;
      const Link& pin_Enable() const;
      const Link& pin_Aset() const;
      const Link& pin_Aclr() const;
      const Link& pin_Sset() const;
      const Link& pin_Sclr() const;
      const Link& pin_Data(unsigned) const;
      const Link& pin_Q(unsigned) const;

      void aset_value(const verinum&val);
      const verinum& aset_value() const;

      void sset_value(const verinum&val);
      const verinum& sset_value() const;

      NetDecode* get_demux();
      const NetDecode* get_demux() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
	// If there is a demux associated with this gate, the demux_
	// member will point to the decoder.
      friend class NetDecode;
      NetDecode*demux_;

    private:
      verinum aset_value_;
      verinum sset_value_;
};

/*
 * This class represents an LPM_LATCH device. There is no literal gate
 * type in Verilog that maps, but gates of this type can be inferred.
 */
class NetLatch  : public NetNode {

    public:
      NetLatch(NetScope*s, perm_string n, unsigned width);
      ~NetLatch();

      unsigned width() const;

      Link& pin_Clock();
      Link& pin_Aset();
      Link& pin_Aclr();
      Link& pin_Data(unsigned);
      Link& pin_Q(unsigned);

      const Link& pin_Aset() const;
      const Link& pin_Aclr() const;
      const Link& pin_Clock() const;
      const Link& pin_Data(unsigned) const;
      const Link& pin_Q(unsigned) const;

      void aset_value(const verinum&val);
      const verinum& aset_value() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      verinum aset_value_;
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
      NetMemory(NetScope*sc, perm_string n, long w, long s, long e);
      ~NetMemory();

	// This is the BASE name of the memory object. It does not
	// include scope name, get that from the scope itself.
      perm_string name() const;

	// This is the width (in bits) of a single memory position.
      unsigned width() const { return width_; }

      // NetScope*scope();
      const NetScope*scope() const { return scope_; };
       NetScope*scope() { return scope_; };

	// This is the number of memory positions.
      unsigned count() const;

	// This method returns a 0 based address of a memory entry as
	// indexed by idx. The Verilog source may give index ranges
	// that are not zero based.
      unsigned index_to_address(long idx) const;

	// This method returns a NetNet::REG that has the same number
	// of bits as the memory as a whole. This is used to represent
	// memories that are synthesized to individual bits.
      NetNet* explode_to_reg();
      NetNet* reg_from_explode();
      const NetNet* reg_from_explode() const;

      void dump(ostream&o, unsigned lm) const;

    private:
      perm_string name_;
      unsigned width_;
      long idxh_;
      long idxl_;

      friend class NetRamDq;
      class NetRamDq* ram_list_;

      friend class NetScope;
      NetMemory*snext_, *sprev_;
      NetScope*scope_;

      NetNet*explode_;

    private: // not implemented
      NetMemory(const NetMemory&);
      NetMemory& operator= (const NetMemory&);
};

/*
 * This class implements the LPM_MULT component as described in the
 * EDIF LPM Version 2 1 0 standard. It is used as a structural
 * implementation of the * operator. The device has inputs DataA and
 * DataB that can have independent widths, as can the result. If the
 * result is smaller than the widths of a and b together, then the
 * device drops the least significant bits of the product.
 */
class NetMult  : public NetNode {

    public:
      NetMult(NetScope*sc, perm_string n, unsigned width,
	      unsigned wa, unsigned wb, unsigned width_s =0);
      ~NetMult();

      bool get_signed() const;
      void set_signed(bool);

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
      bool signed_;
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
      NetMux(NetScope*scope, perm_string n,
	     unsigned width, unsigned size, unsigned selw);
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
      NetRamDq(NetScope*s, perm_string name, NetMemory*mem, unsigned awid);
      ~NetRamDq();

      unsigned width() const;
      unsigned awidth() const;
      unsigned size() const;

      NetMemory*mem();
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

      void functor_node(Design*des, functor_t*fun);

    private:
      NetMemory*mem_;
      NetRamDq*next_;
      unsigned awidth_;

    private:
      void make_pins_(unsigned wid);
};

/*
 * This node represents the call of a user defined function in a
 * structural context.
 */
class NetUserFunc  : public NetNode {

    public:
      NetUserFunc(NetScope*s, perm_string n, NetScope*def);
      ~NetUserFunc();

      unsigned port_count() const;
      unsigned port_width(unsigned port) const;
      Link& port_pin(unsigned port, unsigned idx);

      const NetScope* def() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      NetScope*def_;
};

/* =========
 * There are cases where expressions need to be represented. The
 * NetExpr class is the root of a hierarchy that serves that purpose.
 *
 * The expr_width() is the width of the expression, that accounts
 * for the widths of the sub-expressions I might have. It is up to the
 * derived classes to properly set the expr width, if need be. The
 * set_width() method is used to compel an expression to have a
 * certain width, and is used particularly when the expression is an
 * rvalue in an assignment statement.
 */
class NetExpr  : public LineInfo {
    public:
      explicit NetExpr(unsigned w =0);
      virtual ~NetExpr() =0;

      virtual void expr_scan(struct expr_scan_t*) const =0;
      virtual void dump(ostream&) const;

	// Expressions have type. The most common type is ET_VECTOR,
	// which is a vector (possibly 1 bit) of 4-value bits. The
	// ET_VOID is not generally used.
	//
	// ET_VOID     - No value at all.
	// ET_VECTOR   - Vector of Verilog 4-value bits
	// ET_REAL     - real/realtime expression
      enum TYPE { ET_VOID=0, ET_VECTOR, ET_REAL };
      virtual TYPE expr_type() const;

	// How wide am I?
      unsigned expr_width() const { return width_; }

	// Coerce the expression to have a specific width. If the
	// coercion works, then return true. Otherwise, return false.
      virtual bool set_width(unsigned);

	// This method returns true if the expression is
	// signed. Unsigned expressions return false.
      bool has_sign() const;
      void cast_signed(bool flag);

	// This returns true if the expression has a definite
	// width. This is generally true, but in some cases the
	// expression is amorphous and desires a width from its
	// environment. For example, 'd5 has indefinite width, but
	// 5'd5 has a definite width.

	// This method is only really used within concatenation
	// expressions to check validity.
      virtual bool has_width() const;


	// This method evaluates the expression and returns an
	// equivalent expression that is reduced as far as compile
	// time knows how. Essentially, this is designed to fold
	// constants.
      virtual NetExpr*eval_tree();

	// Make a duplicate of myself, and subexpressions if I have
	// any. This is a deep copy operation.
      virtual NetExpr*dup_expr() const =0;

	// Get the Nexus that are the input to this
	// expression. Normally this descends down to the reference to
	// a signal that reads from its input.
      virtual NexusSet* nex_input(bool rem_out = true) =0;

	// Return a version of myself that is structural. This is used
	// for converting expressions to gates.
      virtual NetNet*synthesize(Design*);


    protected:
      void expr_width(unsigned w) { width_ = w; }

    private:
      unsigned width_;
      bool signed_flag_;

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

      const verinum&value() const;

      virtual bool set_width(unsigned w);

      virtual bool has_width() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetEConst* dup_expr() const;
      virtual NetNet*synthesize(Design*);
      virtual NexusSet* nex_input(bool rem_out = true);

    private:
      verinum value_;
};

class NetEConstParam  : public NetEConst {

    public:
      explicit NetEConstParam(NetScope*scope, perm_string name,
			      const verinum&val);
      ~NetEConstParam();

      perm_string name() const;
      const NetScope*scope() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetEConstParam* dup_expr() const;

    private:
      NetScope*scope_;
      perm_string name_;
};

/*
 * This class represents a constant real value.
 */
class NetECReal  : public NetExpr {

    public:
      explicit NetECReal(const verireal&val);
      ~NetECReal();

      const verireal&value() const;

	// Reals can be used in vector expressions. Conversions will
	// be done at the right time.
      virtual bool set_width(unsigned w);

	// This type has no self-determined width. This is false.
      virtual bool has_width() const;

	// The type of this expression is ET_REAL
      TYPE expr_type() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetECReal* dup_expr() const;
      virtual NetNet*synthesize(Design*);
      virtual NexusSet* nex_input(bool rem_out = true);

    private:
      verireal value_;
};

class NetECRealParam  : public NetECReal {

    public:
      explicit NetECRealParam(NetScope*scope, perm_string name,
			      const verireal&val);
      ~NetECRealParam();

      perm_string name() const;
      const NetScope*scope() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetECRealParam* dup_expr() const;

    private:
      NetScope*scope_;
      perm_string name_;
};

/*
 * This is a special, magical NetNet object. It represents a constant
 * bit or part select of another NetNet, so is used to return that
 * selection from elaborate function. None of these should remain once
 * the elaboration is complete.
 */
class NetSubnet  : public NetNet {

    public:
      explicit NetSubnet(NetNet*sig, unsigned off, unsigned wid);
      virtual void dump_net(ostream&, unsigned) const;
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
      explicit NetBUFZ(NetScope*s, perm_string n);
      ~NetBUFZ();

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
};

/*
 * This node is used to represent case equality in combinational
 * logic. Although this is not normally synthesizable, it makes sense
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
      explicit NetCaseCmp(NetScope*s, perm_string n);
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
      explicit NetConst(NetScope*s, perm_string n, verinum::V v);
      explicit NetConst(NetScope*s, perm_string n, const verinum&val);
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
 *
 * The pullup and pulldown gates have no inputs at all, and pin0 is
 * the output 1 or 0, depending on the gate type. It is the strength
 * of that value that is important.
 */
class NetLogic  : public NetNode {

    public:
      enum TYPE { AND, BUF, BUFIF0, BUFIF1, NAND, NMOS, NOR, NOT,
		  NOTIF0, NOTIF1, OR, PULLDOWN, PULLUP, RNMOS, RPMOS,
		  PMOS, XNOR, XOR };

      explicit NetLogic(NetScope*s, perm_string n, unsigned pins, TYPE t);

      TYPE type() const { return type_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);

    private:
      const TYPE type_;
};

/*
 * The UDP is a User Defined Primitive from the Verilog source. Do not
 * expand it out any further than this in the netlist, as this can be
 * used to represent target device primitives.
 *
 * The UDP can be combinational or sequential. The sequential UDP
 * includes the current output in the truth table, and supports edges,
 * whereas the combinational does not and is entirely level sensitive.
 * In any case, pin 0 is an output, and all the remaining pins are
 * inputs.
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
 * than one item.
 *
 *      p   -- 01, 0x or x1 // check this with the lexer
 *      n   -- 10, 1x or x0 // check this with the lexer
 *      ?   -- 0, 1, or x
 *      *   -- any edge
 *      +   -- 01 or x1
 *      _   -- 10 or x0  (Note that this is not the output '-'.)
 *      %   -- 0x or 1x
 *
 * SEQUENTIAL
 * These objects have a single bit of memory. The logic table includes
 * an entry for the current value, and allows edges on the inputs. In
 * canonical form, only the entries that generate 0, 1 or - (no change)
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
#include "PUdp.h"

class NetUDP  : public NetNode {

    public:
      explicit NetUDP(NetScope*s, perm_string n, unsigned pins, PUdp*u);

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;

	/* Use these methods to scan the truth table of the
	   device. "first" returns the first item in the table, and
	   "next" returns the next item in the table. The method will
	   return false when the scan is done. */
      bool first(string&inp, char&out) const;
      bool next(string&inp, char&out) const;
      unsigned rows() const { return udp->tinput.count(); }

      unsigned nin() const { return pin_count()-1; }
      bool is_sequential() const { return udp->sequential; }
      perm_string udp_name() const { return udp->name_; }
      char get_initial() const;

    private:
      mutable unsigned table_idx;
      PUdp *udp;
};


/* =========
 * A process is a behavioral-model description. A process is a
 * statement that may be compound. the various statement types may
 * refer to places in a netlist (by pointing to nodes) but is not
 * linked into the netlist. However, elaborating a process may cause
 * special nodes to be created to handle things like events.
 */
class NetProc : public virtual LineInfo, public Attrib {

    public:
      explicit NetProc();
      virtual ~NetProc();

	// Find the Nexa that are input by the statement. This is used
	// for example by @* to find the inputs to the process for the
	// sensitivity list.
      virtual NexusSet* nex_input(bool rem_out = true);

	// Find the nexa that are set by the statement. Add the output
	// values to the set passed as a parameter.
      virtual void nex_output(NexusSet&);

	// This method is called to emit the statement to the
	// target. The target returns true if OK, false for errors.
      virtual bool emit_proc(struct target_t*) const;

	// This method is called by functors that want to scan a
	// process in search of matchable patterns.
      virtual int match_proc(struct proc_match_t*);

	// Return true if this represents the root of a combinational
	// process. Most process types are not.
      virtual bool is_asynchronous();

	// Return true if this represents the root of a synchronous
	// process. Most process types are not.
      virtual bool is_synchronous();

	// synthesize as asynchronous logic, and return true. The
	// sync_flag is used to tell the async synthesizer that the
	// output nex_map is ultimately connected to a DFF Q
	// output. This can affect how cycles are handled.

      bool synth_async_noaccum(Design*des, NetScope*scope, bool sync_flag,
			       struct sync_accounting_cell*nex_ff,
			       NetNet*nex_map, NetNet*nex_out);

      virtual bool synth_async(Design*des, NetScope*scope, bool sync_flag,
                               struct sync_accounting_cell*nex_ff,
                               NetNet*nex_map, NetNet*nex_out, NetNet*accum_in,
                               bool&latch_inferred, NetNet *gsig = 0);

	// Synthesize synchronous logic, and return true. The nex_out
	// is where outputs are actually connected, and the nex_map
	// maps nexa to bit positions. The ff is the initial DFF that
	// was created to receive the Data inputs. The method *may*
	// delete that DFF in favor of multiple smaller devices, but
	// in that case it will set the ff argument to nil.
      virtual bool synth_sync(Design*des, NetScope*scope,
			      struct sync_accounting_cell*nex_ff,
			      NetNet*nex_map, NetNet*nex_out,
			      const svector<NetEvProbe*>&events);

      virtual void dump(ostream&, unsigned ind) const;
      void dump_proc_attr(ostream&, unsigned ind) const;

    private:
      friend class NetBlock;
      NetProc*next_;

    private: // not implemented
      NetProc(const NetProc&);
      NetProc& operator= (const NetProc&);
};

/*
 * Procedural assignment is broken into a suite of classes. These
 * classes represent the various aspects of the assignment statement
 * in behavioral code. (The continuous assignment is *not*
 * represented here.)
 *
 * The NetAssignBase carries the common aspects of an assignment,
 * including the r-value. This class has no cares of blocking vs
 * non-blocking, however it carries nearly all the other properties
 * of the assignment statement. It is abstract because it does not
 * differentiate the virtual behaviors.
 *
 * The NetAssign and NetAssignNB classes are the concrete classes that
 * give the assignment its final, precise meaning. These classes fill
 * in the NetProc behaviors.
 *
 * The l-value of the assignment is a collection of NetAssign_
 * objects that are connected to the structural netlist where the
 * assignment has its effect. The NetAssign_ class is not to be
 * derived from.
 *
 * The collection is arranged from lsb up to msb, and represents the
 * concatenation of l-values. The elaborator may collapse some
 * concatenations into a single NetAssign_. The "more" member of the
 * NetAssign_ object points to the next most significant bits of l-value.
 *
 * NOTE: The elaborator will make an effort to match the width of the
 * r-value to the width of the l-value, but targets and functions
 * should know that this is not a guarantee.
 */

class NetAssign_ {

    public:
      NetAssign_(NetNet*sig);
      NetAssign_(NetMemory*mem);
      NetAssign_(NetVariable*var);
      ~NetAssign_();

	// If this expression exists, then only a single bit is to be
	// set from the rval, and the value of this expression selects
	// the pin that gets the value.
      NetExpr*bmux();
      const NetExpr*bmux() const;

      unsigned get_loff() const;

      void set_bmux(NetExpr*);
      void set_part(unsigned loff, unsigned wid);

	// Get the width of the r-value that this node expects. This
	// method accounts for the presence of the mux, so it not
	// necessarily the same as the pin_count().
      unsigned lwidth() const;

	// Get the name of the underlying object.
      perm_string name() const;

      NetNet* sig() const;
      NetMemory*mem() const;
      NetVariable*var() const;

	// Mark that the synthesizer has worked with this l-value, so
	// when it is released, the l-value signal should be turned
	// into a wire.
      void turn_sig_to_wire_on_release();

      void incr_mem_lref();

	// It is possible that l-values can have *inputs*, as well as
	// being outputs. For example foo[idx] = ... is the l-value
	// (NetAssign_ object) with a foo l-value and the input
	// expression idx.
      NexusSet* nex_input(bool rem_out = true);

	// This pointer is for keeping simple lists.
      NetAssign_* more;

      void dump_lval(ostream&o) const;

    private:
      NetNet *sig_;
      NetMemory*mem_;
      NetVariable*var_;
      NetExpr*bmux_;

      bool turn_sig_to_wire_on_release_;
      unsigned loff_;
      unsigned lwid_;
      bool mem_lref_;
};

class NetAssignBase : public NetProc {

    public:
      NetAssignBase(NetAssign_*lv, NetExpr*rv);
      virtual ~NetAssignBase() =0;

	// This is the (procedural) value that is to be assigned when
	// the assignment is executed.
      NetExpr*rval();
      const NetExpr*rval() const;

      void set_rval(NetExpr*);

      NetAssign_* l_val(unsigned);
      const NetAssign_* l_val(unsigned) const;
      unsigned l_val_count() const;

      void set_delay(NetExpr*);
      const NetExpr* get_delay() const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&o);


	// This returns the total width of the accumulated l-value. It
	// accounts for any grouping of NetAssign_ objects that might happen.
      unsigned lwidth() const;

      virtual bool synth_async(Design*des, NetScope*scope, bool sync_flag,
                               struct sync_accounting_cell*nex_ff,
                               NetNet*nex_map, NetNet*nex_out, NetNet*accum_in,
                               bool&latch_inferred, NetNet *gsig = 0);
      bool synth_sync(Design*des, NetScope*scope,
		      struct sync_accounting_cell*nex_ff,
		      NetNet*nex_map, NetNet*nex_out,
		      const svector<NetEvProbe*>&events);

	// This dumps all the lval structures.
      void dump_lval(ostream&) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      bool synth_async_mem_sync_(Design*des, NetScope*scope,
				 NetAssign_*cur, NetNet*rsig, unsigned&roff,
				 NetNet*nex_map, NetNet*nex_out);

    private:
      NetAssign_*lval_;
      NetExpr   *rval_;
      NetExpr   *delay_;
};

class NetAssign : public NetAssignBase {

    public:
      explicit NetAssign(NetAssign_*lv, NetExpr*rv);
      ~NetAssign();

      bool is_asynchronous();

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
};

class NetAssignNB  : public NetAssignBase {
    public:
      explicit NetAssignNB(NetAssign_*lv, NetExpr*rv);
      ~NetAssignNB();


      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
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

      NetBlock(Type t, NetScope*subscope);
      ~NetBlock();

      Type type() const    { return type_; }
      NetScope* subscope() const { return subscope_; }

      void append(NetProc*);

      const NetProc*proc_first() const;
      const NetProc*proc_next(const NetProc*cur) const;


	// synthesize as asynchronous logic, and return true.
      virtual bool synth_async(Design*des, NetScope*scope, bool sync_flag,
                               struct sync_accounting_cell*nex_ff,
                               NetNet*nex_map, NetNet*nex_out, NetNet*accum_in,
                               bool&latch_inferred, NetNet *gsig = 0);

      bool synth_sync(Design*des, NetScope*scope,
		      struct sync_accounting_cell*nex_ff,
		      NetNet*nex_map, NetNet*nex_out,
		      const svector<NetEvProbe*>&events);

	// This version of emit_recurse scans all the statements of
	// the begin-end block sequentially. It is typically of use
	// for sequential blocks.
      void emit_recurse(struct target_t*) const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
      const Type type_;
      NetScope*subscope_;

      NetProc*last_;
};

/*
 * A CASE statement in the verilog source leads, eventually, to one of
 * these. This is different from a simple conditional because of the
 * way the comparisons are performed. Also, it is likely that the
 * target may be able to optimize differently.
 *
 * Case can be one of three types:
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

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&out);

      virtual bool synth_async(Design*des, NetScope*scope, bool sync_flag,
                               struct sync_accounting_cell*nex_ff,
                               NetNet*nex_map, NetNet*nex_out, NetNet*accum_in,
                               bool&latch_inferred, NetNet *gsig = 0);

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      bool synth_async_1hot_(Design*des, NetScope*scope, bool sync_flag,
                             struct sync_accounting_cell*nex_ff,
                             NetNet*nex_map, NetNet*nex_out, NetNet*accum_in,
                             NetNet*esig, unsigned hot_items,
                             bool&latch_inferred, NetNet *gsig = 0);
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
      explicit NetCAssign(NetScope*s, perm_string n, NetNet*l);
      ~NetCAssign();

      const Link& lval_pin(unsigned) const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void dump(ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

      const NetNet*lval() const;

    private:
      NetNet*lval_;

    private: // not implemented
      NetCAssign(const NetCAssign&);
      NetCAssign& operator= (const NetCAssign&);
};


/*
 * A condit represents a conditional. It has an expression to test,
 * and a pair of statements to select from. If the original statement
 * has empty clauses, then the NetProc for it will be a null pointer.
 */
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

      bool emit_recurse_if(struct target_t*) const;
      bool emit_recurse_else(struct target_t*) const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&o);

      bool is_asynchronous();
      virtual bool synth_async(Design*des, NetScope*scope, bool sync_flag,
                               struct sync_accounting_cell*nex_ff,
                               NetNet*nex_map, NetNet*nex_out, NetNet*accum,
                               bool&latch_inferred, NetNet *gsig = 0);

      bool synth_sync(Design*des, NetScope*scope,
		      struct sync_accounting_cell*nex_ff,
		      NetNet*nex_map, NetNet*nex_out,
		      const svector<NetEvProbe*>&events);

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

    private:
      int connect_set_clr_range_( struct sync_accounting_cell*nex_ff,
				  unsigned bits, NetNet*rst,
				  const verinum&val);
      int connect_enable_range_(Design*des, NetScope*scope,
				struct sync_accounting_cell*nex_ff,
				unsigned bits, NetNet*ce);
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
 * the event trigger statements. The NetEvent class may have a name and
 * a scope. The name is a simple name (no hierarchy) and the scope is
 * the NetScope that contains the object. The scope member is written
 * by the NetScope object when the NetEvent is stored.
 *
 * The NetEvWait class represents a thread wait for an event. When
 * this statement is executed, it starts waiting on the
 * event. Conceptually, it puts itself on the event list for the
 * referenced event. When the event is triggered, the wait ends its
 * block and starts the associated statement.
 *
 * The NetEvTrig class represents trigger statements. Executing this
 * statement causes the referenced event to be triggered, which it
 * turn awakens the waiting threads. Each NetEvTrig object references
 * exactly one event object.
 *
 * The NetEvProbe class is the structural equivalent of the NetEvTrig,
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
      friend class NetEEvent;

    public:
	// The name of the event is the basename, and should not
	// include the scope. Also, the name passed here should be
	// perm-allocated.
      explicit NetEvent (perm_string n);
      ~NetEvent();

      perm_string name() const;
      string full_name() const;

	// Get information about probes connected to me.
      unsigned nprobe() const;
      NetEvProbe* probe(unsigned);
      const NetEvProbe* probe(unsigned) const;

	// Return the number of NetEvWait nodes that reference me.
      unsigned nwait() const;
      unsigned ntrig() const;
      unsigned nexpr() const;

      NetScope* scope();
      const NetScope* scope() const;

      void nex_output(NexusSet&);

	// Locate the first event that matches my behavior and
	// monitors the same signals.
      void find_similar_event(list<NetEvent*>&);

	// This method replaces pointers to me with pointers to
	// that. It is typically used to replace similar events
	// located by the find_similar_event method.
      void replace_event(NetEvent*that);

    private:
	// This returns a nexus set if it represents possibly
	// asynchronous inputs, otherwise 0.
      NexusSet*nex_async_();

    private:
      perm_string name_;

	// The NetScope class uses these to list the events.
      NetScope*scope_;
      NetEvent*snext_;

	// Use these methods to list the probes attached to me.
      NetEvProbe*probes_;

	// Use these methods to list the triggers attached to me.
      class NetEvTrig* trig_;

	// Use This member to count references by NetEvWait objects.
      unsigned waitref_;
      struct wcell_ {
	    class NetEvWait*obj;
	    struct wcell_*next;
      };
      struct wcell_ *wlist_;

	// expression references, ala. task/funcs
      unsigned exprref_;

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

	// It is possible that this is the root of a combinational
	// process. This method checks.
      virtual bool is_asynchronous();

	// It is possible that this is the root of a synchronous
	// process? This method checks.
      virtual bool is_synchronous();

      virtual void nex_output(NexusSet&out);

      virtual bool synth_async(Design*des, NetScope*scope, bool sync_flag,
                               struct sync_accounting_cell*nex_ff,
                               NetNet*nex_map, NetNet*nex_out, NetNet*accum_in,
                               bool&latch_inferred, NetNet *gsig = 0);

      virtual bool synth_sync(Design*des, NetScope*scope,
			      struct sync_accounting_cell*nex_ff,
			      NetNet*nex_map, NetNet*nex_out,
			      const svector<NetEvProbe*>&events);

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

      explicit NetEvProbe(NetScope*s, perm_string n,
			  NetEvent*tgt, edge_t t, unsigned p);
      ~NetEvProbe();

      edge_t edge() const;
      NetEvent* event();
      const NetEvent* event() const;

      void find_similar_probes(list<NetEvProbe*>&);

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
      explicit NetForce(NetScope*s, perm_string n, NetNet*l);
      ~NetForce();

      const Link& lval_pin(unsigned) const;

      const NetNet*lval() const;

      virtual NexusSet* nex_input(bool rem_out = true);

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

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetProc*statement_;
};

/*
 * A function definition is elaborated just like a task, though by now
 * it is certain that the first parameter (a phantom parameter) is the
 * output and all the remaining parameters are the inputs. This makes
 * for easy code generation in targets that support behavioral
 * descriptions.
 *
 * The NetNet array that is passed in as a parameter is the set of
 * signals that make up its parameter list. These are all internal to
 * the scope of the function.
 */
class NetFuncDef {

    public:
      NetFuncDef(NetScope*, NetNet*result, const svector<NetNet*>&po);
      NetFuncDef(NetScope*, NetVariable*result, const svector<NetNet*>&po);
      ~NetFuncDef();

      void set_proc(NetProc*st);

      const string name() const;
      const NetProc*proc() const;
      NetScope*scope();

      unsigned port_count() const;
      const NetNet*port(unsigned idx) const;

      const NetNet*return_sig() const;
      const NetVariable*return_var() const;

      NetNet* synthesize(Design*des, const svector<NetNet*>&inports_);

      void dump(ostream&, unsigned ind) const;

    private:
      NetScope*scope_;
      NetProc*statement_;
      NetNet*result_sig_;
      NetVariable*result_var_;
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
 * object with a NetExpr* instead of the d value, and use the expr()
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

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&);

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

      virtual NexusSet* nex_input(bool rem_out = true);
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
	// Used to manage list within NetNet objects.
      friend class NetNet;
      NetRelease*release_next_;
};


/*
 * The NetSTask class is a call to a system task. These kinds of tasks
 * are generally handled very simply in the target. They certainly are
 * handled differently from user defined tasks because ivl knows all
 * about the user defined tasks.
 */
class NetSTask  : public NetProc {

    public:
      NetSTask(const char*na, const svector<NetExpr*>&);
      ~NetSTask();

      const char* name() const;

      unsigned nparms() const;

      const NetExpr* parm(unsigned idx) const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      const char* name_;
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
 * parameters, then invoking the thread, and finally assigning out the
 * output and inout variables. The variables accessible as ports are
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
 * Variable object such as real and realtime are represented by
 * instances of this class.
 */
class NetVariable : public LineInfo {

      friend class NetScope;

    public:
	// The name must be a permallocated string. This class makes
	// no attempt to preserve it.
      NetVariable(perm_string name);
      ~NetVariable();

      perm_string basename() const;

      NetScope* scope();
      const NetScope* scope() const;

    private:
      perm_string name_;

      NetScope*scope_;
      NetVariable*snext_;

    private:
      NetVariable(const NetVariable&);
      NetVariable& operator= (const NetVariable&);
};

/*
 * This node represents a function call in an expression. The object
 * contains a pointer to the function definition, which is used to
 * locate the value register and input expressions.
 */
class NetEUFunc  : public NetExpr {

    public:
      NetEUFunc(NetScope*, NetESignal*,   svector<NetExpr*>&);
      NetEUFunc(NetScope*, NetEVariable*, svector<NetExpr*>&);
      ~NetEUFunc();

      const string name() const;

      const NetESignal*result_sig() const;
      const NetEVariable*result_var() const;

      unsigned parm_count() const;
      const NetExpr* parm(unsigned idx) const;

      const NetScope* func() const;

      virtual bool set_width(unsigned);
      virtual TYPE expr_type() const;
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEUFunc*dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true);

      virtual NetNet* synthesize(Design*);

    private:
      NetScope*func_;
      NetESignal*result_sig_;
      NetEVariable*result_var_;
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
      NetUTask(NetScope*);
      ~NetUTask();

      const string name() const;

      const NetScope* task() const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetScope*task_;
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

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

      virtual bool synth_async(Design*des, NetScope*scope, bool sync_flag,
                               struct sync_accounting_cell*nex_ff,
                               NetNet*nex_map, NetNet*nex_out, NetNet*accum_in,
                               bool&latch_inferred, NetNet *gsig = 0);

    private:
      NetExpr* cond_;
      NetProc*proc_;
};


/*
 * The is the top of any process. It carries the type (initial or
 * always) and a pointer to the statement, probably a block, that
 * makes up the process.
 */
class NetProcTop  : public LineInfo, public Attrib {

    public:
      enum Type { KINITIAL, KALWAYS };

      NetProcTop(NetScope*s, Type t, class NetProc*st);
      ~NetProcTop();

      Type type() const { return type_; }
      NetProc*statement();
      const NetProc*statement() const;

      NetScope*scope();
      const NetScope*scope() const;

	/* Return true if this process represents combinational logic. */
      bool is_asynchronous() const;

	/* Create asynchronous logic from this thread and return true,
	   or return false if that cannot be done. */
      bool synth_async(Design*des);

	/* Return true if this process represents synchronous logic. */
      bool is_synchronous() const;

	/* Create synchronous logic from this thread and return true,
	   or return false if that cannot be done. */
      bool synth_sync(Design*des);

      void dump(ostream&, unsigned ind) const;
      bool emit(struct target_t*tgt) const;

    private:
      const Type type_;
      NetProc*const statement_;

      NetScope*scope_;
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
 *   <  -- Less than
 *   >  -- Greater than
 *   e  -- Logical equality (==)
 *   E  -- Case equality (===)
 *   L  -- Less or equal
 *   G  -- Greater or equal
 *   n  -- Logical inequality (!=)
 *   N  -- Case inequality (!==)
 *   a  -- Logical AND (&&)
 *   A  -- Bitwise NAND (~&)
 *   o  -- Logical OR (||)
 *   O  -- Bit-wise NOR (~|)
 *   l  -- Left shift (<<)
 *   r  -- Right shift (>>)
 *   R  -- signed right shift (>>>)
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

	// A binary expression node only has a definite
	// self-determinable width if the operands both have definite
	// widths.
      virtual bool has_width() const;

      virtual NetEBinary* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    protected:
      char op_;
      NetExpr* left_;
      NetExpr* right_;

      void eval_sub_tree_();
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

      virtual TYPE expr_type() const;

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

      virtual TYPE expr_type() const;

      virtual bool set_width(unsigned w);
      virtual NetEBDiv* dup_expr() const;
      virtual NetExpr* eval_tree();
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
      virtual NetEConst* eval_tree();

      virtual NetNet* synthesize(Design*);
};

/*
 * The binary comparison operators are handled by this class. This
 * this case the bit width of the expression is 1 bit, and the
 * operands take their natural widths. The supported operators are:
 *
 *   <  -- Less than
 *   >  -- Greater than
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

	/* A compare expression has a definite width. */
      virtual bool has_width() const;
      virtual NetEBComp* dup_expr() const;
      virtual NetEConst* eval_tree();

      virtual NetNet* synthesize(Design*);

    private:
      NetEConst*eval_eqeq_();
      NetEConst*eval_less_();
      NetEConst*eval_leeq_();
      NetEConst*eval_leeq_real_(bool gt_flag, bool include_eq_flag);
      NetEConst*eval_gt_();
      NetEConst*eval_gteq_();
      NetEConst*eval_neeq_();
      NetEConst*eval_eqeqeq_();
      NetEConst*eval_neeqeq_();
};

/*
 * The binary logical operators are those that return boolean
 * results. The supported operators are:
 *
 *   a  -- Logical AND (&&)
 *   o  -- Logical OR (||)
 */
class NetEBLogic : public NetEBinary {

    public:
      NetEBLogic(char op, NetExpr*l, NetExpr*r);
      ~NetEBLogic();

      virtual bool set_width(unsigned w);
      virtual NetEBLogic* dup_expr() const;
      virtual NetEConst* eval_tree();
      virtual NetNet* synthesize(Design*);

    private:
};


/*
 * Support the binary multiplication (*) operator.
 */
class NetEBMult : public NetEBinary {

    public:
      NetEBMult(char op, NetExpr*l, NetExpr*r);
      ~NetEBMult();

      virtual TYPE expr_type() const;

      virtual bool set_width(unsigned w);
      virtual NetEBMult* dup_expr() const;
      virtual NetExpr* eval_tree();
      virtual NetNet* synthesize(Design*);

    private:

      NetExpr* eval_tree_real_();

};


/*
 * The binary logical operators are those that return boolean
 * results. The supported operators are:
 *
 *   l  -- left shift (<<)
 *   r  -- right shift (>>)
 *   R  -- right shift arithmetic (>>>)
 */
class NetEBShift : public NetEBinary {

    public:
      NetEBShift(char op, NetExpr*l, NetExpr*r);
      ~NetEBShift();

      virtual bool set_width(unsigned w);

	// A shift expression only needs the left expression to have a
	// definite width to give the expression a definite width.
      virtual bool has_width() const;

      virtual NetEBShift* dup_expr() const;
      virtual NetEConst* eval_tree();

      virtual NetNet* synthesize(Design*);

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
      NetEConcat(unsigned cnt, NetExpr* repeat =0);
      ~NetEConcat();

	// Manipulate the parameters.
      void set(unsigned idx, NetExpr*e);

      unsigned repeat();
      unsigned repeat() const;
      unsigned nparms() const { return parms_.count() ; }
      NetExpr* parm(unsigned idx) const { return parms_[idx]; }

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual bool has_width() const;
      virtual bool set_width(unsigned w);
      virtual NetEConcat* dup_expr() const;
      virtual NetEConst*  eval_tree();
      virtual NetNet*synthesize(Design*);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      svector<NetExpr*>parms_;
      NetExpr* repeat_;
      unsigned repeat_value_;
      bool repeat_calculated_;
};

/*
 * This node represents a reference to a variable.
 */
class NetEVariable  : public NetExpr {

    public:
      NetEVariable(NetVariable*);
      ~NetEVariable();

      const NetVariable* variable() const;

      TYPE expr_type() const;

      void expr_scan(struct expr_scan_t*) const;
      void dump(ostream&) const;

      NetEVariable*dup_expr() const;
      NexusSet* nex_input(bool rem_out = true);

    private:
      NetVariable*var_;
};

/*
 * This class is a placeholder for a parameter expression. When
 * parameters are first created, an instance of this object is used to
 * hold the place where the parameter expression goes. Then, when the
 * parameters are resolved, these objects are removed.
 *
 * If the parameter object is created with a path and name, then the
 * object represents a reference to a parameter that is known to exist.
 */
class NetEParam  : public NetExpr {
    public:
      NetEParam();
      NetEParam(class Design*des, NetScope*scope, perm_string name);
      ~NetEParam();

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual bool set_width(unsigned w);
      virtual bool has_width() const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetExpr* eval_tree();
      virtual NetEParam* dup_expr() const;

      virtual void dump(ostream&) const;

    private:
      Design*des_;
      NetScope*scope_;
      perm_string name_;
};


/*
 * This expression node supports bit/part selects from general
 * expressions. The sub-expression is self-sized, and has bits
 * selected from it. The base is the expression that identifies the
 * lsb of the expression, and the wid is the width of the part select,
 * or 1 for a bit select.
 *
 * If the base expression is null, then this expression node can be
 * used to express width expansion, signed or unsigned depending on
 * the has_sign() flag.
 */
class NetESelect  : public NetExpr {

    public:
      NetESelect(NetExpr*exp, NetExpr*base, unsigned wid);
      ~NetESelect();

      const NetExpr*sub_expr() const;
      const NetExpr*select() const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual bool set_width(unsigned w);
      virtual bool has_width() const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEConst* eval_tree();
      virtual NetESelect* dup_expr() const;
      virtual NetNet*synthesize(Design*des);
      virtual void dump(ostream&) const;

    private:
      NetExpr*expr_;
      NetExpr*base_;
};

/*
 * This node is for representation of named events.
 */
class NetEEvent : public NetExpr {

    public:
      NetEEvent(NetEvent*);
      ~NetEEvent();

      const NetEvent* event() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEEvent* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true);

      virtual void dump(ostream&os) const;

    private:
      NetEvent*event_;
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
      virtual NexusSet* nex_input(bool rem_out = true);

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
      NetESFunc(const char*name, NetExpr::TYPE t,
		unsigned width, unsigned nprms);
      ~NetESFunc();

      const char* name() const;

      unsigned nparms() const;
      void parm(unsigned idx, NetExpr*expr);
      NetExpr* parm(unsigned idx);
      const NetExpr* parm(unsigned idx) const;

      virtual TYPE expr_type() const;
      virtual NexusSet* nex_input(bool rem_out = true);
      virtual bool set_width(unsigned);
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetESFunc*dup_expr() const;

    private:
      const char* name_;
      TYPE type_;
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
      virtual NetExpr* eval_tree();

      virtual NexusSet* nex_input(bool rem_out = true);
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

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    protected:
      char op_;
      NetExpr* expr_;

      void eval_expr_();
};

class NetEUBits : public NetEUnary {

    public:
      NetEUBits(char op, NetExpr*ex);
      ~NetEUBits();

      virtual NetNet* synthesize(Design*);

      virtual NetEConst* eval_tree();
};

class NetEUReduce : public NetEUnary {

    public:
      NetEUReduce(char op, NetExpr*ex);
      ~NetEUReduce();

      virtual bool set_width(unsigned w);
      virtual NetNet* synthesize(Design*);
      virtual NetEUReduce* dup_expr() const;
      virtual NetEConst* eval_tree();

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

      perm_string name () const;
      const NetExpr* index() const;

      virtual bool set_width(unsigned);
      virtual NetNet* synthesize(Design*);
      NetExpr* eval_tree();
      virtual NetEMemory*dup_expr() const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      const NetMemory*memory() const { return mem_; };

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
 * activity can invoke the expression. This node also supports part
 * select by indexing a range of the NetNet that is associated with
 * it. The msi() is the more significant index, and lsi() the least
 * significant index.
 */
class NetESignal  : public NetExpr {

    public:
      NetESignal(NetNet*n);
      NetESignal(NetNet*n, unsigned msi, unsigned lsi);
      ~NetESignal();

      perm_string name() const;
      virtual bool set_width(unsigned);

      virtual NetESignal* dup_expr() const;
      NetNet* synthesize(Design*des);
      NexusSet* nex_input(bool rem_out = true);

	// These methods actually reference the properties of the
	// NetNet object that I point to.
      unsigned bit_count() const;
      Link& bit(unsigned idx);

      const NetNet* sig() const;
      unsigned msi() const;
      unsigned lsi() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      NetNet*net_;
      unsigned msi_;
      unsigned lsi_;
};

/*
 * An expression that takes a bit of a signal is represented as
 * one of these. For example, ``foo[x+5]'' is a signal and x+5 is an
 * expression to select a single bit from that signal. I can't just
 * make a new NetESignal node connected to the single net because the
 * expression may vary during execution, so the structure is not known
 * at compile (elaboration) time.
 */
class NetEBitSel  : public NetExpr {

    public:
      NetEBitSel(NetESignal*sig, NetExpr*ex);
      ~NetEBitSel();

      perm_string name() const;
      const NetExpr*index() const { return idx_; }

      virtual bool set_width(unsigned);

      const NetNet* sig() const;

      NetEBitSel* dup_expr() const;
      NetNet* synthesize(Design*des);

      virtual NexusSet* nex_input(bool rem_out = true);
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
class NetScope : public Attrib {

    public:
      enum TYPE { MODULE, TASK, FUNC, BEGIN_END, FORK_JOIN };

	/* Create a new scope, and attach it to the given parent. The
	   name is expected to have been permallocated. */
      NetScope(NetScope*up, perm_string name, TYPE t);
      ~NetScope();

	/* Parameters exist within a scope, and these methods allow
	   one to manipulate the set. In these cases, the name is the
	   *simple* name of the parameter, the hierarchy is implicit in
	   the scope. The return value from set_parameter is the
	   previous expression, if there was one. */

      NetExpr* set_parameter(perm_string name, NetExpr*val,
			     NetExpr*msb, NetExpr*lsb, bool signed_flag);
      NetExpr* set_localparam(perm_string name, NetExpr*val);
      const NetExpr*get_parameter(const char* name) const;

	/* These are used by defparam elaboration to replace the
	   expression with a new expression, without affecting the
	   range or signed_flag. Return false if the name does not
	   exist. */
      bool replace_parameter(perm_string name, NetExpr*val);

	/* These methods set or access events that live in this
	   scope. */

      void add_event(NetEvent*);
      void rem_event(NetEvent*);
      NetEvent*find_event(const char*name);

      void add_variable(NetVariable*);
      void rem_variable(NetVariable*);
      NetVariable*find_variable(const char*name);


	/* These methods manage signals. The add_ and rem_signal
	   methods are used by the NetNet objects to make themselves
	   available to the scope, and the find_signal method can be
	   used to locate signals within a scope. */

      void add_signal(NetNet*);
      void rem_signal(NetNet*);

      NetNet* find_signal(const char*name);
      NetNet* find_signal_in_child(const hname_t&name);


	/* ... and these methods manage memory the same way as signals
	   are managed above. */

      void add_memory(NetMemory*);
      void rem_memory(NetMemory*);

      NetMemory* find_memory(const string&name);


	/* The parent and child() methods allow users of NetScope
	   objects to locate nearby scopes. */
      NetScope* parent();
      NetScope* child(const char*name);
      const NetScope* parent() const;
      const NetScope* child(const char*name) const;

      TYPE type() const;

      void set_task_def(NetTaskDef*);
      void set_func_def(NetFuncDef*);
      void set_module_name(perm_string);

      NetTaskDef* task_def();
      NetFuncDef* func_def();

      const NetTaskDef* task_def() const;
      const NetFuncDef* func_def() const;

	/* If the scope represents a module instance, the module_name
	   is the name of the module itself. */
      perm_string module_name() const;

	/* Scopes have their own time units and time precision. The
	   unit and precision are given as power of 10, i.e., -3 is
	   units of milliseconds.

	   If a NetScope is created with a parent scope, the new scope
	   will initially inherit the unit and precision of the
	   parent scope. */

      void time_unit(int);
      void time_precision(int);

      int time_unit() const;
      int time_precision() const;

      void default_nettype(NetNet::Type);
      NetNet::Type default_nettype() const;

	/* The name of the scope is the fully qualified hierarchical
	   name, whereas the basename is just my name within my parent
	   scope. */
      perm_string basename() const;
      string name() const;

      void run_defparams(class Design*);
      void evaluate_parameters(class Design*);

	/* This method generates a non-hierarchical name that is
	   guaranteed to be unique within this scope. */
      perm_string local_symbol();
	/* This method generates a hierarchical name that is
	   guaranteed to be unique globally. */
      string local_hsymbol();

      void dump(ostream&) const;
      void emit_scope(struct target_t*tgt) const;
      bool emit_defs(struct target_t*tgt) const;

	/* This method runs the functor on me. Recurse through the
	   children of this node as well. */
      void run_functor(Design*des, functor_t*fun);


	/* This member is used during elaboration to pass defparam
	   assignments from the scope pass to the parameter evaluation
	   step. After that, it is not used. */

      map<hname_t,NetExpr*>defparams;

    public:
	/* After everything is all set up, the code generators like
	   access to these things to make up the parameter lists. */
      struct param_expr_t {
	    NetExpr*expr;
	    NetExpr*msb;
	    NetExpr*lsb;
	    bool signed_flag;
      };
      map<perm_string,param_expr_t>parameters;
      map<perm_string,param_expr_t>localparams;

	/* Module instance arrays are collected here for access during
	   the multiple elaboration passes. */
      typedef svector<NetScope*> scope_vec_t;
      map<perm_string, scope_vec_t>instance_arrays;

    private:
      TYPE type_;
      perm_string name_;

      signed char time_unit_, time_prec_;
      NetNet::Type default_nettype_;

      NetEvent *events_;
      NetVariable*vars_;
      NetNet   *signals_;
      NetMemory*memories_;

      perm_string module_name_;
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

      void set_flags(const map<string,const char*>&f) { flags_ = f; }

      const char* get_flag(const string&key) const;

      NetScope* make_root_scope(perm_string name);
      NetScope* find_root_scope();
      list<NetScope*> find_root_scopes();

      const list<NetScope*> find_root_scopes() const;

	/* Attempt to set the precision to the specified value. If the
	   precision is already more precise, the keep the precise
	   setting. This is intended to hold the simulation precision
	   for use throughout the entire design. */

      void set_precision(int val);
      int  get_precision() const;

	/* This function takes a delay value and a scope, and returns
	   the delay value scaled to the precision of the design. */
      unsigned long scale_to_precision(unsigned long, const NetScope*)const;

	/* Look up a scope. If no starting scope is passed, then the
	   path is taken as an absolute scope name. Otherwise, the
	   scope is located starting at the passed scope and working
	   up if needed. */
      NetScope* find_scope(const hname_t&path) const;
      NetScope* find_scope(NetScope*, const hname_t&path) const;

	// PARAMETERS

	/* This method searches for a parameter, starting in the given
	   scope. This method handles the upward searches that the
	   NetScope class itself does not support.

	   The scope of the located expression is stored in the
	   found_in argument. */
      const NetExpr*find_parameter( NetScope*, const hname_t&path,
				    NetScope*&found_in) const;
      const NetExpr*find_parameter( const NetScope*, const hname_t&path) const;

      void run_defparams();
      void evaluate_parameters();

	/* This method locates a signal, starting at a given
	   scope. The name parameter may be partially hierarchical, so
	   this method, unlike the NetScope::find_signal method,
	   handles global name binding. */

      NetNet*find_signal(NetScope*scope, hname_t path);

	// Functions
      NetFuncDef* find_function(NetScope*scope, const hname_t&key);
      NetFuncDef* find_function(const hname_t&path);

	// Tasks
      NetScope* find_task(NetScope*scope, const hname_t&name);
      NetScope* find_task(const hname_t&key);

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
      list<NetScope*>root_scopes_;

	// List the nodes in the design.
      NetNode*nodes_;
	// These are in support of the node functor iterator.
      NetNode*nodes_functor_cur_;
      NetNode*nodes_functor_nxt_;

	// List the processes in the design.
      NetProcTop*procs_;
      NetProcTop*procs_idx_;

      map<string,const char*> flags_;

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

/* Return the number of links in the ring that are of the specified
   type. */
extern unsigned count_inputs(const Link&pin);
extern unsigned count_outputs(const Link&pin);
extern unsigned count_signals(const Link&pin);

/* Find the next link that is an output into the nexus. */
extern Link* find_next_output(Link*lnk);

/* Find the signal connected to the given node pin. There should
   always be exactly one signal. The bidx parameter gets filled with
   the signal index of the Net, in case it is a vector. */
const NetNet* find_link_signal(const NetObj*net, unsigned pin,
			       unsigned&bidx);

inline ostream& operator << (ostream&o, const NetExpr&exp)
{ exp.dump(o); return o; }

extern ostream& operator << (ostream&, NetNet::Type);

#endif
