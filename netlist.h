#ifndef IVL_netlist_H
#define IVL_netlist_H
/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
# include  <memory>
# include  <vector>
# include  <set>
# include  <utility>
# include  "ivl_target.h"
# include  "ivl_target_priv.h"
# include  "pform_types.h"
# include  "config.h"
# include  "nettypes.h"
# include  "verinum.h"
# include  "verireal.h"
# include  "StringHeap.h"
# include  "HName.h"
# include  "LineInfo.h"
# include  "Attrib.h"
# include  "PScope.h"
# include  "PUdp.h"

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
# include  <iostream>
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
class NetEConstEnum;
class NetESignal;
class NetFuncDef;
class NetRamDq;
class NetTaskDef;
class NetEvTrig;
class NetEvNBTrig;
class NetEvWait;
class PClass;
class PExpr;
class PFunction;
class PPackage;
class PTaskFunc;
class PWire;
class data_type_t;
struct enum_type_t;
class netclass_t;
class netdarray_t;
class netparray_t;
class netuarray_t;
class netqueue_t;
class netenum_t;
class netstruct_t;
class netvector_t;

struct target;
struct functor_t;

#if defined(__cplusplus) && defined(_MSC_VER)
# define ENUM_UNSIGNED_INT : unsigned int
#else
# define ENUM_UNSIGNED_INT
#endif

std::ostream& operator << (std::ostream&o, ivl_variable_type_t val);

extern void join_island(NetPins*obj);

class Link {

      friend void connect(Link&, Link&);
      friend class NetPins;
      friend class Nexus;
      friend class NexusSet;

    public:
      enum DIR ENUM_UNSIGNED_INT { PASSIVE, INPUT, OUTPUT };
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
      void drive0(ivl_drive_t);
      void drive1(ivl_drive_t);

	// This sets the drives for all drivers of this link, and not
	// just the current link.
      void drivers_drive(ivl_drive_t d0, ivl_drive_t d1);

      ivl_drive_t drive0() const;
      ivl_drive_t drive1() const;

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
	// a part of. Note that the get_obj() method can return NIL if
	// this Link is part of a NexusSet. That should be OK, because
	// they are collection variables, and not functional parts of
	// a design.
      const NetPins*get_obj() const;
      NetPins*get_obj();
      unsigned get_pin() const;

      void dump_link(std::ostream&fd, unsigned ind) const;

    private:
	// The NetNode manages these. They point back to the
	// NetNode so that following the links can get me here.
      union {
	    NetPins *node_;
	    unsigned pin_;
      };

      bool pin_zero_     : 1;
      DIR dir_           : 2;
      ivl_drive_t drive0_ : 3;
      ivl_drive_t drive1_ : 3;

    private:
      Nexus* find_nexus_() const;

    private:
	// The Nexus uses these to maintain its list of Link
	// objects. If this link is not connected to anything,
	// then these pointers are both nil.
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

      void dump_node_pins(std::ostream&, unsigned, const char**pin_names =0) const;
      void set_default_dir(Link::DIR d);

      bool is_linked() const;
      bool pins_are_virtual(void) const;
      void devirtualize_pins(void);

	// This is for showing a brief description of the object to
	// the stream. It is used for debug and diagnostics.
      virtual void show_type(std::ostream&fd) const;

    private:
      Link*pins_;
      const unsigned npins_;
      Link::DIR default_dir_;
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
	// The name of the object must be a permallocated string. A
	// lex_strings string, for example.
      explicit NetObj(NetScope*s, perm_string n, unsigned npins);
      virtual ~NetObj();

      NetScope* scope();
      const NetScope* scope() const;

      perm_string name() const { return name_; }
      void rename(perm_string n) { name_ = n; }

      const NetExpr* rise_time() const { return delay1_; }
      const NetExpr* fall_time() const { return delay2_; }
      const NetExpr* decay_time() const { return delay3_; }

      void rise_time(const NetExpr* d) { delay1_ = d; }
      void fall_time(const NetExpr* d) { delay2_ = d; }
      void decay_time(const NetExpr* d) { delay3_ = d; }

      void dump_obj_attr(std::ostream&, unsigned) const;

      virtual void show_type(std::ostream&fd) const;

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
      explicit IslandBranch(ivl_discipline_t dis =0) : island_(0), discipline_(dis) { }

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

      void dump(std::ostream&, unsigned) const;

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
 * The links in a nexus are grouped into a circularly linked list,
 * with the nexus pointing to the last Link. Each link in turn points
 * to the next link in the nexus, with the last link pointing back to
 * the first. The last link also has a non-nil nexus_ pointer back to
 * this nexus.
 *
 * The t_cookie() is an ivl_nexus_t that the code generator uses to
 * store data in the nexus. When a Nexus is created, this cookie is
 * set to nil. The code generator may set the cookie once. This locks
 * the nexus, and rewrites the Link list to be optimal for the code
 * generator. In the process, *all* of the other methods are no longer
 * functional.
 */
class Nexus {

      friend void connect(Link&, Link&);
      friend class Link;

    private:
	// Only Link objects can create (or delete) Nexus objects
      explicit Nexus(Link&r);
      ~Nexus();

    public:

      void connect(Link&r);

      const char* name() const;

      void drivers_delays(NetExpr*rise, NetExpr*fall, NetExpr*decay);
      void drivers_drive(ivl_drive_t d0, ivl_drive_t d1);

      Link*first_nlink();
      const Link* first_nlink()const;

	/* Get the width of the Nexus, or 0 if there are no vectors
	   (in the form of NetNet objects) linked. */
      unsigned vector_width() const;

      NetNet* pick_any_net();

      NetNode* pick_any_node();

      /* This method counts the number of input and output links for
         this nexus, and assigns the results to the output arguments. */
      void count_io(unsigned&inp, unsigned&out) const;

	/* This method returns true if there are any assignments that
	   use this nexus as an l-value. This can be true if the nexus
	   is a variable, but also if this is a net with a force. */
      bool assign_lval() const;

	/* This method returns true if there are any inputs
	   attached to this nexus but no drivers. */
      bool has_floating_input() const;

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
      verinum driven_vector() const;

	/* Return a mask of the bits of this vector that are
	   driven. This is usually all false or all true, but in
	   special cases it may be a blend. */
      std::vector<bool> driven_mask(void)const;

	/* The code generator sets an ivl_nexus_t to attach code
	   generation details to the nexus. */
      ivl_nexus_t t_cookie() const { return t_cookie_; }
      void t_cookie(ivl_nexus_t) const;

    private:
      Link*list_;
      void unlink(Link*);

      mutable char* name_; /* Cache the calculated name for the Nexus. */
      mutable ivl_nexus_t t_cookie_;

      enum VALUE { NO_GUESS, V0, V1, Vx, Vz, VAR };
      mutable VALUE driven_;

    private: // not implemented
      Nexus(const Nexus&);
      Nexus& operator= (const Nexus&);
};

inline void connect(Nexus*l, Link&r) { l->connect(r); }

class NexusSet {

    public:
      struct elem_t {
	    inline elem_t(Nexus*n, unsigned b, unsigned w)
	    : base(b), wid(w)
	    {
		  lnk.set_dir(Link::PASSIVE);
		  n->connect(lnk);
	    }
	    inline elem_t() : base(0), wid(0)
	    {
	    }
	    inline bool operator == (const struct elem_t&that) const
	    { return lnk.is_linked(that.lnk) && base==that.base && wid==that.wid; }

	    bool contains(const struct elem_t&that) const;

	    Link lnk;
	    unsigned base;
	    unsigned wid;
	  private:
	    elem_t(const elem_t&);
	    elem_t& operator= (elem_t&);
      };

    public:
      ~NexusSet();
      NexusSet();

      size_t size() const;

	// Add the nexus/part to the set, if it is not already present.
      void add(Nexus*that, unsigned base, unsigned wid);
      void add(NexusSet&that);

	// Remove the nexus from the set, if it is present.
      void rem(const NexusSet&that);

      unsigned find_nexus(const elem_t&that) const;

      elem_t& at(unsigned idx);
      inline elem_t& operator[] (unsigned idx) { return at(idx); }

	// Return true if this set contains every nexus/part in that
	// set. That means that every bit of that set is accounted for
	// this set.
      bool contains(const NexusSet&that) const;

	// Return true if this set contains any nexus in that set.
      bool intersect(const NexusSet&that) const;

    private:
	// NexSet items are canonical part selects of vectors.
      std::vector<struct elem_t*> items_;

      size_t bsearch_(const struct elem_t&that) const;
      void rem_(const struct elem_t*that);
      bool contains_(const elem_t&that) const;

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

      unsigned find_link(const Link&that) const;

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
      virtual void dump_node(std::ostream&, unsigned) const;

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
                           bool condit_src, bool conditional, bool parallel);
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

      bool is_parallel() const;

      void dump(std::ostream&, unsigned ind) const;

    private:
      uint64_t transition_delays_[12];
      bool condit_flag_;
      bool conditional_;
      bool parallel_;
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
 * The pins of a NetNet object are usually PASSIVE: they do not drive
 * anything and they are not a data sink, per se. The pins follow the
 * values on the nexus. The exceptions are reg, trireg, tri0, tri1,
 * supply0, and supply1 objects, whose pins are classed as OUTPUT.
 */

class PortType
{
public:
	enum Enum ENUM_UNSIGNED_INT { NOT_A_PORT, PIMPLICIT, PINPUT, POUTPUT, PINOUT, PREF };

    /*
     * Merge Port types (used to construct a sane combined port-type
     * for module ports with complex defining expressions).
     *
     */
    static Enum merged( Enum lhs, Enum rhs );
};

extern std::ostream& operator << (std::ostream&, PortType::Enum);

  /*
   * Information on actual ports (rather than port-connected signals) of
   * module.
   * N.b. must be POD as passed through a "C" interface in the t-dll-api.
   */
struct PortInfo
{
    PortType::Enum  type;
    unsigned long   width;
    perm_string     name;
    ivl_net_logic_t buffer;
};


class NetNet  : public NetObj, public PortType {

    public:
      enum Type ENUM_UNSIGNED_INT { NONE, IMPLICIT, IMPLICIT_REG, WIRE, TRI, TRI1,
		  SUPPLY0, SUPPLY1, WAND, TRIAND, TRI0, WOR, TRIOR, REG,
		  UNRESOLVED_WIRE };

      typedef PortType::Enum PortType;

    public:
	// This form is the more generic form of the constructor. For
	// now, the unpacked type is not buried into an ivl_type_s object.
      explicit NetNet(NetScope*s, perm_string n, Type t,
		      const netranges_t &unpacked,
		      ivl_type_t type);

      explicit NetNet(NetScope*s, perm_string n, Type t, ivl_type_t type);

      virtual ~NetNet();

      Type type() const;
      void type(Type t);

        // This method returns true if we have changed the net type from being
        // a variable to being an unresolved wire. This happens in SystemVerilog
        // when we find a continuous assignment to a variable.
      bool coerced_to_uwire() { return coerced_to_uwire_; }

      PortType port_type() const;
      void port_type(PortType t);

      unsigned lexical_pos() const { return lexical_pos_; }
      void lexical_pos(unsigned lp) { lexical_pos_ = lp; }

      // If this net net is a port (i.e. a *sub*port net of a module port)
      // its port index is number of the module it connects through
      int get_module_port_index() const;                // -1 Not connected to port...
      void set_module_port_index(unsigned idx);

      ivl_variable_type_t data_type() const;

	/* If a NetNet is signed, then its value is to be treated as
	   signed. Otherwise, it is unsigned. */
      bool get_signed() const;

      void set_const(bool is_const) { is_const_ = is_const; }
      bool get_const() const { return is_const_; }

      bool get_scalar() const;

      inline const ivl_type_s* net_type(void) const { return net_type_; }
      const netenum_t*enumeration(void) const;
      const netstruct_t*struct_type(void) const;
      const netdarray_t*darray_type(void) const;
      const netqueue_t*queue_type(void) const;
      const netclass_t*class_type(void) const;
      const netarray_t*array_type(void) const;

	/* Attach a discipline to the net. */
      ivl_discipline_t get_discipline() const;
      void set_discipline(ivl_discipline_t dis);

