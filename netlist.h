#ifndef __netlist_H
#define __netlist_H
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
#ident "$Id: netlist.h,v 1.22 1999/03/15 02:43:32 steve Exp $"
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

class NetNode;
class NetProc;
class NetProcTop;
class NetExpr;
class ostream;


struct target;

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

      unsigned delay1() const { return delay1_; }
      unsigned delay2() const { return delay2_; }
      unsigned delay3() const { return delay3_; }

      void delay1(unsigned d) { delay1_ = d; }
      void delay2(unsigned d) { delay2_ = d; }
      void delay3(unsigned d) { delay3_ = d; }

      void set_attributes(const map<string,string>&);
      string attribute(const string&key) const;
      void attribute(const string&key, const string&value);

	// Return true if this has all the attributes in that and they
	// all have the same values.
      bool has_compat_attributes(const NetObj&that) const;

      bool test_mark() const { return mark_; }
      void set_mark(bool flag=true) { mark_ = flag; }

      Link&pin(unsigned idx) { return pins_[idx]; }
      const Link&pin(unsigned idx) const { return pins_[idx]; }

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
 * order to support ranges, or busses.
 */
class NetNet  : public NetObj {

    public:
      enum Type { IMPLICIT, WIRE, TRI, TRI1, SUPPLY0, WAND, TRIAND,
		  TRI0, SUPPLY1, WOR, TRIOR, REG };

      enum PortType { NOT_A_PORT, PIMPLICIT, PINPUT, POUTPUT, PINOUT };

      explicit NetNet(const string&n, Type t, unsigned npins =1)
      : NetObj(n, npins), sig_next_(0), sig_prev_(0), design_(0),
	type_(t), port_type_(NOT_A_PORT), msb_(npins-1), lsb_(0),
	local_flag_(false)
	    { ivalue_ = new verinum::V[npins];
	      for (unsigned idx = 0 ;  idx < npins ;  idx += 1)
		    ivalue_[idx] = verinum::Vz;
	    }

      explicit NetNet(const string&n, Type t, long ms, long ls)
      : NetObj(n, ((ms>ls)?ms-ls:ls-ms) + 1), sig_next_(0),
	sig_prev_(0), design_(0), type_(t), port_type_(NOT_A_PORT),
	msb_(ms), lsb_(ls), local_flag_(false)
	    { ivalue_ = new verinum::V[pin_count()];
	      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1)
		    ivalue_[idx] = verinum::Vz;
	    }

      virtual ~NetNet();

	// Every signal has a name, even if it is null.
      Type type() const { return type_; }
      void type(Type t) { type_ = t; }

      PortType port_type() const { return port_type_; }
      void port_type(PortType t) { port_type_ = t; }

      long msb() const { return msb_; }
      long lsb() const { return lsb_; }

      unsigned sb_to_idx(long sb) const
	    { if (msb_ >= lsb_)
		    return sb - lsb_;
	      else
		    return lsb_ - sb;
	    }

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
 *
 * The NetExpr::REF type can be used sort of like a pointer to
 * NetExpr objects. The NetExpr uses this list to know if it is still
 * being referenced, so can handle garbage collection. Also, this
 * trick can be used to replace subexpressions.
 */
class NetExpr {
    public:
      explicit NetExpr(unsigned w =0) : width_(w), reflist_(0)  { }
      virtual ~NetExpr() =0;

      virtual void expr_scan(struct expr_scan_t*) const =0;
      virtual void dump(ostream&) const;

      unsigned expr_width() const { return width_; }
      virtual void set_width(unsigned);

    public:
      class REF {
	    friend class NetExpr;
	  public:
	    void clr();
	    void set(NetExpr*);
	    NetExpr*ref() const { return ref_; }

	    void clr_and_delete()
		  { NetExpr*tmp = ref_;
		    clr();
		    if (tmp && tmp->is_referenced() == false)
			  delete tmp;
		  }

	    NetExpr*operator-> () const { return ref_; }
	    REF& operator=(NetExpr*that) { set(that); return *this; }

	    REF() : ref_(0), next_(0) { }
	    REF(NetExpr*that) : ref_(0), next_(0) { set(that); }
	    REF(const REF&that) : ref_(0), next_(0) { set(that.ref_); }
	    ~REF() { clr(); }
	  private:
	    NetExpr*ref_;
	    REF*next_;
	  private:// not implemented
	    REF& operator=(const REF&);
      };
      friend class NetExpr::REF;

	/* This method causes every item that references this object
	   to reference that object instead. When this complete,
	   no references to me will remain. */
      void substitute(NetExpr*that);

