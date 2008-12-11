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

      svector<string>ports;
      unsigned find_port(const char*name);

      bool sequential;

      svector<string>tinput;
      svector<char>  tcurrent;
      svector<char>  toutput;

      verinum::V initial;

      map<string,PExpr*> attributes;

      void dump(ostream&out) const;

      perm_string name_;
    private:

    private: // Not implemented
      PUdp(const PUdp&);
      PUdp& operator= (const PUdp&);
};

#endif
