/*
 * Copyright (c) 2000-2021 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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

# include  "config.h"
# include  "compiler.h"
# include  "netlist.h"

using namespace std;

NetUDP::NetUDP(NetScope*s, perm_string n, unsigned pins, PUdp *u)
  : NetNode(s, n, pins), udp(u)
{
      pin(0).set_dir(Link::OUTPUT);
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
      }
      table_idx = udp->tinput.size() - 1;
}

bool NetUDP::first(string&inp, char&out) const
{
      table_idx = (unsigned) -1;
      return next(inp, out);
}

bool NetUDP::next(string&inp, char&out) const
{
      table_idx++;

      if (table_idx >= udp->tinput.size()) return false;

      if (is_sequential()) {
	    inp = string("") + udp->tcurrent[table_idx] +
	          udp->tinput[table_idx];
	    assert(inp.length() == pin_count());
      } else {
	    inp = udp->tinput[table_idx];
	    assert(inp.length() == (pin_count()-1));
      }

      out = udp->toutput[table_idx];
      assert((out == '0') ||
             (out == '1') ||
             (out == 'x') ||
             (is_sequential() && (out == '-')));

      return true;
}

char NetUDP::get_initial() const
{
      assert (is_sequential());

      switch (udp->initial) {
	case verinum::V0:
	    return '0';
	case verinum::V1:
	    return '1';
	case verinum::Vx:
	case verinum::Vz:
	    return 'x';
      }

      assert(0);
      return 'x';
}

unsigned NetUDP::port_count() const
{
      return udp->ports.size();
}

string NetUDP::port_name(unsigned idx) const
{
      assert(idx < udp->ports.size());
      return udp->ports[idx];
}