      bool is_referenced() const { return reflist_ != 0; }

    protected:
      void expr_width(unsigned w) { width_ = w; }

    private:
      unsigned width_;

      REF*reflist_;

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
 * The UDP can be combinational or sequential. The sequentianl UDP
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
 */
class NetAssign  : public NetProc, public NetNode, public LineInfo {
    public:
      explicit NetAssign(NetNet*lv, NetExpr*rv);
      ~NetAssign();

      const NetExpr*rval() const { return rval_.ref(); }

      void find_lval_range(const NetNet*&net, unsigned&msb,
			   unsigned&lsb) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      NetExpr::REF rval_;
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

      const NetExpr*expr() const { return expr_.ref(); }
      unsigned nitems() const { return nitems_; }

      const NetExpr*expr(unsigned idx) const { return items_[idx].guard.ref();}
      const NetProc*stat(unsigned idx) const { return items_[idx].statement; }

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:

      struct Item {
	    NetExpr::REF guard;
	    NetProc*statement;
      };

      NetExpr::REF expr_;
      unsigned nitems_;
      Item*items_;
};


/* A condit represents a conditional. It has an expression to test,
   and a pair of statements to select from. */
class NetCondit  : public NetProc {

    public:
      NetCondit(NetExpr*ex, NetProc*i, NetProc*e)
      : expr_(ex), if_(i), else_(e) { }

      NetExpr*expr() const { return expr_.ref(); }
      void emit_recurse_if(ostream&, struct target_t*) const;
      void emit_recurse_else(ostream&, struct target_t*) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr::REF expr_;
      NetProc*if_;
      NetProc*else_;
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
 * The NetPEvent is a NetNode that connects to the structural part of
 * the design. It has only inputs, which cause the side effect of
 * triggering an event that the procedural part of the design can use.
 */
class NetPEvent  : public NetProc, public NetNode {

    public:
      enum Type { ANYEDGE, POSEDGE, NEGEDGE, POSITIVE };

    public:
      NetPEvent(const string&ev, Type ed, NetProc*st)
      : NetNode(ev, 1), edge_(ed), statement_(st) { }

      Type   edge() const  { return edge_; }

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;
      virtual void dump_node(ostream&, unsigned ind) const;

      void emit_proc_recurse(ostream&, struct target_t*) const;

    private:
      Type edge_;
      NetProc*statement_;
};


/* The elaborator should expand all the user defined tasks in line, so
   this leaves the NetTask to represent activations of system tasks,
   or external tasks that are not known at compile time. */
class NetTask  : public NetProc {

    public:
      NetTask(const string&na, unsigned np)
      : name_(na), nparms_(np)
      { parms_ = new NetExpr::REF[nparms_]; }
      ~NetTask();

      const string& name() const { return name_; }

      unsigned nparms() const { return nparms_; }

      void parm(unsigned idx, NetExpr*p)
      { assert(idx < nparms_);
        parms_[idx] = p;
      }

      NetExpr* parm(unsigned idx) const
      { assert(idx < nparms_);
        return parms_[idx].ref();
      }

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      string name_;
      unsigned nparms_;
      NetExpr::REF*parms_;
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

      const NetExpr*expr() const { return cond_.ref(); }

      void emit_proc_recurse(ostream&, struct target_t*) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr::REF cond_;
      NetProc*proc_;
};


/* The is the top of any process. It carries the type (initial or
   always) and a pointer to the statement, probably a block, that
   makes up the process. */
class NetProcTop  : public LineInfo {

    public:
      enum Type { KINITIAL, KALWAYS };

      NetProcTop(Type t, NetProc*st) : type_(t), statement_(st) { }

      Type type() const { return type_; }
      const NetProc*statement() const { return statement_; }

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
 *   -  -- Arighmetic minus
 *   &  -- Bit-wise AND
 *   |  -- Bit-wise OR
 *   e  -- Logical equality (==)
 *   E  -- Case equality (===)
 *   n  -- Logical inequality (!=)
 *   N  -- Case inequality (!==)
 *   a  -- Logical AND (&&)
 *   o  -- Logical OR (||)
 */
class NetEBinary  : public NetExpr {

    public:
      NetEBinary(char op, NetExpr*l, NetExpr*r);
      ~NetEBinary();

      const NetExpr*left() const { return left_.ref(); }
      const NetExpr*right() const { return right_.ref(); }

      char op() const { return op_; }

      void set_width(unsigned w);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      char op_;
      NetExpr::REF left_;
      NetExpr::REF right_;
};

class NetEConst  : public NetExpr {

