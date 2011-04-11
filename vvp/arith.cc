/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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

# include  "arith.h"
# include  "schedule.h"
# include  <climits>
# include  <iostream>
# include  <cassert>
# include  <cstdlib>
# include  <cmath>

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
	    fprintf(stderr, "Unsupported port type %d.\n", port);
	    assert(0);
      }
}


vvp_arith_abs::vvp_arith_abs()
{
}

vvp_arith_abs::~vvp_arith_abs()
{
}

void vvp_arith_abs::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                              vvp_context_t)
{
      vvp_vector4_t out (bit.size(), BIT4_0);;

      vvp_bit4_t cmp = compare_gtge_signed(bit, out, BIT4_1);
      switch (cmp) {
	  case BIT4_1: // bit >= 0
	    out = bit;
	    break;
	  case BIT4_0: //  bit < 0
	    out = ~bit;
	    out += 1;
	    break;
	  default: // There's an X.
	    out = vvp_vector4_t(bit.size(), BIT4_X);
	    break;
      }

      vvp_send_vec4(ptr.ptr()->out, out, 0);
}

void vvp_arith_abs::recv_real(vvp_net_ptr_t ptr, double bit,
                              vvp_context_t)
{
      double out = fabs(bit);
      vvp_send_real(ptr.ptr()->out, out, 0);
}

vvp_arith_cast_int::vvp_arith_cast_int(unsigned wid)
: wid_(wid)
{
}

vvp_arith_cast_int::~vvp_arith_cast_int()
{
}

void vvp_arith_cast_int::recv_real(vvp_net_ptr_t ptr, double bit,
                                   vvp_context_t)
{
      vvp_send_vec4(ptr.ptr()->out, vvp_vector4_t(wid_, bit), 0);
}

vvp_arith_cast_real::vvp_arith_cast_real(bool signed_flag)
: signed_(signed_flag)
{
}

vvp_arith_cast_real::~vvp_arith_cast_real()
{
}

void vvp_arith_cast_real::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                                    vvp_context_t)
{
      double val;
      vector4_to_value(bit, val, signed_);
      vvp_send_real(ptr.ptr()->out, val, 0);
}

// Division

vvp_arith_div::vvp_arith_div(unsigned wid, bool signed_flag)
: vvp_arith_(wid), signed_flag_(signed_flag)
{
}

vvp_arith_div::~vvp_arith_div()
{
}

void vvp_arith_div::wide4_(vvp_net_ptr_t ptr)
{
      vvp_vector2_t a2 (op_a_);
      if (a2.is_NaN()) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      vvp_vector2_t b2 (op_b_);
      if (b2.is_NaN() || b2.is_zero()) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      bool negate = false;
      if (signed_flag_) {
	    if (a2.value(a2.size()-1)) {
		  a2 = -a2;
		  negate = true;
            }
	    if (b2.value(b2.size()-1)) {
		  b2 = -b2;
		  negate = !negate;
            }
      }
      vvp_vector2_t res = a2 / b2;
      if (negate) res = -res;
      vvp_send_vec4(ptr.ptr()->out, vector2_to_vector4(res, wid_), 0);
}

