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
#ident "$Id: arith.cc,v 1.18 2001/10/31 04:27:46 steve Exp $"
#endif

# include  "arith.h"
# include  "schedule.h"
# include  <limits.h>
# include  <stdio.h>
# include  <assert.h>
# include  <malloc.h>

void vvp_arith_::output_x_(vvp_ipoint_t base, bool push, unsigned val)
{
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);
	    
	    obj->put_oval(ptr, push, val);
      }
}

void vvp_arith_::output_val_(vvp_ipoint_t base, bool push, unsigned long sum)
{
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);
	    
	    unsigned val = sum & 1;
	    sum >>= 1;
	    
	    obj->put_oval(ptr, push, val);
      }
}

// Make sure the static sum_ scratch space is large enough for everybody

vvp_wide_arith_::vvp_wide_arith_(unsigned wid)
      : vvp_arith_(wid)
{
      unsigned np = (wid + pagesize - 1)/pagesize;
      sum_ = (unsigned long *)malloc(np*sizeof(unsigned long));
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

	    obj->put_oval(ptr, push, val);
      }
}


// Division

inline void vvp_arith_div::wide(vvp_ipoint_t base, bool push)
{
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
		  a += 1 << idx;
	    if (val & 0x04)
		  b += 1 << idx;
      }

      if (b == 0) {
	    output_x_(base, push);
	    return;
      }

      output_val_(base, push, a/b);
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
		  a += 1 << idx;
	    if (val & 0x04)
		  b += 1 << idx;
      }

      output_val_(base, push, a*b);
}

void vvp_arith_mult::wide(vvp_ipoint_t base, bool push)
{
      unsigned char *a, *b, *sum;
      a = new unsigned char[wid_];
      b = new unsigned char[wid_];
      sum = new unsigned char[wid_];

      int mxa = -1;
      int mxb = -1;

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

//    do "unsigned ZZ sum = a * b" the hard way..
      for(int i=0;i<=mxb;i++)
		{
		if(b[i])
			{
			unsigned char carry=0;
			unsigned char temp;			

			for(int j=0;j<=mxa;j++)
				{
				if(i+j>=(int)wid_) break;
				temp=sum[i+j]+a[j]+carry;
				sum[i+j]=(temp&1);
				carry=(temp>>1);
				}
			}
		}

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(base,idx);
	    functor_t obj = functor_index(ptr);

	    unsigned val = sum[idx];

	    obj->put_oval(ptr, push, val);
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
		  sum_[page] = carry;
		  carry = 0;
	    }
      }

      output_val_(base, push);
}


void vvp_cmp_ge::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
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

	    if (a > b) {
		  out_val = 1;
		  break;
	    }

	    if (a < b) {
		  out_val = 0;
		  break;
	    }
      }

      put_oval(base, push, out_val);
}

void vvp_cmp_gt::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
      put(i, val);
      vvp_ipoint_t base = ipoint_make(i,0);

      unsigned out_val = 0;
      
      for (unsigned idx = wid_ ;  idx > 0 ;  idx -= 1) {
	    vvp_ipoint_t ptr = ipoint_index(base, idx-1);
	    functor_t obj = functor_index(ptr);

	    unsigned val = obj->ival;
	    if (val & 0x0a) {
		  out_val = 2;
		  break;
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

      put_oval(base, push, out_val);
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
		  ofp->put_oval(optr, push, 0);
	    }

	    for (unsigned idx = amount ;  idx < wid_ ;  idx += 1) {
		  optr = ipoint_index(base, idx);
		  ofp = functor_index(optr);
		  iptr = ipoint_index(base, idx - amount);
		  ifp = functor_index(iptr);

		  ofp->put_oval(optr, push, ifp->ival & 3);
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

		  ofp->put_oval(optr, push, ifp->ival & 3);
	    }

	    for (unsigned idx = wid_-amount; idx < wid_ ;  idx += 1) {
		  optr = ipoint_index(base, idx);
		  ofp = functor_index(optr);
		  ofp->put_oval(optr, push, 0);
	    }
      }
}


/*
 * $Log: arith.cc,v $
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

