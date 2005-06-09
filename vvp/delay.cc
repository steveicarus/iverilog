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
#ident "$Id: delay.cc,v 1.7 2005/06/09 05:04:45 steve Exp $"
#endif

#include "delay.h"
#include "schedule.h"
#include <string.h>
#include <stream.h>
#include <assert.h>

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

      min_delay_ = rise_;
      if (fall_ < min_delay_)
	    min_delay_ = fall_;
      if (decay_ < min_delay_)
	    min_delay_ = decay_;
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

vvp_fun_delay::vvp_fun_delay(vvp_net_t*n, vvp_bit4_t init, const vvp_delay_t&d)
: net_(n), delay_(d), cur_(1)
{
      cur_.set_bit(0, init);
}

vvp_fun_delay::~vvp_fun_delay()
{
}

/*
 * FIXME: This implementation currently only uses the LSB to determine
 * the delay type for the entire vector. It needs to be upgraded to
 * account for different delays for different bits by generating a
 * stream of vectors that lead up to the actual value.
 */
void vvp_fun_delay::recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit)
{
      if (cur_.eeq(bit))
	    return;

      vvp_time64_t use_delay;
      use_delay = delay_.get_delay(cur_.value(0), bit.value(0));

      cur_ = bit;
      if (use_delay == 0)
	    vvp_send_vec4(net_->out, cur_);
      else
	    schedule_generic(this, use_delay, false);
}

void vvp_fun_delay::run_run()
{
      vvp_send_vec4(net_->out, cur_);
}

/*
 * $Log: delay.cc,v $
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