void vvp_arith_div::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                              vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      if (wid_ > 8 * sizeof(unsigned long)) {
	    wide4_(ptr);
	    return ;
      }

      unsigned long a;
      if (! vector4_to_value(op_a_, a)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      unsigned long b;
      if (! vector4_to_value(op_b_, b)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      bool negate = false;
	/* If we are doing signed divide, then take the sign out of
	   the operands for now, and remember to put the sign back
	   later. */
      if (signed_flag_) {
	    unsigned long sign_mask = 0;
	    if (op_a_.size() != 8 * sizeof(unsigned long)) {
		  sign_mask = -1UL << op_a_.size();
	    }
	    if (op_a_.value(op_a_.size()-1)) {
		  a = (-a) & ~sign_mask;
		  negate = !negate;
	    }

	    sign_mask = 0;
	    if (op_b_.size() != 8 * sizeof(unsigned long)) {
		  sign_mask = -1UL << op_b_.size();
	    }
	    if (op_b_.value(op_b_.size()-1)) {
		  b = (-b) & ~sign_mask;
		  negate = ! negate;
	    }
      }

      if (b == 0) {
	    vvp_vector4_t xval (wid_);
	    for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1)
		  xval.set_bit(idx, BIT4_X);

	    vvp_send_vec4(ptr.ptr()->out, xval, 0);
	    return;
      }

      unsigned long val = a / b;
      if (negate)
	    val = -val;

      assert(wid_ <= 8*sizeof(val));

      vvp_vector4_t vval (wid_);
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    if (val & 1)
		  vval.set_bit(idx, BIT4_1);
	    else
		  vval.set_bit(idx, BIT4_0);

	    val >>= 1;
      }

      vvp_send_vec4(ptr.ptr()->out, vval, 0);
}


vvp_arith_mod::vvp_arith_mod(unsigned wid, bool sf)
: vvp_arith_(wid), signed_flag_(sf)
{
}

vvp_arith_mod::~vvp_arith_mod()
{
}

void vvp_arith_mod::wide_(vvp_net_ptr_t ptr)
{
      vvp_vector2_t a2 (op_a_);
      if (a2.is_NaN()) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      vvp_vector2_t b2 (op_b_);
      if (b2.is_NaN() || b2.is_zero()) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      bool negate = false;
      if (signed_flag_) {
	    if (a2.value(a2.size()-1)) {
		  a2 = -a2;
		  negate = true;
            }
	    if (b2.value(b2.size()-1)) {
		  b2 = -b2;
            }
      }
      vvp_vector2_t res = a2 % b2;
      if (negate) res = -res;
      vvp_send_vec4(ptr.ptr()->out, vector2_to_vector4(res, res.size()), 0);
}

void vvp_arith_mod::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                              vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      if (wid_ > 8 * sizeof(unsigned long)) {
	    wide_(ptr);
	    return ;
      }

      unsigned long a;
      if (! vector4_to_value(op_a_, a)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      unsigned long b;
      if (! vector4_to_value(op_b_, b)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      bool negate = false;
	/* If we are doing signed divide, then take the sign out of
	   the operands for now, and remember to put the sign back
	   later. */
      if (signed_flag_) {
	    unsigned long sign_mask = 0;
	    if (op_a_.size() != 8 * sizeof(unsigned long)) {
		  sign_mask = -1UL << op_a_.size();
	    }
	    if (op_a_.value(op_a_.size()-1)) {
		  a = (-a) & ~sign_mask;
		  negate = !negate;
	    }

	    sign_mask = 0;
	    if (op_b_.size() != 8 * sizeof(unsigned long)) {
		  sign_mask = -1UL << op_b_.size();
	    }
	    if (op_b_.value(op_b_.size()-1)) {
		  b = (-b) & ~sign_mask;
	    }
      }

      if (b == 0) {
	    vvp_vector4_t xval (wid_);
	    for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1)
		  xval.set_bit(idx, BIT4_X);

	    vvp_send_vec4(ptr.ptr()->out, xval, 0);
	    return;
      }

      unsigned long val = a % b;
      if (negate)
	    val = -val;

      assert(wid_ <= 8*sizeof(val));

      vvp_vector4_t vval (wid_);
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    if (val & 1)
		  vval.set_bit(idx, BIT4_1);
	    else
		  vval.set_bit(idx, BIT4_0);

	    val >>= 1;
      }

      vvp_send_vec4(ptr.ptr()->out, vval, 0);
}


// Multiplication

vvp_arith_mult::vvp_arith_mult(unsigned wid)
: vvp_arith_(wid)
{
}

vvp_arith_mult::~vvp_arith_mult()
{
}

