#ifndef IVL_PModport_H
#define IVL_PModport_H
/*
 * Copyright (c) 2015-2021 Stephen Williams (steve@icarus.com)
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

# include  "PNamedItem.h"
# include  "PScope.h"
# include  "StringHeap.h"
# include  "netlist.h"
# include  <vector>

/*
 * The PModport class represents a parsed SystemVerilog modport list.
 */
class PModport : public PNamedItem {

    public:
	// The name is a perm-allocated string. It is the simple name
	// of the modport, without any scope.
      explicit PModport(perm_string name);
      ~PModport();

      perm_string name() const { return name_; }

      typedef std::pair <NetNet::PortType,PExpr*> simple_port_t;
      std::map<perm_string,simple_port_t> simple_ports;

      SymbolType symbol_type() const;

    private:
      perm_string name_;

    private: // not implemented
      PModport(const PModport&);
      PModport& operator= (const PModport&);
};

#endif /* IVL_PModport_H */