	/* This method returns a reference to the packed dimensions
	   for the vector. These are arranged as a list where the
	   first range in the list (front) is the left-most range in
	   the Verilog declaration. These packed dims are compressed
	   to represent the dimensions of all the subtypes. */
      const netranges_t& packed_dims() const { return slice_dims_; }

      const netranges_t& unpacked_dims() const { return unpacked_dims_; }

	/* The vector_width returns the bit width of the packed array,
	   vector or scalar that is this NetNet object.  */
      inline unsigned long vector_width() const { return slice_width(0); }

	/* Given a prefix of indices, figure out how wide the
	   resulting slice would be. This is a generalization of the
	   vector_width(), where the depth would be 0. */
      unsigned long slice_width(size_t depth) const;

	/* This method converts a signed index (the type that might be
	   found in the Verilog source) to canonical. It accounts
	   for variation in the definition of the
	   reg/wire/whatever. Note that a canonical index of a
	   multi-dimensioned packed array is a single dimension. For
	   example, "reg [4:1][3:0]..." has the canonical dimension
	   [15:0] and the sb_to_idx() method will convert [2][2] to
	   the canonical index [6]. */
      long sb_to_idx(const std::list<long>&prefix, long sb) const;

	/* This method converts a partial packed indices list and a
	   tail index, and generates a canonical slice offset and
	   width. */
      bool sb_to_slice(const std::list<long>&prefix, long sb, long&off, unsigned long&wid) const;

	/* This method checks that the signed index is valid for this
	   signal. If it is, the above sb_to_idx can be used to get
	   the pin# from the index. */
      bool sb_is_valid(const std::list<long>&prefix, long sb) const;

	/* This method returns 0 for scalars and vectors, and greater
	   for arrays. The value is the number of array
	   indices. (Currently only one array index is supported.) */
      inline unsigned unpacked_dimensions() const { return unpacked_dims_.size(); }

	/* This method returns 0 for scalars, but vectors and other
	   PACKED arrays have packed dimensions. */
      inline size_t packed_dimensions() const { return slice_dims_.size(); }

	// This is the number of array elements.
      unsigned unpacked_count() const;

      bool local_flag() const { return local_flag_; }
      void local_flag(bool f) { local_flag_ = f; }

	// NetESignal objects may reference this object. Keep a
	// reference count so that I keep track of them.
      void incr_eref();
      void decr_eref();
      unsigned peek_eref() const;

	// Assignment statements count their lrefs here. And by
	// assignment statements, we mean BEHAVIORAL assignments.
      void incr_lref();
      void decr_lref();
      unsigned peek_lref() const { return lref_count_; }

	// Treating this node as a uwire, this function tests whether
	// any bits in the canonical part are already driven. This is
	// only useful for UNRESOLVED_WIRE objects. The msb and lsb
	// are the part select of the signal, and the widx is the word
	// index if this is an unpacked array.
      bool test_part_driven(unsigned msb, unsigned lsb, int widx =0);

	// Treating this node as a uwire, this function tests whether
	// any bits in the canonical part are already driven and sets
	// them if not. This is only useful for UNRESOLVED_WIRE objects.
	// The msb and lsb are the part select of the signal, and the
	// widx is the word index if this is an unpacked array.
      bool test_and_set_part_driver(unsigned msb, unsigned lsb, int widx =0);

      unsigned get_refs() const;

	/* Manage path delays */
      void add_delay_path(class NetDelaySrc*path);
      unsigned delay_paths(void) const;
      const class NetDelaySrc*delay_path(unsigned idx) const;

      virtual void dump_net(std::ostream&, unsigned) const;

    private:
      void initialize_dir_();

    private:
      Type   type_    : 5;
      PortType port_type_ : 3;
      bool coerced_to_uwire_: 1;
      bool local_flag_: 1;
      unsigned lexical_pos_;
      ivl_type_t net_type_;
      netuarray_t *array_type_ = nullptr;
      ivl_discipline_t discipline_;

        // Whether the net is variable declared with the const keyword.
      bool is_const_ = false;

      netranges_t unpacked_dims_;

	// These are the widths of the various slice depths. There is
	// one entry in this vector for each packed dimension. The
	// value at N is the slice width if N indices are provided.
	//
	// For example: slice_wids_[0] is vector_width().
      void calculate_slice_widths_from_packed_dims_(void);
      netranges_t slice_dims_;
      std::vector<unsigned long> slice_wids_;

      unsigned eref_count_;
      unsigned lref_count_;

	// When the signal is an unresolved wire, we need more detail
	// which bits are assigned. This mask is true for each bit
	// that is known to be driven.
      std::vector<bool> lref_mask_;

      std::vector<class NetDelaySrc*> delay_paths_;
      int       port_index_ = -1;
};

/*
 * This object type is used for holding local variable values when
 * evaluating constant user functions.
 */
struct LocalVar {
      int nwords;  // zero for a simple variable, -1 for reference
      union {
	    NetExpr*  value;  // a simple variable
	    NetExpr** array;  // an array variable
	    LocalVar* ref;    // A reference to a previous scope
      };
};

class NetBaseDef {
    public:
      NetBaseDef(NetScope*n, const std::vector<NetNet*>&po,
		 const std::vector<NetExpr*>&pd);
      virtual ~NetBaseDef();

      const NetScope*scope() const;
      NetScope*scope();

      unsigned port_count() const;
      NetNet*port(unsigned idx) const;
      NetExpr*port_defe(unsigned idx) const;

      void set_proc(NetProc*p);

	//const string& name() const;
      const NetProc*proc() const;

      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

    private:
      NetScope*scope_;
      std::vector<NetNet*>ports_;
      std::vector<NetExpr*>pdefaults_;

    protected:
      NetProc*proc_;
};

/*
 * Some definitions (and methods to manipulate them) are common to a
 * couple of types. Keep them here.
 */
class Definitions {

    public:
      Definitions();
      ~Definitions();

	// Add the enumeration to the set of enumerations in this
	// scope. Include a key that the elaboration can use to look
	// up this enumeration based on the pform type.
      void add_enumeration_set(const enum_type_t*key, netenum_t*enum_set);

      bool add_enumeration_name(netenum_t*enum_set, perm_string enum_name);

	// Look up the enumeration set that was added with the given
	// key. This is used by enum_type_t::elaborate_type to locate
	// a previously elaborated enumeration.
      netenum_t* enumeration_for_key(const enum_type_t*key) const;

	// Look up an enumeration literal in this scope. If the
	// literal is present, return the expression that defines its
	// value.
      const NetExpr* enumeration_expr(perm_string key);

	// Definitions scopes can also hold classes, by name.
      void add_class(netclass_t*class_type);

    protected:
	// Enumerations. The enum_sets_ is a list of all the
	// enumerations present in this scope. The enum_names_ is a
	// map of all the enumeration names back to the sets that
	// contain them.
      std::map<const enum_type_t*,netenum_t*> enum_sets_;
      std::map<perm_string,NetEConstEnum*> enum_names_;

	// This is a map of all the classes (by name) in this scope.
      std::map<perm_string,netclass_t*> classes_;

};

/*
 * This object type is used to contain a logical scope within a
 * design. The scope doesn't represent any executable hardware, but is
 * just a handle that netlist processors can use to grab at the design.
 */
class NetScope : public Definitions, public Attrib {

    public:
      enum TYPE { MODULE, CLASS, TASK, FUNC, BEGIN_END, FORK_JOIN, GENBLOCK, PACKAGE };

	/* Create a new scope associated with a given compilation unit,
	   and attach it to the given parent. If no compilation unit is
	   specified, the parent's compilation unit is used. The name
	   is expected to have been permallocated. */
      NetScope(NetScope*up, const hname_t&name, TYPE t, NetScope*in_unit=0,
	       bool nest=false, bool program=false, bool interface=false,
               bool compilation_unit=false);
      ~NetScope();

	/* Rename the scope using the name generated by inserting as
	   many pad characters as required between prefix and suffix
	   to make the name unique in the parent scope. Return false
	   if a unique name couldn't be generated. */
      bool auto_name(const char* prefix, char pad, const char* suffix);

      void add_imports(const std::map<perm_string,PPackage*>*imports);
      NetScope*find_import(const Design*des, perm_string name);

      void add_typedefs(const std::map<perm_string,typedef_t*>*typedefs);

        /* Search the scope hierarchy for the scope where 'type' was defined. */
      NetScope*find_typedef_scope(const Design*des, const typedef_t*type);

	/* Parameters exist within a scope, and these methods allow
	   one to manipulate the set. In these cases, the name is the
	   *simple* name of the parameter, the hierarchy is implicit in
	   the scope. */

      struct range_t;
      void set_parameter(perm_string name, bool is_annotatable,
			 const LexicalScope::param_expr_t &param,
			 NetScope::range_t *range_list);
      void set_parameter(perm_string name, NetExpr*val,
			 const LineInfo&file_line);

      const NetExpr*get_parameter(Design*des, const char* name,
				  ivl_type_t&ivl_type);
      const NetExpr*get_parameter(Design*des, perm_string name,
				  ivl_type_t&ivl_type);

	/* These are used by defparam elaboration to replace the
	   expression with a new expression, without affecting the
	   range or signed_flag. Return false if the name does not
	   exist. */
      void replace_parameter(Design *des, perm_string name, PExpr*val,
			     NetScope*scope, bool defparam = false);

	/* This is used to ensure the value of a parameter cannot be
	   changed at run-time. This is required if a specparam is used
	   in an expression that must be evaluated at compile-time.
	   Returns true if the named parameter is a specparam and has
	   not already been set to be unannotatable. */
      bool make_parameter_unannotatable(perm_string name);

	/* These methods set or access events that live in this
	   scope. */

      void add_event(NetEvent*);
      void rem_event(NetEvent*);
      NetEvent*find_event(perm_string name);

	/* These methods add or find a genvar that lives in this scope. */
      void add_genvar(perm_string name, LineInfo *li);
      LineInfo* find_genvar(perm_string name);

	/* These methods manage unelaborated signals. These are added to
	   the scope as placeholders during the scope elaboration phase,
	   to allow signal declarations to refer to other signals (e.g.
	   when using $bits in a range definition), regardless of the
	   order in which the signals are elaborated. */
      void add_signal_placeholder(PWire*);
      void rem_signal_placeholder(PWire*);
      PWire* find_signal_placeholder(perm_string name);

	/* These methods manage signals. The add_ and rem_signal
	   methods are used by the NetNet objects to make themselves
	   available to the scope, and the find_signal method can be
	   used to locate signals within a scope. */

      void add_signal(NetNet*);
      void rem_signal(NetNet*);
      NetNet* find_signal(perm_string name);

      netclass_t* find_class(const Design*des, perm_string name);

	/* The unit(), parent(), and child() methods allow users of
	   NetScope objects to locate nearby scopes. */
      NetScope* unit() { return unit_; }
      NetScope* parent() { return up_; }
      NetScope* child(const hname_t&name);
      const NetScope* unit() const { return unit_; }
      const NetScope* parent() const { return up_; }
      const NetScope* child(const hname_t&name) const;

	/* A helper function to find the enclosing class scope. */
      const NetScope* get_class_scope() const;

	// Look for a child scope by name. This ignores the number
	// part of the child scope name, so there may be multiple
	// matches. Only return one. This function is only really
	// useful for some elaboration error checking.
      const NetScope* child_byname(perm_string name) const;

	// Nested modules have slightly different scope search rules.
      inline bool nested_module() const { return nested_module_; }
	// Program blocks and interfaces have elaboration constraints.
      inline bool program_block() const { return program_block_; }
      inline bool is_interface() const { return is_interface_; }
      inline bool is_unit() const { return is_unit_; }
      inline TYPE type() const { return type_; }
      void print_type(std::ostream&) const;

	// This provides a link to the variable initialization process
	// for use when evaluating a constant function. Note this is
	// only used for static functions - the variable initialization
	// for automatic functions is included in the function definition.
      void set_var_init(const NetProc*proc) { var_init_ = proc; }
      const NetProc* var_init() const { return var_init_; }

      void set_task_def(NetTaskDef*);
      void set_func_def(NetFuncDef*);
      void set_class_def(netclass_t*);
      void set_module_name(perm_string);

      NetTaskDef* task_def();
      NetFuncDef* func_def();

	// This is used by the evaluate_function setup to collect
	// local variables from the scope.
      void evaluate_function_find_locals(const LineInfo&loc,
					 std::map<perm_string,LocalVar>&ctx) const;

