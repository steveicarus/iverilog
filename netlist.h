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
# include  <vector>
# include  <set>
# include  <utility>
# include  "ivl_target.h"
# include  "ivl_target_priv.h"
# include  "pform_types.h"
# include  "config.h"
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
class NetEvent;
class NetNet;
class NetNode;
class NetObj;
class NetPins;
class NetProc;
class NetProcTop;
class NetRelease;
class NetScope;
class NetEvProbe;
class NetExpr;
class NetEAccess;
class NetESignal;
class NetFuncDef;
class NetRamDq;
class NetTaskDef;
class NetEvTrig;
class NetEvWait;

struct target;
struct functor_t;

ostream& operator << (ostream&o, ivl_variable_type_t val);

extern void join_island(NetPins*obj);

class Link {

      friend void connect(Link&, Link&);
      friend void connect(Nexus*, Link&);
      friend class NetPins;
      friend class Nexus;

    public:
      enum DIR { PASSIVE, INPUT, OUTPUT };

      enum strength_t { HIGHZ, WEAK, PULL, STRONG, SUPPLY };

    private: // Only NetPins can create/delete Link objects
      Link();
      ~Link();

    public:
	// Manipulate the link direction.
      void set_dir(DIR d);
      DIR get_dir() const;

	// Set the delay for all the drivers to this nexus.
      void drivers_delays(NetExpr*rise, NetExpr*fall, NetExpr*decay);

	// A link has a drive strength for 0 and 1 values. The drive0
	// strength is for when the link has the value 0, and drive1
	// strength is for when the link has a value 1.
      void drive0(strength_t);
      void drive1(strength_t);

	// This sets the drives for all drivers of this link, and not
	// just the current link.
      void drivers_drive(strength_t d0, strength_t d1);

      strength_t drive0() const;
      strength_t drive1() const;

	// A link has an initial value that is used by the nexus to
	// figure out its initial value. Normally, only the object
	// that contains the link sets the initial value, and only the
	// attached Nexus gets it. The default link value is Vx.
      void set_init(verinum::V val);
      verinum::V get_init() const;

      void cur_link(NetPins*&net, unsigned &pin);
      void cur_link(const NetPins*&net, unsigned &pin) const;

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
      const NetPins*get_obj() const;
      NetPins*get_obj();
      unsigned get_pin() const;

    private:
	// The NetNode manages these. They point back to the
	// NetNode so that following the links can get me here.
      union {
	    NetPins *node_;
	    unsigned pin_;
      };

      bool pin_zero_     : 1;
      DIR dir_           : 2;
      strength_t drive0_ : 3;
      strength_t drive1_ : 3;
      verinum::V init_   : 2;

    private:
	// The Nexus uses these to maintain a single linked list of
	// Link objects. If this link is not connected to anything,
	// then these pointers are nil.
      Link *next_;
      Nexus*nexus_;

    private: // not implemented
      Link(const Link&);
      Link& operator= (const Link&);
};


class NetPins : public LineInfo {

    public:
      explicit NetPins(unsigned npins);
      virtual ~NetPins();

      unsigned pin_count() const { return npins_; }

      Link&pin(unsigned idx);
      const Link&pin(unsigned idx) const;

      void dump_node_pins(ostream&, unsigned, const char**pin_names =0) const;
      void set_default_dir(Link::DIR d);
      void set_default_init(verinum::V val);
      bool is_linked();
      bool pins_are_virtual(void) const;
      void devirtualize_pins(void);

    private:
      Link*pins_;
      const unsigned npins_;
      Link::DIR default_dir_;
      verinum::V default_init_;
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
class NetObj  : public NetPins, public Attrib {

    public:
    public:
	// The name of the object must be a permallocated string. A
	// lex_strings string, for example.
      explicit NetObj(NetScope*s, perm_string n, unsigned npins);
      virtual ~NetObj();

      NetScope* scope();
      const NetScope* scope() const;

      perm_string name() const { return name_; }

      const NetExpr* rise_time() const { return delay1_; }
      const NetExpr* fall_time() const { return delay2_; }
      const NetExpr* decay_time() const { return delay3_; }

      void rise_time(const NetExpr* d) { delay1_ = d; }
      void fall_time(const NetExpr* d) { delay2_ = d; }
      void decay_time(const NetExpr* d) { delay3_ = d; }

      void dump_obj_attr(ostream&, unsigned) const;

    private:
      NetScope*scope_;
      perm_string name_;
      const NetExpr* delay1_;
      const NetExpr* delay2_;
      const NetExpr* delay3_;
};

/*
* Objects that can be island branches are derived from this. (It is
* possible for an object to be a NetObj and an IslandBranch.) This is
* used to collect island information about the node.
*/

class IslandBranch {
    public:
      IslandBranch(ivl_discipline_t dis =0) : island_(0), discipline_(dis) { }

      ivl_island_t get_island() const { return island_; }

      friend void join_island(NetPins*);

    private:
      ivl_island_t island_;
      ivl_discipline_t discipline_;
};

/*
 * A NetBranch is a construct of Verilog-A that is a branch between
 * two nodes. The branch has exactly 2 pins and a discipline.
 *
 * pin(0) is the source of flow through a branch and the plus side of
 * potential. Pin(1) is the sink of flow and the minus (or ground) of
 * potential.
 */
class NetBranch  : public NetPins, public IslandBranch {

    public:
      explicit NetBranch(ivl_discipline_t dis);
      explicit NetBranch(ivl_discipline_t dis, perm_string name);
      ~NetBranch();

	// If the branch is named, this returns the name.
      perm_string name() const { return name_; }

      ivl_branch_s* target_obj() const { return &target_obj_; }

      void dump(ostream&, unsigned) const;

    private:
      perm_string name_;

      mutable ivl_branch_s target_obj_;

	// The design class uses this member to list the branches.
      friend class Design;
      NetBranch*next_;
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

      void drivers_delays(NetExpr*rise, NetExpr*fall, NetExpr*decay);
      void drivers_drive(Link::strength_t d0, Link::strength_t d1);

      Link*first_nlink();
      const Link* first_nlink()const;

	/* Get the width of the Nexus, or 0 if there are no vectors
	   (in the form of NetNet objects) linked. */
      unsigned vector_width() const;

      NetNet* pick_any_net();

	/* This method returns true if there are any assignments that
	   use this nexus as an l-value. This can be true if the nexus
	   is a variable, but also if this is a net with a force. */
      bool assign_lval() const;

	/* This method returns true if there are any drivers
	   (including variables) attached to this nexus. */
      bool drivers_present() const;

	/* This method returns true if all the possible drivers of
	   this nexus are constant. It will also return true if there
	   are no drivers at all. */
      bool drivers_constant() const;

	/* Given the nexus has constant drivers, this method returns
	   the value that has been driven. */
      verinum::V driven_value() const;

      ivl_nexus_t t_cookie() const;
      ivl_nexus_t t_cookie(ivl_nexus_t) const;

    private:
      Link*list_;
      void unlink(Link*);
      void relink(Link*);

      mutable char* name_; /* Cache the calculated name for the Nexus. */
      mutable ivl_nexus_t t_cookie_;

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
      unsigned nitems_;

      unsigned bsearch_(Nexus*that) const;

    private: // not implemented
      NexusSet(const NexusSet&);
      NexusSet& operator= (const NexusSet&);
};

/*
 * A NetBus is a transparent device that is merely a bunch of pins
 * used to tie some pins to. It is a convenient way to collect a
 * bundle of pins and pass that bundle around.
 */
class NetBus  : public NetObj {

    public:
      NetBus(NetScope*scope, unsigned pin_count);
      ~NetBus();

    private: // not implemented
      NetBus(const NetBus&);
      NetBus& operator= (const NetBus&);
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
 * A NetDelaySrc is an input-only device that calculates a path delay
 * based on the time that the inputs change. This class is used by the
 * NetNet class, and NetDelaySrc objects cannot exist outside of its
 * association with NetNet objects.
 */
class NetDelaySrc  : public NetObj {

    public:
      explicit NetDelaySrc(NetScope*s, perm_string n, unsigned nsrc,
                           bool condit_src, bool conditional);
      ~NetDelaySrc();

	// These functions set the delays from the values in the
	// source. These set_delays functions implement the various
	// rules wrt collections of transitions.