void vvp_arith_mult::wide_(vvp_net_ptr_t ptr)
{
      vvp_vector2_t a2 (op_a_);
      vvp_vector2_t b2 (op_b_);

      if (a2.is_NaN() || b2.is_NaN()) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      vvp_vector2_t result = a2 * b2;

      vvp_vector4_t res4 = vector2_to_vector4(result, wid_);
      vvp_send_vec4(ptr.ptr()->out, res4, 0);
}

void vvp_arith_mult::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                               vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      if (wid_ > 8 * sizeof(long)) {
	    wide_(ptr);
	    return ;
      }

      long a;
      if (! vector4_to_value(op_a_, a, false, true)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      long b;
      if (! vector4_to_value(op_b_, b, false, true)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      long val = a * b;
      assert(wid_ <= 8*sizeof(val));

      vvp_vector4_t vval (wid_);
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    if (val & 1)
		  vval.set_bit(idx, BIT4_1);
	    else
		  vval.set_bit(idx, BIT4_0);

	    val >>= 1;
      }

      vvp_send_vec4(ptr.ptr()->out, vval, 0);
}


// Power

vvp_arith_pow::vvp_arith_pow(unsigned wid, bool signed_flag)
: vvp_arith_(wid), signed_flag_(signed_flag)
{
}

vvp_arith_pow::~vvp_arith_pow()
{
}

void vvp_arith_pow::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                              vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t res4;
      if (signed_flag_) {
	    if (op_a_.has_xz() || op_b_.has_xz()) {
		  vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
		  return;
	    }

	    double ad, bd, resd;
	    vector4_to_value(op_a_, ad, true);
	    vector4_to_value(op_b_, bd, true);
	      /* 2**-1 and -2**-1 are defined to be zero. */
	    if ((bd == -1) && (fabs(ad) == 2.0)) resd = 0.0;
	    else resd = pow(ad, bd);

	    res4 = vvp_vector4_t(wid_, resd);
      } else {
	    vvp_vector2_t a2 (op_a_);
	    vvp_vector2_t b2 (op_b_);

	    if (a2.is_NaN() || b2.is_NaN()) {
		  vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
		  return;
	    }

	    vvp_vector2_t result = pow(a2, b2);
	    res4 = vector2_to_vector4(result, wid_);
      }

      vvp_send_vec4(ptr.ptr()->out, res4, 0);
}


// Addition

vvp_arith_sum::vvp_arith_sum(unsigned wid)
: vvp_arith_(wid)
{
}

vvp_arith_sum::~vvp_arith_sum()
{
}

void vvp_arith_sum::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                              vvp_context_t)
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
		  vvp_send_vec4(net->out, x_val_, 0);
		  return;
	    }

	    value.set_bit(idx, cur);
      }

      vvp_send_vec4(net->out, value, 0);
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
void vvp_arith_sub::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                              vvp_context_t)
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
		  vvp_send_vec4(net->out, x_val_, 0);
		  return;
	    }

	    value.set_bit(idx, cur);
      }

      vvp_send_vec4(net->out, value, 0);
}

vvp_cmp_eeq::vvp_cmp_eeq(unsigned wid)
: vvp_arith_(wid)
{
}

void vvp_cmp_eeq::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                            vvp_context_t)
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
      vvp_send_vec4(net->out, eeq, 0);
}

vvp_cmp_nee::vvp_cmp_nee(unsigned wid)
: vvp_arith_(wid)
{
}