      void set_line(perm_string file, perm_string def_file,
                    unsigned lineno, unsigned def_lineno);
      void set_line(perm_string file, unsigned lineno);
      void set_line(const LineInfo *info);
      perm_string get_file() const { return file_; };
      perm_string get_def_file() const { return def_file_; };
      unsigned get_lineno() const { return lineno_; };
      unsigned get_def_lineno() const { return def_lineno_; };

      std::string get_fileline() const;
      std::string get_def_fileline() const;

      bool in_func() const;

	/* Provide a link back to the pform to allow early elaboration of
           constant functions. */
      void set_func_pform(const PFunction*pfunc) { func_pform_ = pfunc; };
      const PFunction*func_pform() const { return func_pform_; };

        /* Allow tracking of elaboration stages. The three stages are:
             1 - scope elaboration
             2 - signal elaboration
             3 - statement elaboration
           This is only used for functions, to support early elaboration.
        */
      void set_elab_stage(unsigned stage) { elab_stage_ = stage; };
      unsigned elab_stage() const { return elab_stage_; };

	/* Is this a function called in a constant expression. */
      void need_const_func(bool need_const) { need_const_func_ = need_const; };
      bool need_const_func() const { return need_const_func_; };

	/* Is this a constant function. */
      void is_const_func(bool is_const) { is_const_func_ = is_const; };
      bool is_const_func() const { return is_const_func_; };

	/* Is the task or function automatic. */
      void is_auto(bool is_auto__) { is_auto_ = is_auto__; };
      bool is_auto() const { return is_auto_; };

	/* Is the module a cell (is in a `celldefine) */
      void is_cell(bool is_cell__) { is_cell_ = is_cell__; };
      bool is_cell() const { return is_cell_; };

	/* Is there a call to a system task in this scope. */
      void calls_sys_task(bool calls_stask__) { calls_stask_ = calls_stask__; };
      bool calls_sys_task() const { return calls_stask_; };

        /* Is this scope elaborating a final procedure? */
      void in_final(bool in_final__) { in_final_ = in_final__; };
      bool in_final() const { return in_final_; };

      const NetTaskDef* task_def() const;
      const NetFuncDef* func_def() const;
      const netclass_t* class_def() const;

	/* If the scope represents a module instance, the module_name
	   is the name of the module itself. */
      perm_string module_name() const;
	/* If the scope is a module then it may have ports that we need
	 * to keep track of. */

      void set_num_ports(unsigned int num_ports);
      void add_module_port_net(NetNet*port);
      unsigned module_port_nets() const;
      NetNet*module_port_net(unsigned idx) const;

      void add_module_port_info( unsigned idx,
                            perm_string name,  // May be "" for undeclared port
                            PortType::Enum type,
                            unsigned long width );

      PortInfo* get_module_port_info(unsigned idx);

      const std::vector<PortInfo> &module_port_info() const;

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

      bool symbol_exists(perm_string sym);

	/* This method generates a non-hierarchical name that is
	   guaranteed to be unique within this scope. */
      perm_string local_symbol();

      void dump(std::ostream&) const;
	// Check to see if the scope has items that are not allowed
	// in an always_comb/ff/latch process.
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      void emit_scope(struct target_t*tgt) const;
      bool emit_defs(struct target_t*tgt) const;

	/* This method runs the functor on me. Recurse through the
	   children of this node as well. */
      void run_functor(Design*des, functor_t*fun);

	/* These are used in synthesis. They provide shared pullup and
	   pulldown nodes for this scope. */
      void add_tie_hi(Design*des);
      void add_tie_lo(Design*des);
      Link&tie_hi() const { return tie_hi_->pin(0); };
      Link&tie_lo() const { return tie_lo_->pin(0); };

	/* This member is used during elaboration to pass defparam
	   assignments from the scope pass to the parameter evaluation
	   step. After that, it is not used. */

      std::list<std::pair<pform_name_t,PExpr*> > defparams;
      std::list<std::pair<std::list<hname_t>,PExpr*> > defparams_later;

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
	    param_expr_t() : val_expr(0), val_type(0), val_scope(0),
		             solving(false), is_annotatable(false),
		             local_flag(false),
		             range(0), val(0), ivl_type(0) { }
	    // Source expression and data type (before elaboration)
	    PExpr*val_expr;
	    data_type_t*val_type;
	    // Scope information
            NetScope*val_scope;
	    // Evaluation status
	    bool solving;
	    // specparam status
	    bool is_annotatable;
	    // Is this a localparam?
	    bool local_flag;
	    // Can it be overridden?
	    bool overridable = false;
	    // Is it a type parameter
	    bool type_flag = false;
	    // The lexical position of the declaration
	    unsigned lexical_pos = 0;
	    // range constraints
	    struct range_t*range;

	    // Expression value. Elaborated version of val_expr.
	    // For type parameters this will always be 0.
	    NetExpr*val;

	    // For non-type parameter this contains the elaborate type of the
	    // parameter itself. For type parameters this contains the
	    // elaborated assigned type value.
	    ivl_type_t ivl_type;
      };
      std::map<perm_string,param_expr_t>parameters;

      typedef std::map<perm_string,param_expr_t>::iterator param_ref_t;

      LineInfo get_parameter_line_info(perm_string name) const;

      unsigned get_parameter_lexical_pos(perm_string name) const;

	/* Module instance arrays are collected here for access during
	   the multiple elaboration passes. */
      typedef std::vector<NetScope*> scope_vec_t;
      std::map<perm_string, scope_vec_t>instance_arrays;

	/* Loop generate uses this as scratch space during
	   elaboration. Expression evaluation can use this to match
	   names. */
      perm_string genvar_tmp;
      long genvar_tmp_val;

      std::map<perm_string,LocalVar> loop_index_tmp;

    private:
      void evaluate_type_parameter_(Design*des, param_ref_t cur);
      void evaluate_parameter_logic_(Design*des, param_ref_t cur);
      void evaluate_parameter_real_(Design*des, param_ref_t cur);
      void evaluate_parameter_string_(Design*des, param_ref_t cur);
      void evaluate_parameter_(Design*des, param_ref_t cur);

    private:
      TYPE type_;
      hname_t name_;

	// True if the scope is a nested module/program block
      bool nested_module_;
	// True if the scope is a program block
      bool program_block_;
	// True if the scope is an interface
      bool is_interface_;
	// True if the scope is a compilation unit
      bool is_unit_;

      perm_string file_;
      perm_string def_file_;
      unsigned lineno_;
      unsigned def_lineno_;

      signed char time_unit_, time_prec_;
      bool time_from_timescale_;

      const std::map<perm_string,PPackage*>*imports_;

      std::map<perm_string,typedef_t*>typedefs_;

      NetEvent *events_;

      std::map<perm_string,LineInfo*> genvars_;

      std::map<perm_string,PWire*> signal_placeholders_;

      typedef std::map<perm_string,NetNet*>::const_iterator signals_map_iter_t;
      std::map <perm_string,NetNet*> signals_map_;
      perm_string module_name_;
      std::vector<NetNet*> port_nets;

      std::vector<PortInfo> ports_;

      const NetProc*var_init_;

      union {
	    NetTaskDef*task_;
	    NetFuncDef*func_;
	    netclass_t*class_def_;
      };
      const PFunction*func_pform_;
      unsigned elab_stage_;

      NetScope*unit_;
      NetScope*up_;
      std::map<hname_t,NetScope*> children_;

      unsigned lcounter_;
      bool need_const_func_, is_const_func_, is_auto_, is_cell_, calls_stask_;

      /* Final procedures sets this to notify statements that
	 they are part of a final procedure. */
      bool in_final_;

      NetNode*tie_hi_;
      NetNode*tie_lo_;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      NetNet*mem_;
      unsigned awidth_;

};

/*
 * Convert an IVL_VT_REAL input to a logical value with the
 * given width. The input is pin(1) and the output is pin(0).
 */
class NetCastInt4  : public NetNode {

    public:
      NetCastInt4(NetScope*s, perm_string n, unsigned width);

      unsigned width() const { return width_; }

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
};

class NetCastInt2  : public NetNode {

    public:
      NetCastInt2(NetScope*s, perm_string n, unsigned width);

      unsigned width() const { return width_; }

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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
      virtual void dump_node(std::ostream&, unsigned ind) const;
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
      NetConcat(NetScope*scope, perm_string n, unsigned wid, unsigned cnt,
		bool transparent_flag = false);
      ~NetConcat();

      unsigned width() const;
	// This is true if the concatenation is a transparent
	// concatenation, meaning strengths are passed through as
	// is. In this case, the output strengths of this node will be
	// ignored.
      bool transparent() const { return transparent_; }

      void dump_node(std::ostream&, unsigned ind) const;
      bool emit_node(struct target_t*) const;
      void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_;
      bool transparent_;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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
      NetFF(NetScope*s, perm_string n, bool negedge, unsigned vector_width);
      ~NetFF();

      bool is_negedge() const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      bool negedge_;
      unsigned width_;
      verinum aset_value_;
      verinum sset_value_;
};


/*
 * This class represents an LPM_LATCH device. There is no literal gate
 * type in Verilog that maps, but gates of this type can be inferred.
 */
class NetLatch  : public NetNode {

    public:
      NetLatch(NetScope*s, perm_string n, unsigned vector_width);
      ~NetLatch();

      unsigned width() const;

      Link& pin_Enable();
      Link& pin_Data();
      Link& pin_Q();

      const Link& pin_Enable() const;
      const Link& pin_Data() const;
      const Link& pin_Q() const;

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned width_;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      void dump_node(std::ostream&, unsigned ind) const;
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

      unsigned port_width(unsigned port) const;

      const NetScope* def() const;

      const NetEvWait* trigger() const { return trigger_; }

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      const struct sfunc_return_type*def_;
      NetEvWait*trigger_;
};

class NetTran  : public NetNode, public IslandBranch {

    public:
	// Tran devices other than TRAN_VP
      NetTran(NetScope*scope, perm_string n, ivl_switch_type_t type,
              unsigned wid);
	// Create a TRAN_VP
      NetTran(NetScope*scope, perm_string n, unsigned wid,
	      unsigned part, unsigned off);
      ~NetTran();

      ivl_switch_type_t type() const { return type_; }

	// These are only used for IVL_SW_TRAN_PV
      unsigned vector_width() const;
      unsigned part_width() const;
      unsigned part_offset() const;

      virtual void dump_node(std::ostream&, unsigned ind) const;
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
 * The expr_width() is the width of the expression, which is calculated
 * before the expression is elaborated.
 */
class NetExpr  : public LineInfo {
    public:
      explicit NetExpr(unsigned w =0);
      explicit NetExpr(ivl_type_t t);
      virtual ~NetExpr() =0;

      virtual void expr_scan(struct expr_scan_t*) const =0;
      virtual void dump(std::ostream&) const;

	// This is the advanced description of the type. I think I
	// want to replace the other type description members with
	// this single method. The default for this method returns
	// nil.
      ivl_type_t net_type() const;

	// Expressions have type.
      virtual ivl_variable_type_t expr_type() const;

	// How wide am I?
      unsigned expr_width() const { return width_; }

	// This method returns true if the expression is
	// signed. Unsigned expressions return false.
      bool has_sign() const { return signed_flag_; }
      virtual void cast_signed(bool flag);

	// This returns true if the expression has a definite
	// width. This is generally true, but in some cases the
	// expression is amorphous and desires a width from its
	// environment. For example, 'd5 has indefinite width, but
	// 5'd5 has a definite width.

	// This method is only really used within concatenation
	// expressions to check validity.
      virtual bool has_width() const;

	// Return the enumeration set that defines this expressions
	// enumeration type, or return nil if the expression is not
	// part of the enumeration.
      virtual const netenum_t*enumeration() const;

	// This method evaluates the expression and returns an
	// equivalent expression that is reduced as far as compile
	// time knows how. Essentially, this is designed to fold
	// constants.
      virtual NetExpr*eval_tree();

	// Make a duplicate of myself, and subexpressions if I have
	// any. This is a deep copy operation.
      virtual NetExpr*dup_expr() const =0;

	// Evaluate the expression at compile time, a la within a
	// constant function. This is used by the constant function
	// evaluation function code, and the return value is an
	// allocated constant, or nil if the expression cannot be
	// evaluated for any reason.
      virtual NetExpr*evaluate_function(const LineInfo&loc,
					std::map<perm_string,LocalVar>&ctx) const;

