#ifndef __PWire_H
#define __PWire_H
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: PWire.h,v 1.17 2004/02/20 18:53:33 steve Exp $"
#endif

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

/*
 * $Log: PWire.h,v $
 * Revision 1.17  2004/02/20 18:53:33  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.16  2003/01/30 16:23:07  steve
 *  Spelling fixes.
 *
 * Revision 1.15  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.14  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.13  2002/06/21 04:59:35  steve
 *  Carry integerness throughout the compilation.
 *
 * Revision 1.12  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.11  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.10  2001/01/16 02:44:18  steve
 *  Use the iosfwd header if available.
 *
 * Revision 1.9  2000/12/11 00:31:43  steve
 *  Add support for signed reg variables,
 *  simulate in t-vvm signed comparisons.
 *
 * Revision 1.8  2000/05/02 16:27:38  steve
 *  Move signal elaboration to a seperate pass.
 *
 * Revision 1.7  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.6  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 * Revision 1.5  1999/06/17 05:34:42  steve
 *  Clean up interface of the PWire class,
 *  Properly match wire ranges.
 *
 * Revision 1.4  1999/06/02 15:38:46  steve
 *  Line information with nets.
 *
 * Revision 1.3  1999/04/19 01:59:36  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.2  1998/11/23 00:20:22  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.1  1998/11/03 23:28:55  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