void vvp_cmp_nee::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                            vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t eeq (1);
      eeq.set_bit(0, BIT4_0);

      assert(op_a_.size() == op_b_.size());
      for (unsigned idx = 0 ;  idx < op_a_.size() ;  idx += 1)
	    if (op_a_.value(idx) != op_b_.value(idx)) {
		  eeq.set_bit(0, BIT4_1);
		  break;
	    }


      vvp_net_t*net = ptr.ptr();
      vvp_send_vec4(net->out, eeq, 0);
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
void vvp_cmp_eq::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                           vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      if (op_a_.size() != op_b_.size()) {
	    cerr << "COMPARISON size mismatch. "
		 << "a=" << op_a_ << ", b=" << op_b_ << endl;
	    assert(0);
      }

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
      vvp_send_vec4(net->out, res, 0);
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
void vvp_cmp_ne::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                           vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      if (op_a_.size() != op_b_.size()) {
	    cerr << "internal error: vvp_cmp_ne: op_a_=" << op_a_
		 << ", op_b_=" << op_b_ << endl;
	    assert(op_a_.size() == op_b_.size());
      }

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
      vvp_send_vec4(net->out, res, 0);
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
      vvp_send_vec4(ptr.ptr()->out, val, 0);

      return;
}


vvp_cmp_ge::vvp_cmp_ge(unsigned wid, bool flag)
: vvp_cmp_gtge_base_(wid, flag)
{
}

void vvp_cmp_ge::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                           vvp_context_t)
{
      recv_vec4_base_(ptr, bit, BIT4_1);
}

vvp_cmp_gt::vvp_cmp_gt(unsigned wid, bool flag)
: vvp_cmp_gtge_base_(wid, flag)
{
}

void vvp_cmp_gt::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                           vvp_context_t)
{
      recv_vec4_base_(ptr, bit, BIT4_0);
}


vvp_shiftl::vvp_shiftl(unsigned wid)
: vvp_arith_(wid)
{
}

vvp_shiftl::~vvp_shiftl()
{
}

void vvp_shiftl::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                           vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t out (op_a_.size());

      unsigned long shift;
      if (! vector4_to_value(op_b_, shift)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      if (shift > out.size())
	    shift = out.size();

      for (unsigned idx = 0 ;  idx < shift ;  idx += 1)
	    out.set_bit(idx, BIT4_0);

      for (unsigned idx = shift ;  idx < out.size() ;  idx += 1)
	    out.set_bit(idx, op_a_.value(idx-shift));

      vvp_send_vec4(ptr.ptr()->out, out, 0);
}

vvp_shiftr::vvp_shiftr(unsigned wid, bool signed_flag)
: vvp_arith_(wid), signed_flag_(signed_flag)
{
}

vvp_shiftr::~vvp_shiftr()
{
}

void vvp_shiftr::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                           vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t out (op_a_.size());

      unsigned long shift;
      if (! vector4_to_value(op_b_, shift)) {
	    vvp_send_vec4(ptr.ptr()->out, x_val_, 0);
	    return;
      }

      if (shift > out.size())
	    shift = out.size();

      for (unsigned idx = shift ;  idx < out.size() ;  idx += 1)
	    out.set_bit(idx-shift, op_a_.value(idx));

      vvp_bit4_t pad = BIT4_0;
      if (signed_flag_ && op_a_.size() > 0)
	    pad = op_a_.value(op_a_.size()-1);

      for (unsigned idx = 0 ;  idx < shift ;  idx += 1)
	    out.set_bit(idx+out.size()-shift, pad);

      vvp_send_vec4(ptr.ptr()->out, out, 0);
}


vvp_arith_real_::vvp_arith_real_()
{
      op_a_ = 0.0;
      op_b_ = 0.0;
}

void vvp_arith_real_::dispatch_operand_(vvp_net_ptr_t ptr, double bit)
{
      switch (ptr.port()) {
	  case 0:
	    op_a_ = bit;
	    break;
	  case 1:
	    op_b_ = bit;
	    break;
	  default:
	    fprintf(stderr, "Unsupported port type %d.\n", ptr.port());
	    assert(0);
      }
}


/* Real multiplication. */
vvp_arith_mult_real::vvp_arith_mult_real()
{
}

vvp_arith_mult_real::~vvp_arith_mult_real()
{
}

