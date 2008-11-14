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
#ifdef HAVE_CVS_IDENT
#ident "$Id: arith.cc,v 1.30 2004/10/04 01:10:58 steve Exp $"
#endif

# include  "arith.h"
# include  "schedule.h"
# include  <limits.h>
# include  <stdio.h>
# include  <assert.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

void vvp_arith_::output_x_(vvp_ipoint_t base, bool push, unsigned val)
{
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);

	    obj->put_oval(val, push);
      }
}

void vvp_arith_::output_val_(vvp_ipoint_t base, bool push, unsigned long sum)
{
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = sum & 1;
	    sum >>= 1;

	    obj->put_oval(val, push);
      }
}

// Make sure the static sum_ scratch space is large enough for everybody

vvp_wide_arith_::vvp_wide_arith_(unsigned wid)
      : vvp_arith_(wid)
{
      pagecount_ = (wid + pagesize - 1)/pagesize;
      sum_ = (unsigned long *)calloc(pagecount_, sizeof(unsigned long));
      assert(sum_);
}

void vvp_wide_arith_::output_val_(vvp_ipoint_t base, bool push)
{
      unsigned page = 0;
      unsigned pbit = 0;
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = (sum_[page] >> pbit) & 1;

	    pbit += 1;
	    if (pbit == pagesize) {
		  pbit = 0;
		  page += 1;
	    }

	    obj->put_oval(val, push);
      }
}


// Division

inline void vvp_arith_div::wide(vvp_ipoint_t base, bool push)
{
      fprintf(stderr, "ERROR: division with wide values not currently "
              "supported in V0.8.\n");
      assert(0);
}


void vvp_arith_div::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      if(wid_ > 8*sizeof(unsigned long)) {
	    wide(base, push);
	    return;
      }

      unsigned long a = 0, b = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0xaa) {
		  output_x_(base, push);
		  return;
	    }

	    if (val & 0x01)
		  a += 1UL << idx;
	    if (val & 0x04)
		  b += 1UL << idx;
      }

      unsigned sign_flip = 0;
      if (signed_flag_) {
	    unsigned long sign_mask = 0;
	    if (wid_ != 8*sizeof(unsigned long)) {
		  sign_mask = -1UL << wid_;
	    }
	    if (a & (1UL << (wid_ - 1))) {
		  a ^=  ~sign_mask;
		  a += 1;
		  sign_flip += 1;
	    }

	    if (b & (1UL << (wid_ - 1))) {
		  b ^= ~sign_mask;
		  b += 1;
		  sign_flip += 1;
	    }

      }

      if (b == 0) {
	    output_x_(base, push);
	    return;
      }

      unsigned long result = a / b;
      if (sign_flip % 2 == 1)
	    result = 0 - result;

      output_val_(base, push, result);
}

inline void vvp_arith_mod::wide(vvp_ipoint_t base, bool push)
{
      fprintf(stderr, "ERROR: modulus with wide values not currently "
              "supported in V0.8.\n");
      assert(0);
}

void vvp_arith_mod::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      if(wid_ > 8*sizeof(unsigned long)) {
	    wide(base, push);
	    return;
      }

      unsigned long a = 0, b = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0xaa) {
		  output_x_(base, push);
		  return;
	    }

	    if (val & 0x01)
		  a += 1UL << idx;
	    if (val & 0x04)
		  b += 1UL << idx;
      }

      if (b == 0) {
	    output_x_(base, push);
	    return;
      }

      output_val_(base, push, a%b);
}

// Multiplication

void vvp_arith_mult::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      if(wid_ > 8*sizeof(unsigned long)) {
	    wide(base, push);
	    return;
      }

      unsigned long a = 0, b = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0xaa) {
		  output_x_(base, push);
		  return;
	    }

	    if (val & 0x01)
		  a += 1UL << idx;
	    if (val & 0x04)
		  b += 1UL << idx;
      }

      output_val_(base, push, a*b);
}

