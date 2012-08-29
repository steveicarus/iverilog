/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  <typeinfo>
# include  <iostream>
# include  <iomanip>
# include  <cassert>

# include  "netlist.h"
# include  "compiler.h"

/*
 *  0 -- Result
 *  1 -- DataA
 *  2 -- DataB
 */
NetModulo::NetModulo(NetScope*s, perm_string n, unsigned wr,
		     unsigned wa, unsigned wb)
: NetNode(s, n, 3),
  width_r_(wr), width_a_(wa), width_b_(wb)
{
      pin(0).set_dir(Link::OUTPUT); // Result
      pin(1).set_dir(Link::INPUT);  // DataA
      pin(2).set_dir(Link::INPUT);  // DataB
      signed_flag_ = false;
}

NetModulo::~NetModulo()
{
}

unsigned NetModulo::width_r() const
{
      return width_r_;
}

unsigned NetModulo::width_a() const
{
      return width_a_;
}

unsigned NetModulo::width_b() const
{
      return width_b_;
}

Link& NetModulo::pin_Result()
{
      return pin(0);
}

void NetModulo::set_signed(bool flag)
{
      signed_flag_ = flag;
}

bool NetModulo::get_signed() const
{
      return signed_flag_;
}

const Link& NetModulo::pin_Result() const
{
      return pin(0);
}

Link& NetModulo::pin_DataA()
{
      return pin(1);
}

const Link& NetModulo::pin_DataA() const
{
      return pin(1);
}

Link& NetModulo::pin_DataB()
{
      return pin(2);
}

const Link& NetModulo::pin_DataB() const
{
      return pin(2);
}
