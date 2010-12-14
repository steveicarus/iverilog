#ifndef __PWire_H
#define __PWire_H
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

# include  "netlist.h"
# include  "LineInfo.h"
# include  <map>
# include  "svector.h"

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif

class PExpr;
class Design;

/*
 * Wires include nets, registers and ports. A net or register becomes
 * a port by declaration, so ports are not separate. The module
 * identifies a port by keeping it in its port list.
 *
 * The hname parameter to the constructor is a hierarchical name. It
 * is an array of strings starting with the root, running towards
 * the base name, and terminated by a null pointer. The environment
 * allocates the memory for me.
 */
class PWire : public LineInfo {

    public:
      PWire(const hname_t&hname, NetNet::Type t, NetNet::PortType pt);
      PWire(char*name, NetNet::Type t, NetNet::PortType pt);

	// Return a hierarchical name.
      const hname_t&path() const;

      NetNet::Type get_wire_type() const;
      bool set_wire_type(NetNet::Type);

      NetNet::PortType get_port_type() const;
      bool set_port_type(NetNet::PortType);

      void set_signed(bool flag);
      bool get_signed() const;
      bool get_isint() const;

      void set_range(PExpr*msb, PExpr*lsb);

      void set_memory_idx(PExpr*ldx, PExpr*rdx);

      map<perm_string,PExpr*> attributes;

	// Write myself to the specified stream.
      void dump(ostream&out) const;

      void elaborate_sig(Design*, NetScope*scope) const;

    private:
      hname_t hname_;
      NetNet::Type type_;
      NetNet::PortType port_type_;
      bool signed_;
      bool isint_;		// original type of integer

	// These members hold expressions for the bit width of the
	// wire. If they do not exist, the wire is 1 bit wide.
      svector<PExpr*>msb_;
      svector<PExpr*>lsb_;

	// If this wire is actually a memory, these indices will give
	// me the size and address range of the memory.
      PExpr*lidx_;
      PExpr*ridx_;

    private: // not implemented
      PWire(const PWire&);
      PWire& operator= (const PWire&);
};

#endif
