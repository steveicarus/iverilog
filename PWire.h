#ifndef __PWire_H
#define __PWire_H
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
#ident "$Id: PWire.h,v 1.4 1999/06/02 15:38:46 steve Exp $"
#endif

# include  "netlist.h"
# include  "LineInfo.h"
# include <map>
class ostream;
class PExpr;
class Design;

/*
 * Wires include nets, registers and ports. A net or register becomes
 * a port by declaration, so ports are not seperate. The module
 * identifies a port by keeping it in its port list.
 */
class PWire : public LineInfo {

    public:
      PWire(const string&n, NetNet::Type t =NetNet::IMPLICIT)
      : name(n), type(t), port_type(NetNet::NOT_A_PORT), msb(0),
	lsb(0), lidx(0), ridx(0)
      { }

      string name;
      NetNet::Type type;
      NetNet::PortType port_type;

      PExpr*msb;
      PExpr*lsb;

	// If this wire is actually a memory, these indices will give
	// me the size and address range of the memory.
      PExpr*lidx;
      PExpr*ridx;

      map<string,string> attributes;

	// Write myself to the specified stream.
      void dump(ostream&out) const;

      void elaborate(Design*, const string&path) const;

    private: // not implemented
      PWire(const PWire&);
      PWire& operator= (const PWire&);
};

/*
 * $Log: PWire.h,v $
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
