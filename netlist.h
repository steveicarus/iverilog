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
#ident "$Id: netlist.h,v 1.7 1998/11/18 04:25:22 steve Exp $"
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
 */
class NetObj {

    public:
      class Link {
	    friend void connect(Link&, Link&);
	    friend class NetObj;

	  public:
	    Link() : next_(this), prev_(this) { }
	    ~Link() { unlink(); }

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

	      // Remove this link from the set of connected pins. The
	      // destructor will automatically do this if needed.
	    void unlink()
		  { next_->prev_ = prev_;
		    prev_->next_ = next_;
		    next_ = prev_ = this;
		  }

	      // Return true if this link is connected to anything else.
	    bool is_linked() const { return next_ != this; }

	  private:
	      // The NetNode manages these. They point back to the
	      // NetNode so that following the links can get me here.
	    NetObj *node_;
	    unsigned pin_;

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

      bool test_mark() const { return mark_; }
      void set_mark(bool flag=true) { mark_ = flag; }

      Link&pin(unsigned idx) { return pins_[idx]; }
      const Link&pin(unsigned idx) const { return pins_[idx]; }

      void dump_node_pins(ostream&, unsigned) const;


    private:
      string name_;
      Link*pins_;
      const unsigned npins_;
      unsigned delay1_;
      unsigned delay2_;
      unsigned delay3_;

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
	local_flag_(false) { }

      explicit NetNet(const string&n, Type t, long ms, long ls)
      : NetObj(n, ((ms>ls)?ms-ls:ls-ms) + 1), sig_next_(0),
	sig_prev_(0), design_(0), type_(t), port_type_(NOT_A_PORT),
	msb_(ms), lsb_(ls), local_flag_(false) { }

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
      : NetNode(n, 2) { }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual void emit_node(ostream&, struct target_t*) const;
};

class NetConst  : public NetNode {

    public:
      explicit NetConst(const string&n, verinum::V v)
      : NetNode(n, 1), value_(v) { }

      verinum::V value() const { return value_; }

      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      verinum::V value_;
};

/*
 * This class represents all manner of logic gates.
 */
class NetLogic  : public NetNode {

    public:
      enum TYPE { AND, NAND, NOR, NOT, OR, XNOR, XOR };

      explicit NetLogic(const string&n, unsigned pins, TYPE t)
      : NetNode(n, pins), type_(t) { }

