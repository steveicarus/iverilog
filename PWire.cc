/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: PWire.cc,v 1.2 1999/09/10 05:02:09 steve Exp $"
#endif

# include  "PWire.h"
# include  <assert.h>

PWire::PWire(const string&n, NetNet::Type t, NetNet::PortType pt)
: name_(n), type_(t), port_type_(pt), lidx_(0), ridx_(0)
{
}

NetNet::Type PWire::get_wire_type() const
{
      return type_;
}

bool PWire::set_wire_type(NetNet::Type t)
{
      assert(t != NetNet::IMPLICIT);

      switch (type_) {
	  case NetNet::IMPLICIT:
	    type_ = t;
	    return true;
	  case NetNet::IMPLICIT_REG:
	    if (t == NetNet::REG) { type_ = t; return true; }
	    if (t == NetNet::INTEGER) {type_ = t; return true; }
	    return false;
	  case NetNet::REG:
	    if (t == NetNet::REG) return true;
	    if (t == NetNet::INTEGER) {type_ = t; return true; }
	    return false;
	  default:
	    if (type_ != t)
		  return false;
	    else
		  return true;
      }
}

NetNet::PortType PWire::get_port_type() const
{
      return port_type_;
}

bool PWire::set_port_type(NetNet::PortType pt)
{
      assert(pt != NetNet::NOT_A_PORT);
      assert(pt != NetNet::PIMPLICIT);

      switch (port_type_) {
	  case NetNet::PIMPLICIT:
	    port_type_ = pt;
	    return true;

	  case NetNet::NOT_A_PORT:
	    return false;

	  default:
	    if (port_type_ != pt)
		  return false;
	    else
		  return true;
      }
}

void PWire::set_range(PExpr*m, PExpr*l)
{
      msb_ = svector<PExpr*>(msb_,m);
      lsb_ = svector<PExpr*>(lsb_,l);
}

void PWire::set_memory_idx(PExpr*ldx, PExpr*rdx)
{
      assert(lidx_ == 0);
      assert(ridx_ == 0);
      assert((type_ == NetNet::REG) || (type_ == NetNet::INTEGER));
      lidx_ = ldx;
      ridx_ = rdx;
}

/*
 * $Log: PWire.cc,v $
 * Revision 1.2  1999/09/10 05:02:09  steve
 *  Handle integers at task parameters.
 *
 * Revision 1.1  1999/06/17 05:34:42  steve
 *  Clean up interface of the PWire class,
 *  Properly match wire ranges.
 *
 */

