/*
 * Copyright (c) 2001-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: arith.cc,v 1.37 2005/01/30 05:06:49 steve Exp $"
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

      op_a_ = x_val_;
      op_b_ = x_val_;
}

void vvp_arith_::dispatch_operand_(vvp_net_ptr_t ptr, vvp_vector4_t bit)
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

vvp_arith_mult::vvp_arith_mult(unsigned wid)
: vvp_arith_(wid)
{
}

vvp_arith_mult::~vvp_arith_mult()
{
}

void vvp_arith_mult::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      dispatch_operand_(ptr, bit);

      unsigned long a;
      if (! vector4_to_value(op_a_, a)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_);
	    return;
      }

      unsigned long b;
      if (! vector4_to_value(op_b_, b)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_);
	    return;
      }

      unsigned long val = a * b;
      assert(wid_ <= 8*sizeof(val));

      vvp_vector4_t vval (wid_);
      for (int idx = 0 ;  idx < wid_ ;  idx += 1) {
	    if (val & 1)
		  vval.set_bit(idx, BIT4_1);
	    else
		  vval.set_bit(idx, BIT4_0);

	    val >>= 1;
      }

      vvp_send_vec4(ptr.ptr()->out, vval);
}


#if 0
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
#endif

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
      dispatch_operand_(ptr, bit);

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
 * Subtraction works by adding the 2s complement of the B input from
 * the A input. The 2s complement is the 1s complement plus one, so we
 * further reduce the operation to adding in the inverted value and
 * adding a correction.
 */
void vvp_arith_sub::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      dispatch_operand_(ptr, bit);

      vvp_net_t*net = ptr.ptr();

      vvp_vector4_t value (wid_);

	/* Pad input vectors with this value to widen to the desired
	   output width. */
      const vvp_bit4_t pad = BIT4_1;

      vvp_bit4_t carry = BIT4_1;
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    vvp_bit4_t a = (idx >= op_a_.size())? pad : op_a_.value(idx);
	    vvp_bit4_t b = (idx >= op_b_.size())? pad : ~op_b_.value(idx);
	    vvp_bit4_t cur = add_with_carry(a, b, carry);

	    if (cur == BIT4_X) {
		  vvp_send_vec4(net->out, x_val_);
		  return;
	    }

	    value.set_bit(idx, cur);
      }

      vvp_send_vec4(net->out, value);
}

vvp_cmp_eeq::vvp_cmp_eeq(unsigned wid)
: vvp_arith_(wid)
{
}

void vvp_cmp_eeq::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t eeq (1);
      eeq.set_bit(0, BIT4_1);

      assert(op_a_.size() == op_b_.size());
      for (unsigned idx = 0 ;  idx < op_a_.size() ;  idx += 1)
	    if (op_a_.value(idx) != op_b_.value(idx)) {
		  eeq.set_bit(0, BIT4_0);
		  break;
	    }

      vvp_net_t*net = ptr.ptr();
      vvp_send_vec4(net->out, eeq);
}

vvp_cmp_eq::vvp_cmp_eq(unsigned wid)
: vvp_arith_(wid)
{
}

/*
 * Compare Vector a and Vector b. If in any bit position the a and b
 * bits are known and different, then the result is 0. Otherwise, if
 * there are X/Z bits anywhere in A or B, the result is X. Finally,
 * the result is 1.
 */
void vvp_cmp_eq::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      dispatch_operand_(ptr, bit);

      assert(op_a_.size() == op_b_.size());

      vvp_vector4_t res (1);
      res.set_bit(0, BIT4_1);

      for (unsigned idx = 0 ;  idx < op_a_.size() ;  idx += 1) {
	    vvp_bit4_t a = op_a_.value(idx);
	    vvp_bit4_t b = op_b_.value(idx);

	    if (a == BIT4_X)
		  res.set_bit(0, BIT4_X);
	    else if (a == BIT4_Z)
		  res.set_bit(0, BIT4_X);
	    else if (b == BIT4_X)
		  res.set_bit(0, BIT4_X);
	    else if (b == BIT4_Z)
		  res.set_bit(0, BIT4_X);
            else if (a != b) {
		  res.set_bit(0, BIT4_0);
		  break;
	    }
      }

      vvp_net_t*net = ptr.ptr();
      vvp_send_vec4(net->out, res);
}


vvp_cmp_ne::vvp_cmp_ne(unsigned wid)
: vvp_arith_(wid)
{
}

/*
 * Compare Vector a and Vector b. If in any bit position the a and b
 * bits are known and different, then the result is 1. Otherwise, if
 * there are X/Z bits anywhere in A or B, the result is X. Finally,
 * the result is 0.
 */
void vvp_cmp_ne::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      dispatch_operand_(ptr, bit);

      assert(op_a_.size() == op_b_.size());

      vvp_vector4_t res (1);
      res.set_bit(0, BIT4_0);

      for (unsigned idx = 0 ;  idx < op_a_.size() ;  idx += 1) {
	    vvp_bit4_t a = op_a_.value(idx);
	    vvp_bit4_t b = op_b_.value(idx);

	    if (a == BIT4_X)
		  res.set_bit(0, BIT4_X);
	    else if (a == BIT4_Z)
		  res.set_bit(0, BIT4_X);
	    else if (b == BIT4_X)
		  res.set_bit(0, BIT4_X);
	    else if (b == BIT4_Z)
		  res.set_bit(0, BIT4_X);
            else if (a != b) {
		  res.set_bit(0, BIT4_1);
		  break;
	    }
      }

      vvp_net_t*net = ptr.ptr();
      vvp_send_vec4(net->out, res);
}


vvp_cmp_gtge_base_::vvp_cmp_gtge_base_(unsigned wid, bool flag)
: vvp_arith_(wid), signed_flag_(flag)
{
}


void vvp_cmp_gtge_base_::recv_vec4_base_(vvp_net_ptr_t ptr,
					 vvp_vector4_t bit,
					 vvp_bit4_t out_if_equal)
{
      dispatch_operand_(ptr, bit);

      vvp_bit4_t out = signed_flag_
	    ? compare_gtge_signed(op_a_, op_b_, out_if_equal)
	    : compare_gtge(op_a_, op_b_, out_if_equal);
      vvp_vector4_t val (1);
      val.set_bit(0, out);
      vvp_send_vec4(ptr.ptr()->out, val);

      return;
}


vvp_cmp_ge::vvp_cmp_ge(unsigned wid, bool flag)
: vvp_cmp_gtge_base_(wid, flag)
{
}

void vvp_cmp_ge::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      recv_vec4_base_(ptr, bit, BIT4_1);
}

vvp_cmp_gt::vvp_cmp_gt(unsigned wid, bool flag)
: vvp_cmp_gtge_base_(wid, flag)
{
}

void vvp_cmp_gt::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      recv_vec4_base_(ptr, bit, BIT4_0);
}


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
 * Revision 1.37  2005/01/30 05:06:49  steve
 *  Get .arith/sub working.
 *
 * Revision 1.36  2005/01/28 05:34:25  steve
 *  Add vector4 implementation of .arith/mult.
 *
 * Revision 1.35  2005/01/22 17:36:15  steve
 *  .cmp/x supports signed magnitude compare.
 *
 * Revision 1.34  2005/01/22 16:21:11  steve
 *  Implement vectored CMP_EQ and NE
 *
 * Revision 1.33  2005/01/22 01:06:20  steve
 *  Implement the .cmp/eeq LPM node.
 *
 * Revision 1.32  2005/01/16 04:19:08  steve
 *  Reimplement comparators as vvp_vector4_t nodes.
 *
 * Revision 1.31  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */

