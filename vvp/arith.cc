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
#ident "$Id: arith.cc,v 1.1 2001/06/05 03:05:41 steve Exp $"
#endif

# include  "arith.h"
# include  "schedule.h"
# include  <assert.h>

# include  <stdio.h>

vvp_arith_sum::vvp_arith_sum(vvp_ipoint_t b, unsigned w)
: base_(b), wid_(w)
{
}

void vvp_arith_sum::output_x_(bool push)
{
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base_,idx);
	    functor_t obj = functor_index(ptr);

	    if (obj->oval == 2)
		  continue;

	    obj->oval = 2;
	    if (push)
		  functor_propagate(ptr);
	    else
		  schedule_functor(ptr, 0);
      }
}

void vvp_arith_sum::set(vvp_ipoint_t i, functor_t f, bool push)
{
      assert(wid_ <= sizeof(unsigned long));

      unsigned long sum = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base_,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned ival = obj->ival;
	    if (ival & 0xaa) {
		  output_x_(push);
		  return;
	    }

	    unsigned tmp = 0;
	    if (ival & 0x01)
		  tmp += 1;
	    if (ival & 0x04)
		  tmp += 1;
	    if (ival & 0x10)
		  tmp += 1;
	    if (ival & 0x40)
		  tmp += 1;

	    sum += (tmp << idx);
      }
	    
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base_,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned oval = sum & 1;
	    sum >>= 1;

	    if (obj->oval == oval)
		  continue;


	    obj->oval = oval;
	    if (push)
		  functor_propagate(ptr);
	    else
		  schedule_functor(ptr, 0);
      }
}

/*
 * $Log: arith.cc,v $
 * Revision 1.1  2001/06/05 03:05:41  steve
 *  Add structural addition.
 *
 */