	// Get the Nexus that are the input to this
	// expression. Normally this descends down to the reference to
	// a signal that reads from its input.
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const =0;

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
	//  drive0/drive1: Attach these strengths to the driver for
	//                 the expression output.
      virtual NetNet*synthesize(Design*des, NetScope*scope, NetExpr*root);

    protected:
      void expr_width(unsigned wid) { width_ = wid; }
      void cast_signed_base_(bool flag) { signed_flag_ = flag; }
      void set_net_type(ivl_type_t type);

    private:
      ivl_type_t net_type_;
      unsigned width_;
      bool signed_flag_;

    private: // not implemented
      NetExpr(const NetExpr&);
      NetExpr& operator=(const NetExpr&);
};

class NetEArrayPattern  : public NetExpr {

    public:
      NetEArrayPattern(ivl_type_t lv_type, std::vector<NetExpr*>&items);
      ~NetEArrayPattern();

      inline size_t item_size() const { return items_.size(); }
      const NetExpr* item(size_t idx) const { return items_[idx]; }

      void expr_scan(struct expr_scan_t*) const;
      void dump(std::ostream&) const;

      NetEArrayPattern* dup_expr() const;
      NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                          bool nested_func = false) const;
      NetNet* synthesize(Design *des, NetScope *scope, NetExpr *root);

    private:
      std::vector<NetExpr*> items_;
};

/*
 * The expression constant is slightly special, and is sometimes
 * returned from other classes that can be evaluated at compile
 * time. This class represents constant values in expressions.
 */
class NetEConst  : public NetExpr {

    public:
      explicit NetEConst(ivl_type_t type, const verinum&val);
      explicit NetEConst(const verinum&val);
      ~NetEConst();

      const verinum&value() const;

      virtual void cast_signed(bool flag);
      virtual bool has_width() const;
      virtual ivl_variable_type_t expr_type() const;

        /* This method allows the constant value to be converted
           to an unsized value. This is used after evaluating a
           unsized constant expression. */
      void trim();

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;

      virtual NetEConst* dup_expr() const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*);
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual NetExpr*evaluate_function(const LineInfo&loc,
					std::map<perm_string,LocalVar>&ctx) const;

    private:
      verinum value_;
};

class NetEConstEnum  : public NetEConst {

    public:
      explicit NetEConstEnum(perm_string name, const netenum_t*enum_set,
			     const verinum&val);
      ~NetEConstEnum();

      perm_string name() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;

      virtual NetEConstEnum* dup_expr() const;

    private:
      perm_string name_;
};

class NetEConstParam  : public NetEConst {

    public:
      explicit NetEConstParam(const NetScope*scope, perm_string name,
			      const verinum&val);
      ~NetEConstParam();

      perm_string name() const;
      const NetScope*scope() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;

      virtual NetEConstParam* dup_expr() const;

    private:
      const NetScope*scope_;
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

	// The type of this expression is ET_REAL
      ivl_variable_type_t expr_type() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;

      virtual NetECReal* dup_expr() const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*);
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual NetExpr*evaluate_function(const LineInfo&loc,
					std::map<perm_string,LocalVar>&ctx) const;

    private:
      verireal value_;
};

class NetECString  : public NetEConst {
    public:
      explicit NetECString(const std::string& val);
      ~NetECString();

      // The type of a string is IVL_VT_STRING
      ivl_variable_type_t expr_type() const;
};

class NetECRealParam  : public NetECReal {

    public:
      explicit NetECRealParam(const NetScope*scope, perm_string name,
			      const verireal&val);
      ~NetECRealParam();

      perm_string name() const;
      const NetScope*scope() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;

      virtual NetECRealParam* dup_expr() const;

    private:
      const NetScope*scope_;
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
			     unsigned off, unsigned wid, dir_t dir,
			     bool signed_flag__ = false);
      explicit NetPartSelect(NetNet*sig, NetNet*sel,
			     unsigned wid, bool signed_flag__ = false);
      ~NetPartSelect();

      unsigned base()  const;
      unsigned width() const;
      inline dir_t dir()   const { return dir_; }
	/* Is the select signal signed? */
      inline bool signed_flag() const { return signed_flag_; }

      virtual void dump_node(std::ostream&, unsigned ind) const;
      bool emit_node(struct target_t*tgt) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned off_;
      unsigned wid_;
      dir_t    dir_;
      bool signed_flag_;
};

/*
 * This device supports simple substitution of a part within a wider
 * vector. For example, this:
 *
 *      wire [7:0] foo = NetSubstitute(bar, bat, off);
 *
 * means that bar is a vector the same width as foo, bat is a narrower
 * vector. The off is a constant offset into the bar vector. This
 * looks something like this:
 *
 *      foo = bar;
 *      foo[off +: <width_of_bat>] = bat;
 *
 * There is no direct way in Verilog to express this (as a single
 * device), it instead turns up in certain synthesis situation,
 * i.e. the example above.
 */
class NetSubstitute : public NetNode {

    public:
      NetSubstitute(NetNet*sig, NetNet*sub, unsigned wid, unsigned off);
      ~NetSubstitute();

      inline unsigned width() const { return wid_; }
      inline unsigned base() const  { return off_; }

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*tgt) const;
      virtual void functor_node(Design*des, functor_t*fun);

    private:
      unsigned wid_;
      unsigned off_;
};

/*
 * The NetBUFZ is a magic device that represents the continuous
 * assign, with the output being the target register and the input
 * the logic that feeds it. The netlist preserves the directional
 * nature of that assignment with the BUFZ. The target may elide it if
 * that makes sense for the technology.
 *
 * A NetBUFZ is transparent if strengths are passed through it without
 * change. A NetBUFZ is non-transparent if values other than HiZ are
 * converted to the strength of the output.
 */
class NetBUFZ  : public NetNode {

    public:
      explicit NetBUFZ(NetScope*s, perm_string n, unsigned wid, bool transp, int port_info_index = -1);
      ~NetBUFZ();

      unsigned width() const;
      bool transparent() const { return transparent_; }
      int port_info_index() const { return port_info_index_; }

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
      bool transparent_;
      int port_info_index_;
};

/*
 * This node is used to represent case equality in combinational
 * logic. Although this is not normally synthesizable, it makes sense
 * to support an abstract gate that can compare x and z. This node
 * always generates a single bit result, no matter the width of the
 * input. The elaboration, btw, needs to make sure the input widths
 * match.
 *
 * The case compare can be generated to handle ===/!==, or also
 * to test guards in the case/casez/casex statements.
 *
 * This pins are assigned as:
 *
 *     0   -- Output (always returns 0 or 1)
 *     1   -- Input
 *     2   -- Input (wildcard input for EQX and EQZ variants)
 */
class NetCaseCmp  : public NetNode {

    public:
      enum kind_t {
	    EEQ, // ===
	    NEQ, // !==
	    WEQ, // ==?
	    WNE, // !=?
	    XEQ, // casex guard tests
	    ZEQ  // casez guard tests
      };

    public:
      explicit NetCaseCmp(NetScope*s, perm_string n, unsigned wid, kind_t eeq);
      ~NetCaseCmp();

      unsigned width() const;
	// What kind of case compare?
      inline kind_t kind() const { return kind_; }

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;

    private:
      unsigned width_;
      const kind_t kind_;
};

extern std::ostream& operator << (std::ostream&fd, NetCaseCmp::kind_t that);

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

      inline const verinum&value(void) const { return value_; }
      verinum::V value(unsigned idx) const;
      inline unsigned width() const { return value_.len(); }
      inline bool is_string() const { return value_.is_string(); }

      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);
      virtual void dump_node(std::ostream&, unsigned ind) const;

    private:
      verinum value_;
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
      virtual void dump_node(std::ostream&, unsigned ind) const;

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
      enum TYPE { AND, BUF, BUFIF0, BUFIF1, CMOS, EQUIV, IMPL, NAND, NMOS,
		  NOR, NOT, NOTIF0, NOTIF1, OR, PULLDOWN, PULLUP, RCMOS,
		  RNMOS, RPMOS, PMOS, XNOR, XOR };

      explicit NetLogic(NetScope*s, perm_string n, unsigned pins,
			TYPE t, unsigned wid, bool is_cassign__=false);

      TYPE type() const;
      unsigned width() const;
      bool is_cassign() const;

      virtual void dump_node(std::ostream&, unsigned ind) const;
      virtual bool emit_node(struct target_t*) const;
      virtual void functor_node(Design*, functor_t*);

    private:
      TYPE type_;
      unsigned width_;
      bool is_cassign_;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

      virtual void dump_node(std::ostream&, unsigned ind) const;
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

class NetUDP  : public NetNode {

    public:
      explicit NetUDP(NetScope*s, perm_string n, unsigned pins, PUdp*u);

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(std::ostream&, unsigned ind) const;

	/* Use these methods to scan the truth table of the
	   device. "first" returns the first item in the table, and
	   "next" returns the next item in the table. The method will
	   return false when the scan is done. */
      bool first(std::string&inp, char&out) const;
      bool next(std::string&inp, char&out) const;
      unsigned rows() const { return udp->tinput.size(); }

      unsigned nin() const { return pin_count()-1; }
      bool is_sequential() const { return udp->sequential; }
      perm_string udp_name() const { return udp->name_; }
      perm_string udp_file() const { return udp->get_file(); }
      unsigned udp_lineno() const { return udp->get_lineno(); }
      char get_initial() const;

      unsigned port_count() const;
      std::string port_name(unsigned idx) const;

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
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

	// Find the nexa that are set by the statement. Add the output
	// values to the set passed as a parameter.
      virtual void nex_output(NexusSet&);

	// This method is called to emit the statement to the
	// target. The target returns true if OK, false for errors.
      virtual bool emit_proc(struct target_t*) const;

	// This method is used by the NetFuncDef object to evaluate a
	// constant function at compile time. The loc is the location
	// of the function call, and is used for error messages. The
	// ctx is a map of name to expression. This is for mapping
	// identifiers to values. The function returns true if the
	// processing succeeds, or false otherwise.
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

	// This method is called by functors that want to scan a
	// process in search of matchable patterns.
      virtual int match_proc(struct proc_match_t*);

	// Return true if this represents the root of a combinational
	// process. Most process types are not.
      virtual bool is_asynchronous();

	// Return true if this represents the root of a synchronous
	// process. Most process types are not.
      virtual bool is_synchronous();

	// Synthesize as asynchronous logic, and return true on success.
	//
	// nex_map holds the set of nexuses that are driven by this
	// process, nex_out holds the accumulated outputs from this and
	// preceding sequential processes (i.e statements in the same
	// block), enables holds the accumulated clock/gate enables,
	// and bitmasks holds the accumulated masks that flag which bits
	// are unconditionally driven (i.e. driven by every clause in
	// every statement). On output, the values passed in to nex_out,
	// enables, and bitmasks may either be merged with or replaced
	// by the values originating from this process, depending on the
	// type of statement this process represents.
	//
	// The clock/gate enables generated by synthesis operate at a
	// vector level (i.e. they are asserted if any bit(s) in the
	// vector are driven).
      typedef std::vector<bool> mask_t;
      virtual bool synth_async(Design*des, NetScope*scope,
			       NexusSet&nex_map, NetBus&nex_out,
			       NetBus&enables, std::vector<mask_t>&bitmasks);

	// Synthesize as synchronous logic, and return true on success.
	// That means binding the outputs to the data port of a FF, and
	// the event inputs to a FF clock. Only some key NetProc sub-types
	// that have specific meaning in synchronous statements. The
	// remainder reduce to a call to synth_async that connects the
	// output to the Data input of the FF.
	//
	// The nex_map, nex_out, ff_ce, and bitmasks arguments serve
	// the same purpose as in the synth_async method (where ff_ce
	// is equivalent to enables). The events argument is filled
	// in by the NetEvWait implementation of this method with the
	// probes that it does not itself pick off as a clock. These
	// events should be picked off by e.g. condit statements as
	// asynchronous set/reset inputs to the flipflop being generated.
      virtual bool synth_sync(Design*des, NetScope*scope,
			      bool&ff_negedge,
			      NetNet*ff_clock, NetBus&ff_ce,
			      NetBus&ff_aclr,  NetBus&ff_aset,
			      std::vector<verinum>&ff_aset_value,
			      NexusSet&nex_map, NetBus&nex_out,
			      std::vector<mask_t>&bitmasks,
			      const std::vector<NetEvProbe*>&events);

