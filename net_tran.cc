/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include <iostream>

# include  <typeinfo>
# include  <cstdlib>
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "ivl_assert.h"

NetTran::NetTran(NetScope*scope, perm_string n, bool resistive, int enable)
: NetNode(scope, n, enable? 3 : 2)
{
      pin(0).set_dir(Link::PASSIVE); pin(0).set_name(perm_string::literal("A"), 0);
      pin(1).set_dir(Link::PASSIVE); pin(1).set_name(perm_string::literal("B"), 0);
      if (enable) {
	    pin(2).set_dir(Link::INPUT);
	    pin(2).set_name(perm_string::literal("E"), 0);
      }
}

NetTran::~NetTran()
{
}