void vvp_arith_mult::wide(vvp_ipoint_t base, bool push)
{
      unsigned char *a, *b, *sum;
      a = new unsigned char[wid_];
      b = new unsigned char[wid_];
      sum = new unsigned char[wid_];

      unsigned mxa = 0;
      unsigned mxb = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base, idx);
	    functor_t obj = functor_index(ptr);

	    unsigned ival = obj->ival;
	    if (ival & 0xaa) {
		  output_x_(base, push);
		  delete[]sum;
		  delete[]b;
		  delete[]a;
		  return;
	    }

	    if((a[idx] = ((ival & 0x01) != 0))) mxa=idx+1;
	    if((b[idx] = ((ival & 0x04) != 0))) mxb=idx;
            sum[idx] = 0;
      }

	/* do the a*b multiply using the long method we learned in
	   grade school. We know at this point that there are no X or
	   Z values in the a or b vectors. */

      for(unsigned i=0 ;  i<=mxb ;  i += 1) {
	    if(b[i]) {
		  unsigned char carry=0;
		  unsigned char temp;

		  for(unsigned j=0 ;  j<=mxa ;  j += 1) {

			if((i+j) >= wid_)
			      break;

			temp=sum[i+j] + a[j] + carry;
			sum[i+j]=(temp&1);
			carry=(temp>>1);
		  }
	    }
      }

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = sum[idx];

	    obj->put_oval(val, push);
      }

      delete[]sum;
      delete[]b;
      delete[]a;
}


// Addition

void vvp_arith_sum::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      unsigned page = 0;
      unsigned pbit = 0;
      unsigned long carry = 0;

      sum_[0] = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base, idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0xaa) {
		  output_x_(base, push);
		  return;
	    }

	      // Accumulate the sum of the input bits.
	    unsigned long tmp = 0;
	    if (val & 0x01)
		  tmp += 1;
	    if (val & 0x04)
		  tmp += 1;
	    if (val & 0x10)
		  tmp += 1;
	    if (val & 0x40)
		  tmp += 1;

	    // Save carry bits
	    if (pbit >= pagesize - 2)
		  carry += (tmp + (sum_[page]>>pbit)) >> (pagesize-pbit);

	    // Put the next bits into the sum,
	    sum_[page] += tmp << pbit;

	    pbit += 1;
	    if (pbit >= pagesize) {
		  pbit = 0;
		  page += 1;
		  if (page < pagecount_)
			sum_[page] = carry;
		  carry = 0;
	    }
      }

      output_val_(base, push);
}


/*
 * Subtraction works by adding the 2s complement of the B, C and D
 * inputs from the A input. The 2s complement is the 1s complement
 * plus one, so we further reduce the operation to adding in the
 * inverted value and adding a correction.
 */
void vvp_arith_sub::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      unsigned page = 0;
      unsigned pbit = 0;
      unsigned long carry = 0;

	/* There are 3 values subtracted from the first parameter, so
	   there are three 2s complements, so three ~X +1. That's why
	   the sum_ starts with 3. */
      sum_[0] = 3;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base, idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0xaa) {
		  output_x_(base, push);
		  return;
	    }

	      // Accumulate the sum of the input bits. Add in the
	      // first value, and the ones complement of the other values.
	    unsigned long tmp = 0;
	    if (val & 0x01)
		  tmp += 1;
	    if (! (val & 0x04))
		  tmp += 1;
	    if (! (val & 0x10))
		  tmp += 1;
	    if (! (val & 0x40))
		  tmp += 1;

	    // Save carry bits
	    if (pbit >= pagesize - 2)
		  carry += (tmp + (sum_[page]>>pbit)) >> (pagesize-pbit);

	    // Put the next bits into the sum,
	    sum_[page] += tmp << pbit;

	    pbit += 1;
	    if (pbit >= pagesize) {
		  pbit = 0;
		  page += 1;
		  if (page < pagecount_)
			sum_[page] = carry;
		  carry = 0;
	    }
      }

      output_val_(base, push);
}

vvp_cmp_eq::vvp_cmp_eq(unsigned wid)
: vvp_arith_(wid)
{
}

void vvp_cmp_eq::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      unsigned out_val = 1;

      for (unsigned idx = wid_ ;  idx > 0 ;  idx -= 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx-1);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0x0a) {
		  out_val = 2;
		  break;
	    }

	    unsigned a = (val & 0x01)? 1 : 0;
	    unsigned b = (val & 0x04)? 1 : 0;

	    if (a != b) {
		  out_val = 0;
		  break;
	    }
      }

      put_oval(out_val, push);
}

