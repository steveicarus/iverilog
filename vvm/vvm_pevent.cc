/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_pevent.cc,v 1.2 1999/05/01 02:57:53 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_gates.h"

vvm_sync::vvm_sync()
: hold_(0)
{
}

void vvm_sync::wait(vvm_thread*thr)
{
      assert(hold_ == 0);
      hold_ = thr;
}

void vvm_sync::wakeup(vvm_simulation*sim)
{
      vvm_thread*tmp = hold_;
      hold_ = 0;
      if (tmp) sim->thread_active(tmp);
}

vvm_pevent::vvm_pevent(vvm_sync*tgt, EDGE e)
: target_(tgt), value_(V0), edge_(e)
{
}

void vvm_pevent::set(vvm_simulation*sim, unsigned, vvm_bit_t val)
{
      if (value_ != val) {
	    switch (edge_) {
		case ANYEDGE:
		  target_->wakeup(sim);
		  break;
		case POSEDGE:
		  if (val == V1)
			target_->wakeup(sim);
		  break;
		case NEGEDGE:
		  if (val == V0)
			target_->wakeup(sim);
		  break;
	    }
	    value_ = val;
      }
}

/*
 * $Log: vvm_pevent.cc,v $
 * Revision 1.2  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */

