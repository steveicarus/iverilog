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
#ident "$Id: delay.cc,v 1.13 2006/07/08 21:48:00 steve Exp $"
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
      run_run_ptr_ = 0;
}

vvp_fun_delay::~vvp_fun_delay()
{
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

      if (cur_vec4_.eeq(bit))
	    return;

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

	/* And propagate it. */
      cur_vec4_ = bit;
      if (use_delay == 0) {
	    vvp_send_vec4(net_->out, cur_vec4_);
      } else {
	    run_run_ptr_ = &vvp_fun_delay::run_run_vec4_;
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

      cur_vec8_ = bit;
      if (use_delay == 0) {
	    vvp_send_vec8(net_->out, cur_vec8_);
      } else {
	    run_run_ptr_ = &vvp_fun_delay::run_run_vec8_;
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

      cur_real_ = bit;
      if (use_delay == 0) {
	    vvp_send_real(net_->out, cur_real_);
      } else {
	    run_run_ptr_ = &vvp_fun_delay::run_run_real_;
	    schedule_generic(this, use_delay, false);
      }
}

void vvp_fun_delay::run_run()
{
      (this->*run_run_ptr_)();
}

void vvp_fun_delay::run_run_vec4_()
{
      vvp_send_vec4(net_->out, cur_vec4_);
}

void vvp_fun_delay::run_run_vec8_()
{
      vvp_send_vec8(net_->out, cur_vec8_);
}

void vvp_fun_delay::run_run_real_()
{
      vvp_send_real(net_->out, cur_real_);
}


/*
 * $Log: delay.cc,v $
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