      TYPE type() const { return type_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual void emit_node(ostream&, struct target_t*) const;

    private:
      const TYPE type_;
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

/* This is a procedural assignment. The lval is a register, and the
   assignment happens when the code is executed by the design. The
   node part of the NetAssign has as many pins as the width of the
   lvalue object. */
class NetAssign  : public NetProc, public NetNode {
    public:
      explicit NetAssign(NetNet*lv, NetExpr*rv);
      ~NetAssign();

      const NetNet* lval() const { return lval_; }
      const NetExpr*rval() const { return rval_; }

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void emit_node(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      NetNet*const lval_;
      NetExpr*const rval_;
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

/* A condit represents a conditional. It has an expression to test,
   and a pair of statements to select from. */
class NetCondit  : public NetProc {

    public:
      NetCondit(NetExpr*ex, NetProc*i, NetProc*e)
      : expr_(ex), if_(i), else_(e) { }

      NetExpr*expr() const { return expr_; }
      void emit_recurse_if(ostream&, struct target_t*) const;
      void emit_recurse_else(ostream&, struct target_t*) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr*expr_;
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
      { parms_ = new NetExpr*[nparms_];
        for (unsigned idx = 0 ;  idx < nparms_ ;  idx += 1)
	      parms_[idx] = 0;
      }
      ~NetTask();

      const string& name() const { return name_; }

      unsigned nparms() const { return nparms_; }

      void parm(unsigned idx, NetExpr*p)
      { assert(idx < nparms_);
        parms_[idx] = p;
      }

      NetExpr* parm(unsigned idx) const
      { assert(idx < nparms_);
        return parms_[idx];
      }

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      string name_;
      unsigned nparms_;
      NetExpr**parms_;
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

      NetExpr*expr() const { return cond_; }

      void emit_proc_recurse(ostream&, struct target_t*) const;

      virtual void emit_proc(ostream&, struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetExpr*cond_;
      NetProc*proc_;
};


/* The is the top of any process. It carries the type (initial or
   always) and a pointer to the statement, probably a block, that
   makes up the process. */
class NetProcTop {

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
class NetExpr {
    public:
      explicit NetExpr(unsigned w =0) : width_(w)  { }
      virtual ~NetExpr() =0;

      virtual void expr_scan(struct expr_scan_t*) const =0;
      virtual void dump(ostream&) const;

      unsigned expr_width() const { return width_; }
      virtual void set_width(unsigned);

    protected:
      void expr_width(unsigned w) { width_ = w; }

    private:
      unsigned width_;

    private: // not implemented
      NetExpr(const NetExpr&);
      NetExpr& operator=(const NetExpr&);
};

class NetEBinary  : public NetExpr {

    public:
      NetEBinary(char op, NetExpr*l, NetExpr*r);
      ~NetEBinary();

      const NetExpr*left() const { return left_; }
      const NetExpr*right() const { return right_; }

      char op() const { return op_; }

      void set_width(unsigned w);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      char op_;
      NetExpr*left_;
      NetExpr*right_;
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

class NetEUnary  : public NetExpr {

    public:
      NetEUnary(char op, NetExpr*ex)
      : NetExpr(ex->expr_width()), op_(op), expr_(ex) { }
      ~NetEUnary();

      char op() const { return op_; }
      const NetExpr* expr() const { return expr_; }

      void set_width(unsigned w);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      char op_;
      NetExpr*expr_;
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

/* When a signal shows up in an expression, this type represents
   it. From this the expression can get any kind of access to the
   structural signal. */
class NetESignal  : public NetExpr {

    public:
      NetESignal(NetNet*n)
      : NetExpr(n->pin_count()), sig_(n) { }
      ~NetESignal();

      const string& name() const { return sig_->name(); }

      virtual void set_width(unsigned);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      NetNet*sig_;
};

/*
 * This class contains an entire design. It includes processes and a
 * netlist, and can be passed around from function to function.
 */
class Design {

    public:
      Design() : signals_(0), nodes_(0), procs_(0) { }

	/* The flags are a generic way of accepting command line
	   parameters/flags and passing them to the processing steps
	   that deal with the design. The compilation driver sets the
	   entire flags map after elaboration is done. Subsequent
	   steps can then use the get_flag() function to get the value
	   of an interesting key. */

      void set_flags(const map<string,string>&f) { flags_ = f; }

      string get_flag(const string&key) const;


	// SIGNALS

      void add_signal(NetNet*);
      void del_signal(NetNet*);
      NetNet*find_signal(const string&name);

	// NODES
      void add_node(NetNode*);
      void del_node(NetNode*);

	// PROCESSES
      void add_process(NetProcTop*);

	// Iterate over the design...
      void dump(ostream&) const;
      void emit(ostream&, struct target_t*) const;

      void clear_node_marks();
      NetNode*find_node(bool (*test)(const NetNode*));

      void clear_signal_marks();
      NetNet*find_signal(bool (*test)(const NetNet*));

    private:
	// List all the signals in the design.
      NetNet*signals_;

	// List the nodes in the design
      NetNode*nodes_;

	// List the processes in the design.
      NetProcTop*procs_;

      map<string,string> flags_;

    private: // not implemented
      Design(const Design&);
      Design& operator= (const Design&);
};


/* =======
 */


/* Connect the pins of two nodes together. Either may already be
   connected to other things, connect is transitive. */
extern void connect(NetObj::Link&, NetObj::Link&);

/* Find the signal connected to the given node pin. There should
   always be exactly one signal. The bidx parameter get filled with
   the signal index of the Net, in case it is a vector. */
const NetNet* find_link_signal(const NetObj*net, unsigned pin,
			       unsigned&bidx);

inline ostream& operator << (ostream&o, const NetExpr&exp)
{ exp.dump(o); return o; }

/*
 * $Log: netlist.h,v $
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
