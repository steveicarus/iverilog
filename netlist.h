#ifndef __netlist_H
#define __netlist_H
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
#ident "$Id: netlist.h,v 1.66 1999/09/18 01:53:08 steve Exp $"
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
# include  "sref.h"
# include  "LineInfo.h"
# include  "svector.h"

class Design;
class NetNode;
class NetProc;
class NetProcTop;
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
      class Link {
	    friend void connect(Link&, Link&);
	    friend class NetObj;

	  public:
	    enum DIR { PASSIVE, INPUT, OUTPUT };
	    Link() : dir_(PASSIVE), next_(this), prev_(this) { }
	    ~Link() { unlink(); }

	      // Manipulate the link direction.
	    void set_dir(DIR d) { dir_ = d; }
	    DIR get_dir() const { return dir_; }

	    void cur_link(NetObj*&net, unsigned &pin)
		  { net = node_;
		    pin = pin_;
		  }

	    void next_link(NetObj*&net, unsigned&pin)
		  { net = next_->node_;
		    pin = next_->pin_;
		  }

	    void next_link(const NetObj*&net, unsigned&pin) const
		  { net = next_->node_;
		    pin = next_->pin_;
		  }

	    Link* next_link() { return next_; }
	    const Link* next_link() const { return next_; }

	      // Remove this link from the set of connected pins. The
	      // destructor will automatically do this if needed.
	    void unlink()
		  { next_->prev_ = prev_;
		    prev_->next_ = next_;
		    next_ = prev_ = this;
		  }

	      // Return true if this link is connected to anything else.
	    bool is_linked() const { return next_ != this; }

	      // Return true if these pins are connected.
	    bool is_linked(const NetObj::Link&that) const;

	      // Return true if this link is connected to any pin of r.
	    bool is_linked(const NetObj&r) const;

	    bool is_equal(const NetObj::Link&that) const
		  { return (node_ == that.node_) && (pin_ == that.pin_); }

	      // Return information about the object that this link is
	      // a part of.
	    const NetObj*get_obj() const { return node_; }
	    NetObj*get_obj() { return node_; }
	    unsigned get_pin() const { return pin_; }

	  private:
	      // The NetNode manages these. They point back to the
	      // NetNode so that following the links can get me here.
	    NetObj *node_;
	    unsigned pin_;
	    DIR dir_;

	  private:
	    Link *next_;
	    Link *prev_;

	  private: // not implemented
	    Link(const Link&);
	    Link& operator= (const Link&);
      };

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

      bool test_mark() const { return mark_; }
      void set_mark(bool flag=true) { mark_ = flag; }

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

      bool mark_;
};

/*
 * A NetNode is a device of some sort, where each pin has a different
 * meaning. (i.e. pin(0) is the output to an and gate.) NetNode
 * objects are listed in the nodes_ of the Design object.
 */
class NetNode  : public NetObj {

    public:
      explicit NetNode(const string&n, unsigned npins)
      : NetObj(n, npins), node_next_(0), node_prev_(0), design_(0) { }

      virtual ~NetNode();

      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump_node(ostream&, unsigned) const;

    private:
      friend class Design;
      NetNode*node_next_, *node_prev_;
      Design*design_;
};


/*
 * NetNet is a special kind of NetObj that doesn't really do anything,
 * but carries the properties of the wire/reg/trireg. Thus, a set of
 * pins connected together would also be connected to exactly one of
 * these.
 *
 * Note that a net of any sort has exactly one pin. The pins feature
 * of the NetObj class is used to make a set of identical wires, in
 * order to support ranges, or busses. When dealing with vectors,
 * pin(0) is always the least significant bit.
 */
class NetNet  : public NetObj, public LineInfo {

    public:
      enum Type { IMPLICIT, IMPLICIT_REG, WIRE, TRI, TRI1, SUPPLY0,
		  WAND, TRIAND, TRI0, SUPPLY1, WOR, TRIOR, REG, INTEGER };

      enum PortType { NOT_A_PORT, PIMPLICIT, PINPUT, POUTPUT, PINOUT };

      explicit NetNet(const string&n, Type t, unsigned npins =1);

      explicit NetNet(const string&n, Type t, long ms, long ls);

      virtual ~NetNet();


      Type type() const { return type_; }
      void type(Type t) { type_ = t; }

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

