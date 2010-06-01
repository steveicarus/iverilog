/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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

# include  "PGate.h"
# include  "PExpr.h"
# include  "verinum.h"
# include  <cassert>

PGate::PGate(perm_string name,
	     svector<PExpr*>*pins,
	     const svector<PExpr*>*del)
: name_(name), pins_(pins)
{
      if (del) delay_.set_delays(del);
      str0_ = STRONG;
      str1_ = STRONG;
}

PGate::PGate(perm_string name,
	     svector<PExpr*>*pins,
	     PExpr*del)
: name_(name), pins_(pins)
{
      if (del) delay_.set_delay(del);
      str0_ = STRONG;
      str1_ = STRONG;
}

PGate::PGate(perm_string name, svector<PExpr*>*pins)
: name_(name), pins_(pins)
{
      str0_ = STRONG;
      str1_ = STRONG;
}

PGate::~PGate()
{
}

PGate::strength_t PGate::strength0() const
{
      return str0_;
}

void PGate::strength0(PGate::strength_t s)
{
      str0_ = s;
}

PGate::strength_t PGate::strength1() const
{
      return str1_;
}

void PGate::strength1(PGate::strength_t s)
{
      str1_ = s;
}

void PGate::elaborate_scope(Design*, NetScope*) const
{
}

/*
 * This method is used during elaboration to calculate the
 * rise/fall/decay times for the gate. These values were set in pform
 * by the constructor, so here I evaluate the expression in the given
 * design context and save the calculated delays into the output
 * parameters. This method understands how to handle the different
 * numbers of expressions.
 */

void PGate::eval_delays(Design*des, NetScope*scope,
			NetExpr*&rise_expr,
			NetExpr*&fall_expr,
			NetExpr*&decay_expr,
			bool as_net_flag) const
{
      delay_.eval_delays(des, scope,
			 rise_expr, fall_expr, decay_expr,
			 as_net_flag);
}

PGAssign::PGAssign(svector<PExpr*>*pins)
: PGate(perm_string(), pins)
{
      assert(pins->count() == 2);
}

PGAssign::PGAssign(svector<PExpr*>*pins, svector<PExpr*>*dels)
: PGate(perm_string(), pins, dels)
{
      assert(pins->count() == 2);
}

PGAssign::~PGAssign()
{
}

PGBuiltin::PGBuiltin(Type t, perm_string name,
		     svector<PExpr*>*pins,
		     svector<PExpr*>*del)
: PGate(name, pins, del), type_(t), msb_(0), lsb_(0)
{
}

PGBuiltin::PGBuiltin(Type t, perm_string name,
		     svector<PExpr*>*pins,
		     PExpr*del)
: PGate(name, pins, del), type_(t), msb_(0), lsb_(0)
{
}


PGBuiltin::~PGBuiltin()
{
}

void PGBuiltin::set_range(PExpr*msb, PExpr*lsb)
{
      assert(msb_ == 0);
      assert(lsb_ == 0);

      msb_ = msb;
      lsb_ = lsb;
}

PGModule::PGModule(perm_string type, perm_string name, svector<PExpr*>*pins)
: PGate(name, pins), overrides_(0), pins_(0),
  npins_(0), parms_(0), nparms_(0), msb_(0), lsb_(0)
{
      type_ = type;
}

PGModule::PGModule(perm_string type, perm_string name,
		   named<PExpr*>*pins, unsigned npins)
: PGate(name, 0), overrides_(0), pins_(pins),
  npins_(npins), parms_(0), nparms_(0), msb_(0), lsb_(0)
{
      type_ = type;
}

PGModule::~PGModule()
{
}

void PGModule::set_parameters(svector<PExpr*>*o)
{
      assert(overrides_ == 0);
      overrides_ = o;
}

void PGModule::set_parameters(named<PExpr*>*pa, unsigned npa)
{
      assert(parms_ == 0);
      assert(overrides_ == 0);
      parms_ = pa;
      nparms_ = npa;
}

void PGModule::set_range(PExpr*msb, PExpr*lsb)
{
      assert(msb_ == 0);
      assert(lsb_ == 0);

      msb_ = msb;
      lsb_ = lsb;
}

perm_string PGModule::get_type()
{
      return type_;
}
