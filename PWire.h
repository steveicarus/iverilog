#ifndef IVL_PWire_H
#define IVL_PWire_H
/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
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

# include  "netlist.h"
# include  "PNamedItem.h"
# include  <list>
# include  <map>
# include  "StringHeap.h"

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
# include  <iostream>
#endif

class PExpr;
class Design;
class netdarray_t;

/*
 * The different type of PWire::set_range() calls.
 */
enum PWSRType {SR_PORT, SR_NET, SR_BOTH};

/*
 * Wires include nets, registers and ports. A net or register becomes
 * a port by declaration, so ports are not separate. The module
 * identifies a port by keeping it in its port list.
 *
 * The hname parameter to the constructor is a hierarchical name. It
 * is the name of the wire within a module, so does not include the
 * current scope or any instances. Modules contain all the wires, so
 * from that perspective, sub-scopes within the module are a part of
 * the wire name.
 */
class PWire : public PNamedItem {

    public:
      PWire(perm_string name,
	    unsigned lexical_pos,
	    NetNet::Type t,
	    NetNet::PortType pt,
	    PWSRType rt = SR_NET);

	// Return a hierarchical name.
      perm_string basename() const;

      unsigned lexical_pos() const { return lexical_pos_; }

      NetNet::Type get_wire_type() const;
      bool set_wire_type(NetNet::Type);

      NetNet::PortType get_port_type() const;
      bool set_port_type(NetNet::PortType);

      void set_const(bool is_const) { is_const_ = is_const; };
      bool get_const() const { return is_const_; };

      void set_signed(bool flag);
      bool get_signed() const;

      void set_range(const std::list<pform_range_t>&ranges, PWSRType type);

      void set_unpacked_idx(const std::list<pform_range_t>&ranges);

      void set_data_type(data_type_t*type);

      void set_discipline(ivl_discipline_t);
      ivl_discipline_t get_discipline(void) const;

      std::map<perm_string,PExpr*> attributes;

	// Write myself to the specified stream.
      void dump(std::ostream&out, unsigned ind=4) const;

      NetNet* elaborate_sig(Design*, NetScope*scope);

      SymbolType symbol_type() const;

      bool is_net() const { return net_set_; };
      bool is_port() const { return port_set_; };
      void set_net(NetNet::Type t);
      void set_port(NetNet::PortType pt);

    private:
      perm_string name_;
      unsigned lexical_pos_;
      NetNet::Type type_;
      NetNet::PortType port_type_;
      bool signed_;

        // Whether the wire is variable declared with the const keyword.
      bool is_const_ = false;

      bool is_elaborating_ = false;

	// These members hold expressions for the bit width of the
	// wire. If they do not exist, the wire is 1 bit wide. If they
	// do exist, they represent the packed dimensions of the
	// bit. The first item in the list is the first range, and so
	// on. For example "reg [3:0][7:0] ..." will contains the
	// range_t object for [3:0] first and [7:0] last.
      std::list<pform_range_t>port_;
      bool port_set_;
      std::list<pform_range_t>net_;
      bool net_set_;
      bool is_scalar_;
      unsigned error_cnt_;

	// If this wire is actually a memory, these indices will give
	// me the size and address ranges of the memory.
      std::list<pform_range_t>unpacked_;

	// This is the complex type of the wire. the data_type_ may
	// modify how this is interpreted.
      std::unique_ptr<data_type_t> set_data_type_;

      ivl_discipline_t discipline_;

    private: // not implemented
      PWire(const PWire&);
      PWire& operator= (const PWire&);

      ivl_type_t elaborate_type(Design*des, NetScope*scope,
			        const netranges_t &packed_dimensions) const;
      ivl_type_t elaborate_darray_type(Design*des, NetScope*scope,
				       const char *darray_type,
				       const netranges_t &packed_dimensions)
				       const;
};

#endif /* IVL_PWire_H */
