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
#ident "$Id: arith.cc,v 1.31 2004/12/11 02:31:29 steve Exp $"
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

vvp_arith_::vvp_arith_(unsigned wid)
: wid_(wid), x_val_(wid)
{
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
	    x_val_.set_bit(idx, BIT4_X);
}


// Division

inline void vvp_arith_div::wide(vvp_ipoint_t base, bool push)
{
      assert(0);
}


void vvp_arith_div::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
#if 0
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
	    if (a & (1UL << (wid_ - 1))) {
		  a ^=  ~(-1UL << wid_);
		  a += 1;
		  sign_flip += 1;
	    }

	    if (b & (1UL << (wid_ - 1))) {
		  b ^= ~(-1UL << wid_);
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
#else
      fprintf(stderr, "XXXX forgot how to implement vvp_arith_div::set\n");
#endif
}

inline void vvp_arith_mod::wide(vvp_ipoint_t base, bool push)
{
      assert(0);
}

void vvp_arith_mod::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
#if 0
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
#else
      fprintf(stderr, "XXXX forgot how to implement vvp_arith_mod::set\n");
#endif
}

// Multiplication

void vvp_arith_mult::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
#if 0
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
#else
      fprintf(stderr, "XXXX forgot how to implement vvp_arith_mult::set\n");
#endif
}

#if 0
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
#endif

// Addition

vvp_arith_sum::vvp_arith_sum(unsigned wid)
: vvp_arith_(wid)
{
}

vvp_arith_sum::~vvp_arith_sum()
{
}

void vvp_arith_sum::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      unsigned port = ptr.port();

      switch (port) {
	  case 0:
	    op_a_ = bit;
	    break;
	  case 1:
	    op_b_ = bit;
	    break;
	  default:
	    assert(0);
      }

      vvp_net_t*net = ptr.ptr();

      vvp_vector4_t value (wid_);

	/* Pad input vectors with this value to widen to the desired
	   output width. */
      const vvp_bit4_t pad = BIT4_0;

      vvp_bit4_t carry = BIT4_0;
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_bit4_t a = (idx >= op_a_.size())? pad : op_a_.value(idx);
	    vvp_bit4_t b = (idx >= op_b_.size())? pad : op_b_.value(idx);
	    vvp_bit4_t cur = add_with_carry(a, b, carry);

	    if (cur == BIT4_X) {
		  vvp_send_vec4(net->out, x_val_);
		  return;
	    }

	    value.set_bit(idx, cur);
      }

      vvp_send_vec4(net->out, value);
}

vvp_arith_sub::vvp_arith_sub(unsigned wid)
: vvp_arith_(wid)
{
}

vvp_arith_sub::~vvp_arith_sub()
{
}

/*
 * Subtraction works by adding the 2s complement of the B, C and D
 * inputs from the A input. The 2s complement is the 1s complement
 * plus one, so we further reduce the operation to adding in the
 * inverted value and adding a correction.
 */
void vvp_arith_sub::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      unsigned port = ptr.port();

      switch (port) {
	  case 0:
	    op_a_ = bit;
	    break;
	  case 1:
	    op_b_ = bit;
	    break;
	  default:
	    assert(0);
      }

      vvp_net_t*net = ptr.ptr();

      vvp_vector4_t value (wid_);

	/* Pad input vectors with this value to widen to the desired
	   output width. */
      const vvp_bit4_t pad = BIT4_1;

      vvp_bit4_t carry = BIT4_1;
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_bit4_t a = (idx >= op_a_.size())? pad : op_a_.value(idx);
	    vvp_bit4_t b = (idx >= op_b_.size())? pad : op_b_.value(idx);
	    vvp_bit4_t cur = add_with_carry(a, b, carry);

	    if (cur == BIT4_X) {
		  vvp_send_vec4(net->out, x_val_);
		  return;
	    }

	    value.set_bit(idx, cur);
      }

      vvp_send_vec4(net->out, value);
}



vvp_cmp_eq::vvp_cmp_eq(unsigned wid)
: vvp_arith_(wid)
{
}

#if 0
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
#endif

vvp_cmp_ne::vvp_cmp_ne(unsigned wid)
: vvp_arith_(wid)
{
}

#if 0
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
#endif

vvp_cmp_gtge_base_::vvp_cmp_gtge_base_(unsigned wid, bool flag)
: vvp_arith_(wid), signed_flag_(flag)
{
}

#if 0
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
#endif


vvp_cmp_ge::vvp_cmp_ge(unsigned wid, bool flag)
: vvp_cmp_gtge_base_(wid, flag)
{
}


#if 0
void vvp_cmp_ge::set(vvp_ipoint_t i, bool push, unsigned val, unsigned str)
{
      set_base(i, push, val, str, 1);
}
#endif

vvp_cmp_gt::vvp_cmp_gt(unsigned wid, bool flag)
: vvp_cmp_gtge_base_(wid, flag)
{
}

#if 0
void vvp_cmp_gt::set(vvp_ipoint_t i, bool push, unsigned val, unsigned str)
{
      set_base(i, push, val, str, 0);
}
#endif

#if 0
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
#endif

#if 0
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
#endif

/*
 * $Log: arith.cc,v $
 * Revision 1.31  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */

