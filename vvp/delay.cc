/*
 * Copyright (c) 2005 Stephen Williams <steve@icarus.com>
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
#ident "$Id: delay.cc,v 1.17 2007/03/01 06:19:39 steve Exp $"
#endif

#include "delay.h"
#include "schedule.h"
#include <iostream>
#include <assert.h>

void vvp_delay_t::calculate_min_delay_()
{
      min_delay_ = rise_;
      if (fall_ < min_delay_)
	    min_delay_ = fall_;
      if (decay_ < min_delay_)
	    min_delay_ = decay_;
}

vvp_delay_t::vvp_delay_t(vvp_time64_t rise, vvp_time64_t fall)
{
      rise_ = rise;
      fall_ = fall;
      decay_= fall < rise? fall : rise;
      min_delay_ = decay_;
}

vvp_delay_t::vvp_delay_t(vvp_time64_t rise, vvp_time64_t fall, vvp_time64_t decay)
{
      rise_ = rise;
      fall_ = fall;
      decay_= decay;

      calculate_min_delay_();
}

vvp_delay_t::~vvp_delay_t()
{
}

vvp_time64_t vvp_delay_t::get_delay(vvp_bit4_t from, vvp_bit4_t to)
{
      switch (from) {
	  case BIT4_0:
	    switch (to) {
		case BIT4_0: return 0;
		case BIT4_1: return rise_;
		case BIT4_X: return min_delay_;
		case BIT4_Z: return decay_;
	    }
	    break;
	  case BIT4_1:
	    switch (to) {
		case BIT4_0: return fall_;
		case BIT4_1: return 0;
		case BIT4_X: return min_delay_;
		case BIT4_Z: return decay_;
	    }
	    break;
	  case BIT4_X:
	    switch (to) {
		case BIT4_0: return fall_;
		case BIT4_1: return rise_;
		case BIT4_X: return 0;
		case BIT4_Z: return decay_;
	    }
	    break;
	  case BIT4_Z:
	    switch (to) {
		case BIT4_0: return fall_;
		case BIT4_1: return rise_;
		case BIT4_X: return min_delay_;
		case BIT4_Z: return 0;
	    }
	    break;
      }

      assert(0);
      return 0;
}

vvp_time64_t vvp_delay_t::get_min_delay() const
{
      return min_delay_;
}

void vvp_delay_t::set_rise(vvp_time64_t val)
{
      rise_ = val;
      if (val < min_delay_)
	    min_delay_ = val;
      else
	    calculate_min_delay_();
}

void vvp_delay_t::set_fall(vvp_time64_t val)
{
      fall_ = val;
      if (val < min_delay_)
	    min_delay_ = val;
      else
	    calculate_min_delay_();
}

void vvp_delay_t::set_decay(vvp_time64_t val)
{
      decay_ = val;
      if (val < min_delay_)
	    min_delay_ = val;
      else
	    calculate_min_delay_();
}

vvp_fun_delay::vvp_fun_delay(vvp_net_t*n, vvp_bit4_t init, const vvp_delay_t&d)
: net_(n), delay_(d), cur_vec4_(1)
{
      cur_vec4_.set_bit(0, init);
      list_ = 0;
}

vvp_fun_delay::~vvp_fun_delay()
{
      while (struct event_*cur = dequeue_())
	    delete cur;
}

void vvp_fun_delay::clean_pulse_events_(vvp_time64_t use_delay)
{
      if (list_ == 0)
	    return;

      do {
	    struct event_*cur = list_->next;
	      /* If this event is far enough from the event I'm about
	         to create, then that scheduled event is not a pulse
	         to be eliminated, so we're done. */
	    if (cur->sim_time+use_delay <= use_delay+schedule_simtime())
		  break;

	    if (list_ == cur)
		  list_ = 0;
	    else
		  list_->next = cur->next;
	    delete cur;
      } while (list_);
}

/*
 * FIXME: this implementation currently only uses the maximum delay
 * from all the bit changes in the vectors. If there are multiple
 * changes with different delays, then the results would be
 * wrong. What should happen is that if there are multiple changes,
 * multiple vectors approaching the result should be scheduled.
 */
