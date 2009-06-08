/*
 * Copyright (c) 1999-2009 Stephen Williams (steve@icarus.com)
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
# include  "netmisc.h"

bool dly_used_no_timescale = false;
bool dly_used_timescale = false;
bool display_ts_dly_warning = true;


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

static NetExpr*calculate_val(Design*des, NetScope*scope, PExpr*expr)
{
      ivl_variable_type_t tmp_type = IVL_VT_NO_TYPE;
      bool tmp_flag = false;
      expr->test_width(des, scope, 0, 0, tmp_type, tmp_flag);
      NetExpr*dex = expr->elaborate_expr(des, scope, -1, false);
      eval_expr(dex);

	/* Print a warning if we find default and `timescale based
	 * delays in the design, since this is likely an error. */
      if (scope->time_from_timescale()) dly_used_timescale = true;
      else dly_used_no_timescale = true;

      if (display_ts_dly_warning &&
          dly_used_no_timescale && dly_used_timescale) {
	    cerr << "warning: Found both default and "
	            "`timescale based delays. Use" << endl;
	    cerr << "         -Wtimescale to find the "
	            "module(s) with no `timescale." << endl;
	    display_ts_dly_warning = false;
      }

	/* If the delay expression is a real constant or vector
	   constant, then evaluate it, scale it to the local time
	   units, and return an adjusted value. */

      if (NetECReal*tmp = dynamic_cast<NetECReal*>(dex)) {
	    verireal fn = tmp->value();

	    int shift = scope->time_unit() - des->get_precision();
	    int64_t delay = fn.as_long64(shift);
	    if (delay < 0)
		  delay = 0;

	    delete tmp;
	    NetEConst*tmp2 = new NetEConst(verinum(delay));
	    tmp2->set_line(*expr);
	    return tmp2;
      }


      if (NetEConst*tmp = dynamic_cast<NetEConst*>(dex)) {
	    verinum fn = tmp->value();

	    uint64_t delay = des->scale_to_precision(fn.as_ulong64(), scope);

	    delete tmp;
	    NetEConst*tmp2 = new NetEConst(verinum(delay));
	    tmp2->set_line(*expr);
	    return tmp2;
      }

	/* Oops, cannot evaluate down to a constant. */
      return dex;
}

static NetExpr* make_delay_nets(Design*des, NetScope*scope, NetExpr*expr)
{
      if (dynamic_cast<NetESignal*> (expr))
	    return expr;

      if (dynamic_cast<NetEConst*> (expr))
	    return expr;

      NetNet*sig = expr->synthesize(des, scope, expr);
      if (sig == 0) {
	    cerr << expr->get_fileline() << ": error: Expression " << *expr
		 << " is not suitable for delay expression." << endl;
	    return 0;
      }

      expr = new NetESignal(sig);
      return expr;
}

static NetExpr* calc_decay_time(NetExpr *rise, NetExpr *fall)
{
      NetEConst *c_rise = dynamic_cast<NetEConst*>(rise);
      NetEConst *c_fall = dynamic_cast<NetEConst*>(fall);
      if (c_rise && c_fall) {
	    if (c_rise->value() < c_fall->value()) return rise;
	    else return fall;
      }

      cerr << fall->get_fileline() << ": sorry: can not calculate the "
           << "decay time from " << *rise << " and " << *fall << endl;
      return 0;
}

void PDelays::eval_delays(Design*des, NetScope*scope,
			  NetExpr*&rise_time,
			  NetExpr*&fall_time,
			  NetExpr*&decay_time,
			  bool as_nets_flag) const
{
      assert(scope);


      if (delay_[0]) {
	    rise_time = calculate_val(des, scope, delay_[0]);
	    if (as_nets_flag)
		  rise_time = make_delay_nets(des, scope, rise_time);

	    if (delay_[1]) {
		  fall_time = calculate_val(des, scope, delay_[1]);
		  if (as_nets_flag)
			fall_time = make_delay_nets(des, scope, fall_time);

		  if (delay_[2]) {
			decay_time = calculate_val(des, scope, delay_[2]);
			if (as_nets_flag)
			      decay_time = make_delay_nets(des, scope,
			                                   decay_time);

		  } else {
			// If this is zero then we need to do the min()
			// at run time.
			decay_time = calc_decay_time(rise_time, fall_time);
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