      verinum::V get_ival(unsigned pin) const
	    { return ivalue_[pin]; }
      void set_ival(unsigned pin, verinum::V val)
	    { ivalue_[pin] = val; }

      virtual void dump_net(ostream&, unsigned) const;

    private:
	// The Design class uses this for listing signals.
      friend class Design;
      NetNet*sig_next_, *sig_prev_;
      Design*design_;

    private:
      Type   type_;
      PortType port_type_;

      long msb_, lsb_;

      bool local_flag_;

      verinum::V*ivalue_;
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

      NetObj::Link& pin_Aclr();
      NetObj::Link& pin_Add_Sub();
      NetObj::Link& pin_Clock();
      NetObj::Link& pin_Cin();
      NetObj::Link& pin_Cout();
      NetObj::Link& pin_Overflow();

      NetObj::Link& pin_DataA(unsigned idx);
      NetObj::Link& pin_DataB(unsigned idx);
      NetObj::Link& pin_Result(unsigned idx);

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual void emit_node(ostream&, struct target_t*) const;
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
      NetMemory(const string&n, long w, long s, long e);

      const string&name() const { return name_; }

	// This is the width (in bits) of a single memory position.
      unsigned width() const { return width_; }

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
      explicit NetExpr(unsigned w =0) : width_(w)  { }
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

    protected:
      void expr_width(unsigned w) { width_ = w; }

    private:
      unsigned width_;

    private: // not implemented
      NetExpr(const NetExpr&);
      NetExpr& operator=(const NetExpr&);
};

/*
 * The NetTmp object is a network that is only used momentarily by
 * elaboration to carry links around. A completed netlist cannot have
 * any of these within. This is a kind of wire, so it is NetNet type.
 */
class NetTmp  : public NetNet {

    public:
      explicit NetTmp(unsigned npins =1)
      : NetNet("@", IMPLICIT, npins) { }

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
      explicit NetBUFZ(const string&n)
      : NetNode(n, 2)
      { pin(0).set_dir(Link::OUTPUT);
        pin(1).set_dir(Link::INPUT);
      }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual void emit_node(ostream&, struct target_t*) const;
};

class NetConst  : public NetNode {

    public:
      explicit NetConst(const string&n, verinum::V v)
      : NetNode(n, 1), value_(v) { pin(0).set_dir(Link::OUTPUT); }

      verinum::V value() const { return value_; }

      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      verinum::V value_;
};

/*
 * This class represents all manner of logic gates. Pin 0 is OUTPUT and
 * all the remaining pins are INPUT
 */
class NetLogic  : public NetNode {

    public:
      enum TYPE { AND, BUF, BUFIF0, BUFIF1, NAND, NOR, NOT, OR, XNOR,
		  XOR };

      explicit NetLogic(const string&n, unsigned pins, TYPE t);

