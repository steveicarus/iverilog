/*
 * Copyright (c) 1999-2024 Stephen Williams (steve@icarus.com)
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
# include "ivl_assert.h"
# include  "PWire.h"
# include  "PExpr.h"

using namespace std;

PWire::PWire(perm_string n,
	     unsigned lp,
	     NetNet::Type t,
	     NetNet::PortType pt,
	     PWSRType rt)
: name_(n), lexical_pos_(lp), type_(t), port_type_(pt), signed_(false),
  port_set_(false), net_set_(false), is_scalar_(false),
  error_cnt_(0), discipline_(0)
{
      switch (rt) {
	  case SR_PORT:
	    port_set_ = true;
	    break;
	  case SR_NET:
	    net_set_ = true;
	    break;
	  case SR_BOTH:
	    port_set_ = true;
	    net_set_ = true;
	    break;
      }
}

NetNet::Type PWire::get_wire_type() const
{
      return type_;
}

perm_string PWire::basename() const
{
      return name_;
}

bool PWire::set_wire_type(NetNet::Type t)
{
      ivl_assert(*this, t != NetNet::IMPLICIT);

      switch (type_) {
	  case NetNet::IMPLICIT:
	    type_ = t;
	    return true;
	  case NetNet::IMPLICIT_REG:
	    if (t == NetNet::REG) {
		  type_ = t;
		  return true;
	    }
	    if (t == NetNet::IMPLICIT_REG) return true;
	    return false;
	  case NetNet::REG:
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
      ivl_assert(*this, pt != NetNet::NOT_A_PORT);
      ivl_assert(*this, pt != NetNet::PIMPLICIT);

      switch (port_type_) {
	  case NetNet::PIMPLICIT:
	  case NetNet::NOT_A_PORT:
	    port_type_ = pt;
	    return true;

	  default:
	    if (port_type_ != pt)
		  return false;
	    else
		  return true;
      }
}

void PWire::set_signed(bool flag)
{
	// For a non-ANSI style port declaration where the data type is
	// specified in a corresponding variable declaration, the signed
	// attribute may be attached to either the port declaration or to
	// the variable declaration (IEEE 1364-2005 section 12.3.3). The
	// signal is signed if either the port or the variable is signed.
	// Handle that here.
      signed_ = signed_ || flag;
}

bool PWire::get_signed() const
{
      return signed_;
}

void PWire::set_port(NetNet::PortType pt)
{
      ivl_assert(*this, !port_set_);
      port_set_ = true;

      bool rc = set_port_type(pt);
      ivl_assert(*this, rc);
}

void PWire::set_net(NetNet::Type t)
{
      ivl_assert(*this, !net_set_);
      net_set_ = true;

      if (t != NetNet::IMPLICIT) {
	    bool rc = set_wire_type(t);
	    ivl_assert(*this, rc);
      }
}

void PWire::set_range(const list<pform_range_t>&rlist, PWSRType type)
{
      switch (type) {
	  case SR_PORT:
	    if (!port_.empty())
		  return;
	    port_ = rlist;
	    break;
	  case SR_NET:
	    if (!net_.empty())
		  return;
	    net_ = rlist;
	    break;
	  case SR_BOTH:
	    if (!port_.empty() || !net_.empty())
		  return;
	    port_ = rlist;
	    net_ = rlist;
	    break;
      }
}

void PWire::set_unpacked_idx(const list<pform_range_t>&ranges)
{
      if (! unpacked_.empty()) {
	    cerr << get_fileline() << ": error: Array ``" << name_
	         << "'' has already been declared." << endl;
	    error_cnt_ += 1;
      } else {
	    unpacked_ = ranges;
      }
}

void PWire::set_data_type(data_type_t*type)
{
      if (set_data_type_.get() == type)
	    return;

      ivl_assert(*this, !set_data_type_.get());
      set_data_type_.reset(type);
}

void PWire::set_discipline(ivl_discipline_t d)
{
      ivl_assert(*this, discipline_ == 0);
      discipline_ = d;
}

ivl_discipline_t PWire::get_discipline(void) const
{
      return discipline_;
}

PNamedItem::SymbolType PWire::symbol_type() const
{
      switch (type_) {
          case NetNet::IMPLICIT_REG:
          case NetNet::REG:
            return VAR;
          default:
            return NET;
    }
}
