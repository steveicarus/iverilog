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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <typeinfo>
# include  <cstdlib>
# include  <climits>
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "ivl_assert.h"

NetContribution::NetContribution(NetEAccess*l, NetExpr*r)
: lval_(l), rval_(r)
{
}

NetContribution::~NetContribution()
{
}

const NetEAccess* NetContribution::lval() const
{
      return lval_;
}

const NetExpr* NetContribution::rval() const
{
      return rval_;
}
