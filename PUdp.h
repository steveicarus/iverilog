#ifndef IVL_PUdp_H
#define IVL_PUdp_H
/*
 * Copyright (c) 1998-2021 Stephen Williams (steve@picturel.com)
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

# include  <map>
# include  <vector>
# include  "LineInfo.h"
# include  "StringHeap.h"
# include  "verinum.h"

class PExpr;

/*
 * This class represents a parsed UDP. This is a much simpler object
 * than a module or macromodule.
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
class PUdp : public LineInfo {

    public:
      explicit PUdp(perm_string n, unsigned nports);

      std::vector<std::string> ports;
      unsigned find_port(const char*name);

      bool sequential;

      std::vector<std::string> tinput;
      std::vector<char> tcurrent;
      std::vector<char> toutput;

      verinum::V initial;

      std::map<std::string,PExpr*> attributes;

      void dump(std::ostream&out) const;

      perm_string name_;
    private:

    private: // Not implemented
      PUdp(const PUdp&);
      PUdp& operator= (const PUdp&);
};

#endif /* IVL_PUdp_H */