      TYPE type() const { return type_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual void emit_node(ostream&, struct target_t*) const;

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
 * The truth table is canonically represented as a finite state
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
 * COMBINATIONAL
 * The logic table is a map between the input levels and the
 * output. Each input pin can have the value 0, 1 or x and the output
 * can have the values 0 or 1. If the input matches nothing, the
 * output is x. In canonical form, only the entries that generate 0 or
 * 1 are listed.
 *
 * SEQUENTIAL
 * These objects have a single bit of memory. The logic table includes
 * an entry for the current value, and allows edges on the inputs. In
 * canonical form, inly then entries that generate 0, 1 or - (no change)
 * are listed.
 *
 */
class NetUDP  : public NetNode {

    public:
      explicit NetUDP(const string&n, unsigned pins, bool sequ =false);

      virtual void emit_node(ostream&, struct target_t*) const;
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
      const bool sequential_;
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

      void dump_sequ_(ostream&o, unsigned ind) const;
      void dump_comb_(ostream&o, unsigned ind) const;
};

/* =========
 * A process is a behavioral-model description. A process is a
 * statement that may be compound. the various statement types may
 * refer to places in a netlist (by pointing to nodes) but is not
 * linked into the netlist. However, elaborating a process may cause
 * special nodes to be created to handle things like events.
 */
class NetProc {

    public:
      explicit NetProc() : next_(0) { }
      virtual ~NetProc();

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      friend class NetBlock;
      NetProc*next_;
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

class NetAssign_ : public NetProc, public NetNode, public LineInfo {

    protected:
      NetAssign_(const string&n, unsigned w);
      virtual ~NetAssign_() =0;
};

class NetAssign  : public NetAssign_ {
    public:
      explicit NetAssign(const string&, Design*des, unsigned w, NetExpr*rv);
      ~NetAssign();

      NetExpr*rval() { return rval_; }
      const NetExpr*rval() const { return rval_; }

      void find_lval_range(const NetNet*&net, unsigned&msb,
			   unsigned&lsb) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      NetExpr* rval_;
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

	// This is the (procedural) value that is to be assigned when
	// the assignment is executed.
      const NetExpr*rval() const { return rval_; }

	// If this expression exists, then only a single bit is to be
	// set from the rval, and the value of this expression selects
	// the pin that gets the value.
      const NetExpr*bmux() const { return bmux_; }

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      NetExpr* rval_;
      NetExpr* bmux_;
};

/*
 * Assignment to memory is handled separately because memory is
 * not a node. There are blocking and non-blocking variants, just like
 * regular assign, and the NetAssignMem_ base class takes care of all
 * the common stuff.
 */
class NetAssignMem_ : public NetProc, public LineInfo {

    public:
      explicit NetAssignMem_(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMem_();

      const NetMemory*memory()const { return mem_; }
      const NetExpr*index()const { return index_; }
      const NetExpr*rval()const { return rval_; }

    private:
      NetMemory*mem_;
      NetExpr* index_;
      NetExpr* rval_;
};

class NetAssignMem : public NetAssignMem_ {

    public:
      explicit NetAssignMem(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMem();

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
};

class NetAssignMemNB : public NetAssignMem_ {

    public:
      explicit NetAssignMemNB(NetMemory*, NetExpr*idx, NetExpr*rv);
      ~NetAssignMemNB();

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
};

/* A block is stuff line begin-end blocks, that contain and ordered
   list of NetProc statements.

   NOTE: The emit method calls the target->proc_block function but
   does not recurse. It is up to the target-supplied proc_block
   function to call emit_recurse. */
class NetBlock  : public NetProc {

    public:
      enum Type { SEQU, PARA };

      NetBlock(Type t) : type_(t), last_(0) { }
      ~NetBlock();

      const Type type() const { return type_; }

      void append(NetProc*);

      void emit_recurse(ostream&, struct target_t*) const;
      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      const Type type_;

      NetProc*last_;
};

/* A CASE statement in the verilog source leads, eventually, to one of
   these. This is different from a simple conditional because of the
   way the comparisons are performed. Also, it is likely that the
   target may be able to optimize differently. */
class NetCase  : public NetProc {

    public:
      NetCase(NetExpr*ex, unsigned cnt);
      ~NetCase();

      void set_case(unsigned idx, NetExpr*ex, NetProc*st);

      const NetExpr*expr() const { return expr_; }
      unsigned nitems() const { return nitems_; }

      const NetExpr*expr(unsigned idx) const { return items_[idx].guard;}
      const NetProc*stat(unsigned idx) const { return items_[idx].statement; }

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:

      struct Item {
	    NetExpr*guard;
	    NetProc*statement;
      };

      NetExpr* expr_;
      unsigned nitems_;
      Item*items_;
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

      void emit_recurse_if(ostream&, struct target_t*) const;
      void emit_recurse_else(ostream&, struct target_t*) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr* expr_;
      NetProc*if_;
      NetProc*else_;
};

/*
 * A forever statement is executed over and over again forever. Or
 * until its block is disabled.
 */
class NetForever : public NetProc {

    public:
      explicit NetForever(NetProc*s);
      ~NetForever();

      void emit_recurse(ostream&, struct target_t*) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
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
      NetFuncDef(const string&, const svector<NetNet*>&po);
      ~NetFuncDef();

      void set_proc(NetProc*st);

      const string& name() const;
      const NetProc*proc() const;

      unsigned port_count() const;
      const NetNet*port(unsigned idx) const;

      virtual void dump(ostream&, unsigned ind) const;

    private:
      string name_;
      NetProc*statement_;
      svector<NetNet*>ports_;
};

class NetPDelay  : public NetProc {

    public:
      NetPDelay(unsigned long d, NetProc*st)
      : delay_(d), statement_(st) { }

      unsigned long delay() const { return delay_; }

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

      void emit_proc_recurse(ostream&, struct target_t*) const;

    private:
      unsigned long delay_;
      NetProc*statement_;
};

/*
 * The NetPEvent is associated with NetNEvents. The NetPEvent receives
 * events from any one of the associated NetNEvents and in response
 * causes the attached statement to be executed. Objects of this type
 * are not nodes, but require a name anyhow so that backends can
 * generate objects to refer to it.
 *
 * The NetPEvent is the procedural part of the event.
 */
class NetNEvent;
class NetPEvent : public NetProc, public sref_back<NetPEvent,NetNEvent> {

    public:
      NetPEvent(const string&n, NetProc*st);
      ~NetPEvent();

      string name() const { return name_; }
      NetProc* statement();
      const NetProc* statement() const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

      void emit_proc_recurse(ostream&, struct target_t*) const;

    private:
      string name_;
      NetProc*statement_;
};

/*
 * The NetNEvent is a NetNode that connects to the structural part of
 * the design. It has only inputs, which cause the side effect of
 * triggering an event that the procedural part of the design can use.
 *
 * The NetNEvent may have wide input if is is an ANYEDGE type
 * device. This allows detecting changes in wide expressions.
 */
class NetNEvent  : public NetNode, public sref<NetPEvent,NetNEvent> {

    public:
      enum Type { ANYEDGE, POSEDGE, NEGEDGE, POSITIVE };

      NetNEvent(const string&ev, unsigned wid, Type e, NetPEvent*pe);
      ~NetNEvent();

      Type type() const { return edge_; }

      virtual void emit_node(ostream&, struct target_t*) const;

      void dump_proc(ostream&) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      Type edge_;
};


/*
 * A repeat statement is executed some fixed number of times.
 */
class NetRepeat : public NetProc {

    public:
      explicit NetRepeat(NetExpr*e, NetProc*s);
      ~NetRepeat();

      const NetExpr*expr() const;
      void emit_recurse(ostream&, struct target_t*) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr*expr_;
      NetProc*statement_;
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

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      string name_;
      svector<NetExpr*>parms_;
};

/*
 * This class represents an elaborated class definition. NetUTask
 * classes may refer to objects of this type to get the meaning of the
 * defined task.
 */
class NetTaskDef {

    public:
      NetTaskDef(const string&n, NetProc*p, const svector<NetNet*>&po);
      ~NetTaskDef();

      const string& name() const { return name_; }
      const NetProc*proc() const { return proc_; }

      unsigned port_count() const { return ports_.count(); }
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

      const string& name() const;

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

      virtual void emit_proc(ostream&, struct target_t*) const;
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

      void emit_proc_recurse(ostream&, struct target_t*) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr* cond_;
      NetProc*proc_;
};


/* The is the top of any process. It carries the type (initial or
   always) and a pointer to the statement, probably a block, that
   makes up the process. */
class NetProcTop  : public LineInfo {

    public:
      enum Type { KINITIAL, KALWAYS };

      NetProcTop(Type t, class NetProc*st);
      ~NetProcTop();

      Type type() const { return type_; }
      NetProc*statement();
      const NetProc*statement() const;

      void dump(ostream&, unsigned ind) const;
      void emit(ostream&, struct target_t*tgt) const;

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
 *   l  -- Left shift (<<)
 *   r  -- Right shift (>>)
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
 */
class NetEBBits : public NetEBinary {

    public:
      NetEBBits(char op, NetExpr*l, NetExpr*r);
      ~NetEBBits();

      virtual bool set_width(unsigned w);
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
      virtual NetExpr* eval_tree();

    private:
      NetExpr*eval_eqeq_();
      NetExpr*eval_leeq_();

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
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      svector<NetExpr*>parms_;
      unsigned repeat_;
};

class NetEConst  : public NetExpr {

    public:
      NetEConst(const verinum&val)
      : NetExpr(val.len()), value_(val) { }
      ~NetEConst();

      const verinum&value() const { return value_; }

      virtual bool set_width(unsigned w);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetEConst* dup_expr() const;

    private:
      verinum value_;
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
 *   A  -- Reduciton NAND (~&)
 *   N  -- Reduciton NOR (~|)
 *   X  -- Reduciton NXOR (~^ or ^~)
 */
class NetEUnary  : public NetExpr {

    public:
      NetEUnary(char op, NetExpr*ex)
      : NetExpr(ex->expr_width()), op_(op), expr_(ex) { }
      ~NetEUnary();

      char op() const { return op_; }
      const NetExpr* expr() const { return expr_; }

      virtual bool set_width(unsigned w);

      virtual NetEUnary* dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      char op_;
      NetExpr* expr_;
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
 * A reference to a memory is represented by this expression.
 */
class NetEMemory  : public NetExpr {

    public:
      NetEMemory(NetMemory*mem, NetExpr*idx);
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
class NetESignal  : public NetExpr, public NetNode {

    public:
      NetESignal(NetNet*n);
      NetESignal(const string&name, unsigned npins);
      ~NetESignal();

      const string& name() const { return NetNode::name(); }

      virtual bool set_width(unsigned);

      virtual NetESignal* dup_expr() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump(ostream&) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
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
 * This class contains an entire design. It includes processes and a
 * netlist, and can be passed around from function to function.
 */
class Design {

    public:
      Design() : errors(0), signals_(0), nodes_(0), procs_(0), lcounter_(0) { }

	/* The flags are a generic way of accepting command line
	   parameters/flags and passing them to the processing steps
	   that deal with the design. The compilation driver sets the
	   entire flags map after elaboration is done. Subsequent
	   steps can then use the get_flag() function to get the value
	   of an interesting key. */

      void set_flags(const map<string,string>&f) { flags_ = f; }

      string get_flag(const string&key) const;


	// PARAMETERS
      void set_parameter(const string&, NetExpr*);
      const NetExpr*find_parameter(const string&path, const string&name) const;

	// SIGNALS
      void add_signal(NetNet*);
      void del_signal(NetNet*);
      NetNet*find_signal(const string&path, const string&name);

	// Memories
      void add_memory(NetMemory*);
      NetMemory* find_memory(const string&name);

	// Functions
      void add_function(const string&n, NetFuncDef*);
      NetFuncDef* find_function(const string&path, const string&key);
      NetFuncDef* find_function(const string&path);

	// Tasks
      void add_task(const string&n, NetTaskDef*);
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
      void emit(ostream&, struct target_t*) const;

      void clear_node_marks();
      NetNode*find_node(bool (*test)(const NetNode*));

      void clear_signal_marks();
      NetNet*find_signal(bool (*test)(const NetNet*));

	// This is incremented by elaboration when an error is
	// detected. It prevents code being emitted.
      unsigned errors;

    public:
      string local_symbol(const string&path);

    private:
	// List all the parameters in the design. This table includes
	// the parameters of instantiated modules in canonical names.
      map<string,NetExpr*> parameters_;

	// List all the signals in the design.
      NetNet*signals_;

      map<string,NetMemory*> memories_;

	// List the function definitions in the design.
      map<string,NetFuncDef*> funcs_;

	// List the task definitions in the design.
      map<string,NetTaskDef*> tasks_;

	// List the nodes in the design
      NetNode*nodes_;

	// List the processes in the design.
      NetProcTop*procs_;
      NetProcTop*procs_idx_;

      map<string,string> flags_;

      unsigned lcounter_;

    private: // not implemented
      Design(const Design&);
      Design& operator= (const Design&);
};


/* =======
 */

inline bool operator == (const NetObj::Link&l, const NetObj::Link&r)
{ return l.is_equal(r); }

inline bool operator != (const NetObj::Link&l, const NetObj::Link&r)
{ return ! l.is_equal(r); }

/* Connect the pins of two nodes together. Either may already be
   connected to other things, connect is transitive. */
extern void connect(NetObj::Link&, NetObj::Link&);

/* Return true if l and r are connected. */
inline bool connected(const NetObj::Link&l, const NetObj::Link&r)
{ return l.is_linked(r); }

/* Return true if l is fully connected to r. This means, every pin in
   l is connected to a pin in r. This is expecially useful for
   checking signal vectors. */
extern bool connected(const NetObj&l, const NetObj&r);

/* return the number of links in the ring that are of the specified
   type. */
extern unsigned count_inputs(const NetObj::Link&pin);
extern unsigned count_outputs(const NetObj::Link&pin);
extern unsigned count_signals(const NetObj::Link&pin);

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
 * Revision 1.66  1999/09/18 01:53:08  steve
 *  Detect constant lessthen-equal expressions.
 *
 * Revision 1.65  1999/09/16 04:18:15  steve
 *  elaborate concatenation repeats.
 *
 * Revision 1.64  1999/09/15 01:55:06  steve
 *  Elaborate non-blocking assignment to memories.
 *
 * Revision 1.63  1999/09/13 03:10:59  steve
 *  Clarify msb/lsb in context of netlist. Properly
 *  handle part selects in lval and rval of expressions,
 *  and document where the least significant bit goes
 *  in NetNet objects.
 *
 * Revision 1.62  1999/09/11 04:43:17  steve
 *  Support ternary and <= operators in vvm.
 *
 * Revision 1.61  1999/09/08 04:05:30  steve
 *  Allow assign to not match rvalue width.
 *
 * Revision 1.60  1999/09/03 04:28:38  steve
 *  elaborate the binary plus operator.
 *
 * Revision 1.59  1999/09/01 20:46:19  steve
 *  Handle recursive functions and arbitrary function
 *  references to other functions, properly pass
 *  function parameters and save function results.
 *
 * Revision 1.58  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.57  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 * Revision 1.56  1999/08/18 04:00:02  steve
 *  Fixup spelling and some error messages. <LRDoolittle@lbl.gov>
 *
 * Revision 1.55  1999/08/06 04:05:28  steve
 *  Handle scope of parameters.
 *
 * Revision 1.54  1999/08/01 21:48:11  steve
 *  set width of procedural r-values when then
 *  l-value is a memory word.
 *
 * Revision 1.53  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.52  1999/07/31 03:16:54  steve
 *  move binary operators to derived classes.
 *
 * Revision 1.51  1999/07/24 02:11:20  steve
 *  Elaborate task input ports.
 *
 * Revision 1.50  1999/07/18 21:17:50  steve
 *  Add support for CE input to XNF DFF, and do
 *  complete cleanup of replaced design nodes.
 *
 * Revision 1.49  1999/07/18 05:52:47  steve
 *  xnfsyn generates DFF objects for XNF output, and
 *  properly rewrites the Design netlist in the process.
 *
 * Revision 1.48  1999/07/17 22:01:13  steve
 *  Add the functor interface for functor transforms.
 *
 * Revision 1.47  1999/07/17 19:51:00  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.46  1999/07/17 03:08:32  steve
 *  part select in expressions.
 *
 * Revision 1.45  1999/07/16 04:33:41  steve
 *  set_width for NetESubSignal.
 *
 * Revision 1.44  1999/07/07 04:20:57  steve
 *  Emit vvm for user defined tasks.
 *
 * Revision 1.43  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.42  1999/06/24 04:24:18  steve
 *  Handle expression widths for EEE and NEE operators,
 *  add named blocks and scope handling,
 *  add registers declared in named blocks.
 *
 * Revision 1.41  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.40  1999/06/13 23:51:16  steve
 *  l-value part select for procedural assignments.
 *
 * Revision 1.39  1999/06/13 17:30:23  steve
 *  More unary operators.
 *
 * Revision 1.38  1999/06/13 16:30:06  steve
 *  Unify the NetAssign constructors a bit.
 *
 * Revision 1.37  1999/06/09 03:00:06  steve
 *  Add support for procedural concatenation expression.
 *
 * Revision 1.36  1999/06/06 20:45:38  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.35  1999/06/03 05:16:25  steve
 *  Compile time evalutation of constant expressions.
 *
 * Revision 1.34  1999/06/02 15:38:46  steve
 *  Line information with nets.
 *
 * Revision 1.33  1999/05/30 01:11:46  steve
 *  Exressions are trees that can duplicate, and not DAGS.
 *
 * Revision 1.32  1999/05/27 04:13:08  steve
 *  Handle expression bit widths with non-fatal errors.
 *
 * Revision 1.31  1999/05/16 05:08:42  steve
 *  Redo constant expression detection to happen
 *  after parsing.
 *
 *  Parse more operators and expressions.
 *
 * Revision 1.30  1999/05/12 04:03:19  steve
 *  emit NetAssignMem objects in vvm target.
 *
 * Revision 1.29  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 */
#endif
