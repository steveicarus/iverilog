/*
 * Copyright (c) 1999 Stephen Williams (steve@picturel.com)
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
#ident "$Id: PGate.cc,v 1.2 1999/08/01 16:34:50 steve Exp $"
#endif

# include  "PGate.h"
# include  "PExpr.h"
# include  <assert.h>

PGate::PGate(const string&name,
	     svector<PExpr*>*pins,
	     const svector<PExpr*>*del)
: name_(name), pins_(pins)
{
      for (unsigned idx = 0 ;  idx < 3 ;  idx += 1)
	    delay_[idx] = 0;

      if (del) {
	    assert(del->count() <= 3);
	    for (unsigned idx = 0 ;  idx < del->count() ;  idx += 1)
		  delay_[idx] = (*del)[idx];
      }
}

PGate::PGate(const string&name,
	     svector<PExpr*>*pins,
	     PExpr*del)
: name_(name), pins_(pins)
{
      delay_[0] = del;
      delay_[1] = 0;
      delay_[2] = 0;
}

PGate::PGate(const string&name, svector<PExpr*>*pins)
: name_(name), pins_(pins)
{
      delay_[0] = 0;
      delay_[1] = 0;
      delay_[2] = 0;
}

PGate::~PGate()
{
      for (unsigned idx = 0 ;  idx < 3 ;  idx += 1)
	    delete delay_[idx];
}

const PExpr* PGate::get_delay(unsigned idx) const
{
      assert(idx < 3);
      return delay_[idx];
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

/*
 * $Log: PGate.cc,v $
 * Revision 1.2  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.1  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 */