	// One transition specified.
      void set_delays(uint64_t del);
	// Two transitions: rise and fall
      void set_delays(uint64_t rise, uint64_t fall);
	// Three transitions
      void set_delays(uint64_t rise, uint64_t fall, uint64_t tz);
      void set_delays(uint64_t t01, uint64_t t10, uint64_t t0z,
		      uint64_t tz1, uint64_t t1z, uint64_t tz0);
      void set_delays(uint64_t t01, uint64_t t10, uint64_t t0z,
		      uint64_t tz1, uint64_t t1z, uint64_t tz0,
		      uint64_t t0x, uint64_t tx1, uint64_t t1x,
		      uint64_t tx0, uint64_t txz, uint64_t tzx);

      uint64_t get_delay(unsigned pe) const;

      void set_posedge();
      void set_negedge();
      bool is_posedge() const;
      bool is_negedge() const;

      unsigned src_count() const;
      Link&src_pin(unsigned);
      const Link&src_pin(unsigned) const;

      bool is_condit() const;
      bool has_condit() const;
      Link&condit_pin();
      const Link&condit_pin() const;

      void dump(ostream&, unsigned ind) const;

    private:
      uint64_t transition_delays_[12];
      bool condit_flag_;
      bool conditional_;
      bool posedge_;
      bool negedge_;

    private: // Not implemented
      NetDelaySrc(const NetDelaySrc&);
      NetDelaySrc& operator= (const NetDelaySrc&);
};

/*
 * NetNet is a special kind of NetObj that doesn't really do anything,
 * but carries the properties of the wire/reg/trireg, including its
 * name. Scalars and vectors are all the same thing here, a NetNet
 * with a single pin. The difference between a scalar and vector is
 * the width of the atomic vector datum it carries.
 *
 * NetNet objects can also appear as side effects of synthesis or
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
 * The pin of a NetNet object are PASSIVE: they do not drive
 * anything and they are not a data sink, per se. The pins follow the
 * values on the nexus.
 */
class NetNet  : public NetObj {

    public:
      enum Type { NONE, IMPLICIT, IMPLICIT_REG, INTEGER, WIRE, TRI, TRI1,
		  SUPPLY0, SUPPLY1, WAND, TRIAND, TRI0, WOR, TRIOR, REG,
		  UWIRE };

      enum PortType { NOT_A_PORT, PIMPLICIT, PINPUT, POUTPUT, PINOUT };

	// The width in this case is a shorthand for ms=width-1 and
	// ls=0. Only one pin is created, the width is of the vector
	// that passed through.
      explicit NetNet(NetScope*s, perm_string n, Type t, unsigned width =1);

	// This form supports an array of vectors. The [ms:ls] define
	// the base vector, and the [s0:e0] define the array
	// dimensions. If s0==e0, then this is not an array after
	// all.
      explicit NetNet(NetScope*s, perm_string n, Type t,
		      long ms, long ls);
      explicit NetNet(NetScope*s, perm_string n, Type t,
		      long ms, long ls, long s0, long e0);

      virtual ~NetNet();

      Type type() const;
      void type(Type t);

      PortType port_type() const;
      void port_type(PortType t);

      ivl_variable_type_t data_type() const;
      void data_type(ivl_variable_type_t t);

	/* If a NetNet is signed, then its value is to be treated as
	   signed. Otherwise, it is unsigned. */
      bool get_signed() const;
      void set_signed(bool);

	/* Used to maintain original type of net since integers are
	   implemented as 'reg signed [31:0]' in Icarus */
      bool get_isint() const;
      void set_isint(bool);

      bool get_scalar() const;
      void set_scalar(bool);

	/* Attach a discipline to the net. */
      ivl_discipline_t get_discipline() const;
      void set_discipline(ivl_discipline_t dis);

	/* These methods return the msb and lsb indices for the most
	   significant and least significant bits. These are signed
	   longs, and may be different from pin numbers. For example,
	   reg [1:8] has 8 bits, msb==1 and lsb==8. */
      long msb() const;
      long lsb() const;
      unsigned long vector_width() const;

	/* This method converts a signed index (the type that might be
	   found in the Verilog source) to a pin number. It accounts
	   for variation in the definition of the reg/wire/whatever. */
      long sb_to_idx(long sb) const;

	/* This method checks that the signed index is valid for this
	   signal. If it is, the above sb_to_idx can be used to get
	   the pin# from the index. */
      bool sb_is_valid(long sb) const;

	/* This method returns 0 for scalars and vectors, and greater
	   for arrays. The value is the number of array
	   indices. (Currently only one array index is supported.) */
      unsigned array_dimensions() const;
      long array_first() const;
      bool array_addr_swapped() const;

	// This is the number of array elements.
      unsigned array_count() const;

	// This method returns a 0 based address of an array entry as
	// indexed by idx. The Verilog source may give index ranges
	// that are not zero based.
      bool array_index_is_valid(long idx) const;
      unsigned array_index_to_address(long idx) const;

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

	/* Manage path delays */
      void add_delay_path(class NetDelaySrc*path);
      unsigned delay_paths(void) const;
      const class NetDelaySrc*delay_path(unsigned idx) const;

      virtual void dump_net(ostream&, unsigned) const;

      void initialize_value_and_dir(verinum::V init_value, Link::DIR dir);

    private:
      Type   type_;
      PortType port_type_;
      ivl_variable_type_t data_type_;
      bool signed_;
      bool isint_;		// original type of integer
      bool is_scalar_;
      ivl_discipline_t discipline_;

      long msb_, lsb_;
      const unsigned dimensions_;
      long s0_, e0_;

      bool local_flag_;
      unsigned eref_count_;
      unsigned lref_count_;

      vector<class NetDelaySrc*> delay_paths_;
};

/*
 * This object type is used to contain a logical scope within a
 * design. The scope doesn't represent any executable hardware, but is
 * just a handle that netlist processors can use to grab at the design.
 */
class NetScope : public Attrib {

    public:
      enum TYPE { MODULE, TASK, FUNC, BEGIN_END, FORK_JOIN, GENBLOCK };

	/* Create a new scope, and attach it to the given parent. The
	   name is expected to have been permallocated. */
      NetScope(NetScope*up, const hname_t&name, TYPE t);
      ~NetScope();

	/* Rename the scope using the name generated by inserting as
	   many pad characters as required between prefix and suffix
	   to make the name unique in the parent scope. Return false
	   if a unique name couldn't be generated. */
      bool auto_name(const char* prefix, char pad, const char* suffix);

	/* Parameters exist within a scope, and these methods allow
	   one to manipulate the set. In these cases, the name is the
	   *simple* name of the parameter, the hierarchy is implicit in
	   the scope. The return value from set_parameter is the
	   previous expression, if there was one. */

      struct range_t;
      NetExpr* set_parameter(perm_string name, NetExpr*val,
			     ivl_variable_type_t type,
			     NetExpr*msb, NetExpr*lsb, bool signed_flag,
			     NetScope::range_t*range_list,
			     const LineInfo&file_line);
      NetExpr* set_localparam(perm_string name, NetExpr*val,
			      const LineInfo&file_line);

      const NetExpr*get_parameter(const char* name,
				  const NetExpr*&msb,
				  const NetExpr*&lsb) const;

	/* These are used by defparam elaboration to replace the
	   expression with a new expression, without affecting the
	   range or signed_flag. Return false if the name does not
	   exist. */
      bool replace_parameter(perm_string name, NetExpr*val);

	/* These methods set or access events that live in this
	   scope. */

      void add_event(NetEvent*);
      void rem_event(NetEvent*);
      NetEvent*find_event(perm_string name);

	/* These methods add or find a genvar that lives in this scope. */
      void add_genvar(perm_string name, LineInfo *li);
      LineInfo* find_genvar(perm_string name);

	/* These methods manage signals. The add_ and rem_signal
	   methods are used by the NetNet objects to make themselves
	   available to the scope, and the find_signal method can be
	   used to locate signals within a scope. */

      void add_signal(NetNet*);
      void rem_signal(NetNet*);
      NetNet* find_signal(perm_string name);

	/* The parent and child() methods allow users of NetScope
	   objects to locate nearby scopes. */
      NetScope* parent();
      NetScope* child(const hname_t&name);
      const NetScope* parent() const;
      const NetScope* child(const hname_t&name) const;

      TYPE type() const;
      void print_type(ostream&) const;

      void set_task_def(NetTaskDef*);
      void set_func_def(NetFuncDef*);
      void set_module_name(perm_string);

