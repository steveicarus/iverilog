/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: resolv.cc,v 1.1 2001/05/09 02:53:53 steve Exp $"
#endif

# include  "resolv.h"
# include  "schedule.h"

/*
 * For now, cheat and resolve the values without using the actual
 * strengths. The strengths are not yet available to the inputs, so
 * this is all we can do for now.
 */
void vvp_resolv_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      unsigned in1 = (fp->ival >> 0) & 3;
      unsigned in2 = (fp->ival >> 2) & 3;
      unsigned in3 = (fp->ival >> 4) & 3;
      unsigned in4 = (fp->ival >> 5) & 3;

      unsigned val = in1;
      if (in2 != 3) {
	    if (val == 3) {
		  val = in2;
	    } else if (val != in2) {
		  val = 2;
	    } else {
	    }
      }

      if (in3 != 3) {
	    if (val == 3) {
		  val = in3;
	    } else if (val != in3) {
		  val = 2;
	    } else {
	    }
      }

      if (in4 != 3) {
	    if (val == 3) {
		  val = in4;
	    } else if (val != in4) {
		  val = 2;
	    } else {
	    }
      }

	/* If the output changes, then create a propagation event. */
      if (val != fp->oval) {
	    fp->oval = val;
	    if (push)
		  functor_propagate(ptr);
	    else
		  schedule_functor(ptr, 0);
      }
}

/*
 * $Log: resolv.cc,v $
 * Revision 1.1  2001/05/09 02:53:53  steve
 *  Implement the .resolv syntax.
 *
 */

