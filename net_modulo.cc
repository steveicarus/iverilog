/*
 * Copyright (c) 2000-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: net_modulo.cc,v 1.7 2004/02/18 17:11:56 steve Exp $"
#endif

# include "config.h"

# include  <typeinfo>
# include  <iostream>
# include  <iomanip>
# include  <cassert>

# include  "netlist.h"
# include  "compiler.h"


NetModulo::NetModulo(NetScope*s, perm_string n, unsigned wr,
		     unsigned wa, unsigned wb)
: NetNode(s, n, wr+wa+wb),
  width_r_(wr), width_a_(wa), width_b_(wb)
{
      unsigned p = 0;
      for (unsigned idx = 0 ;  idx < width_r_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::OUTPUT);
	    pin(p).set_name(perm_string::literal("Result"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_a_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::INPUT);
	    pin(p).set_name(perm_string::literal("DataA"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_b_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::INPUT);
	    pin(p).set_name(perm_string::literal("DataB"), idx);
      }
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

Link& NetModulo::pin_Result(unsigned idx)
{
      assert(idx < width_r_);
      return pin(idx);
}

const Link& NetModulo::pin_Result(unsigned idx) const
{
      assert(idx < width_r_);
      return pin(idx);
}

Link& NetModulo::pin_DataA(unsigned idx)
{
      assert(idx < width_a_);
      return pin(idx+width_r_);
}

const Link& NetModulo::pin_DataA(unsigned idx) const
{
      assert(idx < width_a_);
      return pin(idx+width_r_);
}

Link& NetModulo::pin_DataB(unsigned idx)
{
      assert(idx < width_b_);
      return pin(idx+width_r_+width_a_);
}

const Link& NetModulo::pin_DataB(unsigned idx) const
{
      assert(idx < width_b_);
      return pin(idx+width_r_+width_a_);
}

/*
 * $Log: net_modulo.cc,v $
 * Revision 1.7  2004/02/18 17:11:56  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.6  2003/03/06 00:28:41  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.5  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 */