      NetTaskDef* task_def();
      NetFuncDef* func_def();

      void set_line(perm_string file, perm_string def_file,
                    unsigned lineno, unsigned def_lineno);
      void set_line(perm_string file, unsigned lineno);
      void set_line(const LineInfo *info);
      perm_string get_file() const { return file_; };
      perm_string get_def_file() const { return def_file_; };
      unsigned get_lineno() const { return lineno_; };
      unsigned get_def_lineno() const { return def_lineno_; };

      bool in_func();
	/* Is the task or function automatic. */
      void is_auto(bool is_auto__) { is_auto_ = is_auto__; };
      bool is_auto() const { return is_auto_; };

	/* Is the module a cell (is in a `celldefine) */
      void is_cell(bool is_cell__) { is_cell_ = is_cell__; };
      bool is_cell() const { return is_cell_; };

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
      void time_from_timescale(bool);

      int time_unit() const;
      int time_precision() const;
      bool time_from_timescale() const;

      void default_nettype(NetNet::Type);
      NetNet::Type default_nettype() const;

	/* The fullname of the scope is the hierarchical name
	   component (which includes the name and array index) whereas
	   the basename is just my name. */
      perm_string basename() const;
      const hname_t& fullname() const { return name_; }

      void run_defparams(class Design*);
      void run_defparams_later(class Design*);

      void evaluate_parameters(class Design*);

	// Look for defparams that never matched, and print warnings.
      void residual_defparams(class Design*);

	/* This method generates a non-hierarchical name that is
	   guaranteed to be unique within this scope. */
      perm_string local_symbol();

      void dump(ostream&) const;
      void emit_scope(struct target_t*tgt) const;
      bool emit_defs(struct target_t*tgt) const;

	/* This method runs the functor on me. Recurse through the
	   children of this node as well. */
      void run_functor(Design*des, functor_t*fun);


	/* This member is used during elaboration to pass defparam
	   assignments from the scope pass to the parameter evaluation
	   step. After that, it is not used. */

      list<pair<pform_name_t,NetExpr*> > defparams;
      list<pair<list<hname_t>,NetExpr*> > defparams_later;

    public:
      struct range_t {
	    bool exclude_flag;
	      // Lower bound
	    bool low_open_flag;
	    NetExpr*low_expr;
	      // Upper bound
	    bool high_open_flag;
	    NetExpr*high_expr;
	      // Link to the next range specification
	    struct range_t*next;
      };

	/* After everything is all set up, the code generators like
	   access to these things to make up the parameter lists. */
      struct param_expr_t : public LineInfo {
	    param_expr_t() : type(IVL_VT_NO_TYPE), signed_flag(false), msb(0), lsb(0), range(0), expr(0) { }
	      // Type information
	    ivl_variable_type_t type;
	    bool signed_flag;
	    NetExpr*msb;
	    NetExpr*lsb;
	      // range constraints
	    struct range_t*range;
	      // Expression value
	    NetExpr*expr;
      };
      map<perm_string,param_expr_t>parameters;
      map<perm_string,param_expr_t>localparams;

      typedef map<perm_string,param_expr_t>::iterator param_ref_t;

      param_ref_t find_parameter(perm_string name);


      struct spec_val_t {
	    ivl_variable_type_t type;
	    union {
		  double real_val; // type == IVL_VT_REAL
		  long integer;    // type == IVL_VT_BOOL
	    };
      };
      map<perm_string,spec_val_t>specparams;

	/* Module instance arrays are collected here for access during
	   the multiple elaboration passes. */
      typedef vector<NetScope*> scope_vec_t;
      map<perm_string, scope_vec_t>instance_arrays;

	/* Loop generate uses this as scratch space during
	   elaboration. Expression evaluation can use this to match
	   names. */
      perm_string genvar_tmp;
      long genvar_tmp_val;

    private:
      void evaluate_parameter_logic_(Design*des, param_ref_t cur);
      void evaluate_parameter_real_(Design*des, param_ref_t cur);

    private:
      TYPE type_;
      hname_t name_;

      perm_string file_;
      perm_string def_file_;
      unsigned lineno_;
      unsigned def_lineno_;

      signed char time_unit_, time_prec_;
      bool time_from_timescale_;
      NetNet::Type default_nettype_;

      NetEvent *events_;

      map<perm_string,LineInfo*> genvars_;

      typedef std::map<perm_string,NetNet*>::const_iterator signals_map_iter_t;
      std::map <perm_string,NetNet*> signals_map_;
      perm_string module_name_;
      union {
	    NetTaskDef*task_;
	    NetFuncDef*func_;
      };

      NetScope*up_;
      NetScope*sib_;
      NetScope*sub_;

      unsigned lcounter_;
      bool is_auto_, is_cell_;
};

/*
 * This class implements the LPM_ABS component. The node has a single
 * input, a signed expression, that it converts to the absolute
 * value. The gate is simple: pin(0) is the output and pin(1) is the input.
 */
class NetAbs  : public NetNode {

    public:
      NetAbs(NetScope*s, perm_string n, unsigned width);
      ~NetAbs();

      unsigned width() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_;
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
	// operands and results).
      unsigned width() const;

      Link& pin_Cout();
      Link& pin_DataA();
      Link& pin_DataB();
      Link& pin_Result();

      const Link& pin_Cout() const;
      const Link& pin_DataA() const;
      const Link& pin_DataB() const;
      const Link& pin_Result() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_;
};

/*
 * The NetArrayDq node represents an array dereference. The NetNet
 * that this object refers to is an array, and the Address pin selects
 * which word of the array to place on the Result.
*/
class NetArrayDq  : public NetNode {

    public:
      NetArrayDq(NetScope*s, perm_string name, NetNet*mem, unsigned awid);
      ~NetArrayDq();

      unsigned width() const;
      unsigned awidth() const;
      unsigned size() const;
      const NetNet*mem() const;

      Link& pin_Address();
      Link& pin_Result();

      const Link& pin_Address() const;
      const Link& pin_Result() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      NetNet*mem_;
      unsigned awidth_;

};

/*
 * Convert an IVL_VT_REAL input to a logical value with the
 * given width. The input is pin(1) and the output is pin(0).
 */
class NetCastInt  : public NetNode {

    public:
      NetCastInt(NetScope*s, perm_string n, unsigned width);

      unsigned width() const { return width_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
};

/*
 * Convert an input to IVL_VT_REAL. The input is pin(1), which can be
 * any vector type (VT_BOOL or VT_LOGIC) and the output is pin(0),
 * which is IVL_VT_REAL. The conversion interprets the input as an
 * unsigned value unless the signed_flag is true.
 */
class NetCastReal  : public NetNode {

    public:
      NetCastReal(NetScope*s, perm_string n, bool signed_flag);

      bool signed_flag() const { return signed_flag_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      bool signed_flag_;
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

      Link& pin_Data();
      Link& pin_Result();
      Link& pin_Distance();

      const Link& pin_Data() const;
      const Link& pin_Result() const;
      const Link& pin_Distance() const;

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
 * inputs is narrower then the other, it is up to the generator to
 * make sure all the data pins are properly driven.
 *
 * The signed() property is true if the comparison is to be done to
 * signed arguments. The result is always UNsigned.
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

      Link& pin_AGB();
      Link& pin_AGEB();
      Link& pin_AEB();
      Link& pin_ANEB();
      Link& pin_ALB();
      Link& pin_ALEB();

      Link& pin_DataA();
      Link& pin_DataB();

      const Link& pin_AGB() const;
      const Link& pin_AGEB() const;
      const Link& pin_AEB() const;
      const Link& pin_ANEB() const;
      const Link& pin_ALB() const;
      const Link& pin_ALEB() const;

      const Link& pin_DataA() const;
      const Link& pin_DataB() const;

      virtual void functor_node(Design*, functor_t*);
      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
      bool signed_flag_;
};


/*
 * This node is a means to connect net inputs together to form a wider
 * vector. The output (pin 0) is a concatenation of the input vectors,
 * with pin-1 at the LSB, pin-2 next, and so on. This node is most
 * like the NetLogic node, as it has one output at pin 0 and the
 * remaining pins are the input that are combined to make the
 * output. It is separated out because it it generally a special case
 * for the code generators.
 *
 * When constructing the node, the width is the vector_width of the
 * output, and the cnt is the number of pins. (the number of input
 * vectors.)
 */
class NetConcat  : public NetNode {