void vvp_arith_mult_real::recv_real(vvp_net_ptr_t ptr, double bit,
                                    vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      double val = op_a_ * op_b_;
      vvp_send_real(ptr.ptr()->out, val, 0);
}

/* Real power. */
vvp_arith_pow_real::vvp_arith_pow_real()
{
}

vvp_arith_pow_real::~vvp_arith_pow_real()
{
}

void vvp_arith_pow_real::recv_real(vvp_net_ptr_t ptr, double bit,
                                   vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      double val = pow(op_a_, op_b_);
      vvp_send_real(ptr.ptr()->out, val, 0);
}

/* Real division. */
vvp_arith_div_real::vvp_arith_div_real()
{
}

vvp_arith_div_real::~vvp_arith_div_real()
{
}

void vvp_arith_div_real::recv_real(vvp_net_ptr_t ptr, double bit,
                                   vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      double val = op_a_ / op_b_;
      vvp_send_real(ptr.ptr()->out, val, 0);
}

/* Real modulus. */
vvp_arith_mod_real::vvp_arith_mod_real()
{
}

vvp_arith_mod_real::~vvp_arith_mod_real()
{
}

void vvp_arith_mod_real::recv_real(vvp_net_ptr_t ptr, double bit,
                                   vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      double val = fmod(op_a_, op_b_);
      vvp_send_real(ptr.ptr()->out, val, 0);
}

/* Real summation. */
vvp_arith_sum_real::vvp_arith_sum_real()
{
}

vvp_arith_sum_real::~vvp_arith_sum_real()
{
}

void vvp_arith_sum_real::recv_real(vvp_net_ptr_t ptr, double bit,
                                   vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      double val = op_a_ + op_b_;
      vvp_send_real(ptr.ptr()->out, val, 0);
}

/* Real subtraction. */
vvp_arith_sub_real::vvp_arith_sub_real()
{
}

vvp_arith_sub_real::~vvp_arith_sub_real()
{
}

void vvp_arith_sub_real::recv_real(vvp_net_ptr_t ptr, double bit,
                                   vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      double val = op_a_ - op_b_;
      vvp_send_real(ptr.ptr()->out, val, 0);
}

/* Real compare equal. */
vvp_cmp_eq_real::vvp_cmp_eq_real()
{
}

void vvp_cmp_eq_real::recv_real(vvp_net_ptr_t ptr, const double bit,
                                vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t res (1);
      if (op_a_ == op_b_) res.set_bit(0, BIT4_1);
      else res.set_bit(0, BIT4_0);

      vvp_send_vec4(ptr.ptr()->out, res, 0);
}

/* Real compare not equal. */
vvp_cmp_ne_real::vvp_cmp_ne_real()
{
}

void vvp_cmp_ne_real::recv_real(vvp_net_ptr_t ptr, const double bit,
                                vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t res (1);
      if (op_a_ != op_b_) res.set_bit(0, BIT4_1);
      else res.set_bit(0, BIT4_0);

      vvp_send_vec4(ptr.ptr()->out, res, 0);
}

/* Real compare greater than or equal. */
vvp_cmp_ge_real::vvp_cmp_ge_real()
{
}

void vvp_cmp_ge_real::recv_real(vvp_net_ptr_t ptr, const double bit,
                                vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t res (1);
      if (op_a_ >= op_b_) res.set_bit(0, BIT4_1);
      else res.set_bit(0, BIT4_0);

      vvp_send_vec4(ptr.ptr()->out, res, 0);
}

/* Real compare greater than. */
vvp_cmp_gt_real::vvp_cmp_gt_real()
{
}

void vvp_cmp_gt_real::recv_real(vvp_net_ptr_t ptr, const double bit,
                                vvp_context_t)
{
      dispatch_operand_(ptr, bit);

      vvp_vector4_t res (1);
      if (op_a_ > op_b_) res.set_bit(0, BIT4_1);
      else res.set_bit(0, BIT4_0);

      vvp_send_vec4(ptr.ptr()->out, res, 0);
}