      virtual void dump(std::ostream&, unsigned ind) const;

	// Recursively checks to see if there is delay in this element.
      virtual DelayType delay_type(bool print_delay=false) const;
	// Check to see if the item is synthesizable.
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

    protected:
      bool synth_async_block_substatement_(Design*des, NetScope*scope,
					   NexusSet&nex_map,
					   NetBus&nex_out,
					   NetBus&enables,
					   std::vector<mask_t>&bitmasks,
					   NetProc*substmt);
    private:
      friend class NetBlock;
      NetProc*next_;

    private: // not implemented
      NetProc(const NetProc&);
      NetProc& operator= (const NetProc&);
};

class NetAlloc  : public NetProc {

    public:
      explicit NetAlloc(NetScope*);
      ~NetAlloc();

      const std::string name() const;

      const NetScope* scope() const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;

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
      explicit NetAssign_(NetAssign_*nest);
      explicit NetAssign_(NetNet*sig);
      ~NetAssign_();

	// This is so NetAssign_ objects can be passed to ivl_assert
	// and other macros that call this method.
      std::string get_fileline() const;

	// If this expression exists, then it is used to select a word
	// from an array/memory.
      NetExpr*word();
      const NetExpr*word() const;

      NetScope*scope()const;

	// Get the base index of the part select, or 0 if there is no
	// part select.
      const NetExpr* get_base() const;
      ivl_select_type_t select_type() const;

      void set_word(NetExpr*);
	// Set a part select expression for the l-value vector. Note
	// that the expression calculates a CANONICAL bit address.
      void set_part(NetExpr* loff, unsigned wid,
                    ivl_select_type_t = IVL_SEL_OTHER);
	// Set a part select expression for the l-value vector. Note
	// that the expression calculates a CANONICAL bit address.
	// The part select has a specific type and the width of the select will
	// be that of the type.
      void set_part(NetExpr *loff, ivl_type_t data_type);
	// Set the member or property name if the signal type is a
	// class.
      void set_property(const perm_string&name, unsigned int idx);
      inline int get_property_idx(void) const { return member_idx_; }

	// Determine if the assigned object is signed or unsigned.
	// This is used when determining the expression type for
	// a compressed assignment statement.
      bool get_signed() const { return signed_; }
      void set_signed(bool flag) { signed_ = flag; }

	// Get the width of the r-value that this node expects. This
	// method accounts for the presence of the mux, so it is not
	// necessarily the same as the pin_count().
      unsigned lwidth() const;
      ivl_variable_type_t expr_type() const;

	// Get the expression type of the l-value. This may be
	// different from the type of the contained signal if for
	// example a darray is indexed.
      ivl_type_t net_type() const;

	// Get the name of the underlying object.
      perm_string name() const;

      NetNet* sig() const;
      inline const NetAssign_* nest() const { return nest_; }

	// Mark that the synthesizer has worked with this l-value, so
	// when it is released, the l-value signal should be turned
	// into a wire.
      void turn_sig_to_wire_on_release();

	// It is possible that l-values can have *inputs*, as well as
	// being outputs. For example foo[idx] = ... is the l-value
	// (NetAssign_ object) with a foo l-value and the input
	// expression idx.
      NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                          bool nested_func = false) const;

	// Figuring out nex_output to process ultimately comes down to
	// this method.
      void nex_output(NexusSet&);

	// This pointer is for keeping simple lists.
      NetAssign_* more;

      void dump_lval(std::ostream&o) const;

    private:
	// Nested l-value. If this is set, sig_ must NOT be set!
      NetAssign_*nest_;
      NetNet *sig_;
	// Memory word index
      NetExpr*word_;
	// member/property if signal is a class.
      perm_string member_;
      int member_idx_ = -1;

      bool signed_;
      bool turn_sig_to_wire_on_release_;
	// indexed part select base
      NetExpr*base_;
      unsigned lwid_;
      ivl_select_type_t sel_type_;
      ivl_type_t part_data_type_ = nullptr;
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

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&o);


	// This returns the total width of the accumulated l-value. It
	// accounts for any grouping of NetAssign_ objects that might happen.
      unsigned lwidth() const;

      bool synth_async(Design*des, NetScope*scope,
		       NexusSet&nex_map, NetBus&nex_out,
		       NetBus&enables, std::vector<mask_t>&bitmasks);

	// This dumps all the lval structures.
      void dump_lval(std::ostream&) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

    private:
      NetAssign_*lval_;
      NetExpr   *rval_;
      NetExpr   *delay_;
};

class NetAssign : public NetAssignBase {

    public:
      explicit NetAssign(NetAssign_*lv, NetExpr*rv);
      explicit NetAssign(NetAssign_*lv, char op, NetExpr*rv);
      ~NetAssign();

      bool is_asynchronous();

      inline char assign_operator(void) const { return op_; }

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

    private:
      void eval_func_lval_op_real_(const LineInfo&loc, verireal&lv, const verireal&rv) const;
      void eval_func_lval_op_(const LineInfo&loc, verinum&lv, verinum&rv) const;
      bool eval_func_lval_(const LineInfo&loc, std::map<perm_string,LocalVar>&ctx,
			   const NetAssign_*lval, NetExpr*rval_result) const;

      char op_;
};

class NetAssignNB  : public NetAssignBase {
    public:
      explicit NetAssignNB(NetAssign_*lv, NetExpr*rv, NetEvWait*ev,
                           NetExpr*cnt);
      ~NetAssignNB();


      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

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
      enum Type { SEQU, PARA, PARA_JOIN_ANY, PARA_JOIN_NONE };

      NetBlock(Type t, NetScope*subscope);
      ~NetBlock();

      Type type() const    { return type_; }
      NetScope* subscope() const { return subscope_; }

      void append(NetProc*);
      void prepend(NetProc*);

      const NetProc*proc_first() const;
      const NetProc*proc_next(const NetProc*cur) const;

      bool evaluate_function(const LineInfo&loc,
			     std::map<perm_string,LocalVar>&ctx) const;

	// synthesize as asynchronous logic, and return true.
      bool synth_async(Design*des, NetScope*scope,
		       NexusSet&nex_map, NetBus&nex_out,
		       NetBus&enables, std::vector<mask_t>&bitmasks);

      bool synth_sync(Design*des, NetScope*scope,
		      bool&ff_negedge,
		      NetNet*ff_clk, NetBus&ff_ce,
		      NetBus&ff_aclr,NetBus&ff_aset,
		      std::vector<verinum>&ff_aset_value,
		      NexusSet&nex_map, NetBus&nex_out,
		      std::vector<mask_t>&bitmasks,
		      const std::vector<NetEvProbe*>&events);

	// This version of emit_recurse scans all the statements of
	// the begin-end block sequentially. It is typically of use
	// for sequential blocks.
      void emit_recurse(struct target_t*) const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

    private:
      const Type type_;
      NetScope*subscope_;

      NetProc*last_;
};

class NetBreak : public NetProc {
    public:
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      bool evaluate_function(const LineInfo &loc,
			     std::map<perm_string,LocalVar> &ctx) const final;
};

/*
 * A CASE statement in the Verilog source leads, eventually, to one of
 * these. This is different from a simple conditional because of the
 * way the comparisons are performed. Also, it is likely that the
 * target may be able to optimize differently.
 *
 * Case statements can have unique, unique0, or priority attached to
 * them. If not otherwise adorned, it is QBASIC.
 *
 * Case can be one of three types:
 *    EQ  -- All bits must exactly match
 *    EQZ -- z bits are don't care
 *    EQX -- x and z bits are don't care.
 */
class NetCase  : public NetProc {

    public:
      enum TYPE { EQ, EQX, EQZ };

      NetCase(ivl_case_quality_t q, TYPE c, NetExpr*ex, unsigned cnt);
      ~NetCase();

      void set_case(unsigned idx, NetExpr*ex, NetProc*st);

      void prune();

      inline ivl_case_quality_t case_quality() const { return quality_; }
      TYPE type() const;
      const NetExpr*expr() const { return expr_; }
      inline unsigned nitems() const { return items_.size(); }

      inline const NetExpr*expr(unsigned idx) const { return items_[idx].guard;}
      inline const NetProc*stat(unsigned idx) const { return items_[idx].statement; }

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&out);

      bool synth_async(Design*des, NetScope*scope,
		       NexusSet&nex_map, NetBus&nex_out,
		       NetBus&enables, std::vector<mask_t>&bitmasks);

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

    private:
      bool evaluate_function_vect_(const LineInfo&loc,
				   std::map<perm_string,LocalVar>&ctx) const;
      bool evaluate_function_real_(const LineInfo&loc,
				   std::map<perm_string,LocalVar>&ctx) const;

      bool synth_async_casez_(Design*des, NetScope*scope,
			      NexusSet&nex_map, NetBus&nex_out,
			      NetBus&enables, std::vector<mask_t>&bitmasks);

      ivl_case_quality_t quality_;
      TYPE type_;

      struct Item {
	    inline Item() : guard(0), statement(0) { }
	    NetExpr*guard;
	    NetProc*statement;
      };

      NetExpr* expr_;
      std::vector<Item>items_;
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

      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

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

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&o);

      bool is_asynchronous();
      bool synth_async(Design*des, NetScope*scope,
		       NexusSet&nex_map, NetBus&nex_out,
		       NetBus&enables, std::vector<mask_t>&bitmasks);

      bool synth_sync(Design*des, NetScope*scope,
		      bool&ff_negedge,
		      NetNet*ff_clk, NetBus&ff_ce,
		      NetBus&ff_aclr,NetBus&ff_aset,
		      std::vector<verinum>&ff_aset_value,
		      NexusSet&nex_map, NetBus&nex_out,
		      std::vector<mask_t>&bitmasks,
		      const std::vector<NetEvProbe*>&events);

      virtual bool emit_proc(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

    private:
      NetExpr* expr_;
      NetProc*if_;
      NetProc*else_;
};

class NetContinue : public NetProc {
    public:
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      bool evaluate_function(const LineInfo &loc,
			     std::map<perm_string,LocalVar> &ctx) const final;
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
      virtual void dump(std::ostream&, unsigned ind) const;

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
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

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
      explicit NetDisable(NetScope*tgt, bool flow_control = false);
      ~NetDisable();

      const NetScope*target() const;
      bool flow_control() const { return flow_control_; }

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

    private:
      NetScope*target_;
       // If false all threads in the target_ scope are disabled. If true only
       // the closest thread in thread hierarchy of the target_ scope is
       // disabled. The latter is used to implement flow control statements like
       // `return`.
      bool flow_control_;

    private: // not implemented
      NetDisable(const NetDisable&);
      NetDisable& operator= (const NetDisable&);
};

/*
 * The do/while statement is a condition that is tested at the end of
 * each iteration, and a statement (a NetProc) that is executed once and
 * then again as long as the condition is true.
 */
class NetDoWhile  : public NetProc {

    public:
      NetDoWhile(NetExpr*c, NetProc*p)
      : cond_(c), proc_(p) { }

      const NetExpr*expr() const { return cond_; }

      void emit_proc_recurse(struct target_t*) const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

    private:
      NetExpr* cond_;
      NetProc*proc_;
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
 * The NetEvNBTrig class represents non-blocking trigger statements.
 * Executing this statement causes the referenced event to be triggered
 * at some time in the future, which in turn awakens the waiting threads.
 * Each NetEvNBTrig object references exactly one event object.
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
      friend class NetEvNBTrig;
      friend class NetEvWait;
      friend class NetEEvent;

    public:
	// The name of the event is the basename, and should not
	// include the scope. Also, the name passed here should be
	// perm-allocated.
      explicit NetEvent (perm_string n);
      ~NetEvent();

      perm_string name() const;

      unsigned lexical_pos() const { return lexical_pos_; }
      void lexical_pos(unsigned lp) { lexical_pos_ = lp; }

      bool local_flag() const { return local_flag_; }
      void local_flag(bool f) { local_flag_ = f; }

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
      void find_similar_event(std::list<NetEvent*>&);

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
      unsigned lexical_pos_;
      bool local_flag_;

