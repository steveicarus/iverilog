/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@picturel.com)
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

# include  "config.h"
# include  "compiler.h"

/*
 * This file contains implementation of the NetForce, NetRelease,
 * NetCAssign and NetDeassign classes. These are similar or related in
 * that they handle the procedural continuous assign and force
 * statements.
 */

# include  "netlist.h"
# include  <assert.h>

/*
 * Construct the procedural continuous assignment statement. This is a
 * bit different from a normal assignment because the the lval is only
 * intermittently connected. The deassign in particular disconnects
 * the signals when they are not being assigned anymore. Because of
 * this, there is no other reference to the lval to make it stay put
 * so we increment the eref.
 *
 * XXXX I'm not sure this is the right way. Perhaps I should create
 * output pins to connect to the netlist? But that would cause the
 * link ring to grow, and that is not quite correct either. Hmm...
 */
NetCAssign::NetCAssign(NetScope*s, perm_string n, NetNet*l)
: NetNode(s, n, l->pin_count()), lval_(l)
{
      lval_->incr_eref();
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("I"), idx);
      }
}

NetCAssign::~NetCAssign()
{
      lval_->decr_eref();
}

const NetNet* NetCAssign::lval() const
{
      return lval_;
}

const Link& NetCAssign::lval_pin(unsigned idx) const
{
      assert(idx < lval_->pin_count());
      return lval_->pin(idx);
}

NetDeassign::NetDeassign(NetNet*l)
: lval_(l)
{
      lval_->incr_eref();
}

NetDeassign::~NetDeassign()
{
      lval_->decr_eref();
}

const NetNet*NetDeassign::lval() const
{
      return lval_;
}

NetForce::NetForce(NetScope*s, perm_string n, NetNet*l)
: NetNode(s, n, l->pin_count()), lval_(l)
{
      lval_->incr_eref();

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("I"), idx);
      }
}

NetForce::~NetForce()
{
      lval_->decr_eref();
}

const Link& NetForce::lval_pin(unsigned idx) const
{
      assert(idx < lval_->pin_count());
      return lval_->pin(idx);
}

const NetNet* NetForce::lval() const
{
      return lval_;
}

NetRelease::NetRelease(NetNet*l)
: lval_(l)
{
	/* Put myself into a release list that the net is
	   keeping. This is so that the NetNet can detach itself if
	   and when it is deleted by the optimizer. */
      release_next_ = lval_->release_list_;
      lval_->release_list_ = this;
}

NetRelease::~NetRelease()
{
      assert(lval_ == 0);
}

const NetNet*NetRelease::lval() const
{
      return lval_;
}
