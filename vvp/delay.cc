/*
 * Copyright (c) 2005-2011 Stephen Williams <steve@icarus.com>
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

#include "delay.h"
#include "schedule.h"
#include "vpi_priv.h"
#include "config.h"
#ifdef CHECK_WITH_VALGRIND
#include "vvp_cleanup.h"
#endif
#include <iostream>
#include <cstdlib>
#include <list>
#include <cassert>
#include <cmath>

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
      cur_vec8_ = vvp_vector8_t(cur_vec4_, 6, 6);
      cur_real_ = 0.0;
      list_ = 0;
      type_ = UNKNOWN_DELAY;
      initial_ = true;
	// Calculate the values used when converting variable delays
	// to simulation time units.
      struct __vpiScope*scope = vpip_peek_current_scope();

      int pow = scope->time_units - scope->time_precision;
      round_ = 1;
      for (int lp = 0; lp < pow; lp += 1) round_ *= 10;

      pow = scope->time_precision - vpip_get_time_precision();
      scale_ = 1;
      for (int lp = 0; lp < pow; lp += 1) scale_ *= 10;
}

vvp_fun_delay::~vvp_fun_delay()
{
      while (struct event_*cur = dequeue_())
	    delete cur;
}

bool vvp_fun_delay::clean_pulse_events_(vvp_time64_t use_delay,
                                        const vvp_vector4_t&bit)
{
      if (list_ == 0) return false;

	/* If the most recent event and the new event have the same
	 * value then we need to skip the new event. */
      if (list_->next->ptr_vec4.eeq(bit)) return true;

      clean_pulse_events_(use_delay);
      return false;
}

bool vvp_fun_delay::clean_pulse_events_(vvp_time64_t use_delay,
                                        const vvp_vector8_t&bit)
{
      if (list_ == 0) return false;

	/* If the most recent event and the new event have the same
	 * value then we need to skip the new event. */
      if (list_->next->ptr_vec8.eeq(bit)) return true;

      clean_pulse_events_(use_delay);
      return false;
}

bool vvp_fun_delay::clean_pulse_events_(vvp_time64_t use_delay,
                                        double bit)
{
      if (list_ == 0) return false;

	/* If the most recent event and the new event have the same
	 * value then we need to skip the new event. */
      if (list_->next->ptr_real == bit) return true;

      clean_pulse_events_(use_delay);
      return false;
}

