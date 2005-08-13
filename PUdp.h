#ifndef __PUdp_H
#define __PUdp_H
/*
 * Copyright (c) 1998-2004 Stephen Williams (steve@picturel.com)
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
#ident "$Id: PUdp.h,v 1.12.2.1 2005/08/13 00:45:53 steve Exp $"
#endif

# include  <map>
# include  "StringHeap.h"
# include  "svector.h"
# include  "verinum.h"

class PExpr;

/*
 * This class represents a parsed UDP. This is a much simpler object
 * then a module or macromodule.
 *
 *   - all ports are scalar,
 *   - pin 0 (the first port) is always output,
 *     and the remaining pins are input.
 *
 * Thus, the ports can be represented as an ordered list of pin names.
 * If the output port is declared as a register in the Verilog source,
 * then this is a sequential UDP and the sequential flag is set to true.
 *
 * STATE TABLE
 * Each entry in the state table is given as a string with the same
 * number of characters as inputs. If the UDP is sequential, a
 * character is also included at the end of the string to represent
 * the current output.
 *
 * If the UDP is sequential, the "initial" member is taken to be the
 * initial value assigned in the source, or 'x' if none is given.
 */
class PUdp {

    public:
      explicit PUdp(perm_string n, unsigned nports);

      svector<std::string>ports;
      unsigned find_port(const char*name);

      bool sequential;

      svector<std::string>tinput;
      svector<char>  tcurrent;
      svector<char>  toutput;

      verinum::V initial;

      std::map<std::string,PExpr*> attributes;

      void dump(std::ostream&out) const;

      perm_string name_;
    private:

    private: // Not implemented
      PUdp(const PUdp&);
      PUdp& operator= (const PUdp&);
};

/*
 * $Log: PUdp.h,v $
 * Revision 1.12.2.1  2005/08/13 00:45:53  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.12  2004/03/08 00:47:44  steve
 *  primitive ports can bind bi name.
 *
 * Revision 1.11  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.10  2003/07/15 05:07:13  steve
 *  Move PUdp constructor into compiled file.
 *
 * Revision 1.9  2003/07/15 03:49:22  steve
 *  Spelling fixes.
 *
 * Revision 1.8  2003/01/30 16:23:07  steve
 *  Spelling fixes.
 *
 * Revision 1.7  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.5  2001/04/22 23:09:45  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.4  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.3  1999/06/15 03:44:53  steve
 *  Get rid of the STL vector template.
 *
 * Revision 1.2  1998/12/01 00:42:13  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.1  1998/11/25 02:35:53  steve
 *  Parse UDP primitives all the way to pform.
 *
 */
#endif
