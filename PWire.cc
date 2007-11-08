/*
 * Copyright (c) 1999-2007 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PWire.cc,v 1.14 2007/05/24 04:07:11 steve Exp $"
#endif

# include "config.h"
# include  "PWire.h"
# include  "PExpr.h"
# include  <assert.h>

PWire::PWire(const pform_name_t&n,
	     NetNet::Type t,
	     NetNet::PortType pt,
	     ivl_variable_type_t dt)
: hname_(n), type_(t), port_type_(pt), data_type_(dt),
  signed_(false), isint_(false), port_set_(false), net_set_(false),
  port_msb_(0), port_lsb_(0), net_msb_(0), net_lsb_(0), error_cnt_(0),
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

const pform_name_t& PWire::path() const
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

/*
 * Since implicitly defined list of port declarations are no longer
 * considered fully defined we no longer need this routine to force
 * them to be fully defined.
 *
void PWire::set_net_range()
{
      net_msb_ = port_msb_;
      net_lsb_ = port_lsb_;
      net_set_ = true;
}
*/

void PWire::set_range(PExpr*m, PExpr*l, PWSRType type)
{
      switch (type) {
	  case SR_PORT:
	    if (port_set_) {
		  cerr << get_line() << ": error: Port ``" << hname_
		       << "'' has already been declared a port." << endl;
		  error_cnt_ += 1;
	    } else {
		  port_msb_ = m;
		  port_lsb_ = l;
		  port_set_ = true;
	    }
	    return;

	  case SR_NET:
	    if (net_set_) {
		  cerr << get_line() << ": error: Net ``" << hname_
		       << "'' has already been declared." << endl;
		  error_cnt_ += 1;
	    } else {
		  net_msb_ = m;
		  net_lsb_ = l;
		  net_set_ = true;
	    }
	    return;

	  case SR_BOTH:
	    if (port_set_ || net_set_) {
		  if (port_set_) {
		        cerr << get_line() << ": error: Port ``" << hname_
		             << "'' has already been declared a port." << endl;
		        error_cnt_ += 1;
		  }
		  if (net_set_) {
		        cerr << get_line() << ": error: Net ``" << hname_
		             << "'' has already been declared." << endl;
		        error_cnt_ += 1;
		  }
	    } else {
		  port_msb_ = m;
		  port_lsb_ = l;
		  port_set_ = true;
		  net_msb_ = m;
		  net_lsb_ = l;
		  net_set_ = true;
	    }
	    return;
      }
}

void PWire::set_memory_idx(PExpr*ldx, PExpr*rdx)
{
      if (lidx_ != 0 || ridx_ != 0) {
	    cerr << get_line() << ": error: Array ``" << hname_
	         << "'' has already been declared." << endl;
	    error_cnt_ += 1;
      } else {
            lidx_ = ldx;
            ridx_ = rdx;
      }
}

