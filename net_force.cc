/*
 * Copyright (c) 2000 Stephen Williams (steve@picturel.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: net_force.cc,v 1.1 2000/05/11 23:37:27 steve Exp $"
#endif

/*
 * This file contains implementatsion of the NetForce, NetRelease,
 * NetCAssign and NetDeassign classes. These are similar or related in
 * that they handle the procedural continuous assign and force
 * statements.
 */

# include  "netlist.h"
# include  <assert.h>

NetCAssign::NetCAssign(const string&n, NetNet*l)
: NetNode(n, l->pin_count()), lval_(l)
{
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name("I", idx);
      }
}

NetCAssign::~NetCAssign()
{
}

const Link& NetCAssign::lval_pin(unsigned idx) const
{
      assert(idx < lval_->pin_count());
      return lval_->pin(idx);
}

NetDeassign::NetDeassign(NetNet*l)
: lval_(l)
{
}

NetDeassign::~NetDeassign()
{
}

const NetNet*NetDeassign::lval() const
{
      return lval_;
}

NetForce::NetForce(const string&n, NetNet*l)
: NetNode(n, l->pin_count()), lval_(l)
{
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name("I", idx);
      }
}

NetForce::~NetForce()
{
}

const Link& NetForce::lval_pin(unsigned idx) const
{
      assert(idx < lval_->pin_count());
      return lval_->pin(idx);
}

NetRelease::NetRelease(NetNet*l)
: lval_(l)
{
}

NetRelease::~NetRelease()
{
}

const NetNet*NetRelease::lval() const
{
      return lval_;
}


/*
 * $Log: net_force.cc,v $
 * Revision 1.1  2000/05/11 23:37:27  steve
 *  Add support for procedural continuous assignment.
 *
 */