	// The NetScope class uses these to list the events.
      NetScope*scope_;
      NetEvent*snext_;

	// Use these methods to list the probes attached to me.
      NetEvProbe*probes_;

	// Use these methods to list the triggers attached to me.
      NetEvTrig* trig_;

	// Use these methods to list the non-blocking triggers attached to me.
      NetEvNBTrig* nb_trig_;

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

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

    private:
      NetEvent*event_;
	// This is used to place me in the NetEvents lists of triggers.
      NetEvTrig*enext_;
};

class NetEvNBTrig  : public NetProc {

      friend class NetEvent;

    public:
      explicit NetEvNBTrig(NetEvent*tgt, NetExpr*dly);
      ~NetEvNBTrig();

      const NetExpr*delay() const;
      const NetEvent*event() const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

    private:
      NetEvent*event_;
      NetExpr*dly_;
	// This is used to place me in the NetEvents lists of triggers.
      NetEvNBTrig*enext_;
};

class NetEvWait  : public NetProc {

    public:
      explicit NetEvWait(NetProc*st);
      ~NetEvWait();

      void add_event(NetEvent*tgt);
      void replace_event(NetEvent*orig, NetEvent*repl);
      inline void set_t0_trigger() { has_t0_trigger_ = true; };

      inline unsigned nevents() const { return events_.size(); }
      inline const NetEvent*event(unsigned idx) const { return events_[idx]; }
      inline NetEvent*event(unsigned idx) { return events_[idx]; }
      inline bool has_t0_trigger() const { return has_t0_trigger_; };

      NetProc*statement();
      const NetProc*statement() const;

      virtual bool emit_proc(struct target_t*) const;
      bool emit_recurse(struct target_t*) const;
      virtual int match_proc(struct proc_match_t*);

	// It is possible that this is the root of a combinational
	// process. This method checks.
      virtual bool is_asynchronous();

	// It is possible that this is the root of a synchronous
	// process? This method checks.
      virtual bool is_synchronous();

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&out);

      virtual bool synth_async(Design*des, NetScope*scope,
			       NexusSet&nex_map, NetBus&nex_out,
			       NetBus&enables, std::vector<mask_t>&bitmasks);

      virtual bool synth_sync(Design*des, NetScope*scope,
			      bool&ff_negedge,
			      NetNet*ff_clk, NetBus&ff_ce,
			      NetBus&ff_aclr,NetBus&ff_aset,
			      std::vector<verinum>&ff_aset_value,
			      NexusSet&nex_map, NetBus&nex_out,
			      std::vector<mask_t>&bitmasks,
			      const std::vector<NetEvProbe*>&events);

      virtual void dump(std::ostream&, unsigned ind) const;
	// This will ignore any statement.
      virtual void dump_inline(std::ostream&) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

    private:
      NetProc*statement_;
	// Events that I might wait for.
      std::vector<NetEvent*>events_;
      bool has_t0_trigger_;
};

std::ostream& operator << (std::ostream&out, const NetEvWait&obj);

class NetEvProbe  : public NetNode {

      friend class NetEvent;

    public:
      enum edge_t { ANYEDGE, POSEDGE, NEGEDGE, EDGE };

      explicit NetEvProbe(NetScope*s, perm_string n,
			  NetEvent*tgt, edge_t t, unsigned p);
      ~NetEvProbe();

      edge_t edge() const;
      NetEvent* event();
      const NetEvent* event() const;

      void find_similar_probes(std::list<NetEvProbe*>&);

      virtual bool emit_node(struct target_t*) const;
      virtual void dump_node(std::ostream&, unsigned ind) const;

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

      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool emit_proc(struct target_t*) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
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

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

    private:
      NetProc*statement_;
};

class NetForLoop : public NetProc {

    public:
      explicit NetForLoop(NetNet*index, NetExpr*initial_expr, NetExpr*cond,
			  NetProc*sub, NetProc*step);
      ~NetForLoop();

      void emit_recurse(struct target_t*) const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

      bool emit_recurse_init(struct target_t*) const;
      bool emit_recurse_stmt(struct target_t*) const;
      bool emit_recurse_step(struct target_t*) const;
      bool emit_recurse_condition(struct expr_scan_t*) const;

	// synthesize as asynchronous logic, and return true.
      bool synth_async(Design*des, NetScope*scope,
		       NexusSet&nex_map, NetBus&nex_out,
		       NetBus&enables, std::vector<mask_t>&bitmasks);

    private:
      NetNet*index_;
      NetExpr*init_expr_;
      NetProc*init_statement_; // Generated form index_ and init_expr_.
      NetExpr*condition_;
      NetProc*statement_;
      NetProc*step_statement_;
};

class NetFree   : public NetProc {

    public:
      explicit NetFree(NetScope*);
      ~NetFree();

      const std::string name() const;

      const NetScope* scope() const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;

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
class NetFuncDef : public NetBaseDef {

    public:
      NetFuncDef(NetScope*, NetNet*result, const std::vector<NetNet*>&po,
		 const std::vector<NetExpr*>&pd);
      ~NetFuncDef();

	// Return true if the function returns "void". We still treat
	// it as a function since we need to check that the contents
	// meet the requirements of a function, but we need to know
	// that it is void because it can be evaluated differently.
      inline bool is_void() const { return result_sig_ == 0; }

	// Non-void functions have a return value as a signal.
      const NetNet*return_sig() const;

	// When we want to evaluate the function during compile time,
	// use this method to pass in the argument and get out a
	// result. The result should be a constant. If the function
	// cannot evaluate to a constant, this returns nil.
      NetExpr* evaluate_function(const LineInfo&loc, const std::vector<NetExpr*>&args) const;

      void dump(std::ostream&, unsigned ind) const;

    private:
      NetNet*result_sig_;
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

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);

      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

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

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

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
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

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
      NetSTask(const char*na, ivl_sfunc_as_task_t sfat,
               const std::vector<NetExpr*>&);
      ~NetSTask();

      const char* name() const;
      ivl_sfunc_as_task_t sfunc_as_task() const;

      unsigned nparms() const;

      const NetExpr* parm(unsigned idx) const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

    private:
      const char* name_;
      ivl_sfunc_as_task_t sfunc_as_task_;
      std::vector<NetExpr*>parms_;
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
class NetTaskDef : public NetBaseDef {

    public:
      NetTaskDef(NetScope*n, const std::vector<NetNet*>&po,
		 const std::vector<NetExpr*>&pd);
      ~NetTaskDef();

      void dump(std::ostream&, unsigned) const;
      DelayType delay_type(bool print_delay=false) const;

    private: // not implemented
      NetTaskDef(const NetTaskDef&);
      NetTaskDef& operator= (const NetTaskDef&);
};

/*
 * The NetELast expression node takes as an argument a net, that is
 * intended to be a queue or dynamic array object. The return value is
 * the index of the last item in the node. This is intended to
 * implement the '$' is the expression "foo[$]".
 */
class NetELast : public NetExpr {

    public:
      explicit NetELast(NetNet*sig);
      ~NetELast();

      inline const NetNet*sig() const { return sig_; }

      virtual ivl_variable_type_t expr_type() const;
      virtual void dump(std::ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetELast*dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

    private:
      NetNet*sig_;
};

/*
 * This node represents a function call in an expression. The object
 * contains a pointer to the function definition, which is used to
 * locate the value register and input expressions.
 */
class NetEUFunc  : public NetExpr {

    public:
      NetEUFunc(NetScope*, NetScope*, NetESignal*, std::vector<NetExpr*>&, bool);
      ~NetEUFunc();

      const NetESignal*result_sig() const;

      unsigned parm_count() const;
      const NetExpr* parm(unsigned idx) const;

      const NetScope* func() const;

      virtual void dump(std::ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEUFunc*dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual NetExpr* eval_tree();
      virtual NetExpr*evaluate_function(const LineInfo&loc,
					std::map<perm_string,LocalVar>&ctx) const;

      virtual NetNet* synthesize(Design*des, NetScope*scope, NetExpr*root);

    private:
      NetScope*scope_;
      NetScope*func_;
      NetESignal*result_sig_;
      std::vector<NetExpr*> parms_;
      bool need_const_;

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
      virtual void dump(std::ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEAccess*dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

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
      explicit NetUTask(NetScope*);
      ~NetUTask();

      const std::string name() const;

      const NetScope* task() const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;

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

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void nex_output(NexusSet&);
      virtual bool emit_proc(struct target_t*) const;
      virtual void dump(std::ostream&, unsigned ind) const;
      virtual DelayType delay_type(bool print_delay=false) const;
      virtual bool check_synth(ivl_process_type_t pr_type, const NetScope*scope) const;
      virtual bool evaluate_function(const LineInfo&loc,
				     std::map<perm_string,LocalVar>&ctx) const;

    private:
      NetExpr*cond_;
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
      bool is_asynchronous() const;

	/* Create asynchronous logic from this thread and return true,
	   or return false if that cannot be done. */
      bool synth_async(Design*des);

	/* Return true if this process represents synchronous logic. */
      bool is_synchronous();

	/* Create synchronous logic from this thread and return true,
	   or return false if that cannot be done. */
      bool synth_sync(Design*des);

      void dump(std::ostream&, unsigned ind) const;
      bool emit(struct target_t*tgt) const;

    private:
      bool tie_off_floating_inputs_(Design*des,
				    NexusSet&nex_map, NetBus&nex_in,
				    const std::vector<NetProc::mask_t>&bitmasks,
				    bool is_ff_input);

      const ivl_process_type_t type_;
      NetProc*const statement_;
      Design*synthesized_design_;

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

      void dump(std::ostream&, unsigned ind) const;
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
      NetEBinary(char op, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag);
      ~NetEBinary();

      const NetExpr*left() const { return left_; }
      const NetExpr*right() const { return right_; }

      char op() const { return op_; }

	// A binary expression node only has a definite
	// self-determinable width if the operands both have definite
	// widths.
      virtual bool has_width() const;

      virtual NetEBinary* dup_expr() const;
      virtual NetExpr* eval_tree();
      virtual NetExpr* evaluate_function(const LineInfo&loc,
					 std::map<perm_string,LocalVar>&ctx) const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;

    protected:
      char op_;
      NetExpr* left_;
      NetExpr* right_;

      virtual NetExpr* eval_arguments_(const NetExpr*l, const NetExpr*r) const;
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
      NetEBAdd(char op, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag);
      ~NetEBAdd();

      virtual ivl_variable_type_t expr_type() const;

      virtual NetEBAdd* dup_expr() const;
      virtual NetExpr* eval_tree();
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      virtual NetExpr* eval_arguments_(const NetExpr*l, const NetExpr*r) const;
      NetECReal* eval_tree_real_(const NetExpr*l, const NetExpr*r) const;
};

/*
 * This class represents the integer division operators.
 *   /  -- Divide
 *   %  -- Modulus
 */
class NetEBDiv : public NetEBinary {

    public:
      NetEBDiv(char op, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag);
      ~NetEBDiv();

      virtual ivl_variable_type_t expr_type() const;

      virtual NetEBDiv* dup_expr() const;
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      virtual NetExpr* eval_arguments_(const NetExpr*l, const NetExpr*r) const;
      NetExpr* eval_tree_real_(const NetExpr*l, const NetExpr*r) const;
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
      NetEBBits(char op, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag);
      ~NetEBBits();

      virtual NetEBBits* dup_expr() const override;
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root) override;

      ivl_variable_type_t expr_type() const override;

    private:
      virtual NetEConst* eval_arguments_(const NetExpr*l, const NetExpr*r) const override;
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

	/* A compare expression has a definite width. */
      virtual bool has_width() const;
      virtual ivl_variable_type_t expr_type() const;
      virtual NetEBComp* dup_expr() const;
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      NetEConst* must_be_leeq_(const NetExpr*le, const verinum&rv, bool eq_flag) const;

      virtual NetEConst*eval_arguments_(const NetExpr*le, const NetExpr*re) const;
      NetEConst*eval_eqeq_(bool ne_flag, const NetExpr*le, const NetExpr*re) const;
      NetEConst*eval_eqeq_real_(bool ne_flag, const NetExpr*le, const NetExpr*re) const;
      NetEConst*eval_less_(const NetExpr*le, const NetExpr*re) const;
      NetEConst*eval_leeq_(const NetExpr*le, const NetExpr*re) const;
      NetEConst*eval_leeq_real_(const NetExpr*le, const NetExpr*ri, bool eq_flag) const;
      NetEConst*eval_gt_(const NetExpr*le, const NetExpr*re) const;
      NetEConst*eval_gteq_(const NetExpr*le, const NetExpr*re) const;
      NetEConst*eval_eqeqeq_(bool ne_flag, const NetExpr*le, const NetExpr*re) const;
      NetEConst*eval_weqeq_(bool ne_flag, const NetExpr*le, const NetExpr*re) const;
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

      virtual NetEBLogic* dup_expr() const override;
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root) override;

