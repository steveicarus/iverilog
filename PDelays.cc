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

# include  <iostream>

# include  "PDelays.h"
# include  "PExpr.h"
# include  "verinum.h"

PDelays::PDelays()
{
      delete_flag_ = true;
      for (unsigned idx = 0 ;  idx < 3 ;  idx += 1)
	    delay_[idx] = 0;
}

PDelays::~PDelays()
{
      if (delete_flag_) {
	    for (unsigned idx = 0 ;  idx < 3 ;  idx += 1)
		  delete delay_[idx];
      }
}

void PDelays::set_delay(PExpr*del)
{
      assert(del);
      assert(delay_[0] == 0);
      delay_[0] = del;
      delete_flag_ = true;
}


void PDelays::set_delays(const svector<PExpr*>*del, bool df)
{
      assert(del);
      assert(del->count() <= 3);
      for (unsigned idx = 0 ;  idx < del->count() ;  idx += 1)
	    delay_[idx] = (*del)[idx];

      delete_flag_ = df;
}

static unsigned long calculate_val(Design*des, NetScope*scope,
				   const PExpr*expr)
{

      NetExpr*dex = expr->elaborate_expr(des, scope);
      if (NetExpr*tmp = dex->eval_tree()) {
	    delete dex;
	    dex = tmp;
      }

      int shift = scope->time_unit() - des->get_precision();
      if (sizeof(unsigned) <= 4 && shift > 9) {
	    cerr << expr->get_line() << ": error: Precision overflow"
		 << " in scope " << scope->name() << "." << endl;
	    cerr << expr->get_line() << ":      :"
		 << " Units are 10e" << scope->time_unit()
		 << " minus precision 10e" << des->get_precision()
		 << " is 10e" << shift << "." << endl;
	    cerr << expr->get_line() << ":      : Perhaps a timescale "
		 << "is missing or incorrect?" << endl;
	    des->errors += 1;
	    return 0;
      }

	/* If the delay expression is a real constant or vector
	   constant, then evaluate it, scale it to the local time
	   units, and return an adjusted value. */

      if (NetECReal*tmp = dynamic_cast<NetECReal*>(dex)) {
	    verireal fn = tmp->value();

	    long delay = fn.as_long(shift);
	    if (delay < 0)
		  delay = 0;

	    delete tmp;
	    return delay;
      }


      if (NetEConst*tmp = dynamic_cast<NetEConst*>(dex)) {
	    verinum fn = tmp->value();

	    unsigned long delay =
		  des->scale_to_precision(fn.as_ulong(), scope);

	    delete tmp;
	    return delay;
      }

	/* Oops, cannot evaluate down to a constant. Error message. */
      delete dex;

      cerr << expr->get_line() << ": sorry: non-constant "
	   << "delays not supported here: " << *expr << endl;
      des->errors += 1;
      return 0;

}

void PDelays::eval_delays(Design*des, NetScope*scope,
			  unsigned long&rise_time,
			  unsigned long&fall_time,
			  unsigned long&decay_time) const
{
      assert(scope);


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
