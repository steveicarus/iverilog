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
#ident "$Id: PDelays.cc,v 1.5 2001/07/25 03:10:48 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

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

static unsigned long calculate_val(Design*des, const NetScope*scope,
				   const PExpr*expr)
{
      assert(expr);
      unsigned long val;

      int shift = scope->time_unit() - des->get_precision();

      if (verireal*dr = expr->eval_rconst(des, scope)) {
	    val = dr->as_long(shift);
	    delete dr;

      } else {
	    verinum*dv = expr->eval_const(des, scope->name());
	    if (dv == 0) {
		  cerr << expr->get_line() << ": sorry: non-constant "
		       << "delays not supported here: " << *expr << endl;
		  des->errors += 1;
		  return 0;
	    }

	    assert(dv);
	    val = dv->as_ulong();
	    val = des->scale_to_precision(val, scope);
	    delete dv;
      }

      return val;
}

void PDelays::eval_delays(Design*des, const string&path,
			  unsigned long&rise_time,
			  unsigned long&fall_time,
			  unsigned long&decay_time) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      int shift = scope->time_unit() - des->get_precision();


      if (delay_[0]) {
	    rise_time = calculate_val(des, scope, delay_[0]);

	    if (delay_[1]) {
		  fall_time = calculate_val(des, scope, delay_[1]);

		  if (delay_[2]) {
			decay_time = calculate_val(des, scope, delay_[2]);

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
 * Revision 1.5  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.4  2001/01/20 02:15:50  steve
 *  apologize for not supporting non-constant delays.
 *
 * Revision 1.3  2001/01/14 23:04:55  steve
 *  Generalize the evaluation of floating point delays, and
 *  get it working with delay assignment statements.
 *
 *  Allow parameters to be referenced by hierarchical name.
 *
 * Revision 1.2  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 */