    public:
      NetConcat(NetScope*scope, perm_string n, unsigned wid, unsigned cnt);
      ~NetConcat();

      unsigned width() const;

      void dump_node(ostream&, unsigned ind) const;
      bool emit_node(struct target_t*) const;

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
      NetDivide(NetScope*scope, perm_string n,
		unsigned width, unsigned wa, unsigned wb);
      ~NetDivide();

      unsigned width_r() const;
      unsigned width_a() const;
      unsigned width_b() const;

      void set_signed(bool);
      bool get_signed() const;

      Link& pin_DataA();
      Link& pin_DataB();
      Link& pin_Result();

      const Link& pin_DataA() const;
      const Link& pin_DataB() const;
      const Link& pin_Result() const;

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

      void set_signed(bool);
      bool get_signed() const;

      Link& pin_DataA();
      Link& pin_DataB();
      Link& pin_Result();

      const Link& pin_DataA() const;
      const Link& pin_DataB() const;
      const Link& pin_Result() const;

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
 * This class represents an LPM_FF device. There is no literal gate
 * type in Verilog that maps, but gates of this type can be inferred.
 */
class NetFF  : public NetNode {

    public:
      NetFF(NetScope*s, perm_string n, unsigned vector_width);
      ~NetFF();

      unsigned width() const;

      Link& pin_Clock();
      Link& pin_Enable();
      Link& pin_Aset();
      Link& pin_Aclr();
      Link& pin_Sset();
      Link& pin_Sclr();
      Link& pin_Data();
      Link& pin_Q();

      const Link& pin_Clock() const;
      const Link& pin_Enable() const;
      const Link& pin_Aset() const;
      const Link& pin_Aclr() const;
      const Link& pin_Sset() const;
      const Link& pin_Sclr() const;
      const Link& pin_Data() const;
      const Link& pin_Q() const;

      void aset_value(const verinum&val);
      const verinum& aset_value() const;

      void sset_value(const verinum&val);
      const verinum& sset_value() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_;
      verinum aset_value_;
      verinum sset_value_;
};

/*
 * This class implements a basic LPM_MULT combinational multiplier. It
 * is used as a structural representation of the * operator. The
 * device has inputs A and B and output Result all with independent
 * widths.
 *
 * NOTE: Check this width thing. I think that the independence of the
 * widths is not necessary or even useful.
 */
class NetMult  : public NetNode {

    public:
      NetMult(NetScope*sc, perm_string n, unsigned width,
	      unsigned wa, unsigned wb);
      ~NetMult();

      bool get_signed() const;
      void set_signed(bool);

	// Get the width of the device bussed inputs. There are these
	// parameterized widths:
      unsigned width_r() const; // Result
      unsigned width_a() const; // DataA
      unsigned width_b() const; // DataB

      Link& pin_DataA();
      Link& pin_DataB();
      Link& pin_Result();

      const Link& pin_DataA() const;
      const Link& pin_DataB() const;
      const Link& pin_Result() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      bool signed_;
      unsigned width_r_;
      unsigned width_a_;
      unsigned width_b_;
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
 *
 * All the data inputs must have the same type, and are the type of
 * the result. The actual type does not matter, as the mux does not
 * process data, just selects alternatives.
 *
 * The select input must be an integral type of some sort. Not real.
 */
class NetMux  : public NetNode {

    public:
      NetMux(NetScope*scope, perm_string n,
	     unsigned width, unsigned size, unsigned selw);
      ~NetMux();

      unsigned width() const;
      unsigned size() const;
      unsigned sel_width() const;

      Link& pin_Result();
      Link& pin_Data(unsigned si);
      Link& pin_Sel();

      const Link& pin_Result() const;
      const Link& pin_Data(unsigned) const;
      const Link& pin_Sel() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_;
      unsigned size_;
      unsigned swidth_;
};


/*
 * This class implements a basic LPM_POW combinational power. It
 * is used as a structural representation of the ** operator. The
 * device has inputs A and B and output Result all with independent
 * widths.
 *
 * NOTE: Check this width thing. I think that the independence of the
 * widths is not necessary or even useful.
 */
class NetPow  : public NetNode {

    public:
      NetPow(NetScope*sc, perm_string n, unsigned width,
	      unsigned wa, unsigned wb);
      ~NetPow();

      bool get_signed() const;
      void set_signed(bool);

	// Get the width of the device bussed inputs. There are these
	// parameterized widths:
      unsigned width_r() const; // Result
      unsigned width_a() const; // DataA
      unsigned width_b() const; // DataB

      Link& pin_DataA();
      Link& pin_DataB();
      Link& pin_Result();

      const Link& pin_DataA() const;
      const Link& pin_DataB() const;
      const Link& pin_Result() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      bool signed_;
      unsigned width_r_;
      unsigned width_a_;
      unsigned width_b_;
};


/*
 * The NetReplicate node takes a vector input and makes it into a larger
 * vector by repeating the input vector some number of times. The
 * repeat count is a fixed value. This is just like the repeat
 * concatenation of Verilog: {<repeat>{<vector>}}.
 *
 * When constructing this node, the wid is the vector width of the
 * output, and the rpt is the repeat count. The wid must be an even
 * multiple of the cnt, and wid/cnt is the expected input width.
 *
 * The device has exactly 2 pins: pin(0) is the output and pin(1) the
 * input.
 */
class NetReplicate  : public NetNode {

    public:
      NetReplicate(NetScope*scope, perm_string n, unsigned wid, unsigned rpt);
      ~NetReplicate();

      unsigned width() const;
      unsigned repeat() const;

      void dump_node(ostream&, unsigned ind) const;
      bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
      unsigned repeat_;
};

/*
 * This node represents the call of a user defined function in a
 * structural context. The pin count is the same as the port count,
 * with pin0 the return value.
 */
class NetUserFunc  : public NetNode {

    public:
      NetUserFunc(NetScope*s, perm_string n, NetScope*def, NetEvWait*trigger__);
      ~NetUserFunc();

      ivl_variable_type_t data_type(unsigned port) const;
      unsigned port_width(unsigned port) const;

      const NetScope* def() const;

      const NetEvWait* trigger() const { return trigger_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      NetScope*def_;
      NetEvWait*trigger_;
};

/*
 * The number of ports includes the return value, so will always be at
 * least 1.
 */
class NetSysFunc  : public NetNode {

    public:
      NetSysFunc(NetScope*s, perm_string n,
		 const struct sfunc_return_type*def,
		 unsigned ports, NetEvWait*trigger__);
      ~NetSysFunc();

      ivl_variable_type_t data_type() const;
      unsigned vector_width() const;
      const char* func_name() const;

      const NetEvWait* trigger() const { return trigger_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      const struct sfunc_return_type*def_;
      NetEvWait*trigger_;
};

class NetTran  : public NetNode, public IslandBranch {

    public:
	// Tran devices other than TRAN_VP
      NetTran(NetScope*scope, perm_string n, ivl_switch_type_t type);
	// Create a TRAN_VP
      NetTran(NetScope*scope, perm_string n, unsigned wid,
	      unsigned part, unsigned off);
      ~NetTran();

      ivl_switch_type_t type() const { return type_; }

	// These are only used for IVL_SW_TRAN_PV
      unsigned vector_width() const;
      unsigned part_width() const;
      unsigned part_offset() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      ivl_switch_type_t type_;
      unsigned wid_;
      unsigned part_;
      unsigned off_;
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

	// Expressions have type.
      virtual ivl_variable_type_t expr_type() const;

	// How wide am I?
      unsigned expr_width() const { return width_; }

	// Coerce the expression to have a specific width. If the
	// coercion works, then return true. Otherwise, return false.
	// A coercion will work or not depending on the implementation
	// in the derived class. Normally, the width will be set if
	// the expression is:
	//    - already the requested size, OR
	//    - otherwise unsized.
	// Normally, the resize will not allow a width size that loses
	// data. For example, it will not reduce a constant expression
	// to the point where significant bits are lost. But if the
	// last_chance flag is true, then the method assumes that high
	// bits will be lost anyhow, so try harder. Loss will be
	// allowed, but it still won't resize fixed size expressions
	// such as vector signals. This flag is meant to be used by
	// elaboration of procedural assignment to set the expression
	// width to the l-value width, if possible.
      virtual bool set_width(unsigned wid, bool last_chance =false);