void vvp_fun_delay::clean_pulse_events_(vvp_time64_t use_delay)
{
      assert(list_ != 0);

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
void vvp_fun_delay::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                              vvp_context_t)
{
      if (port.port() > 0) {
	      // Get the integer value of the bit vector, or 0 if
	      // there are X or Z bits.
	    vvp_time64_t bval = 0;
	      // The following does not work correctly for negative values.
	      // They should be sign extended to 64 bits (1364-2001 9.7.1).
	    vector4_to_value(bit, bval);
	      // Integer values do not need to be rounded so just scale them.
	    vvp_time64_t val = bval * round_ * scale_;

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

      vvp_time64_t use_delay;
	/* This is an initial value so it needs to be compared to all the
	   bits (the order the bits are changed is not deterministic). */
      if (initial_) {
	    type_ = VEC4_DELAY;
	    vvp_bit4_t cur_val = cur_vec4_.value(0);
	    use_delay = delay_.get_delay(cur_val, bit.value(0));
	    for (unsigned idx = 1 ;  idx < bit.size() ;  idx += 1) {
		  vvp_time64_t tmp;
		  tmp = delay_.get_delay(cur_val, bit.value(idx));
		  if (tmp > use_delay) use_delay = tmp;
	    }
      } else {
	    assert(type_ == VEC4_DELAY);

	      // Use as a reference for calculating the delay the
	      // current value of the output. Detect and handle the
	      // special case that the event list contains the current
	      // value as a zero-delay-remaining event.
	    const vvp_vector4_t&use_vec4 = (list_ && list_->next->sim_time == schedule_simtime())? list_->next->ptr_vec4 : cur_vec4_;

	      /* How many bits to compare? */
	    unsigned use_wid = use_vec4.size();
	    if (bit.size() < use_wid) use_wid = bit.size();

	      /* Scan the vectors looking for delays. Select the maximum
	         delay encountered. */
	    use_delay = delay_.get_delay(use_vec4.value(0), bit.value(0));

	    for (unsigned idx = 1 ;  idx < use_wid ;  idx += 1) {
		  vvp_time64_t tmp;
		  tmp = delay_.get_delay(use_vec4.value(idx), bit.value(idx));
		  if (tmp > use_delay) use_delay = tmp;
	    }
      }

      /* what *should* happen here is we check to see if there is a
         transaction in the queue. This would be a pulse that needs to be
         eliminated. */
      if (clean_pulse_events_(use_delay, bit)) return;

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;

	/* And propagate it. */
      if (use_delay == 0 && list_ == 0) {
	    cur_vec4_ = bit;
	    initial_ = false;
	    vvp_send_vec4(net_->out, cur_vec4_, 0);
      } else {
	    struct event_*cur = new struct event_(use_simtime);
	    cur->run_run_ptr = &vvp_fun_delay::run_run_vec4_;
	    cur->ptr_vec4 = bit;
	    enqueue_(cur);
	    schedule_generic(this, use_delay, false);
      }
}

/* See the recv_vec4 comment above. */
void vvp_fun_delay::recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit)
{
      assert(port.port() == 0);

      vvp_time64_t use_delay;
	/* This is an initial value so it needs to be compared to all the
	   bits (the order the bits are changed is not deterministic). */
      if (initial_) {
	    type_ = VEC8_DELAY;
	    vvp_bit4_t cur_val = cur_vec8_.value(0).value();
	    use_delay = delay_.get_delay(cur_val, bit.value(0).value());
	    for (unsigned idx = 1 ;  idx < bit.size() ;  idx += 1) {
		  vvp_time64_t tmp;
		  tmp = delay_.get_delay(cur_val, bit.value(idx).value());
		  if (tmp > use_delay) use_delay = tmp;
	    }
      } else {
	    assert(type_ == VEC8_DELAY);

	      // Use as a reference for calculating the delay the
	      // current value of the output. Detect and handle the
	      // special case that the event list contains the current
	      // value as a zero-delay-remaining event.
	    const vvp_vector8_t&use_vec8 = (list_ && list_->next->sim_time == schedule_simtime())? list_->next->ptr_vec8 : cur_vec8_;

	      /* How many bits to compare? */
	    unsigned use_wid = use_vec8.size();
	    if (bit.size() < use_wid) use_wid = bit.size();

	      /* Scan the vectors looking for delays. Select the maximum
	         delay encountered. */
	    use_delay = delay_.get_delay(use_vec8.value(0).value(),
	                                 bit.value(0).value());

	    for (unsigned idx = 1 ;  idx < use_wid ;  idx += 1) {
		  vvp_time64_t tmp;
		  tmp = delay_.get_delay(use_vec8.value(idx).value(),
		                         bit.value(idx).value());
		  if (tmp > use_delay) use_delay = tmp;
	    }
      }

      /* what *should* happen here is we check to see if there is a
         transaction in the queue. This would be a pulse that needs to be
         eliminated. */
      if (clean_pulse_events_(use_delay, bit)) return;

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;

	/* And propagate it. */
      if (use_delay == 0 && list_ == 0) {
	    cur_vec8_ = bit;
	    initial_ = false;
	    vvp_send_vec8(net_->out, cur_vec8_);
      } else {
	    struct event_*cur = new struct event_(use_simtime);
	    cur->ptr_vec8 = bit;
	    cur->run_run_ptr = &vvp_fun_delay::run_run_vec8_;
	    enqueue_(cur);
	    schedule_generic(this, use_delay, false);
      }
}