      ivl_variable_type_t expr_type() const override;

    private:
      virtual NetEConst* eval_arguments_(const NetExpr*l, const NetExpr*r) const override;
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
      NetEBMinMax(char op, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag);
      ~NetEBMinMax();

      virtual ivl_variable_type_t expr_type() const;

    private:
      virtual NetExpr* eval_arguments_(const NetExpr*l, const NetExpr*r) const;
      NetExpr* eval_tree_real_(const NetExpr*l, const NetExpr*r) const;
};

/*
 * Support the binary multiplication (*) operator.
 */
class NetEBMult : public NetEBinary {

    public:
      NetEBMult(char op, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag);
      ~NetEBMult();

      virtual ivl_variable_type_t expr_type() const;

      virtual NetEBMult* dup_expr() const;
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      virtual NetExpr* eval_arguments_(const NetExpr*l, const NetExpr*r) const;
      NetExpr* eval_tree_real_(const NetExpr*l, const NetExpr*r) const;
};

/*
 * Support the binary power (**) operator.
 */
class NetEBPow : public NetEBinary {

    public:
      NetEBPow(char op, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag);
      ~NetEBPow();

      virtual ivl_variable_type_t expr_type() const;

      virtual NetEBPow* dup_expr() const;
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
      virtual NetExpr* eval_arguments_(const NetExpr*l, const NetExpr*r) const;
      NetExpr* eval_tree_real_(const NetExpr*l, const NetExpr*r) const;
};


/*
 * Support the binary shift operators. The supported operators are:
 *
 *   l  -- left shift (<<)
 *   r  -- right shift (>>)
 *   R  -- right shift arithmetic (>>>)
 */
class NetEBShift : public NetEBinary {

    public:
      NetEBShift(char op, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag);
      ~NetEBShift();

	// A shift expression only needs the left expression to have a
	// definite width to give the expression a definite width.
      virtual bool has_width() const override;

      virtual NetEBShift* dup_expr() const override;
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root) override;

      ivl_variable_type_t expr_type() const override;

    private:
      virtual NetEConst* eval_arguments_(const NetExpr*l, const NetExpr*r) const override;
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
      NetEConcat(unsigned cnt, unsigned repeat, ivl_variable_type_t vt);
      ~NetEConcat();

	// Manipulate the parameters.
      void set(unsigned idx, NetExpr*e);

      unsigned repeat() const { return repeat_; }
      unsigned nparms() const { return parms_.size() ; }
      NetExpr* parm(unsigned idx) const { return parms_[idx]; }

      virtual ivl_variable_type_t expr_type() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual NetEConcat* dup_expr() const;
      virtual NetEConst*  eval_tree();
      virtual NetExpr* evaluate_function(const LineInfo&loc,
					 std::map<perm_string,LocalVar>&ctx) const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*root);
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;

    private:
      std::vector<NetExpr*>parms_;
      unsigned repeat_;
      ivl_variable_type_t expr_type_;

      NetEConst* eval_arguments_(const std::vector<NetExpr*>&vals, unsigned gap) const;
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
 *
 * An alternative form of this expression node is used for dynamic
 * array word selects and for packed struct member selects. In this
 * case use_type indicates the type of the selected element/member.
 */
class NetESelect  : public NetExpr {

    public:
      NetESelect(NetExpr*exp, NetExpr*base, unsigned wid,
                 ivl_select_type_t sel_type = IVL_SEL_OTHER);
      NetESelect(NetExpr*exp, NetExpr*base, unsigned wid,
                 ivl_type_t use_type);
      ~NetESelect();

      const NetExpr*sub_expr() const;
      const NetExpr*select() const;
      ivl_select_type_t select_type() const;

	// The type of a bit/part select is the base type of the
	// sub-expression. The type of an array/member select is
	// the base type of the element/member.
      virtual ivl_variable_type_t expr_type() const;

      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEConst* eval_tree();
      virtual NetExpr*evaluate_function(const LineInfo&loc,
					std::map<perm_string,LocalVar>&ctx) const;
      virtual NetESelect* dup_expr() const;
      virtual NetNet*synthesize(Design*des, NetScope*scope, NetExpr*root);
      virtual void dump(std::ostream&) const;

    private:
      NetExpr*expr_;
      NetExpr*base_;
      ivl_select_type_t sel_type_;
};

/*
 * This node is for representation of named events.
 */
class NetEEvent : public NetExpr {

    public:
      explicit NetEEvent(NetEvent*);
      ~NetEEvent();

      const NetEvent* event() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEEvent* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual void dump(std::ostream&os) const;

    private:
      NetEvent*event_;
};

/*
 * This class is a special (and magical) expression node type that
 * represents enumeration types. These can only be found as parameters
 * to NetSTask objects.
 */
class NetENetenum  : public NetExpr {

    public:
      explicit NetENetenum(const netenum_t*);
      ~NetENetenum();

      const netenum_t* netenum() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetENetenum* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual void dump(std::ostream&os) const;

    private:
      const netenum_t*netenum_;
};

class NetENew : public NetExpr {
    public:
	// Make class object
      explicit NetENew(ivl_type_t);
	// dynamic array of objects.
      explicit NetENew(ivl_type_t, NetExpr*size, NetExpr* init_val=0);
      ~NetENew();

      inline const NetExpr*size_expr() const { return size_; }
      inline const NetExpr*init_expr() const { return init_val_; }

      virtual ivl_variable_type_t expr_type() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetENew* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual void dump(std::ostream&os) const;

    private:
      NetExpr*size_;
      NetExpr*init_val_;
};

/*
 * The NetENull node represents the SystemVerilog (null)
 * expression. This is always a null class handle.
 */
class NetENull : public NetExpr {

    public:
      NetENull();
      ~NetENull();

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetENull* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual void dump(std::ostream&os) const;
};

/*
 * The NetEProperty represents a SystemVerilog property select of a
 * class object. In SV, the expression would look like "a.b", where
 * the "a" is the signal (the NetNet) and "b" is the property name.
 *
 * The canon_index is an optional expression to address an element for
 * parameters that are arrays.
 */
class NetEProperty : public NetExpr {
    public:
      NetEProperty(NetNet*n, size_t pidx_, NetExpr*canon_index =0);
      ~NetEProperty();

      inline const NetNet* get_sig() const { return net_; }
      inline size_t property_idx() const { return pidx_; }
      inline const NetExpr*get_index() const { return index_; }

    public: // Overridden methods
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEProperty* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual void dump(std::ostream&os) const;

    private:
      NetNet*net_;
      size_t pidx_;
      NetExpr*index_;
};

/*
 * This class is a special (and magical) expression node type that
 * represents scope names. These can only be found as parameters to
 * NetSTask objects.
 */
class NetEScope  : public NetExpr {

    public:
      explicit NetEScope(NetScope*);
      ~NetEScope();

      const NetScope* scope() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEScope* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual void dump(std::ostream&os) const;

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
		unsigned width, unsigned nprms, bool is_overridden =false);
      NetESFunc(const char*name, ivl_type_t rtype, unsigned nprms);
      ~NetESFunc();

      const char* name() const;

      unsigned nparms() const;
      void parm(unsigned idx, NetExpr*expr);
      NetExpr* parm(unsigned idx);
      const NetExpr* parm(unsigned idx) const;

      virtual NetExpr* eval_tree();
      virtual NetExpr* evaluate_function(const LineInfo&loc,
					 std::map<perm_string,LocalVar>&ctx) const;

      virtual ivl_variable_type_t expr_type() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void dump(std::ostream&) const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetESFunc*dup_expr() const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*root);

    private:
	/* Use the 32 bit ID as follows:
	 *   The lower sixteen bits are used to identify the individual
	 *   functions.
	 *
	 *   The top sixteen bits are used to indicate the number of
	 *   arguments the function can take by bit position. If more
	 *   than one bit is set the argument can take a different number
	 *   of arguments. This varies from 0 to 14 with the MSB indicating
	 *   fifteen or more (an unbounded value). For example all bit set
	 *   except for the LSB indicate 1 or more arguments are allowed.
	 */
      enum ID { NOT_BUILT_IN = 0x0,
                  /* Available in all version of Verilog/SystemVerilog. */
                ITOR   = 0x00020001,  /* $itor takes one argument. */
                RTOI   = 0x00020002,  /* $rtoi takes one argument. */
                  /* Available in Verilog 2005 and later. */
                ACOS   = 0x00020003,  /* $acos takes one argument. */
                ACOSH  = 0x00020004,  /* $acosh takes one argument. */
                ASIN   = 0x00020005,  /* $asin takes one argument. */
                ASINH  = 0x00020006,  /* $asinh takes one argument. */
                ATAN   = 0x00020007,  /* $atan takes one argument. */
                ATANH  = 0x00020008,  /* $atanh takes one argument. */
                ATAN2  = 0x00040009,  /* $atan2 takes two argument. */
                CEIL   = 0x0002000a,  /* $ceil takes one argument. */
                CLOG2  = 0x0002000b,  /* $clog2 takes one argument. */
                COS    = 0x0002000c,  /* $cos takes one argument. */
                COSH   = 0x0002000d,  /* $cosh takes one argument. */
                EXP    = 0x0002000e,  /* $exp takes one argument. */
                FLOOR  = 0x0002000f,  /* $floor takes one argument. */
                HYPOT  = 0x00040010,  /* $hypot takes two argument. */
                LN     = 0x00020011,  /* $ln takes one argument. */
                LOG10  = 0x00020012,  /* $log10 takes one argument. */
                POW    = 0x00040013,  /* $pow takes two argument. */
                SIN    = 0x00020014,  /* $sin takes one argument. */
                SINH   = 0x00020015,  /* $sinh takes one argument. */
                SQRT   = 0x00020016,  /* $sqrt takes one argument. */
                TAN    = 0x00020017,  /* $tan takes one argument. */
                TANH   = 0x00020018,  /* $tanh takes one argument. */
                  /* Added in SystemVerilog 2005 and later. */
                DIMS   = 0x00020019,  /* $dimensions takes one argument. */
                HIGH   = 0x0006001a,  /* $high takes one or two arguments. */
                INCR   = 0x0006001b,  /* $increment takes one or two arguments. */
                LEFT   = 0x0006001c,  /* $left takes one or two arguments. */
                LOW    = 0x0006001d,  /* $low takes one or two arguments. */
                RIGHT  = 0x0006001e,  /* $right takes one or two arguments. */
                SIZE   = 0x0006001f,  /* $size takes one or two arguments. */
                UPDIMS = 0x00020020,  /* $unpacked_dimensions takes one argument. */
                ISUNKN = 0x00020021,  /* $isunknown takes one argument. */
                ONEHT  = 0x00020022,  /* $onehot takes one argument. */
                ONEHT0 = 0x00020023,  /* $onehot0 takes one argument. */
                  /* Added in SystemVerilog 2009 and later. */
                CTONES = 0x00020024,  /* $countones takes one argument. */
                  /* Added in SystemVerilog 2012 and later. */
                CTBITS = 0xfffc0025,  /* $countbits takes two or more arguments. */
                  /* Added as Icarus extensions to Verilog-A. */
                ABS    = 0x00020026,  /* $abs takes one argument. */
                MAX    = 0x00040027,  /* $max takes two argument. */
                MIN    = 0x00040028,  /* $min takes two argument. */
                  /* A dummy value to properly close the enum. */
		DUMMY  = 0xffffffff };

      bool takes_nargs_(ID func, unsigned nargs) {
	    if (nargs > 15) nargs = 15;
	    return func & (1U << (nargs + 16));
      }

      const char* name_;
      ivl_variable_type_t type_;
      std::vector<NetExpr*>parms_;
      bool is_overridden_;

      ID built_in_id_() const;

      NetExpr* evaluate_one_arg_(ID id, const NetExpr*arg) const;
      NetExpr* evaluate_two_arg_(ID id, const NetExpr*arg0,
					const NetExpr*arg1) const;

      NetEConst* evaluate_rtoi_(const NetExpr*arg) const;
      NetECReal* evaluate_itor_(const NetExpr*arg) const;

      NetEConst* evaluate_clog2_(const NetExpr*arg) const;

      NetECReal* evaluate_math_one_arg_(ID id, const NetExpr*arg) const;
      NetECReal* evaluate_math_two_arg_(ID id, const NetExpr*arg0,
					       const NetExpr*arg1) const;

      NetExpr* evaluate_abs_(const NetExpr*arg) const;
      NetExpr* evaluate_min_max_(ID id, const NetExpr*arg0,
					const NetExpr*arg1) const;

	/* Constant SystemVerilog functions. */
      NetEConst* evaluate_countones_(const NetExpr*arg) const;
      NetEConst* evaluate_dimensions_(const NetExpr*arg) const;
      NetEConst* evaluate_isunknown_(const NetExpr*arg) const;
      NetEConst* evaluate_onehot_(const NetExpr*arg) const;
      NetEConst* evaluate_onehot0_(const NetExpr*arg) const;
      NetEConst* evaluate_unpacked_dimensions_(const NetExpr*arg) const;

	/* This value is used as a default when the array functions are
	 * called with a single argument. */
      static const NetEConst*const_one_;

      NetEConst* evaluate_array_funcs_(ID id,
                                       const NetExpr*arg0,
                                       const NetExpr*arg1) const;
      NetEConst* evaluate_countbits_(void) const;

    public:
      bool is_built_in() const { return built_in_id_() != NOT_BUILT_IN; };

    private: // not implemented
      NetESFunc(const NetESFunc&);
      NetESFunc& operator= (const NetESFunc&);
};