    public:
      NetEConst(const verinum&val)
      : NetExpr(val.len()), value_(val) { }
      ~NetEConst();

      const verinum&value() const { return value_; }

      virtual void set_width(unsigned w);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      verinum value_;
};

/*
 * This class represents a unaru operator, with the single operand
 * and a single character for the operator. The operator values are:
 *
 *   ~  -- Bit-wise negation
 *   !  -- Logical negation
 *   &  -- Reduction AND
 */
class NetEUnary  : public NetExpr {

    public:
      NetEUnary(char op, NetExpr*ex)
      : NetExpr(ex->expr_width()), op_(op), expr_(ex) { }
      ~NetEUnary();

      char op() const { return op_; }
      const NetExpr* expr() const { return expr_.ref(); }

      void set_width(unsigned w);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      char op_;
      NetExpr::REF expr_;
};

/* System identifiers are represented here. */
class NetEIdent  : public NetExpr {

    public:
      NetEIdent(const string&n, unsigned w)
      : NetExpr(w), name_(n) { }

      const string& name() const { return name_; }

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      string name_;
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
      ~NetESignal();

      const string& name() const { return NetNode::name(); }

      virtual void set_width(unsigned);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump(ostream&) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
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
      NetExpr*get_parameter(const string&name) const;

	// SIGNALS
      void add_signal(NetNet*);
      void del_signal(NetNet*);
      NetNet*find_signal(const string&name);

	// NODES
      void add_node(NetNode*);
      void del_node(NetNode*);

	// ESIGNALS
      NetESignal* get_esignal(NetNet*net);

	// PROCESSES
      void add_process(NetProcTop*);

	// Iterate over the design...
      void dump(ostream&) const;
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
      map<string,NetExpr::REF> parameters_;

	// List all the signals in the design.
      NetNet*signals_;

	// List the nodes in the design
      NetNode*nodes_;

	// List the processes in the design.
      NetProcTop*procs_;

      map<string,string> flags_;

	// Use this map to prevent duplicate signals.
      map<string,NetESignal*> esigs_;

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
 * Revision 1.22  1999/03/15 02:43:32  steve
 *  Support more operators, especially logical.
 *
 * Revision 1.21  1999/03/01 03:27:53  steve
 *  Prevent the duplicate allocation of ESignal objects.
 *
 * Revision 1.20  1999/02/21 17:01:57  steve
 *  Add support for module parameters.
 *
 * Revision 1.19  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 * Revision 1.18  1999/02/08 02:49:56  steve
 *  Turn the NetESignal into a NetNode so
 *  that it can connect to the netlist.
 *  Implement the case statement.
 *  Convince t-vvm to output code for
 *  the case statement.
 *
 * Revision 1.17  1999/02/03 04:20:11  steve
 *  Parse and elaborate the Verilog CASE statement.
 *
 * Revision 1.16  1999/02/01 00:26:49  steve
 *  Carry some line info to the netlist,
 *  Dump line numbers for processes.
 *  Elaborate prints errors about port vector
 *  width mismatch
 *  Emit better handles null statements.
 *
 * Revision 1.15  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 * Revision 1.14  1998/12/18 05:16:25  steve
 *  Parse more UDP input edge descriptions.
 *
 * Revision 1.13  1998/12/17 23:54:58  steve
 *  VVM support for small sequential UDP objects.
 *
 * Revision 1.12  1998/12/14 02:01:35  steve
 *  Fully elaborate Sequential UDP behavior.
 *
 * Revision 1.11  1998/12/07 04:53:17  steve
 *  Generate OBUF or IBUF attributes (and the gates
 *  to garry them) where a wire is a pad. This involved
 *  figuring out enough of the netlist to know when such
 *  was needed, and to generate new gates and signales
 *  to handle what's missing.
 *
 * Revision 1.10  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful will wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
 * Revision 1.9  1998/12/01 00:42:14  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.8  1998/11/23 00:20:23  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.7  1998/11/18 04:25:22  steve
 *  Add -f flags for generic flag key/values.
 *
 * Revision 1.6  1998/11/16 05:03:53  steve
 *  Add the sigfold function that unlinks excess
 *  signal nodes, and add the XNF target.
 *
 * Revision 1.5  1998/11/13 06:23:17  steve
 *  Introduce netlist optimizations with the
 *  cprop function to do constant propogation.
 *
 * Revision 1.4  1998/11/09 18:55:34  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.3  1998/11/07 19:17:10  steve
 *  Calculate expression widths at elaboration time.
 *
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:29:01  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