void vvp_fun_delay::recv_real(vvp_net_ptr_t port, double bit,
                              vvp_context_t)
{
      if (port.port() > 0) {
	    /* If the port is not 0, then this is a delay value that
	    should be rounded and converted to an integer delay. */
	    vvp_time64_t val = 0;
	    if (bit > -0.5) {
		  val = (vvp_time64_t) (bit * round_ + 0.5) * scale_;
	    } else if (bit != bit) {
		    // For a NaN we use the default (0).
	    } else {
		  vvp_vector4_t vec4(8*sizeof(vvp_time64_t),
		                     floor(-bit * round_ + 0.5) * -1 * scale_);
		  vector4_to_value(vec4, val);
	    }

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

      if (initial_) type_ = REAL_DELAY;
      else assert(type_ == REAL_DELAY);

      vvp_time64_t use_delay;
      use_delay = delay_.get_min_delay();

      /* Eliminate glitches. */
      if (clean_pulse_events_(use_delay, bit)) return;

      /* This must be done after cleaning pulses to avoid propagating
       * an incorrect value. */
      if (cur_real_ == bit) return;

      vvp_time64_t use_simtime = schedule_simtime() + use_delay;

      if (use_delay == 0 && list_ == 0) {
	    cur_real_ = bit;
	    initial_ = false;
	    vvp_send_real(net_->out, cur_real_, 0);
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
      initial_ = false;
      delete cur;
}

void vvp_fun_delay::run_run_vec4_(struct event_*cur)
{
      cur_vec4_ = cur->ptr_vec4;
      vvp_send_vec4(net_->out, cur_vec4_, 0);
}

void vvp_fun_delay::run_run_vec8_(struct vvp_fun_delay::event_*cur)
{
      cur_vec8_ = cur->ptr_vec8;
      vvp_send_vec8(net_->out, cur_vec8_);
}

void vvp_fun_delay::run_run_real_(struct vvp_fun_delay::event_*cur)
{
      cur_real_ = cur->ptr_real;
      vvp_send_real(net_->out, cur_real_, 0);
}

vvp_fun_modpath::vvp_fun_modpath(vvp_net_t*net)
: net_(net), src_list_(0), ifnone_list_(0)
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
      while (ifnone_list_) {
	    vvp_fun_modpath_src*tmp = ifnone_list_;
	    ifnone_list_ = tmp->next_;
	    delete tmp;
      }
}

void vvp_fun_modpath::add_modpath_src(vvp_fun_modpath_src*that, bool ifnone)
{
      assert(that->next_ == 0);
      if (ifnone) {
	    that->next_ = ifnone_list_;
	    ifnone_list_ = that;
      } else {
	    that->next_ = src_list_;
	    src_list_ = that;
      }
}

static vvp_time64_t delay_from_edge(vvp_bit4_t a, vvp_bit4_t b,
                                           vvp_time64_t array[12])
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

void vvp_fun_modpath::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                vvp_context_t)
{
	/* Only the first port is used. */
      if (port.port() > 0)
	    return;

      if (cur_vec4_.eeq(bit))
	    return;

	/* Select a time delay source that applies. Notice that there
	   may be multiple delay sources that apply, so collect all
	   the candidates into a list first. */
      list<vvp_fun_modpath_src*>candidate_list;
      vvp_time64_t candidate_wake_time = 0;
      for (vvp_fun_modpath_src*cur = src_list_ ;  cur ;  cur=cur->next_) {
	      /* Skip paths that are disabled by conditions. */
	    if (cur->condition_flag_ == false)
		  continue;

	    if (candidate_list.empty()) {
		  candidate_list.push_back(cur);
		  candidate_wake_time = cur->wake_time_;
	    } else if (cur->wake_time_ == candidate_wake_time) {
		  candidate_list.push_back(cur);
	    } else if (cur->wake_time_ > candidate_wake_time) {
		  candidate_list.assign(1, cur);
		  candidate_wake_time = cur->wake_time_;
	    } else {
		  continue; /* Skip this entry. */
	    }
      }

	/* Only add the ifnone delay if it has a later wake_time_ or
	 * if there are no normal delays. */
      vvp_time64_t ifnone_wake_time = candidate_wake_time;
      for (vvp_fun_modpath_src*cur = ifnone_list_ ;  cur ;  cur=cur->next_) {
	    if (candidate_list.empty()) {
		  candidate_list.push_back(cur);
		  ifnone_wake_time = cur->wake_time_;
	    } else if (cur->wake_time_ == ifnone_wake_time &&
	               ifnone_wake_time > candidate_wake_time) {
		  candidate_list.push_back(cur);
	    } else if (cur->wake_time_ > ifnone_wake_time) {
		  candidate_list.assign(1, cur);
		  ifnone_wake_time = cur->wake_time_;
	    } else {
		  continue; /* Skip this entry. */
	    }
      }

	/* Handle the special case that there are no delays that
	   match. This may happen, for example, if the set of
	   conditional delays is incomplete, leaving some cases
	   uncovered. In that case, just pass the data without delay */
      if (candidate_list.empty()) {
	    cur_vec4_ = bit;
	    schedule_generic(this, 0, false);
	    return;
      }

	/* Now given that we have a list of candidate delays, find for
	   each if the 12 numbers the minimum from all the
	   candidates. This minimum set becomes the chosen delay to
	   use. */
      vvp_time64_t out_at[12];
      vvp_time64_t now = schedule_simtime();

      typedef list<vvp_fun_modpath_src*>::const_iterator iter_t;

      iter_t cur = candidate_list.begin();
      vvp_fun_modpath_src*src = *cur;

      for (unsigned idx = 0 ;  idx < 12 ;  idx += 1) {
	    out_at[idx] = src->wake_time_ + src->delay_[idx];
	    if (out_at[idx] <= now)
		  out_at[idx] = 0;
	    else
		  out_at[idx] -= now;
      }

      for (cur ++ ; cur != candidate_list.end() ; cur ++) {
	    src = *cur;
	    for (unsigned idx = 0 ;  idx < 12 ;  idx += 1) {
		  vvp_time64_t tmp = src->wake_time_ + src->delay_[idx];
		  if (tmp <= now)
			tmp = 0;
		  else
			tmp -= now;
		  if (tmp < out_at[idx])
			out_at[idx] = tmp;
	    }
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
      vvp_send_vec4(net_->out, cur_vec4_, 0);
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

void vvp_fun_modpath_src::get_delay12(vvp_time64_t val[12]) const
{
      for (unsigned idx = 0 ;  idx < 12 ;  idx += 1)
	    val[idx] = delay_[idx];
}

void vvp_fun_modpath_src::put_delay12(const vvp_time64_t val[12])
{
      for (unsigned idx = 0 ;  idx < 12 ;  idx += 1)
	    delay_[idx] = val[idx];
}

void vvp_fun_modpath_src::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                    vvp_context_t)
{
      if (port.port() == 0) {
	      // The modpath input...
	    if (test_vec4(bit))
		  wake_time_ = schedule_simtime();

      } else if (port.port() == 1) {
	      // The modpath condition input...
	    if (bit.value(0) == BIT4_1)
		  condition_flag_ = true;
	    else
		  condition_flag_ = false;
      }
}

bool vvp_fun_modpath_src::test_vec4(const vvp_vector4_t&)
{
      return true;
}

vvp_fun_modpath_edge::vvp_fun_modpath_edge(vvp_time64_t del[12],
					   bool pos, bool neg)
: vvp_fun_modpath_src(del)
{
      old_value_ = BIT4_X;
      posedge_ = pos;
      negedge_ = neg;
}

bool vvp_fun_modpath_edge::test_vec4(const vvp_vector4_t&bit)
{
      vvp_bit4_t tmp = old_value_;
      old_value_ = bit.value(0);

      int edge_flag = edge(tmp, old_value_);
      if (edge_flag > 0) return posedge_;
      if (edge_flag < 0) return negedge_;
      return false;
}


/*
 * All the below routines that begin with
 * modpath_src_* belong the internal function
 * of an vpiModPathIn object. This is used to
 * make some specific delays path operations
 *
 */
static int modpath_src_get(int code, vpiHandle ref)
{
      struct __vpiModPathSrc*obj = vpip_modpath_src_from_handle(ref);
      assert(obj);
      return 0 ;
}

static void modpath_src_get_value(vpiHandle ref, p_vpi_value vp)
{
      assert((ref->vpi_type->type_code == vpiModPathIn));
      struct __vpiModPathSrc* modpathsrc = vpip_modpath_src_from_handle( ref) ;
      assert ( modpathsrc ) ;
      return  ;
}

static vpiHandle modpath_src_put_value(vpiHandle ref, s_vpi_value *vp, int )
{
      assert((ref->vpi_type->type_code == vpiModPathIn));
      struct __vpiModPathSrc* modpathsrc = vpip_modpath_src_from_handle( ref) ;
      assert ( modpathsrc ) ;
      return 0 ;
}

static vpiHandle modpath_src_get_handle(int code, vpiHandle ref)
{
      struct __vpiModPathSrc*rfp = vpip_modpath_src_from_handle(ref);
      assert(rfp);

      switch (code) {

	case vpiScope:
	  return vpi_handle(rfp->dest->scope);

	  case vpiModule:
	      { struct __vpiScope*scope = rfp->dest->scope;
		while (scope && scope->base.vpi_type->type_code != vpiModule)
		      scope = scope->scope;
		assert(scope);
		return vpi_handle(scope);
	      }

	    // Handles to path term objects should really be obtained via
	    // the vpi_iterate and vpi_scan functions. Continue to allow
	    // them to be obtained here for backwards compatibility with
	    // older versions of Icarus Verilog.

	  case vpiModPathIn:
	    return vpi_handle(&rfp->path_term_in);

	  case vpiModPathOut:
	    return vpi_handle(&rfp->dest->path_term_out);
      }
      return 0;
}

static vpiHandle modpath_src_iterate(int code, vpiHandle ref)
{
      struct __vpiModPathSrc*rfp = vpip_modpath_src_from_handle(ref);
      assert(rfp);

	// Module paths with multiple sources or destinations are
	// currently represented by a separate modpath object for
	// each source/destination combination, so there is only
	// ever one input path term and one output path term.
      switch (code) {
	  case vpiModPathIn: {
	    vpiHandle*args = (vpiHandle*)calloc(1, sizeof(vpiHandle*));
	    args[0] = vpi_handle(&rfp->path_term_in);
	    return vpip_make_iterator(1, args, true);
	  }
	  case vpiModPathOut: {
	    vpiHandle*args = (vpiHandle*)calloc(1, sizeof(vpiHandle*));
	    args[0] = vpi_handle(&rfp->dest->path_term_out);
	    return vpip_make_iterator(1, args, true);
	  }
      }
      return 0;
}

static vpiHandle modpath_src_index ( vpiHandle ref, int)
{
      assert( (ref->vpi_type->type_code == vpiModPathIn ) );
      return 0 ;
}


static int modpath_src_free_object( vpiHandle ref )
{
      assert( (ref->vpi_type->type_code == vpiModPathIn ) );
      free ( ref ) ;
      return 1 ;
}

/*
 * This routine will put specific dimension of delay[] values
 * into a vpiHandle. In this case, he will put
 * specific delays values in a vpiModPathIn object
 *
 */
static void modpath_src_put_delays (vpiHandle ref, p_vpi_delay delays)
{
      vvp_time64_t tmp[12];
      int idx;
      struct __vpiModPathSrc * src = vpip_modpath_src_from_handle( ref) ;
      assert(src) ;

      vvp_fun_modpath_src *fun = dynamic_cast<vvp_fun_modpath_src*>(src->net->fun);
      assert( fun );

      typedef unsigned char map_array_t[12];
      static const map_array_t map_2 = {0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0};
      static const map_array_t map12 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

      const map_array_t*use_map = 0;
      switch (delays->no_of_delays) {
	  case 2:
	    use_map = &map_2;
	    break;
	  case 12:
	    use_map = &map12;
	    break;
	  default:
	    assert(0);
	    break;
      }

      if (delays->time_type == vpiSimTime) {
	    for (idx = 0 ; idx < 12 ; idx += 1) {
		  tmp[idx] = vpip_timestruct_to_time(delays->da+use_map[0][idx]);
	    }
      } else {
	    for (idx = 0 ; idx < 12 ; idx += 1) {
		  tmp[idx] = vpip_scaled_real_to_time64(delays->da[use_map[0][idx]].real,
							src->dest->scope);
	    }
      }

      /* Now clean up any to-from-x delays to me the min/max based on
         the rules for selecting X delays. This only needs to happen
         if the X delays are not already explicitly given. */
      if (delays->no_of_delays <= 6) {
	    vvp_time64_t t_max = tmp[0];
	    vvp_time64_t t_min = tmp[1];
	    for (idx = 1 ; idx < delays->no_of_delays ; idx += 1) {
		  if (tmp[idx] > t_max) t_max = tmp[idx];
		  if (tmp[idx] < t_min) t_min = tmp[idx];
	    }
	    tmp[DELAY_EDGE_0x] = t_min;
	    tmp[DELAY_EDGE_x1] = t_max;
	    tmp[DELAY_EDGE_1x] = t_min;
	    tmp[DELAY_EDGE_x0] = t_max;
	    tmp[DELAY_EDGE_xz] = t_max;
	    tmp[DELAY_EDGE_zx] = t_min;
      }

      fun->put_delay12(tmp);
}

/*
 * This routine will retrieve the delay[12] values
 * of a vpiHandle. In this case, he will get an
 * specific delays values from a vpiModPathIn
 * object
 *
 */

static void modpath_src_get_delays ( vpiHandle ref, p_vpi_delay delays )
{
      struct __vpiModPathSrc*src = vpip_modpath_src_from_handle( ref) ;
      assert(src);

      vvp_fun_modpath_src *fun = dynamic_cast<vvp_fun_modpath_src*>(src->net->fun);
      assert(fun);

      int idx;
      vvp_time64_t tmp[12];
      fun->get_delay12(tmp);

      switch (delays->no_of_delays) {
	  case 1:
	  case 2:
	  case 3:
	  case 6:
	  case 12:
	    break;

	  default:
	    assert(0);
	    break;
      }

      if (delays->time_type == vpiSimTime) {
	    for (idx = 0; idx < delays->no_of_delays; idx += 1) {
		  vpip_time_to_timestruct(delays->da+idx, tmp[idx]);
	    }
      } else {
	    for (idx = 0; idx < delays->no_of_delays; idx += 1) {
		  delays->da[idx].real = vpip_time_to_scaled_real(tmp[idx], src->dest->scope);
	    }
      }
}

static int pathterm_get(int code, vpiHandle ref)
{
      struct __vpiModPathTerm*obj = vpip_modpath_term_from_handle(ref);
      assert(obj);

      switch (code) {
	  case vpiEdge:
	    return obj->edge;
	  default:
	    return 0;
      }
}

static vpiHandle pathterm_get_handle(int code, vpiHandle ref)
{
      struct __vpiModPathTerm*obj = vpip_modpath_term_from_handle(ref);
      assert(obj);

      switch (code) {
	  case vpiExpr:
	    return obj->expr;
	  default:
	    return 0;
      }
}

/*
* The __vpiModPathSrc class is what the VPI client sees as a
* vpiModPath object. The __vpiModPath structure contains items that
* are common to a bunch of modpaths, including the destination term.
*/
static const struct __vpirt vpip_modpath_src_rt = {
      vpiModPath,
      modpath_src_get,
      0, /* vpi_get_str */
      modpath_src_get_value,
      modpath_src_put_value,
      modpath_src_get_handle,
      modpath_src_iterate,
      modpath_src_index,
      modpath_src_free_object,
      modpath_src_get_delays,
      modpath_src_put_delays
};

static const struct __vpirt vpip_modpath_term_rt = {
      vpiPathTerm,
      pathterm_get,
      0, // vpi_get_str
      0, // vpi_get_value,
      0, // vpi_put_value,
      pathterm_get_handle,
      0, // vpi_iterate,
      0, // vpi_index,
      0, // vpi_free_object,
      0, // vpi_get_delays,
      0  // vpi_put_delays
};

static void initialize_path_term(struct __vpiModPathTerm&obj)
{
      obj.base.vpi_type = &vpip_modpath_term_rt;
      obj.expr = 0;
      obj.edge = vpiNoEdge;
}

/*
 * This function will construct a vpiModPath Object.
 * give a respective "net", and will point to his
 * respective functor
 */

#ifdef CHECK_WITH_VALGRIND
static struct __vpiModPath**mp_list = 0;
static unsigned mp_count = 0;
#endif

struct __vpiModPath* vpip_make_modpath(vvp_net_t *net)
{
      struct __vpiModPath*obj = (struct __vpiModPath *)calloc(1, sizeof ( struct __vpiModPath ) );
      obj->scope = vpip_peek_current_scope ( );

      initialize_path_term(obj->path_term_out);
      obj->input_net = net ;

#ifdef CHECK_WITH_VALGRIND
      mp_count += 1;
      mp_list = (struct __vpiModPath **) realloc(mp_list,
                mp_count*sizeof(struct __vpiModPath **));
      mp_list[mp_count-1] = obj;
#endif
      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void modpath_delete()
{
      for (unsigned idx = 0; idx < mp_count; idx += 1) {
	    free(mp_list[idx]);
      }
      free(mp_list);
      mp_list = 0;
      mp_count = 0;
}
#endif

/*
 * This function will construct a vpiModPathIn
 * ( struct __vpiModPathSrc ) Object. will give
 * a delays[12] values, and point to the specified functor
 *
 */

struct __vpiModPathSrc* vpip_make_modpath_src (struct __vpiModPath*path, vvp_time64_t use_delay[12] ,  vvp_net_t *net )
{
      struct __vpiModPathSrc *obj = (struct __vpiModPathSrc *) calloc (1, sizeof ( struct __vpiModPathSrc ) ) ;

      obj->base.vpi_type  = &vpip_modpath_src_rt;
      obj->dest = path;
      obj->net = net;
      initialize_path_term(obj->path_term_in);

      return obj;
}


/*
  this routine will safely convert a modpath vpiHandle
  to a struct __vpiModPath { }
*/

struct __vpiModPathTerm* vpip_modpath_term_from_handle(vpiHandle ref)
{
      if (ref->vpi_type->type_code != vpiPathTerm)
            return 0;

      return (struct __vpiModPathTerm*) ref;
}

/*
  this routine will safely convert a modpathsrc vpiHandle
  to a struct __vpiModPathSrc { }, This is equivalent to a
  vpiModPathIn handle
*/

struct __vpiModPathSrc* vpip_modpath_src_from_handle(vpiHandle ref)
{
      if (ref->vpi_type->type_code != vpiModPath)
            return 0;

      return (struct __vpiModPathSrc *) ref;
}