void vvp_fun_delay::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      if (port.port() > 0) {
	      // Get the integer value of the bit vector, or 0 if
	      // there are X or Z bits.
	    unsigned long val = 0;
	    vector4_to_value(bit, val);

	    switch (port.port()) {
		case 1:
		  delay_.set_rise(val);
		  return;
		case 2:
		  delay_.set_fall(val);
		  return;
		case 3:
		  delay_.set_decay(val);
		  return;
	    }
	    return;
      }

	/* How many bits to compare? */
      unsigned use_wid = cur_vec4_.size();
      if (bit.size() < use_wid)
	    use_wid = bit.size();

	/* Scan the vectors looking for delays. Select the maximim
	   delay encountered. */
      vvp_time64_t use_delay;
      use_delay = delay_.get_delay(cur_vec4_.value(0), bit.value(0));

      for (unsigned idx = 1 ;  idx < use_wid ;  idx += 1) {
	    vvp_time64_t tmp;
	    tmp = delay_.get_delay(cur_vec4_.value(idx), bit.value(idx));
	    if (tmp > use_delay)
		  use_delay = tmp;
      }

      /* what *should* happen here is we check to see if there is a
         transaction in the queue. This would be a pulse that needs to be
         eliminated. */
      clean_pulse_events_(use_delay);

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;

	/* And propagate it. */
      if (use_delay == 0) {
	    cur_vec4_ = bit;
	    vvp_send_vec4(net_->out, cur_vec4_);
      } else {
	    struct event_*cur = new struct event_(use_simtime);
	    cur->run_run_ptr = &vvp_fun_delay::run_run_vec4_;
	    cur->ptr_vec4 = bit;
	    enqueue_(cur);
	    schedule_generic(this, use_delay, false);
      }
}

void vvp_fun_delay::recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit)
{
      assert(port.port() == 0);

      if (cur_vec8_.eeq(bit))
	    return;

	/* XXXX FIXME: For now, just use the minimum delay. */
      vvp_time64_t use_delay;
      use_delay = delay_.get_min_delay();

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;
      if (use_delay == 0) {
	    cur_vec8_ = bit;
	    vvp_send_vec8(net_->out, cur_vec8_);
      } else {
	    struct event_*cur = new struct event_(use_simtime);
	    cur->ptr_vec8 = bit;
	    cur->run_run_ptr = &vvp_fun_delay::run_run_vec8_;
	    enqueue_(cur);
	    schedule_generic(this, use_delay, false);
      }
}

void vvp_fun_delay::recv_real(vvp_net_ptr_t port, double bit)
{
      if (port.port() > 0) {
	    /* If the port is not 0, then this is a delay value that
	    should be rounded and converted to an integer delay. */
	    unsigned long long val = 0;
	    if (bit > 0)
		  val = (unsigned long long) (bit+0.5);

	    switch (port.port()) {
		case 1:
		  delay_.set_rise(val);
		  return;
		case 2:
		  delay_.set_fall(val);
		  return;
		case 3:
		  delay_.set_decay(val);
		  return;
	    }
	    return;
      }

      if (cur_real_ == bit)
	    return;

      vvp_time64_t use_delay;
      use_delay = delay_.get_min_delay();

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;

      if (use_delay == 0) {
	    cur_real_ = bit;
	    vvp_send_real(net_->out, cur_real_);
      } else {
	    struct event_*cur = new struct event_(use_simtime);
	    cur->run_run_ptr = &vvp_fun_delay::run_run_real_;
	    cur->ptr_real = bit;
	    enqueue_(cur);

	    schedule_generic(this, use_delay, false);
      }
}

void vvp_fun_delay::run_run()
{
      vvp_time64_t sim_time = schedule_simtime();
      if (list_ == 0 || list_->next->sim_time > sim_time)
	    return;

      struct event_*cur = dequeue_();
      if (cur == 0)
	    return;

      (this->*(cur->run_run_ptr))(cur);
      delete cur;
}

void vvp_fun_delay::run_run_vec4_(struct event_*cur)
{
      cur_vec4_ = cur->ptr_vec4;
      vvp_send_vec4(net_->out, cur_vec4_);
}

void vvp_fun_delay::run_run_vec8_(struct vvp_fun_delay::event_*cur)
{
      cur_vec8_ = cur->ptr_vec8;
      vvp_send_vec8(net_->out, cur_vec8_);
}

void vvp_fun_delay::run_run_real_(struct vvp_fun_delay::event_*cur)
{
      cur_real_ = cur->ptr_real;
      vvp_send_real(net_->out, cur_real_);
}

vvp_fun_modpath::vvp_fun_modpath(vvp_net_t*net)
: net_(net), src_list_(0)
{
}

vvp_fun_modpath::~vvp_fun_modpath()
{
	// Delete the source probes.
      while (src_list_) {
	    vvp_fun_modpath_src*tmp = src_list_;
	    src_list_ = tmp->next_;
	    delete tmp;
      }
}

void vvp_fun_modpath::add_modpath_src(vvp_fun_modpath_src*that)
{
      assert(that->next_ == 0);
      that->next_ = src_list_;
      src_list_ = that;
}

