/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: net_udp.cc,v 1.10 2004/10/04 01:10:54 steve Exp $"
#endif

# include  "config.h"
# include  "compiler.h"

# include  "netlist.h"

NetUDP::NetUDP(NetScope*s, perm_string n, unsigned pins, PUdp *u)
  : NetNode(s, n, pins), udp(u)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("I"), idx-1);
      }
}

bool NetUDP::first(string&inp, char&out) const
{
  table_idx = (unsigned) -1;
  return next(inp, out);
}

bool NetUDP::next(string&inp, char&out) const
{
  table_idx++;

  if (table_idx >= udp->tinput.count())
    return false;

  if (is_sequential())
    {
      inp = string("") + udp->tcurrent[table_idx] + udp->tinput[table_idx];
      assert(inp.length() == pin_count());
    }
  else
    {
      inp = udp->tinput[table_idx];
      assert(inp.length() == (pin_count()-1));
    }

  out = udp->toutput[table_idx];
  assert( (out == '0')
       || (out == '1')
       || (out == 'x')
       || (is_sequential() && (out == '-')));

  return true;
}

char NetUDP::get_initial() const
{
  assert (is_sequential());

  switch (udp->initial)
    {
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


/*
 * $Log: net_udp.cc,v $
 * Revision 1.10  2004/10/04 01:10:54  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.9  2004/02/18 17:11:56  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.8  2003/03/06 00:28:42  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.7  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.5  2001/04/24 02:23:58  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.4  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.3  2000/12/15 01:24:17  steve
 *  Accept x in outputs of primitive. (PR#84)
 *
 * Revision 1.2  2000/11/04 06:36:24  steve
 *  Apply sequential UDP rework from Stephan Boettcher  (PR#39)
 *
 * Revision 1.1  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 */

