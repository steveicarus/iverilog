/*
 * Copyright (c) 1999-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PWire.cc,v 1.13 2007/04/26 03:06:21 steve Exp $"
#endif

# include "config.h"
# include  "PWire.h"
# include  <assert.h>

PWire::PWire(const hname_t&n,
	     NetNet::Type t,
	     NetNet::PortType pt,
	     ivl_variable_type_t dt)
: hname_(n), type_(t), port_type_(pt), data_type_(dt),
  signed_(false), isint_(false),
lidx_(0), ridx_(0)
{
      if (t == NetNet::INTEGER) {
	    type_ = NetNet::REG;
	    signed_ = true;
	    isint_ = true;
      }
}

PWire::PWire(perm_string n,
	     NetNet::Type t,
	     NetNet::PortType pt,
	     ivl_variable_type_t dt)
: hname_(n), type_(t), port_type_(pt), data_type_(dt),
  signed_(false), isint_(false),
lidx_(0), ridx_(0)
{
      if (t == NetNet::INTEGER) {
	    type_ = NetNet::REG;
	    signed_ = true;
	    isint_ = true;
      }
}

NetNet::Type PWire::get_wire_type() const
{
      return type_;
}

const hname_t& PWire::path() const
{
      return hname_;
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
	    return false;
	  case NetNet::REG:
	    if (t == NetNet::INTEGER) {
		  isint_ = true;
		  return true;
	    }
	    if (t == NetNet::REG) return true;
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

bool PWire::set_data_type(ivl_variable_type_t dt)
{
      if (data_type_ != IVL_VT_NO_TYPE)
	    if (data_type_ != dt)
		  return false;
	    else
		  return true;

      assert(data_type_ == IVL_VT_NO_TYPE);
      data_type_ = dt;
      return true;
}

void PWire::set_signed(bool flag)
{
      signed_ = flag;
}

bool PWire::get_signed() const
{
      return signed_;
}

bool PWire::get_isint() const
{
      return isint_;
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
      lidx_ = ldx;
      ridx_ = rdx;
}

/*
 * $Log: PWire.cc,v $
 * Revision 1.13  2007/04/26 03:06:21  steve
 *  Rework hname_t to use perm_strings.
 *
 * Revision 1.12  2007/01/16 05:44:14  steve
 *  Major rework of array handling. Memories are replaced with the
 *  more general concept of arrays. The NetMemory and NetEMemory
 *  classes are removed from the ivl core program, and the IVL_LPM_RAM
 *  lpm type is removed from the ivl_target API.
 *
 * Revision 1.11  2005/07/07 16:22:49  steve
 *  Generalize signals to carry types.
 *
 * Revision 1.10  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.9  2002/06/21 04:59:35  steve
 *  Carry integerness throughout the compilation.
 *
 * Revision 1.8  2002/01/26 05:28:28  steve
 *  Detect scalar/vector declarion mismatch.
 *
 * Revision 1.7  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.6  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.5  2001/01/06 02:29:35  steve
 *  Support arrays of integers.
 *
 * Revision 1.4  2000/12/11 00:31:43  steve
 *  Add support for signed reg variables,
 *  simulate in t-vvm signed comparisons.
 *
 * Revision 1.3  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.2  1999/09/10 05:02:09  steve
 *  Handle integers at task parameters.
 *
 * Revision 1.1  1999/06/17 05:34:42  steve
 *  Clean up interface of the PWire class,
 *  Properly match wire ranges.
 *
 */