static vvp_time64_t delay_from_edge(vvp_bit4_t a, vvp_bit4_t b, vvp_time64_t array[12])
{
      typedef delay_edge_t bit4_table4[4];
      const static bit4_table4 edge_table[4] = {
	    { DELAY_EDGE_01, DELAY_EDGE_01, DELAY_EDGE_0x, DELAY_EDGE_0z },
	    { DELAY_EDGE_10, DELAY_EDGE_10, DELAY_EDGE_1x, DELAY_EDGE_1z },
	    { DELAY_EDGE_x0, DELAY_EDGE_x1, DELAY_EDGE_x0, DELAY_EDGE_xz },
	    { DELAY_EDGE_z0, DELAY_EDGE_z1, DELAY_EDGE_zx, DELAY_EDGE_z0 }
      };

      return array[ edge_table[a][b] ];
}

void vvp_fun_modpath::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
	/* Only the first port is used. */
      if (port.port() > 0)
	    return;

      if (cur_vec4_.eeq(bit))
	    return;

	/* Select a time delay source that applies. */
      vvp_fun_modpath_src*src = 0;
      for (vvp_fun_modpath_src*cur = src_list_ ;  cur ;  cur=cur->next_) {
	      /* Skip paths that are disabled by conditions. */
	    if (cur->condition_flag_ == false)
		  continue;

	    if (src == 0) {
		  src = cur;
	    } else if (cur->wake_time_ > src->wake_time_) {
		  src = cur;
	    } else {
		  continue; /* Skip this entry. */
	    }
      }

      vvp_time64_t out_at[12];
      vvp_time64_t now = schedule_simtime();
      for (unsigned idx = 0 ;  idx < 12 ;  idx += 1) {
	    out_at[idx] = src->wake_time_ + src->delay_[idx];
	    if (out_at[idx] <= now)
		  out_at[idx] = 0;
	    else
		  out_at[idx] -= now;
      }

	/* Given the scheduled output time, create an output event. */
      vvp_time64_t use_delay = delay_from_edge(cur_vec4_.value(0),
					       bit.value(0),
					       out_at);

	/* FIXME: This bases the edge delay on only the least
	   bit. This is WRONG! I need to find all the possible delays,
	   and schedule an event for each partial change. Hard! */
      for (unsigned idx = 1 ;  idx < bit.size() ;  idx += 1) {
	    vvp_time64_t tmp = delay_from_edge(cur_vec4_.value(idx),
					       bit.value(0),
					       out_at);
	    assert(tmp == use_delay);
      }

      cur_vec4_ = bit;
      schedule_generic(this, use_delay, false);
}

void vvp_fun_modpath::run_run()
{
      vvp_send_vec4(net_->out, cur_vec4_);
}

vvp_fun_modpath_src::vvp_fun_modpath_src(vvp_time64_t del[12])
{
      for (unsigned idx = 0 ;  idx < 12 ;  idx += 1)
	    delay_[idx] = del[idx];

      next_ = 0;
      wake_time_ = 0;
      condition_flag_ = true;
}

vvp_fun_modpath_src::~vvp_fun_modpath_src()
{
}

void vvp_fun_modpath_src::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      if (port.port() == 0) {
	      // The modpath input...
	    wake_time_ = schedule_simtime();

      } else if (port.port() == 1) {
	      // The modpath condition input...
	    if (bit.value(0) == BIT4_1)
		  condition_flag_ = true;
	    else
		  condition_flag_ = false;
      }
}

/*
 * $Log: delay.cc,v $
 * Revision 1.17  2007/03/01 06:19:39  steve
 *  Add support for conditional specify delay paths.
 *
 * Revision 1.16  2007/01/26 05:15:41  steve
 *  More literal implementation of inertial delay model.
 *
 * Revision 1.15  2006/09/29 03:57:01  steve
 *  Modpath delay chooses correct delay for edge.
 *
 * Revision 1.14  2006/09/23 04:57:19  steve
 *  Basic support for specify timing.
 *
 * Revision 1.13  2006/07/08 21:48:00  steve
 *  Delay object supports real valued delays.
 *
 * Revision 1.12  2006/01/02 05:32:07  steve
 *  Require explicit delay node from source.
 *
 * Revision 1.11  2005/11/10 13:27:16  steve
 *  Handle very wide % and / operations using expanded vector2 support.
 *
 * Revision 1.10  2005/09/20 18:34:02  steve
 *  Clean up compiler warnings.
 *
 * Revision 1.9  2005/07/06 04:29:25  steve
 *  Implement real valued signals and arith nodes.
 *
 * Revision 1.8  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.7  2005/06/09 05:04:45  steve
 *  Support UDP initial values.
 *
 * Revision 1.6  2005/06/02 16:02:11  steve
 *  Add support for notif0/1 gates.
 *  Make delay nodes support inertial delay.
 *  Add the %force/link instruction.
 *
 * Revision 1.5  2005/05/14 19:43:23  steve
 *  Move functor delays to vvp_delay_fun object.
 *
 * Revision 1.4  2005/04/03 05:45:51  steve
 *  Rework the vvp_delay_t class.
 *
 */
