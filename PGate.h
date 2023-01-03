#ifndef IVL_PGate_H
#define IVL_PGate_H
/*
 * Copyright (c) 1998-2021 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
# include  "named.h"
# include  "PNamedItem.h"
# include  "PDelays.h"
# include  "netlist.h"
# include  <map>
# include  <list>
# include  <vector>
# include  <string>
class PExpr;
class PUdp;
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
class PGate : public PNamedItem {

    public:
      explicit PGate(perm_string name, std::list<PExpr*>*pins,
		     const std::list<PExpr*>*del);

      explicit PGate(perm_string name, std::list<PExpr*>*pins,
		     PExpr*del);

      explicit PGate(perm_string name, std::list<PExpr*>*pins);

      virtual ~PGate();

      void set_ranges(std::list<pform_range_t>*ranges);
      bool is_array() const { return ranges_ != 0; }

      perm_string get_name() const { return name_; }

	// This evaluates the delays as far as possible, but returns
	// an expression, and do not signal errors.
      void eval_delays(Design*des, NetScope*scope,
		       NetExpr*&rise_time,
		       NetExpr*&fall_time,
		       NetExpr*&decay_time,
		       bool as_net_flag =false) const;

      unsigned delay_count() const;

      unsigned pin_count() const { return pins_.size(); }
      PExpr*pin(unsigned idx) const { return pins_[idx]; }

      ivl_drive_t strength0() const;
      ivl_drive_t strength1() const;

      void strength0(ivl_drive_t);
      void strength1(ivl_drive_t);

      std::map<perm_string,PExpr*> attributes;

      virtual void dump(std::ostream&out, unsigned ind =4) const;
      virtual void elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*sc) const;
      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

      SymbolType symbol_type() const;

    protected:
      const std::vector<PExpr*>& get_pins() const { return pins_; }

      unsigned calculate_array_size_(Design*, NetScope*,
				     long&high, long&low) const;

      void dump_pins(std::ostream&out) const;
      void dump_delays(std::ostream&out) const;
      void dump_ranges(std::ostream&out) const;

    private:
      perm_string name_;
      PDelays delay_;
      std::vector<PExpr*>pins_;

      std::list<pform_range_t>*ranges_;

      ivl_drive_t str0_, str1_;

      void set_pins_(std::list<PExpr*>*pins);

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
      explicit PGAssign(std::list<PExpr*>*pins);
      explicit PGAssign(std::list<PExpr*>*pins, std::list<PExpr*>*dels);
      ~PGAssign();

      void dump(std::ostream&out, unsigned ind =4) const;
      virtual void elaborate(Design*des, NetScope*scope) const;
      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

    private:
      void elaborate_unpacked_array_(Design*des, NetScope*scope, NetNet*lval) const;
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
			 std::list<PExpr*>*pins,
			 std::list<PExpr*>*del);
      explicit PGBuiltin(Type t, perm_string name,
			 std::list<PExpr*>*pins,
			 PExpr*del);
      ~PGBuiltin();

      Type type() const { return type_; }
      const char * gate_name() const;

      virtual void dump(std::ostream&out, unsigned ind =4) const;
      virtual void elaborate(Design*, NetScope*scope) const;
      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

    private:
      void calculate_gate_and_lval_count_(unsigned&gate_count,
                                          unsigned&lval_count) const;

      NetNode* create_gate_for_output_(Design*, NetScope*,
				       perm_string gate_name,
				       unsigned instance_width) const;

      bool check_delay_count(Design*des) const;

      Type type_;
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
			std::list<PExpr*>*pins);

	// If the binding of ports is by name, this constructor takes
	// the bindings and stores them for later elaboration.
      explicit PGModule(perm_string type, perm_string name,
			named_pexpr_t *pins, unsigned npins);

	// If the module type is known by design, then use this
	// constructor.
      explicit PGModule(Module*type, perm_string name);

      ~PGModule();

	// Parameter overrides can come as an ordered list, or a set
	// of named expressions.
      void set_parameters(std::list<PExpr*>*o);
      void set_parameters(named_pexpr_t *pa, unsigned npa);

      std::map<perm_string,PExpr*> attributes;

      virtual void dump(std::ostream&out, unsigned ind =4) const;
      virtual void elaborate(Design*, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*sc) const;
      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

	// This returns the module name of this module. It is a
	// permallocated string.
      perm_string get_type() const;

    private:
      Module*bound_type_;
      perm_string type_;
      std::list<PExpr*>*overrides_;
      named_pexpr_t *pins_;
      unsigned npins_;

	// These members support parameter override by name
      named_pexpr_t *parms_;
      unsigned nparms_;

      friend class delayed_elaborate_scope_mod_instances;
      void elaborate_mod_(Design*, Module*mod, NetScope*scope) const;
      void elaborate_udp_(Design*, PUdp  *udp, NetScope*scope) const;
      void elaborate_scope_mod_(Design*des, Module*mod, NetScope*sc) const;
      void elaborate_scope_mod_instances_(Design*des, Module*mod, NetScope*sc) const;
      bool elaborate_sig_mod_(Design*des, NetScope*scope, Module*mod) const;
	// Not currently used.
#if 0
      bool elaborate_sig_udp_(Design*des, NetScope*scope, PUdp*udp) const;
#endif

      NetNet*resize_net_to_port_(Design*des, NetScope*scope,
				 NetNet*sig, unsigned port_wid,
				 NetNet::PortType dir, bool as_signed) const;
};

#endif /* IVL_PGate_H */