	// This method returns true if the expression is
	// signed. Unsigned expressions return false.
      bool has_sign() const;
      virtual void cast_signed(bool flag);

	// This returns true if the expression has a definite
	// width. This is generally true, but in some cases the
	// expression is amorphous and desires a width from its
	// environment. For example, 'd5 has indefinite width, but
	// 5'd5 has a definite width.

	// This method is only really used within concatenation
	// expressions to check validity.
      virtual bool has_width() const;

	// Expressions in parameter declarations may have encountered
	// arguments that are themselves untyped parameters. These
	// cannot be fully resolved for type when elaborated (they are
	// elaborated before all parameter overrides are complete) so
	// this virtual method needs to be called right before
	// evaluating the expression. This wraps up the evaluation of
	// the type.
      virtual void resolve_pexpr_type();

	// This method evaluates the expression and returns an
	// equivalent expression that is reduced as far as compile
	// time knows how. Essentially, this is designed to fold
	// constants.
	//
	// The prune_to_width is the maximum width that the result
	// should be. If it is 0 or -1, then do not prune the
	// result. If it is -1, go through special efforts to preserve
	// values that may expand. A width of 0 corresponds to a
	// self-determined context, and a width of -1 corresponds to
	// an infinitely wide context.
      virtual NetExpr*eval_tree(int prune_to_width = -1);

	// Make a duplicate of myself, and subexpressions if I have
	// any. This is a deep copy operation.
      virtual NetExpr*dup_expr() const =0;

	// Get the Nexus that are the input to this
	// expression. Normally this descends down to the reference to
	// a signal that reads from its input.
      virtual NexusSet* nex_input(bool rem_out = true) =0;

	// Return a version of myself that is structural. This is used
	// for converting expressions to gates. The arguments are:
	//
	//  des, scope:  The context where this work is done
	//
        //  root: The root expression of which this expression is a part.
        //
	//  rise/fall/decay: Attach these delays to the driver for the
	//                   expression output.
	//
	//  drive0/drive1: Attach these strengths tp the driver for
	//                 the expression output.
      virtual NetNet*synthesize(Design*des, NetScope*scope, NetExpr*root);


    protected:
      void expr_width(unsigned w);
      void cast_signed_base_(bool flag) {signed_flag_ = flag; }

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

      virtual bool set_width(unsigned w, bool last_chance =false);
      virtual void cast_signed(bool sign_flag);
      virtual bool has_width() const;
      virtual ivl_variable_type_t expr_type() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetEConst* dup_expr() const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*);
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

      virtual bool set_width(unsigned w, bool last_chance =false);
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
      virtual bool set_width(unsigned w, bool last_chance);

	// This type has no self-determined width. This is false.
      virtual bool has_width() const;

	// The type of this expression is ET_REAL
      ivl_variable_type_t expr_type() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

      virtual NetECReal* dup_expr() const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*);
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
 * The NetPartSelect device represents a netlist part select of a
 * signal vector. Pin 0 is a vector that is a part select of pin 1,
 * which connected to the NetNet of the signal being selected from.
 *
 * The part to be selected is the canonical (0-based) offset and the
 * specified number of bits (wid).
 *
 * If the offset is non-constant, then pin(2) is the input vector for
 * the selector. If this pin is present, then use the non-constant
 * selector as the input.
 *
 * The NetPartSelect can be output from the signal (i.e. reading a
 * part) or input into the signal. The DIR method gives the type of
 * the node.
 *
 * VP (Vector-to-Part)
 *  Output pin 0 is the part select, and input pin 1 is connected to
 *  the NetNet object.
 *
 * PV (Part-to-Vector)
 *  Output pin 1 is connected to the NetNet, and input pin 0 is the
 *  part select. In this case, the node is driving the NetNet.
 *
 * Note that whatever the direction that data is intended to flow,
 * pin-0 is the part select and pin-1 is connected to the NetNet.
 */
class NetPartSelect  : public NetNode {

    public:
	// enum for the device direction
      enum dir_t { VP, PV};

      explicit NetPartSelect(NetNet*sig,
			     unsigned off, unsigned wid, dir_t dir);
      explicit NetPartSelect(NetNet*sig, NetNet*sel,
			     unsigned wid, bool signed_flag__ = false);
      ~NetPartSelect();

      unsigned base()  const;
      unsigned width() const;
      dir_t    dir()   const;
	/* Is the select signal signed? */
      bool signed_flag() const { return signed_flag_; }

      virtual void dump_node(ostream&, unsigned ind) const;
      bool emit_node(struct target_t*tgt) const;

    private:
      unsigned off_;
      unsigned wid_;
      dir_t    dir_;
      bool signed_flag_;
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
      explicit NetBUFZ(NetScope*s, perm_string n, unsigned wid);
      ~NetBUFZ();

      unsigned width() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
};

/*
 * This node is used to represent case equality in combinational
 * logic. Although this is not normally synthesizable, it makes sense
 * to support an abstract gate that can compare x and z. This node
 * always generates a single bit result, no matter the width of the
 * input. The elaboration, btw, needs to make sure the input widths
 * match.
 *
 * This pins are assigned as:
 *
 *     0   -- Output (always returns 0 or 1)
 *     1   -- Input
 *     2   -- Input
 */
class NetCaseCmp  : public NetNode {

    public:
      explicit NetCaseCmp(NetScope*s, perm_string n, unsigned wid, bool eeq);
      ~NetCaseCmp();

      unsigned width() const;
	// true if this is ===, false if this is !==
      bool eeq() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
      bool eeq_;
};

/* NOTE: This class should be replaced with the NetLiteral class
 * below, that is more general in that it supports different types of
 * values.
 *
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
      unsigned width() const;

      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      unsigned width_;
      verinum::V*value_;
};

/*
 * This class represents instances of the LPM_CONSTANT device. The
 * node has only outputs and a constant value. The width is available
 * by getting the pin_count(), and the value bits are available one at
 * a time. There is no meaning to the aggregation of bits to form a
 * wide NetConst object, although some targets may have an easier time
 * detecting interesting constructs if they are combined.
 */
class NetLiteral  : public NetNode {

    public:
	// A read-valued literal.
      explicit NetLiteral(NetScope*s, perm_string n, const verireal&val);
      ~NetLiteral();

      ivl_variable_type_t data_type() const;

      const verireal& value_real() const;

      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);
      virtual void dump_node(ostream&, unsigned ind) const;

    private:
      verireal real_;
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
 *
 * All these devices process vectors bitwise, so each bit can be
 * logically separated. The exception is the CONCAT gate, which is
 * really an abstract gate that takes the inputs and turns it into a
 * vector of bits.
 */
class NetLogic  : public NetNode {

    public:
      enum TYPE { AND, BUF, BUFIF0, BUFIF1, CMOS, NAND, NMOS, NOR, NOT,
		  NOTIF0, NOTIF1, OR, PULLDOWN, PULLUP, RCMOS, RNMOS, RPMOS,
		  PMOS, XNOR, XOR };

      explicit NetLogic(NetScope*s, perm_string n, unsigned pins,
			TYPE t, unsigned wid);

      TYPE type() const;
      unsigned width() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);

    private:
      TYPE type_;
      unsigned width_;
};

/*
 * This class represents a structural sign extension. The pin-0 is a
 * vector of the input pin-1 sign-extended. The input is taken to be
 * signed. This generally matches a hardware implementation of
 * replicating the top bit enough times to create the desired output
 * width.
 */
class NetSignExtend  : public NetNode {

    public:
      explicit NetSignExtend(NetScope*s, perm_string n, unsigned wid);
      ~NetSignExtend();

      unsigned width() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);

    private:
      unsigned width_;
};

/*
 * This class represents *reduction* logic operators. Certain boolean
 * logic operators have reduction forms which take in a vector and
 * return a single bit that is calculated by applying the logic
 * operation through the width of the input vector. These correspond
 * to reduction unary operators in Verilog.
 */
class NetUReduce  : public NetNode {

    public:
      enum TYPE {NONE, AND, OR, XOR, NAND, NOR, XNOR};

      NetUReduce(NetScope*s, perm_string n, TYPE t, unsigned wid);

      TYPE type() const;
      unsigned width() const;

      virtual void dump_node(ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);

