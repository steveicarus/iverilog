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
#ident "$Id: arith.cc,v 1.6 2001/06/29 01:20:20 steve Exp $"
#endif

# include  "arith.h"
# include  "schedule.h"
# include  <assert.h>

# include  <stdio.h>

vvp_arith_::vvp_arith_(vvp_ipoint_t b, unsigned w)
: base_(b), wid_(w)
{
}

void vvp_arith_::output_x_(bool push)
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

vvp_arith_mult::vvp_arith_mult(vvp_ipoint_t b, unsigned w)
: vvp_arith_(b, w)
{
}

void vvp_arith_mult::set(vvp_ipoint_t i, functor_t f, bool push)
{
      assert(wid_ <= 8*sizeof(unsigned long));

      unsigned long a = 0, b = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base_,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned ival = obj->ival;
	    if (ival & 0xaa) {
		  output_x_(push);
		  return;
	    }

	    if (ival & 0x01)
		  a += 1 << idx;
	    if (ival & 0x04)
		  b += 1 << idx;

      }

      unsigned long sum = a * b;

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

vvp_arith_sum::vvp_arith_sum(vvp_ipoint_t b, unsigned w)
: vvp_arith_(b, w)
{
      sum_ = new unsigned long[(w+1) / 8*sizeof(unsigned long) + 1];
}

vvp_arith_sum::~vvp_arith_sum()
{
      delete[]sum_;
}

void vvp_arith_sum::set(vvp_ipoint_t i, functor_t f, bool push)
{
      assert(wid_ <= 8*sizeof(unsigned long));

      unsigned page = 0;
      unsigned pbit = 0;
      unsigned carry = 0;

      sum_[0] = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base_,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned ival = obj->ival;
	    if (ival & 0xaa) {
		  output_x_(push);
		  return;
	    }

	      // Accumulate the sum of the input bits.
	    unsigned tmp = 0;
	    if (ival & 0x01)
		  tmp += 1;
	    if (ival & 0x04)
		  tmp += 1;
	    if (ival & 0x10)
		  tmp += 1;
	    if (ival & 0x40)
		  tmp += 1;

	      // Add in the carry carried over.
	    tmp += carry;
	      // Put the next bit into the sum,
	    sum_[page] |= ((tmp&1) << pbit);
	      // ... and carry the remaining bits.
	    carry = tmp >> 1;

	    pbit += 1;
	    if (pbit == 8 * sizeof sum_[page]) {
		  pbit = 0;
		  page += 1;
		  sum_[page] = 0;
	    }
      }

      page = 0;
      pbit = 0;
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base_,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned oval = (sum_[page] >> pbit) & 1;

	    pbit += 1;
	    if (pbit == 8 * sizeof sum_[page]) {
		  pbit = 0;
		  page += 1;
	    }

	    if (obj->oval == oval)
		  continue;


	    obj->oval = oval;
	    if (push)
		  functor_propagate(ptr);
	    else
		  schedule_functor(ptr, 0);
      }
}

vvp_arith_sub::vvp_arith_sub(vvp_ipoint_t b, unsigned w)
: vvp_arith_(b, w)
{
}

void vvp_arith_sub::set(vvp_ipoint_t i, functor_t f, bool push)
{
      assert(wid_ <= 8*sizeof(unsigned long));

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
		  tmp -= 1;
	    if (ival & 0x10)
		  tmp -= 1;
	    if (ival & 0x40)
		  tmp -= 1;

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

vvp_cmp_ge::vvp_cmp_ge(vvp_ipoint_t b, unsigned w)
: vvp_arith_(b, w)
{
}

void vvp_cmp_ge::set(vvp_ipoint_t i, functor_t f, bool push)
{
      functor_t base_obj = functor_index(base_);
      unsigned out_val = 1;

      for (unsigned idx = wid_ ;  idx > 0 ;  idx -= 1) {
	    vvp_ipoint_t ptr = ipoint_index(base_,idx-1);
	    functor_t obj = functor_index(ptr);

	    unsigned ival = obj->ival;
	    if (ival & 0x0a) {
		  out_val = 2;
		  break;
	    }

	    unsigned a = (ival & 0x01)? 1 : 0;
	    unsigned b = (ival & 0x04)? 1 : 0;

	    if (a > b) {
		  out_val = 1;
		  break;
	    }

	    if (a < b) {
		  out_val = 0;
		  break;
	    }
      }

      if (out_val != base_obj->oval) {
	    base_obj->oval = out_val;
	    if (push)
		  functor_propagate(base_);
	    else
		  schedule_functor(base_, 0);
      }
}

vvp_cmp_gt::vvp_cmp_gt(vvp_ipoint_t b, unsigned w)
: vvp_arith_(b, w)
{
}

void vvp_cmp_gt::set(vvp_ipoint_t i, functor_t f, bool push)
{
      functor_t base_obj = functor_index(base_);
      unsigned out_val = 0;

      for (unsigned idx = wid_ ;  idx > 0 ;  idx -= 1) {
	    vvp_ipoint_t ptr = ipoint_index(base_,idx-1);
	    functor_t obj = functor_index(ptr);

	    unsigned ival = obj->ival;
	    if (ival & 0x0a) {
		  out_val = 2;
		  break;
	    }

	    unsigned a = (ival & 0x01)? 1 : 0;
	    unsigned b = (ival & 0x04)? 1 : 0;

	    if (a > b) {
		  out_val = 1;
		  break;
	    }

	    if (a < b) {
		  out_val = 0;
		  break;
	    }
      }

      if (out_val != base_obj->oval) {
	    base_obj->oval = out_val;
	    if (push)
		  functor_propagate(base_);
	    else
		  schedule_functor(base_, 0);
      }
}

/*
 * $Log: arith.cc,v $
 * Revision 1.6  2001/06/29 01:20:20  steve
 *  Relax limit on width of structural sum.
 *
 * Revision 1.5  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 *
 * Revision 1.4  2001/06/16 03:36:03  steve
 *  Relax width restriction for structural comparators.
 *
 * Revision 1.3  2001/06/15 04:07:58  steve
 *  Add .cmp statements for structural comparison.
 *
 * Revision 1.2  2001/06/07 03:09:03  steve
 *  Implement .arith/sub subtraction.
 *
 * Revision 1.1  2001/06/05 03:05:41  steve
 *  Add structural addition.
 *
 */