vvp_cmp_ne::vvp_cmp_ne(unsigned wid)
: vvp_arith_(wid)
{
}

void vvp_cmp_ne::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      unsigned out_val = 0;

      for (unsigned idx = wid_ ;  idx > 0 ;  idx -= 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx-1);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0x0a) {
		  out_val = 2;
		  break;
	    }

	    unsigned a = (val & 0x01)? 1 : 0;
	    unsigned b = (val & 0x04)? 1 : 0;

	    if (a != b) {
		  out_val = 1;
		  break;
	    }
      }

      put_oval(out_val, push);
}


vvp_cmp_gtge_base_::vvp_cmp_gtge_base_(unsigned wid, bool flag)
: vvp_arith_(wid), signed_flag_(flag)
{
}


void vvp_cmp_gtge_base_::set_base(vvp_ipoint_t i,
				 bool push,
				 unsigned val,
				 unsigned,
				 unsigned out_if_equal)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      unsigned out_val = out_if_equal;

      unsigned idx = wid_;

	/* If this is a signed compare, then check the MSB of the
	   input vectors. If they are different, then the values are
	   on the different sides of zero, and we know the result. */
      if (signed_flag_) {
	    vvp_ipoint_t ptr = ipoint_index(base, wid_-1);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0x0a) {
		  out_val = 2;
		  goto check_for_x_complete;
	    }

	    unsigned a = (val & 0x01)? 1 : 0;
	    unsigned b = (val & 0x04)? 1 : 0;

	      /* If a==0 and b==1, then a>=0 and b<0 so return true.
		 If a==1 and b==0, then a<0 and b>=0 so return false.
		 It turns out that out_val=b gets the right result. */
	    if (a ^ b) {
		  out_val = b;
		  idx = wid_-1;
		  goto check_for_x;
	    }
      }

      for (idx = wid_ ;  idx > 0 ;  idx -= 1) {
	    vvp_ipoint_t ptr = ipoint_index(base, idx-1);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0x0a) {
		  out_val = 2;
		  goto check_for_x_complete;
	    }

	    unsigned a = (val & 0x01)? 1 : 0;
	    unsigned b = (val & 0x04)? 1 : 0;

	    if (a > b) {
		  out_val = 1;
		  break;
	    }

	    if (a < b) {
		  out_val = 0;
		  break;
	    }
      }

 check_for_x:
	/* Continue further checking bits, looking for unknown
	   results. */
      while ((idx > 0) && (out_val != 2)) {
	    vvp_ipoint_t ptr = ipoint_index(base, idx-1);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0x0a) {
		  out_val = 2;
		  break;
	    }

	    idx -= 1;
      }

 check_for_x_complete:

      put_oval(out_val, push);
}



vvp_cmp_ge::vvp_cmp_ge(unsigned wid, bool flag)
: vvp_cmp_gtge_base_(wid, flag)
{
}



void vvp_cmp_ge::set(vvp_ipoint_t i, bool push, unsigned val, unsigned str)
{
      set_base(i, push, val, str, 1);
}

vvp_cmp_gt::vvp_cmp_gt(unsigned wid, bool flag)
: vvp_cmp_gtge_base_(wid, flag)
{
}

void vvp_cmp_gt::set(vvp_ipoint_t i, bool push, unsigned val, unsigned str)
{
      set_base(i, push, val, str, 0);
}

void vvp_shiftl::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      unsigned amount = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base, idx);
	    functor_t fp = functor_index(ptr);

	    unsigned val = (fp->ival >> 2) & 0x03;
	    switch (val) {
		case 0:
		  break;
		case 1:
		  amount |= 1 << idx;
		  break;
		default:
		  output_x_(base, push);
		  return;
	    }
      }

      if (amount >= wid_) {
	    output_x_(base, push, 0);
	    return;

      } else {
	    vvp_ipoint_t optr, iptr;
	    functor_t ofp, ifp;

	    for (unsigned idx = 0 ;  idx < amount ;  idx += 1) {
		  optr = ipoint_index(base, idx);
		  ofp = functor_index(optr);
		  ofp->put_oval(0, push);
	    }

	    for (unsigned idx = amount ;  idx < wid_ ;  idx += 1) {
		  optr = ipoint_index(base, idx);
		  ofp = functor_index(optr);
		  iptr = ipoint_index(base, idx - amount);
		  ifp = functor_index(iptr);

		  ofp->put_oval(ifp->ival & 3, push);
	    }
      }
}