    private:
      TYPE type_;
      unsigned width_;
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

enum DelayType { NO_DELAY, ZERO_DELAY, POSSIBLE_DELAY, DEFINITE_DELAY };

/* =========
 * A process is a behavioral-model description. A process is a
 * statement that may be compound. The various statement types may
 * refer to places in a netlist (by pointing to nodes) but is not
 * linked into the netlist. However, elaborating a process may cause
 * special nodes to be created to handle things like events.
 */
class NetProc : public virtual LineInfo {

    public:
      explicit NetProc();
      virtual ~NetProc();

	// Find the nexa that are input by the statement. This is used
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

	// Synthesize as asynchronous logic, and return true.
      virtual bool synth_async(Design*des, NetScope*scope,
			       const NetBus&nex_map, NetBus&nex_out);

      virtual bool synth_sync(Design*des, NetScope*scope, NetFF*ff,
			      const NetBus&nex_map, NetBus&nex_out,
			      const svector<NetEvProbe*>&events);

      virtual void dump(ostream&, unsigned ind) const;

	// Recursively checks to see if there is delay in this element.
      virtual DelayType delay_type() const;

    private:
      friend class NetBlock;
      NetProc*next_;

    private: // not implemented
      NetProc(const NetProc&);
      NetProc& operator= (const NetProc&);
};

class NetAlloc  : public NetProc {

    public:
      NetAlloc(NetScope*);
      ~NetAlloc();

      const string name() const;

      const NetScope* scope() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetScope*scope_;
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
      ~NetAssign_();

	// If this expression exists, then it is used to select a word
	// from an array/memory.
      NetExpr*word();
      const NetExpr*word() const;

	// Get the base index of the part select, or 0 if there is no
	// part select.
      const NetExpr* get_base() const;

      void set_word(NetExpr*);
      void set_part(NetExpr* loff, unsigned wid);

	// Get the width of the r-value that this node expects. This
	// method accounts for the presence of the mux, so it is not
	// necessarily the same as the pin_count().
      unsigned lwidth() const;
      ivl_variable_type_t expr_type() const;

	// Get the name of the underlying object.
      perm_string name() const;

      NetNet* sig() const;

	// Mark that the synthesizer has worked with this l-value, so
	// when it is released, the l-value signal should be turned
	// into a wire.
      void turn_sig_to_wire_on_release();

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
	// Memory word index
      NetExpr*word_;

      bool turn_sig_to_wire_on_release_;
	// indexed part select base
      NetExpr*base_;
      unsigned lwid_;
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

      bool synth_async(Design*des, NetScope*scope,
		       const NetBus&nex_map, NetBus&nex_out);

	// This dumps all the lval structures.
      void dump_lval(ostream&) const;
      virtual void dump(ostream&, unsigned ind) const;

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
      explicit NetAssignNB(NetAssign_*lv, NetExpr*rv, NetEvWait*ev,
                           NetExpr*cnt);
      ~NetAssignNB();


      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;

      unsigned nevents() const;
      const NetEvent*event(unsigned) const;
      const NetExpr* get_count() const;

    private:
      NetEvWait*event_;
      NetExpr*count_;
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
      bool synth_async(Design*des, NetScope*scope,
		       const NetBus&nex_map, NetBus&nex_out);

      bool synth_sync(Design*des, NetScope*scope, NetFF*ff,
		      const NetBus&nex_map, NetBus&nex_out,
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
      virtual DelayType delay_type() const;

    private:
      const Type type_;
      NetScope*subscope_;

      NetProc*last_;
};

/*
 * A CASE statement in the Verilog source leads, eventually, to one of
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

      bool synth_async(Design*des, NetScope*scope,
		       const NetBus&nex_map, NetBus&nex_out);

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;
      virtual DelayType delay_type() const;

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
 * know what that means.
 */
class NetCAssign  : public NetAssignBase {

    public:
      explicit NetCAssign(NetAssign_*lv, NetExpr*rv);
      ~NetCAssign();

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void dump(ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;

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
      bool synth_async(Design*des, NetScope*scope,
		       const NetBus&nex_map, NetBus&nex_out);

      bool synth_sync(Design*des, NetScope*scope, NetFF*ff,
		      const NetBus&nex_map, NetBus&nex_out,
		      const svector<NetEvProbe*>&events);

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(ostream&, unsigned ind) const;
      virtual DelayType delay_type() const;

    private:
      NetExpr* expr_;
      NetProc*if_;
      NetProc*else_;
};

/*
 * This represents the analog contribution statement. The l-val is a
 * branch expression, and the r-value is an arbitrary expression that
 * may include branches and real values.
 */
class NetContribution : public NetProc {

    public:
      explicit NetContribution(NetEAccess*lval, NetExpr*rval);
      ~NetContribution();

      const NetEAccess* lval() const;
      const NetExpr* rval() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetEAccess*lval_;
      NetExpr*rval_;
};

/*
 * The procedural deassign statement (the opposite of assign) releases
 * any assign expressions attached to the bits of the reg. The
 * lval is the expression of the "deassign <expr>;" statement with the
 * expr elaborated to a net.
 */
class NetDeassign : public NetAssignBase {

    public:
      explicit NetDeassign(NetAssign_*l);
      ~NetDeassign();

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

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
 * statement causes the referenced event to be triggered, which in
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
      NetEvTrig* trig_;

	// Use This member to count references by NetEvWait objects.
      unsigned waitref_;
      struct wcell_ {
	    NetEvWait*obj;
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

      virtual bool synth_async(Design*des, NetScope*scope,
			       const NetBus&nex_map, NetBus&nex_out);

      virtual bool synth_sync(Design*des, NetScope*scope, NetFF*ff,
			      const NetBus&nex_map, NetBus&nex_out,
			      const svector<NetEvProbe*>&events);

      virtual void dump(ostream&, unsigned ind) const;
	// This will ignore any statement.
      virtual void dump_inline(ostream&) const;
      virtual DelayType delay_type() const;

    private:
      NetProc*statement_;

      unsigned nevents_;
      NetEvent**events_;
};

ostream& operator << (ostream&out, const NetEvWait&obj);

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
 * know what that means.
 */
class NetForce  : public NetAssignBase {

    public:
      explicit NetForce(NetAssign_*l, NetExpr*r);
      ~NetForce();

      virtual NexusSet* nex_input(bool rem_out = true);

      virtual void dump(ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
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
      virtual DelayType delay_type() const;

    private:
      NetProc*statement_;
};

class NetFree   : public NetProc {

    public:
      NetFree(NetScope*);
      ~NetFree();

      const string name() const;

      const NetScope* scope() const;

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
      NetScope*scope_;
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
      ~NetFuncDef();

      void set_proc(NetProc*st);

	//const string name() const;
      const NetProc*proc() const;
      const NetScope*scope() const;
      NetScope*scope();

      unsigned port_count() const;
      const NetNet*port(unsigned idx) const;

      const NetNet*return_sig() const;

      void dump(ostream&, unsigned ind) const;

    private:
      NetScope*scope_;
      NetProc*statement_;
      NetNet*result_sig_;
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
      NetPDelay(uint64_t d, NetProc*st);
      NetPDelay(NetExpr* d, NetProc*st);
      ~NetPDelay();

      uint64_t delay() const;
      const NetExpr*expr() const;

      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void nex_output(NexusSet&);

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;
      virtual DelayType delay_type() const;

      bool emit_proc_recurse(struct target_t*) const;

    private:
      uint64_t delay_;
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
      virtual DelayType delay_type() const;

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
class NetRelease : public NetAssignBase {

    public:
      explicit NetRelease(NetAssign_*l);
      ~NetRelease();

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(ostream&, unsigned ind) const;

    private:
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
      NetTaskDef(NetScope*n, const svector<NetNet*>&po);
      ~NetTaskDef();

      void set_proc(NetProc*p);

	//const string& name() const;
      const NetScope* scope() const;
      const NetProc*proc() const;

      unsigned port_count() const;
      NetNet*port(unsigned idx);

      void dump(ostream&, unsigned) const;
      DelayType delay_type() const;

    private:
      NetScope*scope_;
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
 */
class NetEUFunc  : public NetExpr {

    public:
      NetEUFunc(NetScope*, NetScope*, NetESignal*, svector<NetExpr*>&);
      ~NetEUFunc();

      const NetESignal*result_sig() const;

      unsigned parm_count() const;
      const NetExpr* parm(unsigned idx) const;

      const NetScope* func() const;