class NetEShallowCopy : public NetExpr {
    public:
	// Make a shallow copy from arg2 into arg1.
      explicit NetEShallowCopy(NetExpr*arg1, NetExpr*arg2);
      ~NetEShallowCopy();

      virtual ivl_variable_type_t expr_type() const;

      virtual void expr_scan(struct expr_scan_t*) const;
      virtual NetEShallowCopy* dup_expr() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;

      virtual void dump(std::ostream&os) const;

      void expr_scan_oper1(struct expr_scan_t*) const;
      void expr_scan_oper2(struct expr_scan_t*) const;

    private:
      NetExpr*arg1_;
      NetExpr*arg2_;
};

/*
 * This class represents the ternary (?:) operator. It has 3
 * expressions, one of which is a condition used to select which of
 * the other two expressions is the result.
 */
class NetETernary  : public NetExpr {

    public:
      NetETernary(NetExpr*c, NetExpr*t, NetExpr*f, unsigned wid, bool signed_flag);
      ~NetETernary();

      const netenum_t* enumeration() const;

      const NetExpr*cond_expr() const;
      const NetExpr*true_expr() const;
      const NetExpr*false_expr() const;

      virtual NetETernary* dup_expr() const;
      virtual NetExpr* eval_tree();
      virtual NetExpr*evaluate_function(const LineInfo&loc,
					std::map<perm_string,LocalVar>&ctx) const;
      virtual ivl_variable_type_t expr_type() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;
      virtual NetNet*synthesize(Design*, NetScope*scope, NetExpr*root);

    public:
      static bool test_operand_compat(ivl_variable_type_t tru, ivl_variable_type_t fal);

    private:
      NetExpr* blended_arguments_(const NetExpr*t, const NetExpr*f) const;

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
 *   v  -- Cast from real to integer (vector)
 *   2  -- Cast from real or logic (vector) to bool (vector)
 *   r  -- Cast from integer (vector) to real
 *   i  -- post-increment
 *   I  -- pre-increment
 *   d  -- post-decrement
 *   D  -- pre-decrement
 */
class NetEUnary  : public NetExpr {

    public:
      NetEUnary(char op, NetExpr*ex, unsigned wid, bool signed_flag);
      ~NetEUnary();

      char op() const { return op_; }
      const NetExpr* expr() const { return expr_; }

      virtual NetEUnary* dup_expr() const;
      virtual NetExpr* eval_tree();
      virtual NetExpr* evaluate_function(const LineInfo&loc,
					 std::map<perm_string,LocalVar>&ctx) const;
      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

      virtual ivl_variable_type_t expr_type() const;
      virtual NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                                  bool nested_func = false) const;
      virtual void expr_scan(struct expr_scan_t*) const;
      virtual void dump(std::ostream&) const;

    protected:
      char op_;
      NetExpr* expr_;

    private:
      virtual NetExpr* eval_arguments_(const NetExpr*ex) const;
      virtual NetExpr* eval_tree_real_(const NetExpr*ex) const;
};

class NetEUBits : public NetEUnary {

    public:
      NetEUBits(char op, NetExpr*ex, unsigned wid, bool signed_flag);
      ~NetEUBits();

      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);

      virtual NetEUBits* dup_expr() const;
      virtual ivl_variable_type_t expr_type() const;
};

class NetEUReduce : public NetEUnary {

    public:
      NetEUReduce(char op, NetExpr*ex);
      ~NetEUReduce();

      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);
      virtual NetEUReduce* dup_expr() const;
      virtual ivl_variable_type_t expr_type() const;

    private:
      virtual NetEConst* eval_arguments_(const NetExpr*ex) const;
      virtual NetEConst* eval_tree_real_(const NetExpr*ex) const;
};

class NetECast : public NetEUnary {

    public:
      NetECast(char op, NetExpr*ex, unsigned wid, bool signed_flag);
      ~NetECast();

      virtual NetNet* synthesize(Design*, NetScope*scope, NetExpr*root);
      virtual NetECast* dup_expr() const;
      virtual ivl_variable_type_t expr_type() const;
      virtual void dump(std::ostream&) const;

    private:
      virtual NetExpr* eval_arguments_(const NetExpr*ex) const;
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
      explicit NetESignal(NetNet*n);
      NetESignal(NetNet*n, NetExpr*word_index);
      ~NetESignal();

      perm_string name() const;

      virtual NetESignal* dup_expr() const;
      NetNet* synthesize(Design*des, NetScope*scope, NetExpr*root);
      NexusSet* nex_input(bool rem_out = true, bool always_sens = false,
                          bool nested_func = false) const;
      NexusSet* nex_input_base(bool rem_out, bool always_sens, bool nested_func,
                               unsigned base, unsigned width) const;

      virtual NetExpr*evaluate_function(const LineInfo&loc,
					std::map<perm_string,LocalVar>&ctx) const;

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
      virtual void dump(std::ostream&) const;

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

	/* We need to pass the tool delay selection for $sdf_annotate. */
      enum delay_sel_t { MIN, TYP, MAX };
      void set_delay_sel(delay_sel_t sel);
      const char* get_delay_sel() const;

	/* The flags are a generic way of accepting command line
	   parameters/flags and passing them to the processing steps
	   that deal with the design. The compilation driver sets the
	   entire flags map after elaboration is done. Subsequent
	   steps can then use the get_flag() function to get the value
	   of an interesting key. */

      void set_flags(const std::map<std::string,const char*>&f) { flags_ = f; }

      const char* get_flag(const std::string&key) const;

      NetScope* make_root_scope(perm_string name, NetScope*unit_scope,
				bool program_block, bool is_interface);
      NetScope* find_root_scope();
      std::list<NetScope*> find_root_scopes() const;

      NetScope* make_package_scope(perm_string name, NetScope*unit_scope,
				   bool is_unit);
      std::list<NetScope*> find_package_scopes() const;

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
      NetScope* find_scope(const hname_t&path) const;
      NetScope* find_scope(NetScope*, const hname_t&name,
                           NetScope::TYPE type = NetScope::MODULE) const;

      NetScope* find_package(perm_string name) const;

	// Note: Try to remove these versions of find_scope. Avoid
	// using these in new code, use the above forms (or
	// symbol_search) instead.
      NetScope* find_scope(const std::list<hname_t>&path) const;
      NetScope* find_scope(NetScope*, const std::list<hname_t>&path,
                           NetScope::TYPE type = NetScope::MODULE) const;

	/* These members help manage elaboration of scopes. When we
	   get to a point in scope elaboration where we want to put
	   off a scope elaboration, an object of scope_elaboration_t
	   is pushed onto the scope_elaborations list. The scope
	   elaborator will go through this list elaborating scopes
	   until the list is empty. */
      std::list<elaborator_work_item_t*>elaboration_work_list;
      void run_elaboration_work(void);

      std::set<NetScope*> defparams_later;

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
      bool check_proc_delay() const;
      bool check_proc_synth() const;

      NetNet* find_discipline_reference(ivl_discipline_t dis, NetScope*scope);

	// Iterate over the design...
      void dump(std::ostream&) const;
      void functor(struct functor_t*);
      void join_islands(void);
      int emit(struct target_t*) const;

	// This is incremented by elaboration when an error is
	// detected. It prevents code being emitted.
      unsigned errors;

      void fork_enter() { in_fork++; };
      void fork_exit() { in_fork--; };
      bool is_in_fork() { return in_fork != 0; }

      unsigned int in_fork = 0;

    private:
      NetScope* find_scope_(NetScope*, const hname_t&name,
                            NetScope::TYPE type = NetScope::MODULE) const;

      NetScope* find_scope_(NetScope*, const std::list<hname_t>&path,
                            NetScope::TYPE type = NetScope::MODULE) const;

	// Keep a tree of scopes. The NetScope class handles the wide
	// tree and per-hop searches for me.
      std::list<NetScope*>root_scopes_;

	// Keep a map of all the elaborated packages. Note that
	// packages do not nest.
      std::map<perm_string,NetScope*>packages_;

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
      std::map<perm_string,NetNet*>discipline_references_;

	// Map the design arguments to values.
      std::map<std::string,const char*> flags_;

      int des_precision_;
      delay_sel_t des_delay_sel_;

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

/* Return the number of signals in the nexus. */
extern unsigned count_signals(const Link&pin);

/* Find the next link that is an output into the nexus. */
extern Link* find_next_output(Link*lnk);

/* Find the signal connected to the given node pin. There should
   always be exactly one signal. The bidx parameter gets filled with
   the signal index of the Net, in case it is a vector. */
const NetNet* find_link_signal(const NetObj*net, unsigned pin,
			       unsigned&bidx);

inline std::ostream& operator << (std::ostream&o, const NetExpr&exp)
{ exp.dump(o); return o; }

extern std::ostream& operator << (std::ostream&, NetNet::Type);

/*
 * Manipulator to dump a scope complete path to the output. The
 * manipulator is "scope_path" and works like this:
 *
 *   out << .... << scope_path(sc) << ... ;
 */
struct __ScopePathManip { const NetScope*scope; };
inline __ScopePathManip scope_path(const NetScope*scope)
{ __ScopePathManip tmp; tmp.scope = scope; return tmp; }

extern std::ostream& operator << (std::ostream&o, __ScopePathManip);

struct __ObjectPathManip { const NetObj*obj; };
inline __ObjectPathManip scope_path(const NetObj*obj)
{ __ObjectPathManip tmp; tmp.obj = obj; return tmp; }

extern std::ostream& operator << (std::ostream&o, __ObjectPathManip);

/*
 * If this link has a nexus_ pointer, then it is the last Link in the
 * list. next_nlink() returns 0 for the last Link.
 */
inline Link* Link::next_nlink()
{
      if (nexus_) return 0;
      else return next_;
}

inline const Link* Link::next_nlink() const
{
      if (nexus_) return 0;
      else return next_;
}

inline NetPins*Link::get_obj()
{
      if (pin_zero_)
	    return node_;
      Link*tmp = this - pin_;
      assert(tmp->pin_zero_);
      return tmp->node_;
}

inline const NetPins*Link::get_obj() const
{
      if (pin_zero_)
	    return node_;
      const Link*tmp = this - pin_;
      assert(tmp->pin_zero_);
      return tmp->node_;
}

inline unsigned Link::get_pin() const
{
      if (pin_zero_)
	    return 0;
      else
	    return pin_;
}

#undef ENUM_UNSIGNED_INT
#endif /* IVL_netlist_H */
