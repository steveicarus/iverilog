/*
 * Copyright (c) 1999-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PGate.cc,v 1.16 2004/02/18 17:11:54 steve Exp $"
#endif

# include "config.h"

# include  "PGate.h"
# include  "PExpr.h"
# include  "verinum.h"
# include  <assert.h>

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
			unsigned long&rise_time,
			unsigned long&fall_time,
			unsigned long&decay_time) const
{
      delay_.eval_delays(des, scope, rise_time, fall_time, decay_time);
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

/*
 * $Log: PGate.cc,v $
 * Revision 1.16  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.15  2003/03/06 04:37:12  steve
 *  lex_strings.add module names earlier.
 *
 * Revision 1.14  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.13  2001/11/22 06:20:59  steve
 *  Use NetScope instead of string for scope path.
 *
 * Revision 1.12  2001/10/21 00:42:47  steve
 *  Module types in pform are char* instead of string.
 *
 * Revision 1.11  2001/10/19 01:55:32  steve
 *  Method to get the type_ member
 *
 * Revision 1.10  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.9  2000/05/06 15:41:56  steve
 *  Carry assignment strength to pform.
 *
 * Revision 1.8  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
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