      virtual bool set_width(unsigned, bool last_chance);
      virtual ivl_variable_type_t expr_type() const;
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEUFunc*dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true);
      virtual NetExpr* eval_tree(int prune_to_width = -1);
      virtual NetNet* synthesize(Design*des, NetScope*scope, NetExpr*root);

    private:
      NetScope*scope_;
      NetScope*func_;
      NetESignal*result_sig_;
      svector<NetExpr*> parms_;

    private: // not implemented
      NetEUFunc(const NetEUFunc&);
      NetEUFunc& operator= (const NetEUFunc&);
};

/*
 * A call to a nature access function for a branch.
 */
class NetEAccess : public NetExpr {

    public:
      explicit NetEAccess(NetBranch*br, ivl_nature_t nat);
      ~NetEAccess();

      ivl_nature_t get_nature() const { return nature_; }
      NetBranch*   get_branch() const { return branch_; }

      virtual ivl_variable_type_t expr_type() const;
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEAccess*dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true);

    private:
      NetBranch*branch_;
      ivl_nature_t nature_;
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
      virtual DelayType delay_type() const;

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
      virtual DelayType delay_type() const;

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
      NetProcTop(NetScope*s, ivl_process_type_t t, class NetProc*st);
      ~NetProcTop();

      ivl_process_type_t type() const { return type_; }
      NetProc*statement();
      const NetProc*statement() const;

      NetScope*scope();
      const NetScope*scope() const;

	/* Return true if this process represents combinational logic. */
      bool is_asynchronous();

	/* Create asynchronous logic from this thread and return true,
	   or return false if that cannot be done. */
      bool synth_async(Design*des);

	/* Return true if this process represents synchronous logic. */
      bool is_synchronous();

	/* Create synchronous logic from this thread and return true,
	   or return false if that cannot be done. */
      bool synth_sync(Design*des);

      void dump(ostream&, unsigned ind) const;
      bool emit(struct target_t*tgt) const;

    private:
      const ivl_process_type_t type_;
      NetProc*const statement_;

      NetScope*scope_;
      friend class Design;
      NetProcTop*next_;
};

class NetAnalogTop  : public LineInfo, public Attrib {

    public:
      NetAnalogTop(NetScope*scope, ivl_process_type_t t, NetProc*st);
      ~NetAnalogTop();

      ivl_process_type_t type() const { return type_; }

      NetProc*statement();
      const NetProc*statement() const;

      NetScope*scope();
      const NetScope*scope() const;

      void dump(ostream&, unsigned ind) const;
      bool emit(struct target_t*tgt) const;

    private:
      const ivl_process_type_t type_;
      NetProc* statement_;

      NetScope*scope_;
      friend class Design;
      NetAnalogTop*next_;
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
 *   p  -- Arithmetic power (**)
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
 *   m  -- min(a,b)
 *   M  -- max(a,b)
 */
class NetEBinary  : public NetExpr {

    public:
      NetEBinary(char op, NetExpr*l, NetExpr*r);
      ~NetEBinary();

      const NetExpr*left() const { return left_; }
      const NetExpr*right() const { return right_; }

      char op() const { return op_; }

      virtual bool set_width(unsigned w, bool last_chance =false);

	// A binary expression node only has a definite
	// self-determinable width if the operands both have definite
	// widths.
      virtual bool has_width() const;

      virtual void resolve_pexpr_type();
      virtual NetEBinary* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true);

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    protected:
      char op_;
      NetExpr* left_;
      NetExpr* right_;

      bool get_real_arguments_(verireal&lv, verireal&rv);
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
      NetEBAdd(char op, NetExpr*l, NetExpr*r, bool lossless_flag =false);
      ~NetEBAdd();

      virtual ivl_variable_type_t expr_type() const;

      virtual bool set_width(unsigned w, bool last_chance);
      virtual void cast_signed(bool sign_flag);
      virtual NetEBAdd* dup_expr() const;
      virtual NetExpr* eval_tree(int prune_to_width = -1);
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      NetECReal* eval_tree_real_();
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

      virtual ivl_variable_type_t expr_type() const;

      virtual bool set_width(unsigned w, bool last_chance);
      virtual void cast_signed(bool sign_flag);
      virtual NetEBDiv* dup_expr() const;
      virtual NetExpr* eval_tree(int prune_to_width = -1);
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      NetExpr* eval_tree_real_();
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

      virtual bool set_width(unsigned w, bool last_chance);
      virtual NetEBBits* dup_expr() const;
      virtual NetEConst* eval_tree(int prune_to_width = -1);

      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);
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

      virtual bool set_width(unsigned w, bool last_chance =false);

	/* A compare expression has a definite width. */
      virtual bool has_width() const;
      virtual ivl_variable_type_t expr_type() const;
      virtual NetEBComp* dup_expr() const;
      virtual NetEConst* eval_tree(int prune_to_width = -1);

      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      NetEConst* must_be_leeq_(NetExpr*le, const verinum&rv, bool eq_flag);

      NetEConst*eval_eqeq_(bool ne_flag);
      NetEConst*eval_eqeq_real_(NetExpr*le, NetExpr*ri, bool ne_flag);
      NetEConst*eval_less_();
      NetEConst*eval_leeq_();
      NetEConst*eval_leeq_real_(NetExpr*le, NetExpr*ri, bool eq_flag);
      NetEConst*eval_gt_();
      NetEConst*eval_gteq_();
      NetEConst*eval_eqeqeq_(bool ne_flag);
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

      virtual bool set_width(unsigned w, bool last_chance =false);
      virtual NetEBLogic* dup_expr() const;
      virtual NetEConst* eval_tree(int prune_to_width = -1);
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      NetEConst* eval_tree_real_();
};

/*
 * Support the binary min(l,r) and max(l,r) operators. The opcodes
 * supported are:
 *
 *   m -- min
 *   M -- max
 */
class NetEBMinMax : public NetEBinary {

    public:
      NetEBMinMax(char op, NetExpr*l, NetExpr*r);
      ~NetEBMinMax();

      virtual ivl_variable_type_t expr_type() const;

    private:
};

/*
 * Support the binary multiplication (*) operator.
 */
class NetEBMult : public NetEBinary {

    public:
      NetEBMult(char op, NetExpr*l, NetExpr*r);
      ~NetEBMult();

      virtual ivl_variable_type_t expr_type() const;

      virtual bool set_width(unsigned w, bool last_chance);
      virtual void cast_signed(bool sign_flag);
      virtual NetEBMult* dup_expr() const;
      virtual NetExpr* eval_tree(int prune_to_width = -1);
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      NetExpr* eval_tree_real_();
};

/*
 * Support the binary power (**) operator.
 */
class NetEBPow : public NetEBinary {

    public:
      NetEBPow(char op, NetExpr*l, NetExpr*r);
      ~NetEBPow();

      virtual ivl_variable_type_t expr_type() const;

      virtual bool set_width(unsigned w, bool last_chance);
      virtual NetEBPow* dup_expr() const;
      virtual NetExpr* eval_tree(int prune_to_width = -1);
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

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

      virtual bool set_width(unsigned w, bool last_chance);

	// A shift expression only needs the left expression to have a
	// definite width to give the expression a definite width.
      virtual bool has_width() const;

      virtual NetEBShift* dup_expr() const;
      virtual NetEConst* eval_tree(int prune_to_width = -1);

      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

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
      virtual bool set_width(unsigned w, bool last_chance =false);
      virtual NetEConcat* dup_expr() const;
      virtual NetEConst*  eval_tree(int prune_to_width = -1);
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*root);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      svector<NetExpr*>parms_;
      NetExpr* repeat_;
      unsigned repeat_value_;
      bool repeat_calculated_;
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
      virtual void resolve_pexpr_type();
      virtual bool set_width(unsigned w, bool last_chance);
      virtual bool has_width() const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual ivl_variable_type_t expr_type() const;
      virtual NetExpr* eval_tree(int prune_to_width = -1);
      virtual NetEParam* dup_expr() const;
      void solving(bool arg);
      bool solving() const;

      virtual void dump(ostream&) const;

    private:
      Design*des_;
      NetScope*scope_;
      typedef map<perm_string,NetScope::param_expr_t>::iterator ref_t;
      ref_t reference_;
      bool solving_;

