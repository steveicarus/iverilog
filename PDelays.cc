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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: PDelays.cc,v 1.2 2000/02/23 02:56:53 steve Exp $"
#endif

# include  "PDelays.h"
# include  "PExpr.h"
# include  "verinum.h"

PDelays::PDelays()
{
      for (unsigned idx = 0 ;  idx < 3 ;  idx += 1)
	    delay_[idx] = 0;
}

PDelays::~PDelays()
{
      for (unsigned idx = 0 ;  idx < 3 ;  idx += 1)
	    delete delay_[idx];
}

void PDelays::set_delay(PExpr*del)
{
      assert(del);
      assert(delay_[0] == 0);
      delay_[0] = del;
}


void PDelays::set_delays(const svector<PExpr*>*del)
{
      assert(del);
      assert(del->count() <= 3);
      for (unsigned idx = 0 ;  idx < del->count() ;  idx += 1)
	    delay_[idx] = (*del)[idx];
}

void PDelays::eval_delays(Design*des, const string&path,
			unsigned long&rise_time,
			unsigned long&fall_time,
			unsigned long&decay_time) const
{
      verinum*dv;

      if (delay_[0]) {
	    dv = delay_[0]->eval_const(des, path);
	    assert(dv);
	    rise_time = dv->as_ulong();
	    delete dv;

	    if (delay_[1]) {
		  dv = delay_[1]->eval_const(des, path);
		  assert(dv);
		  fall_time = dv->as_ulong();
		  delete dv;

		  if (delay_[2]) {
			dv = delay_[2]->eval_const(des, path);
			assert(dv);
			decay_time = dv->as_ulong();
			delete dv;
		  } else {
			if (rise_time < fall_time)
			      decay_time = rise_time;
			else
			      decay_time = fall_time;
		  }
	    } else {
		  assert(delay_[2] == 0);
		  fall_time = rise_time;
		  decay_time = rise_time;
	    }
      } else {
	    rise_time = 0;
	    fall_time = 0;
	    decay_time = 0;
      }
}

/*
 * $Log: PDelays.cc,v $
 * Revision 1.2  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 */