void vvp_shiftr::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      unsigned amount = 0;

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base, idx);
	    functor_t fp = functor_index(ptr);

	    unsigned val = (fp->ival >> 2) & 0x03;
	    switch (val) {
		case 0:
		  break;
		case 1:
		  amount |= 1 << idx;
		  break;
		default:
		  output_x_(base, push);
		  return;
	    }
      }

      if (amount >= wid_) {
	    output_x_(base, push, 0);
	    return;

      } else {
	    vvp_ipoint_t optr, iptr;
	    functor_t ofp, ifp;

	    for (unsigned idx = 0 ;  idx < (wid_-amount) ;  idx += 1) {
		  optr = ipoint_index(base, idx);
		  ofp = functor_index(optr);
		  iptr = ipoint_index(base, idx + amount);
		  ifp = functor_index(iptr);

		  ofp->put_oval(ifp->ival & 3, push);
	    }

	    for (unsigned idx = wid_-amount; idx < wid_ ;  idx += 1) {
		  optr = ipoint_index(base, idx);
		  ofp = functor_index(optr);
		  ofp->put_oval(0, push);
	    }
      }
}


/*
 * $Log: arith.cc,v $
 * Revision 1.30  2004/10/04 01:10:58  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.29  2004/09/22 16:44:07  steve
 *  Fix LPM GE to match LPM GT behavior.
 *
 * Revision 1.28  2004/06/30 02:15:57  steve
 *  Add signed LPM divide.
 *
 * Revision 1.27  2004/06/16 16:33:26  steve
 *  Add structural equality compare nodes.
 *
 * Revision 1.26  2003/08/01 00:58:34  steve
 *  Fix arithmetic operators in 64bit processors.
 *
 * Revision 1.25  2003/04/11 05:15:38  steve
 *  Add signed versions of .cmp/gt/ge
 *
 * Revision 1.24  2002/08/12 01:35:07  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.23  2002/05/07 04:15:43  steve
 *  Fix uninitialized memory accesses.
 *
 * Revision 1.22  2002/01/03 04:19:02  steve
 *  Add structural modulus support down to vvp.
 *
 * Revision 1.21  2001/12/06 03:31:24  steve
 *  Support functor delays for gates and UDP devices.
 *  (Stephan Boettcher)
 *
 * Revision 1.20  2001/11/07 03:34:41  steve
 *  Use functor pointers where vvp_ipoint_t is unneeded.
 *
 * Revision 1.19  2001/11/04 05:03:21  steve
 *  MacOSX 10.1 updates.
 *
 * Revision 1.18  2001/10/31 04:27:46  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.17  2001/10/27 03:22:26  steve
 *  Minor rework of summation carry propagation (Stephan Boettcher)
 *
 * Revision 1.16  2001/10/16 03:10:20  steve
 *  Get Division error into the division method!
 *
 * Revision 1.15  2001/10/16 03:06:18  steve
 *  Catch division by zero in .arith/div.
 *
 * Revision 1.14  2001/10/16 02:47:37  steve
 *  Add arith/div object.
 *
 * Revision 1.13  2001/10/14 17:36:18  steve
 *  Forgot to propagate carry.
 *
 * Revision 1.12  2001/10/14 16:36:43  steve
 *  Very wide multiplication (Anthony Bybell)
 *
 * Revision 1.11  2001/07/13 00:38:57  steve
 *  Remove width restriction on subtraction.
 *
 * Revision 1.10  2001/07/11 02:27:21  steve
 *  Add support for REadOnlySync and monitors.
 *
 * Revision 1.9  2001/07/07 02:57:33  steve
 *  Add the .shift/r functor.
 *
 * Revision 1.8  2001/07/06 04:46:44  steve
 *  Add structural left shift (.shift/l)
 *
 * Revision 1.7  2001/06/29 01:21:48  steve
 *  Relax limit on width of structural sum.
 *
 * Revision 1.6  2001/06/29 01:20:20  steve
 *  Relax limit on width of structural sum.
 *
 * Revision 1.5  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 */