      NetEParam(class Design*des, NetScope*scope, ref_t ref);
};


/*
 * This expression node supports bit/part selects from general
 * expressions. The sub-expression is self-sized, and has bits
 * selected from it. The base is the expression that identifies the
 * lsb of the expression, and the wid is the width of the part select,
 * or 1 for a bit select. No matter what the subexpression is, the
 * base is translated in canonical bits. It is up to the elaborator
 * to figure this out and adjust the expression if the subexpression
 * has a non-canonical base or direction.
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
      virtual bool set_width(unsigned w, bool last_chance =false);
      virtual bool has_width() const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEConst* eval_tree(int prune_to_width = -1);
      virtual NetESelect* dup_expr() const;
      virtual NetNet*synthesize(Design*des, NetScope*scope, NetExpr*root);
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
      NetESFunc(const char*name, ivl_variable_type_t t,
		unsigned width, unsigned nprms);
      ~NetESFunc();

      const char* name() const;

      unsigned nparms() const;
      void parm(unsigned idx, NetExpr*expr);
      NetExpr* parm(unsigned idx);
      const NetExpr* parm(unsigned idx) const;

      virtual NetExpr* eval_tree(int prune_to_width = -1);

      virtual ivl_variable_type_t expr_type() const;
      virtual NexusSet* nex_input(bool rem_out = true);
      virtual bool set_width(unsigned, bool last_chance);
      virtual void dump(ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetESFunc*dup_expr() const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      const char* name_;
      ivl_variable_type_t type_;
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

      virtual bool set_width(unsigned w, bool last_chance);

      const NetExpr*cond_expr() const;
      const NetExpr*true_expr() const;
      const NetExpr*false_expr() const;

      virtual NetETernary* dup_expr() const;
      virtual NetExpr* eval_tree(int prune_to_width = -1);

      virtual ivl_variable_type_t expr_type() const;
      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*root);

    public:
      static bool test_operand_compat(ivl_variable_type_t tru, ivl_variable_type_t fal);

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
 *   m  -- abs(x)  (i.e. "magnitude")
 */
class NetEUnary  : public NetExpr {

    public:
      NetEUnary(char op, NetExpr*ex);
      ~NetEUnary();

      char op() const { return op_; }
      const NetExpr* expr() const { return expr_; }

      virtual bool set_width(unsigned w, bool last_chance);

      virtual NetEUnary* dup_expr() const;
      virtual NetExpr* eval_tree(int prune_to_width = -1);
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

      virtual ivl_variable_type_t expr_type() const;
      virtual NexusSet* nex_input(bool rem_out = true);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    protected:
      char op_;
      NetExpr* expr_;

    private:
      virtual NetExpr* eval_tree_real_();
};

class NetEUBits : public NetEUnary {

    public:
      NetEUBits(char op, NetExpr*ex);
      ~NetEUBits();

      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

      virtual NetEUBits* dup_expr() const;
      virtual NetExpr* eval_tree(int prune_to_width = -1);
      virtual ivl_variable_type_t expr_type() const;
};

class NetEUReduce : public NetEUnary {

    public:
      NetEUReduce(char op, NetExpr*ex);
      ~NetEUReduce();

      virtual bool set_width(unsigned w, bool last_chance);
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);
      virtual NetEUReduce* dup_expr() const;
      virtual NetEConst* eval_tree(int prune_to_width = -1);
      virtual ivl_variable_type_t expr_type() const;

    private:
      virtual NetEConst* eval_tree_real_();
};

/*
 * When a signal shows up in an expression, this type represents
 * it. From this the expression can get any kind of access to the
 * structural signal, including arrays.
 *
 * The NetESignal may refer to an array, if the word_index is
 * included. This expression calculates the index of the word in the
 * array. It may only be nil if the expression refers to the whole
 * array, and that is legal only in limited situation.
 */
class NetESignal  : public NetExpr {

    public:
      NetESignal(NetNet*n);
      NetESignal(NetNet*n, NetExpr*word_index);
      ~NetESignal();

      perm_string name() const;
      virtual bool set_width(unsigned, bool last_chance);

      virtual NetESignal* dup_expr() const;
      NetNet* synthesize(Design*des, NetScope*scope, NetExpr*root);
      NexusSet* nex_input(bool rem_out = true);

	// This is the expression for selecting an array word, if this
	// signal refers to an array.
      const NetExpr* word_index() const;

	// This is the width of the vector that this signal refers to.
      unsigned vector_width() const;
	// Point back to the signal that this expression node references.
      const NetNet* sig() const;
      NetNet* sig();
	// Declared vector dimensions for the signal.
      long msi() const;
      long lsi() const;

      virtual ivl_variable_type_t expr_type() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(ostream&) const;

    private:
      NetNet*net_;
	// Expression to select a word from the net.
      NetExpr*word_;
};

/*
 * The Design object keeps a list of work items for processing
 * elaboration. This is the type of those work items.
 */
struct elaborator_work_item_t {
      explicit elaborator_work_item_t(Design*d)
      : des(d) { }
      virtual ~elaborator_work_item_t() { }
      virtual void elaborate_runrun() =0;
    protected:
      Design*des;
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
      uint64_t scale_to_precision(uint64_t, const NetScope*)const;

	/* Look up a scope. If no starting scope is passed, then the
	   path is taken as an absolute scope name. Otherwise, the
	   scope is located starting at the passed scope and working
	   up if needed. */
      NetScope* find_scope(const std::list<hname_t>&path) const;
      NetScope* find_scope(NetScope*, const std::list<hname_t>&path,
                           NetScope::TYPE type = NetScope::MODULE) const;

	/* These members help manage elaboration of scopes. When we
	   get to a point in scope elaboration where we want to put
	   off a scope elaboration, an object of scope_elaboration_t
	   is pushed onto the scope_elaborations list. The scope
	   elaborator will go through this list elaborating scopes
	   until the list is empty. */
      list<elaborator_work_item_t*>elaboration_work_list;
      void run_elaboration_work(void);

      set<NetScope*> defparams_later;

	// PARAMETERS

      void run_defparams();
      void evaluate_parameters();
	// Look for defparams that never matched, and print warnings.
      void residual_defparams();

	/* This method locates a signal, starting at a given
	   scope. The name parameter may be partially hierarchical, so
	   this method, unlike the NetScope::find_signal method,
	   handles global name binding. */

      NetNet*find_signal(NetScope*scope, pform_name_t path);

	// Functions
      NetFuncDef* find_function(NetScope*scope, const pform_name_t&key);

	// Tasks
      NetScope* find_task(NetScope*scope, const pform_name_t&name);

	// NODES
      void add_node(NetNode*);
      void del_node(NetNode*);

	// BRANCHES
      void add_branch(NetBranch*);

	// PROCESSES
      void add_process(NetProcTop*);
      void add_process(NetAnalogTop*);
      void delete_process(NetProcTop*);
      bool check_always_delay() const;

      NetNet* find_discipline_reference(ivl_discipline_t dis, NetScope*scope);

	// Iterate over the design...
      void dump(ostream&) const;
      void functor(struct functor_t*);
      void join_islands(void);
      int emit(struct target_t*) const;

	// This is incremented by elaboration when an error is
	// detected. It prevents code being emitted.
      unsigned errors;

    private:
	// Keep a tree of scopes. The NetScope class handles the wide
	// tree and per-hop searches for me.
      list<NetScope*>root_scopes_;

	// List the nodes in the design.
      NetNode*nodes_;
	// These are in support of the node functor iterator.
      NetNode*nodes_functor_cur_;
      NetNode*nodes_functor_nxt_;

	// List the branches in the design.
      NetBranch*branches_;

	// List the processes in the design.
      NetProcTop*procs_;
      NetProcTop*procs_idx_;

	// List the ANALOG processes in the design.
      NetAnalogTop*aprocs_;

	// Map of discipline take to NetNet for the reference node.
      map<perm_string,NetNet*>discipline_references_;

	// Map the design arguments to values.
      map<string,const char*> flags_;

      int des_precision_;

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

/*
 * Manipulator to dump a scope complete path to the output. The
 * manipulator is "scope_path" and works like this:
 *
 *   out << .... << scope_path(sc) << ... ;
 */
struct __ScopePathManip { const NetScope*scope; };
inline __ScopePathManip scope_path(const NetScope*scope)
{ __ScopePathManip tmp; tmp.scope = scope; return tmp; }

extern ostream& operator << (ostream&o, __ScopePathManip);

#endif
