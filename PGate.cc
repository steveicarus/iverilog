/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PGate.cc,v 1.7 2000/02/23 02:56:53 steve Exp $"
#endif

# include  "PGate.h"
# include  "PExpr.h"
# include  "verinum.h"
# include  <assert.h>

PGate::PGate(const string&name,
	     svector<PExpr*>*pins,
	     const svector<PExpr*>*del)
: name_(name), pins_(pins)
{
      if (del) delay_.set_delays(del);
}

PGate::PGate(const string&name,
	     svector<PExpr*>*pins,
	     PExpr*del)
: name_(name), pins_(pins)
{
      if (del) delay_.set_delay(del);
}

PGate::PGate(const string&name, svector<PExpr*>*pins)
: name_(name), pins_(pins)
{
}

PGate::~PGate()
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
void PGate::eval_delays(Design*des, const string&path,
			unsigned long&rise_time,
			unsigned long&fall_time,
			unsigned long&decay_time) const
{
      delay_.eval_delays(des, path, rise_time, fall_time, decay_time);
}

PGAssign::PGAssign(svector<PExpr*>*pins)
: PGate("", pins)
{
      assert(pins->count() == 2);
}

PGAssign::PGAssign(svector<PExpr*>*pins, svector<PExpr*>*dels)
: PGate("", pins, dels)
{
      assert(pins->count() == 2);
}

PGAssign::~PGAssign()
{
}

PGBuiltin::PGBuiltin(Type t, const string&name,
		     svector<PExpr*>*pins,
		     svector<PExpr*>*del)
: PGate(name, pins, del), type_(t), msb_(0), lsb_(0)
{
}

PGBuiltin::PGBuiltin(Type t, const string&name,
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

PGModule::PGModule(const string&type, const string&name, svector<PExpr*>*pins)
: PGate(name, pins), type_(type), overrides_(0), pins_(0),
  npins_(0), parms_(0), nparms_(0), msb_(0), lsb_(0)
{
}

PGModule::PGModule(const string&type, const string&name,
		   named<PExpr*>*pins, unsigned npins)
: PGate(name, 0), type_(type), overrides_(0), pins_(pins),
  npins_(npins), parms_(0), nparms_(0), msb_(0), lsb_(0)
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

/*
 * $Log: PGate.cc,v $
 * Revision 1.7  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.6  2000/02/18 05:15:02  steve
 *  Catch module instantiation arrays.
 *
 * Revision 1.5  1999/09/14 01:50:35  steve
 *  Handle gates without delays.
 *
 * Revision 1.4  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 * Revision 1.3  1999/08/01 21:18:55  steve
 *  elaborate rise/fall/decay for continuous assign.
 *
 * Revision 1.2  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.1  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 */

