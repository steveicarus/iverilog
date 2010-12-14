/*
 * Copyright (c) 1998-2010 Stephen Williams <steve@icarus.com>
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

# include "config.h"

# include  <iostream>

# include  "PExpr.h"
# include  "Module.h"
# include  <typeinfo>

PExpr::PExpr()
{
}

PExpr::~PExpr()
{
}

bool PExpr::is_the_same(const PExpr*that) const
{
      return typeid(this) == typeid(that);
}

bool PExpr::is_constant(Module*) const
{
      return false;
}

NetNet* PExpr::elaborate_lnet(Design*des, NetScope*, bool) const
{
      cerr << get_line() << ": error: expression not valid in assign l-value: "
	   << *this << endl;
      return 0;
}

PEBinary::PEBinary(char op, PExpr*l, PExpr*r)
: op_(op), left_(l), right_(r)
{
}

PEBinary::~PEBinary()
{
}

bool PEBinary::is_constant(Module*mod) const
{
      return left_->is_constant(mod) && right_->is_constant(mod);
}

PECallFunction::PECallFunction(const hname_t&n, const svector<PExpr *> &parms)
: path_(n), parms_(parms)
{
}

PECallFunction::PECallFunction(const hname_t&n)
: path_(n)
{
}

PECallFunction::~PECallFunction()
{
}

PEConcat::PEConcat(const svector<PExpr*>&p, PExpr*r)
: parms_(p), repeat_(r)
{
}

bool PEConcat::is_constant(Module *mod) const
{
      bool constant = repeat_? repeat_->is_constant(mod) : true;
      for (unsigned i = 0; constant && i < parms_.count(); ++i) {
	    if (parms_[i] == 0)
		  continue;
	    constant = constant && parms_[i]->is_constant(mod);
      }
      return constant;
}

PEConcat::~PEConcat()
{
      delete repeat_;
}

PEEvent::PEEvent(PEEvent::edge_t t, PExpr*e)
: type_(t), expr_(e)
{
}

PEEvent::~PEEvent()
{
}

PEEvent::edge_t PEEvent::type() const
{
      return type_;
}

PExpr* PEEvent::expr() const
{
      return expr_;
}

PEFNumber::PEFNumber(verireal*v)
: value_(v)
{
}

PEFNumber::~PEFNumber()
{
      delete value_;
}

const verireal& PEFNumber::value() const
{
      return *value_;
}

bool PEFNumber::is_constant(Module*) const
{
      return true;
}

PEIdent::PEIdent(const hname_t&s)
: path_(s), msb_(0), lsb_(0), idx_(0)
{
}

PEIdent::~PEIdent()
{
}

const hname_t& PEIdent::path() const
{
      return path_;
}

/*
 * An identifier can be in a constant expression if (and only if) it is
 * a parameter.
 */
bool PEIdent::is_constant(Module*mod) const
{
      if (mod == 0) return false;

	/* This is a work-around for map not matching < even when
	   there is a perm_string operator that can do the comprare.

	   The real fix is to make the path_ carry perm_strings. */
      perm_string tmp = perm_string::literal(path_.peek_name(0));

      { map<perm_string,Module::param_expr_t>::const_iterator cur;
        cur = mod->parameters.find(tmp);
	if (cur != mod->parameters.end()) return true;
      }

      { map<perm_string,Module::param_expr_t>::const_iterator cur;
        cur = mod->localparams.find(tmp);
	if (cur != mod->localparams.end()) return true;
      }

      return false;
}

PENumber::PENumber(verinum*vp)
: value_(vp)
{
      assert(vp);
}

PENumber::~PENumber()
{
      delete value_;
}

const verinum& PENumber::value() const
{
      return *value_;
}

bool PENumber::is_the_same(const PExpr*that) const
{
      const PENumber*obj = dynamic_cast<const PENumber*>(that);
      if (obj == 0)
	    return false;

      return *value_ == *obj->value_;
}

bool PENumber::is_constant(Module*) const
{
      return true;
}

PEString::PEString(char*s)
: text_(s)
{
}

PEString::~PEString()
{
      delete[]text_;
}

string PEString::value() const
{
      return text_;
}

bool PEString::is_constant(Module*) const
{
      return true;
}

PETernary::PETernary(PExpr*e, PExpr*t, PExpr*f)
: expr_(e), tru_(t), fal_(f)
{
}

PETernary::~PETernary()
{
}

bool PETernary::is_constant(Module*m) const
{
      return expr_->is_constant(m)
	    && tru_->is_constant(m)
	    && fal_->is_constant(m);
}

PEUnary::PEUnary(char op, PExpr*ex)
: op_(op), expr_(ex)
{
}

PEUnary::~PEUnary()
{
}

bool PEUnary::is_constant(Module*m) const
{
      return expr_->is_constant(m);
}
