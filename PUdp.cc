/*
 * Copyright (c) 2003-2004 Stephen Williams (steve@icarus.com)
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

# include  "PUdp.h"

PUdp::PUdp(perm_string n, unsigned nports)
: ports(nports), sequential(false), initial(verinum::Vx), name_(n)
{
}

unsigned PUdp::find_port(const char*name)
{
      for (unsigned idx = 0 ;  idx < ports.size() ;  idx += 1) {

	    if (ports[idx] == name)
		  return idx;
      }

      return ports.size();
}

