/*
 * Copyright (c) 1998-2008 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "verinum.h"
# include  <iostream>
# include  <cassert>
# include  <math.h> // Needed to get pow for as_double().
# include  <stdio.h> // Needed to get snprintf for as_string().

#if !defined(HAVE_LROUND)
/*
 * If the system doesn't provide the lround function, then we provide
 * it ourselves here. It is simply the nearest integer, rounded away
 * from zero.
 */
extern "C" long int lround(double x)
{
      if (x >= 0.0)
	    return (long)floor(x+0.5);
      else
	    return (long)ceil(x-0.5);
}
#endif

static verinum::V add_with_carry(verinum::V l, verinum::V r, verinum::V&c);

verinum::verinum()
: bits_(0), nbits_(0), has_len_(false), has_sign_(false), string_flag_(false)
{
}

verinum::verinum(const V*bits, unsigned nbits, bool has_len__)
: has_len_(has_len__), has_sign_(false), string_flag_(false)
{
      nbits_ = nbits;
      bits_ = new V [nbits];
      for (unsigned idx = 0 ;  idx < nbits ;  idx += 1) {
	    bits_[idx] = bits[idx];
      }
}

static string process_verilog_string_quotes(const string&str)
{
      string res;

      int idx = 0;
      int str_len = str.length();

      while (idx < str_len) {
	    if (str[idx] == '\\') {
		  idx += 1;
		  assert(idx < str_len);
		  switch (str[idx]) {
		      case 'n':
			res = res + '\n';
			idx += 1;
			break;
		      case 't':
			res = res + '\t';
			idx += 1;
			break;
		      case '0':
		      case '1':
		      case '2':
		      case '3':
		      case '4':
		      case '5':
		      case '6':
		      case '7': {
			    char byte_val = 0;
			    int odx = 0;
			    while (odx < 3 && idx+odx < str_len
				   && str[idx+odx] >= '0'
				   && str[idx+odx] <= '7') {
				  byte_val = 8*byte_val + str[idx+odx]-'0';
				  odx += 1;
			    }
			    idx += odx;
			    res = res + byte_val;
			    break;
		      }
		      default:
			res = res + str[idx];
			idx += 1;
			break;
		  }
	    } else {
		  res = res + str[idx];
		  idx += 1;
	    }
      }
      return res;
}

verinum::verinum(const string&s)
: has_len_(true), has_sign_(false), string_flag_(true)
{
      string str = process_verilog_string_quotes(s);
      nbits_ = str.length() * 8;

	// Special case: The string "" is 8 bits of 0.
      if (nbits_ == 0) {
	    nbits_ = 8;
	    bits_ = new V [nbits_];
	    bits_[0] = V0;
	    bits_[1] = V0;
	    bits_[2] = V0;
	    bits_[3] = V0;
	    bits_[4] = V0;
	    bits_[5] = V0;
	    bits_[6] = V0;
	    bits_[7] = V0;
	    return;
      }

      bits_ = new V [nbits_];

      unsigned idx, cp;
      V*bp = bits_+nbits_;
      for (idx = nbits_, cp = 0 ;  idx > 0 ;  idx -= 8, cp += 1) {
	    char ch = str[cp];
	    *(--bp) = (ch&0x80) ? V1 : V0;
	    *(--bp) = (ch&0x40) ? V1 : V0;
	    *(--bp) = (ch&0x20) ? V1 : V0;
	    *(--bp) = (ch&0x10) ? V1 : V0;
	    *(--bp) = (ch&0x08) ? V1 : V0;
	    *(--bp) = (ch&0x04) ? V1 : V0;
	    *(--bp) = (ch&0x02) ? V1 : V0;
	    *(--bp) = (ch&0x01) ? V1 : V0;
      }
}

verinum::verinum(verinum::V val, unsigned n, bool h)
: has_len_(h), has_sign_(false), string_flag_(false)
{
      nbits_ = n;
      bits_ = new V[nbits_];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = val;
}

verinum::verinum(uint64_t val, unsigned n)
: has_len_(true), has_sign_(false), string_flag_(false)
{
      nbits_ = n;
      bits_ = new V[nbits_];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1) {
	    bits_[idx] = (val&1) ? V1 : V0;
	    val >>= (uint64_t)1;
      }
}

/* The second argument is not used! It is there to make this
 * constructor unique. */
verinum::verinum(double val, bool dummy)
: has_len_(false), has_sign_(true), string_flag_(false)
{
      bool is_neg = false;
      double fraction;
      int exponent;
      const unsigned BITS_IN_LONG = 8*sizeof(long);

	/* We return `bx for a NaN or +/- infinity. */
      if (val != val || (val && (val == 0.5*val))) {
	    nbits_ = 1;
	    bits_ = new V[nbits_];
	    bits_[0] = Vx;
	    return;
      }

	/* Convert to a positive result. */
      if (val < 0.0) {
	    is_neg = true;
	    val = -val;
      }

	/* Get the exponent and fractional part of the number. */
      fraction = frexp(val, &exponent);
      nbits_ = exponent+1;
      bits_ = new V[nbits_];
      const verinum const_one(1);

	/* If the value is small enough just use lround(). */
      if (nbits_ <= BITS_IN_LONG) {
	    long sval = lround(val);
	    if (is_neg) sval = -sval;
	    for (unsigned idx = 0; idx < nbits_; idx += 1) {
		  bits_[idx] = (sval&1) ? V1 : V0;
		  sval >>= 1;
	    }
	      /* Trim the result. */
	    signed_trim();
	    return;
      }

      unsigned nwords = (exponent-1)/BITS_IN_LONG;

      fraction = ldexp(fraction, (exponent-1) % BITS_IN_LONG + 1);

      if (nwords == 0) {
	    unsigned long bits = (unsigned long) fraction;
	    fraction = fraction - (double) bits;
	    for (unsigned idx = 0; idx < nbits_; idx += 1) {
		  bits_[idx] = (bits&1) ? V1 : V0;
		  bits >>= 1;
	    }
	    if (fraction >= 0.5) *this = *this + const_one;
      } else {
	    for (int wd = nwords; wd >= 0; wd -= 1) {
		  unsigned long bits = (unsigned long) fraction;
		  fraction = fraction - (double) bits;
		  unsigned max = (wd+1)*BITS_IN_LONG;
		  if (max > nbits_) max = nbits_;
		  for (unsigned idx = wd*BITS_IN_LONG; idx < max; idx += 1) {
			bits_[idx] = (bits&1) ? V1 : V0;
			bits >>= 1;
		  }
		  fraction = ldexp(fraction, BITS_IN_LONG);
	    }
	    if (fraction >= ldexp(0.5, BITS_IN_LONG)) *this = *this + const_one;
      }

	/* Convert a negative number if needed. */
      if (is_neg) {
	    *this = v_not(*this) + const_one;
      }

	/* Trim the result. */
      signed_trim();
}


/* This is used by the double constructor above. It is needed to remove
 * extra sign bits that can occur when calculating a negative value. */
void verinum::signed_trim()
{
	/* Do we have any extra digits? */
      unsigned tlen = nbits_-1;
      verinum::V sign = bits_[tlen];
      while ((tlen > 0) && (bits_[tlen] == sign)) tlen -= 1;

	/* tlen now points to the first digit that is not the sign.
	 * or bit 0. Set the length to include this bit and one proper
	 * sign bit if needed. */
      if (bits_[tlen] != sign) tlen += 1;
      tlen += 1;

	/* Trim the bits if needed. */
      if (tlen < nbits_) {
	    V* tbits = new V[tlen];
	    for (unsigned idx = 0; idx < tlen; idx += 1)
		  tbits[idx] = bits_[idx];
	    delete[] bits_;
	    bits_ = tbits;
	    nbits_ = tlen;
      }
}

verinum::verinum(const verinum&that)
{
      string_flag_ = that.string_flag_;
      nbits_ = that.nbits_;
      bits_ = new V[nbits_];
      has_len_ = that.has_len_;
      has_sign_ = that.has_sign_;
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = that.bits_[idx];
}

verinum::verinum(const verinum&that, unsigned nbits)
{
      string_flag_ = false;
      nbits_ = nbits;
      bits_ = new V[nbits_];
      has_len_ = true;
      has_sign_ = that.has_sign_;

      unsigned copy = nbits;
      if (copy > that.nbits_)
	    copy = that.nbits_;
      for (unsigned idx = 0 ;  idx < copy ;  idx += 1)
	    bits_[idx] = that.bits_[idx];

      if (copy < nbits_) {
	    if (has_sign_) {
		  for (unsigned idx = copy ;  idx < nbits_ ;  idx += 1)
			bits_[idx] = bits_[idx-1];
	    } else {
		  for (unsigned idx = copy ;  idx < nbits_ ;  idx += 1)
			bits_[idx] = verinum::V0;
	    }
      }
}

verinum::verinum(int64_t that)
: has_len_(false), has_sign_(true), string_flag_(false)
{
      int64_t tmp;

      if (that < 0) tmp = (that+1)/2;
      else tmp = that/2;
      nbits_ = 1;
      while (tmp != 0) {
	    nbits_ += 1;
	    tmp /= 2;
      }

      nbits_ += 1;

      bits_ = new V[nbits_];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1) {
	    bits_[idx] = (that & 1)? V1 : V0;
	    that >>= 1;
      }
}

verinum::~verinum()
{
      delete[]bits_;
}

verinum& verinum::operator= (const verinum&that)
{
      if (this == &that) return *this;
      delete[]bits_;
      nbits_ = that.nbits_;
      bits_ = new V[that.nbits_];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = that.bits_[idx];

      has_len_ = that.has_len_;
      has_sign_ = that.has_sign_;
      string_flag_ = that.string_flag_;
      return *this;
}

verinum::V verinum::get(unsigned idx) const
{
      assert(idx < nbits_);
      return bits_[idx];
}

verinum::V verinum::set(unsigned idx, verinum::V val)
{
      assert(idx < nbits_);
      return bits_[idx] = val;
}

unsigned long verinum::as_ulong() const
{
      if (nbits_ == 0)
	    return 0;

      if (!is_defined())
	    return 0;

      unsigned top = nbits_;
      if (top >= (8 * sizeof(unsigned long)))
	  top = 8 * sizeof(unsigned long);

      unsigned long val = 0;
      unsigned long mask = 1;
      for (unsigned idx = 0 ;  idx < top ;  idx += 1, mask <<= 1)
	    if (bits_[idx] == V1)
		  val |= mask;

      return val;
}

uint64_t verinum::as_ulong64() const
{
      if (nbits_ == 0)
	    return 0;

      if (!is_defined())
	    return 0;

      unsigned top = nbits_;
      if (top >= (8 * sizeof(uint64_t)))
	  top = 8 * sizeof(uint64_t);

      uint64_t val = 0;
      uint64_t mask = 1;
      for (unsigned idx = 0 ;  idx < top ;  idx += 1, mask <<= 1)
	    if (bits_[idx] == V1)
		  val |= mask;

      return val;
}

/*
 * This function returns the native long integer that represents the
 * value of this object. It accounts for sign extension if the value
 * is signed.
 *
 * If the value is undefined, return 0.
 *
 * This function presumes that the native format is 2s complement
 * (pretty safe these days) and masks/sets bits accordingly. If the
 * value is too large for the native form, it truncates the high bits.
 */
signed long verinum::as_long() const
{
      if (nbits_ == 0)
	    return 0;

      if (!is_defined())
	    return 0;

      signed long val = 0;

      if (has_sign_ && (bits_[nbits_-1] == V1)) {
	    unsigned top = nbits_;
	    if (top > (8 * sizeof(long) - 1))
		  top = 8 * sizeof(long) - 1;

	    val = -1;
	    signed long mask = ~1L;
	    for (unsigned idx = 0 ;  idx < top ;  idx += 1) {
		  if (bits_[idx] == V0)
			val &= mask;

		  mask = (mask << 1) | 1L;
	    }

      } else {
	    unsigned top = nbits_;
	    if (top > (8 * sizeof(long) - 1))
		  top = 8 * sizeof(long) - 1;

	    signed long mask = 1;
	    for (unsigned idx = 0 ;  idx < top ;  idx += 1, mask <<= 1)
		  if (bits_[idx] == V1)
			val |= mask;
      }

      return val;
}

double verinum::as_double() const
{
      if (nbits_ == 0) return 0.0;

      double val = 0.0;
        /* Do we have/want a signed value? */
      if (has_sign_ && bits_[nbits_-1] == V1) {
	    V carry = V1;
	    for (unsigned idx = 0; idx < nbits_; idx += 1) {
		  V sum = add_with_carry(~bits_[idx], V0, carry);
		  if (sum == V1)
			val += pow(2.0, (double)idx);
	    }
	    val *= -1.0;
      } else {
	    for (unsigned idx = 0; idx < nbits_; idx += 1) {
		  if (bits_[idx] == V1)
			val += pow(2.0, (double)idx);
	    }
      }
      return val;
}

string verinum::as_string() const
{
      assert( nbits_%8 == 0 );
      if (nbits_ == 0)
	    return "";

      string res;
      bool leading_nuls = true;
      for (unsigned idx = nbits_ ;  idx > 0 ;  idx -= 8) {
	    char char_val = 0;
	    V*bp = bits_+idx;

	    if (*(--bp) == V1) char_val |= 0x80;
	    if (*(--bp) == V1) char_val |= 0x40;
	    if (*(--bp) == V1) char_val |= 0x20;
	    if (*(--bp) == V1) char_val |= 0x10;
	    if (*(--bp) == V1) char_val |= 0x08;
	    if (*(--bp) == V1) char_val |= 0x04;
	    if (*(--bp) == V1) char_val |= 0x02;
	    if (*(--bp) == V1) char_val |= 0x01;
	    if (char_val == 0 && leading_nuls)
		  continue;

	    if (char_val == '"' || char_val == '\\') {
		  char tmp[5];
		  snprintf(tmp, sizeof tmp, "\\\%03o", char_val);
		  res = res + tmp;
	    } else if (char_val == ' ' || isgraph(char_val)) {
		  res = res + char_val;

	    } else {
		  char tmp[5];
		  snprintf(tmp, sizeof tmp, "\\\%03o", char_val);
		  res = res + tmp;
	    }
      }
      return res;
}

bool verinum::is_before(const verinum&that) const
{
      if (that.nbits_ > nbits_) return true;
      if (that.nbits_ < nbits_) return false;

      for (unsigned idx = nbits_  ;  idx > 0 ;  idx -= 1) {
	    if (bits_[idx-1] < that.bits_[idx-1]) return true;
	    if (bits_[idx-1] > that.bits_[idx-1]) return false;
      }
      return false;
}

bool verinum::is_defined() const
{
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1) {
	    if (bits_[idx] == Vx) return false;
	    if (bits_[idx] == Vz) return false;
      }
      return true;
}

bool verinum::is_zero() const
{
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    if (bits_[idx] != V0) return false;

      return true;
}

bool verinum::is_negative() const
{
      return (bits_[nbits_-1] == V1) && has_sign();
}

verinum pad_to_width(const verinum&that, unsigned width)
{
      if (that.len() >= width)
	    return that;

      if (that.len() == 0) {
	    verinum val (verinum::V0, width, that.has_len());
	    val.has_sign(that.has_sign());
	    return val;
      }

      verinum::V pad = that[that.len()-1];
      if (pad==verinum::V1 && !that.has_sign())
	    pad = verinum::V0;
      if (that.has_len() && !that.has_sign()) {
	    if (pad==verinum::Vx)
		  pad = verinum::V0;
	    if (pad==verinum::Vz)
		  pad = verinum::V0;
      }

      verinum val(pad, width, that.has_len());

      for (unsigned idx = 0 ;  idx < that.len() ;  idx += 1)
	    val.set(idx, that[idx]);

      val.has_sign(that.has_sign());
      return val;
}

/*
 * This function returns a version of the verinum that has only as
 * many bits as are needed to accurately represent the value. It takes
 * into account the signedness of the value.
 *
 * If the input value has a definite length, then that value is just
 * returned as is.
 */
verinum trim_vnum(const verinum&that)
{
      unsigned tlen;

      if (that.has_len())
	    return that;

      if (that.len() < 2)
	    return that;

      if (that.has_sign()) {
	    unsigned top = that.len()-1;
	    verinum::V sign = that.get(top);

	    while ((top > 0) && (that.get(top) == sign))
		  top -= 1;

	      /* top points to the first digit that is not the
		 sign. Set the length to include this and one proper
		 sign bit. */

	    if (that.get(top) != sign)
		  top += 1;

	    tlen = top+1;

      } else {

	      /* If the result is unsigned and has an indefinite
		 length, then trim off all but one leading zero. */
	    unsigned top = that.len()-1;
	    while ((top > 0) && (that.get(top) == verinum::V0))
		  top -= 1;

	      /* Now top is the index of the highest non-zero bit. If
		 that turns out to the highest bit in the vector, then
		 there is no trimming possible. */
	    if (top+1 == that.len())
		  return that;

	      /* Make tlen wide enough to include the highest non-zero
		 bit, plus one extra 0 bit. */
	    tlen = top+2;

	      /* This can only happen when the verinum is all zeros,
		 so make it a single bit wide. */
	    if (that.get(top) == verinum::V0) tlen -= 1;
      }

      verinum tmp (verinum::V0, tlen, false);
      tmp.has_sign(that.has_sign());
      for (unsigned idx = 0 ;  idx < tmp.len() ;  idx += 1)
	    tmp.set(idx, that.get(idx));

      return tmp;
}

ostream& operator<< (ostream&o, verinum::V v)
{
      switch (v) {
	  case verinum::V0:
	    o << "0";
	    break;
	  case verinum::V1:
	    o << "1";
	    break;
	  case verinum::Vx:
	    o << "x";
	    break;
	  case verinum::Vz:
	    o << "z";
	    break;
      }
      return o;
}

/*
 * This operator is used by various dumpers to write the Verilog
 * number in a Verilog format.
 */
ostream& operator<< (ostream&o, const verinum&v)
{
      if (v.is_string()) {
	    o << "\"" << v.as_string() << "\"";
	    return o;
      }

	/* If the verinum number has a fixed length, dump all the bits
	   literally. This is how we express the fixed length in the
	   output. */
      if (v.has_len()) {
	    o << v.len();
      }

	/* If the number is fully defined (no x or z) then print it
	   out as a decimal number. */
      if (v.is_defined() && v.len() < 8*sizeof(long)) {
	    if (v.has_sign())
		  o << "'sd" << v.as_long();
	    else
		  o << "'d" << v.as_ulong();
	    return o;
      }

	/* Oh, well. Print the minimum to get the value properly
	   displayed. */
      if (v.has_sign())
	    o << "'sb";
      else
	    o << "'b";

      if (v.len() == 0) {
	    o << "0";
	    return o;
      }

      verinum::V trim_left = v.get(v.len()-1);
      unsigned idx;

      if (v.has_sign()) {
	    for (idx = v.len()-1;  idx > 0;  idx -= 1)
		  if (trim_left != v.get(idx-1))
			break;

	    o << trim_left;
      } else {
	    idx = v.len();
      }

      while (idx > 0) {
	    o << v.get(idx-1);
	    idx -= 1;
      }

      return o;
}

verinum::V operator == (const verinum&left, const verinum&right)
{
      verinum::V left_pad = verinum::V0;
      verinum::V right_pad = verinum::V0;
      if (left.has_sign() && right.has_sign()) {
	    left_pad = left.get(left.len()-1);
	    right_pad = right.get(right.len()-1);

	    if (left_pad == verinum::V1 && right_pad == verinum::V0)
		  return verinum::V1;
	    if (left_pad == verinum::V0 && right_pad == verinum::V1)
		  return verinum::V0;
      }

      unsigned max_len = left.len();
      if (right.len() > max_len)
	    max_len = right.len();

      for (unsigned idx = 0 ;  idx < max_len ;  idx += 1) {
	    verinum::V left_bit  = idx < left.len() ? left[idx]  : left_pad;
	    verinum::V right_bit = idx < right.len()? right[idx] : right_pad;
	    if (left_bit != right_bit)
		  return verinum::V0;
      }

      return verinum::V1;
}

verinum::V operator <= (const verinum&left, const verinum&right)
{
      verinum::V left_pad = verinum::V0;
      verinum::V right_pad = verinum::V0;
      if (left.has_sign() && right.has_sign()) {
	    left_pad = left.get(left.len()-1);
	    right_pad = right.get(right.len()-1);

	    if (left_pad == verinum::V1 && right_pad == verinum::V0)
		  return verinum::V1;
	    if (left_pad == verinum::V0 && right_pad == verinum::V1)
		  return verinum::V0;
      }

      unsigned idx;
      for (idx = left.len() ; idx > right.len() ;  idx -= 1) {
	    if (left[idx-1] != right_pad) return verinum::V0;
      }

      for (idx = right.len() ; idx > left.len() ;  idx -= 1) {
	    if (right[idx-1] != left_pad) return verinum::V1;
      }

      idx = right.len();
      if (left.len() < idx) idx = left.len();

      while (idx > 0) {
	    if (left[idx-1] == verinum::Vx) return verinum::Vx;
	    if (left[idx-1] == verinum::Vz) return verinum::Vx;
	    if (right[idx-1] == verinum::Vx) return verinum::Vx;
	    if (right[idx-1] == verinum::Vz) return verinum::Vx;
	    if (left[idx-1] > right[idx-1]) return verinum::V0;
	    if (left[idx-1] < right[idx-1]) return verinum::V1;
	    idx -= 1;
      }

      return verinum::V1;
}

verinum::V operator < (const verinum&left, const verinum&right)
{
      verinum::V left_pad = verinum::V0;
      verinum::V right_pad = verinum::V0;
      if (left.has_sign() && right.has_sign()) {
	    left_pad = left.get(left.len()-1);
	    right_pad = right.get(right.len()-1);

	    if (left_pad == verinum::V1 && right_pad == verinum::V0)
		  return verinum::V1;
	    if (left_pad == verinum::V0 && right_pad == verinum::V1)
		  return verinum::V0;
      }

      unsigned idx;
      for (idx = left.len() ; idx > right.len() ;  idx -= 1) {
	    if (left[idx-1] != right_pad) return verinum::V0;
      }

      for (idx = right.len() ; idx > left.len() ;  idx -= 1) {
	    if (right[idx-1] != left_pad) return verinum::V1;
      }

      while (idx > 0) {
	    if (left[idx-1] == verinum::Vx) return verinum::Vx;
	    if (left[idx-1] == verinum::Vz) return verinum::Vx;
	    if (right[idx-1] == verinum::Vx) return verinum::Vx;
	    if (right[idx-1] == verinum::Vz) return verinum::Vx;
	    if (left[idx-1] > right[idx-1]) return verinum::V0;
	    if (left[idx-1] < right[idx-1]) return verinum::V1;
	    idx -= 1;
      }

      return verinum::V0;
}

static verinum::V add_with_carry(verinum::V l, verinum::V r, verinum::V&c)
{
      unsigned sum = 0;
      switch (c) {
	  case verinum::Vx:
	  case verinum::Vz:
	    c = verinum::Vx;
	    return verinum::Vx;
	  case verinum::V0:
	    break;
	  case verinum::V1:
	    sum += 1;
      }

      switch (l) {
	  case verinum::Vx:
	  case verinum::Vz:
	    c = verinum::Vx;
	    return verinum::Vx;
	  case verinum::V0:
	    break;
	  case verinum::V1:
	    sum += 1;
	    break;
      }

      switch (r) {
	  case verinum::Vx:
	  case verinum::Vz:
	    c = verinum::Vx;
	    return verinum::Vx;
	  case verinum::V0:
	    break;
	  case verinum::V1:
	    sum += 1;
	    break;
      }

      if (sum & 2)
	    c = verinum::V1;
      else
	    c = verinum::V0;
      if (sum & 1)
	    return verinum::V1;
      else
	    return verinum::V0;
}

verinum v_not(const verinum&left)
{
      verinum val = left;
      for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
	    switch (val[idx]) {
		case verinum::V0:
		  val.set(idx, verinum::V1);
		  break;
		case verinum::V1:
		  val.set(idx, verinum::V0);
		  break;
		default:
		  val.set(idx, verinum::Vx);
		  break;
	    }

      return val;
}

/*
 * Addition works a bit at a time, from the least significant up to
 * the most significant. The result is signed only if both of the
 * operands are signed. The result is also expanded as needed to
 * prevent overflow. It is up to the caller to shrink the result back
 * down if that is the desire.
 */
verinum operator + (const verinum&left, const verinum&right)
{
      unsigned min = left.len();
      if (right.len() < min) min = right.len();

      unsigned max = left.len();
      if (right.len() > max) max = right.len();

      bool signed_flag = left.has_sign() && right.has_sign();
      verinum::V*val_bits = new verinum::V[max+1];

      verinum::V carry = verinum::V0;
      for (unsigned idx = 0 ;  idx < min ;  idx += 1)
	    val_bits[idx] = add_with_carry(left[idx], right[idx], carry);

      verinum::V rpad = signed_flag? right[right.len()-1] : verinum::V0;
      verinum::V lpad = signed_flag? left[left.len()-1]   : verinum::V0;

      if (left.len() > right.len()) {

	    for (unsigned idx = min ;  idx < left.len() ;  idx += 1)
		  val_bits[idx] = add_with_carry(left[idx], rpad, carry);

      } else {

	    for (unsigned idx = min ;  idx < right.len() ;  idx += 1)
		  val_bits[idx] = add_with_carry(lpad, right[idx], carry);
      }

      val_bits[max] = add_with_carry(lpad, rpad, carry);
#if 0
      if (signed_flag) {
	    if (val_bits[max] != val_bits[max-1])
		  max += 1;
      }
#endif
      verinum val (val_bits, max+1, false);
      val.has_sign(signed_flag);

      delete[]val_bits;

      return val;
}

verinum operator - (const verinum&left, const verinum&right)
{
      unsigned min = left.len();
      if (right.len() < min) min = right.len();

      unsigned max = left.len();
      if (right.len() > max) max = right.len();

      bool signed_flag = left.has_sign() && right.has_sign();
      verinum::V*val_bits = new verinum::V[max+1];

      verinum::V carry = verinum::V1;
      for (unsigned idx = 0 ;  idx < min ;  idx += 1)
	    val_bits[idx] = add_with_carry(left[idx], ~right[idx], carry);

      verinum::V rpad = signed_flag? ~right[right.len()-1] : verinum::V1;
      verinum::V lpad = signed_flag?  left[left.len()-1]   : verinum::V0;

      if (left.len() > right.len()) {

	    for (unsigned idx = min ;  idx < left.len() ;  idx += 1)
		  val_bits[idx] = add_with_carry(left[idx], rpad, carry);

      } else {

	    for (unsigned idx = min ;  idx < right.len() ;  idx += 1)
		  val_bits[idx] = add_with_carry(lpad, ~right[idx], carry);
      }

      if (signed_flag) {
	    val_bits[max] = add_with_carry(lpad, rpad, carry);
	    if (val_bits[max] != val_bits[max-1])
		  max += 1;
      }

      verinum val (val_bits, max, false);
      val.has_sign(signed_flag);

      delete[]val_bits;

      return val;
}

/*
 * This multiplies two verinum numbers together into a verinum
 * result. The resulting number is as large as the sum of the sizes of
 * the operand.
 *
 * The algorithm used is successive shift and add operations,
 * implemented as the nested loops.
 *
 * If either value is not completely defined, then the result is not
 * defined either.
 */
verinum operator * (const verinum&left, const verinum&right)
{
      const bool has_len_flag = left.has_len() && right.has_len();

	/* If either operand is not fully defined, then the entire
	   result is undefined. Create a result that is the right size
	   and is filled with 'bx bits. */
      if (! (left.is_defined() && right.is_defined())) {
	    verinum result (verinum::Vx, left.len()+right.len(), has_len_flag);
	    result.has_sign(left.has_sign() || right.has_sign());
	    return result;
      }

      verinum result(verinum::V0, left.len() + right.len(), has_len_flag);
      result.has_sign(left.has_sign() || right.has_sign());

      verinum::V r_sign = sign_bit(right);
      for (unsigned rdx = 0 ;  rdx < result.len() ;  rdx += 1) {

	    verinum::V r_bit = rdx < right.len()? right.get(rdx) : r_sign;
	    if (r_bit == verinum::V0)
		  continue;

	    verinum::V l_sign = sign_bit(left);
	    verinum::V carry = verinum::V0;
	    for (unsigned ldx = 0 ;  ldx < result.len()-rdx ;  ldx += 1) {
		  verinum::V l_bit = ldx < left.len()? left[ldx] : l_sign;
		  result.set(ldx+rdx, add_with_carry(l_bit,
						     result[rdx+ldx],
						     carry));
	    }
      }

      return trim_vnum(result);
}

verinum pow(const verinum&left, const verinum&right)
{
      verinum result = left;
      unsigned pow_count = right.as_ulong();

      for (unsigned idx = 1 ;  idx < pow_count ;  idx += 1)
	    result = result * left;

      return result;
}

verinum operator << (const verinum&that, unsigned shift)
{
      verinum result(verinum::V0, that.len() + shift, that.has_len());

      for (unsigned idx = 0 ;  idx < that.len() ;  idx += 1)
	    result.set(idx+shift, that.get(idx));

      return result;
}

verinum operator >> (const verinum&that, unsigned shift)
{
      if (shift >= that.len()) {
	    if (that.has_sign()) {
		  verinum result (that.get(that.len()-1), 1);
		  result.has_sign(true);
		  return result;
	    } else {
		  verinum result(verinum::V0, 1);
		  return result;
	    }
      }

      verinum result(that.has_sign()? that.get(that.len()-1) : verinum::V0,
		     that.len() - shift, that.has_len());

      for (unsigned idx = shift ;  idx < that.len() ;  idx += 1)
	    result.set(idx-shift, that.get(idx));

      return result;
}

static verinum unsigned_divide(verinum num, verinum den, bool signed_result)
{
      unsigned nwid = num.len();
      while (nwid > 0 && (num.get(nwid-1) == verinum::V0))
	    nwid -= 1;

      unsigned dwid = den.len();
      while (dwid > 0 && (den.get(dwid-1) == verinum::V0))
	    dwid -= 1;

      if (dwid > nwid)
	    return verinum(verinum::V0, 1);

      den = den << (nwid-dwid);

      unsigned idx = nwid - dwid + 1;
      verinum result (verinum::V0, signed_result ? idx + 1 : idx);
      if (signed_result) {
	    result.set(idx, verinum::V0);
	    result.has_sign(true);
      }
      while (idx > 0) {
	    if (den <= num) {
		  verinum dif = num - den;
		  num = dif;
		  result.set(idx-1, verinum::V1);
	    }
	    den = den >> 1;
	    idx -= 1;
      }

      return result;
}

static verinum unsigned_modulus(verinum num, verinum den)
{
      unsigned nwid = num.len();
      while (nwid > 0 && (num.get(nwid-1) == verinum::V0))
	    nwid -= 1;

      unsigned dwid = den.len();
      while (dwid > 0 && (den.get(dwid-1) == verinum::V0))
	    dwid -= 1;

      if (dwid > nwid)
	    return num;

      den = den << (nwid-dwid);

      unsigned idx = nwid - dwid + 1;
      while (idx > 0) {
	    if (den <= num) {
		  verinum dif = num - den;
		  num = dif;
	    }
	    den = den >> 1;
	    idx -= 1;
      }

      return num;
}

/*
 * This operator divides the left number by the right number. If
 * either value is signed, the result is signed. If both values have a
 * defined length, then the result has a defined length.
 */
verinum operator / (const verinum&left, const verinum&right)
{
      const bool has_len_flag = left.has_len() && right.has_len();

      unsigned use_len = left.len();

	/* If either operand is not fully defined, then the entire
	   result is undefined. Create a result that is the right size
	   and is filled with 'bx bits. */
      if (! (left.is_defined() && right.is_defined())) {
	    verinum result (verinum::Vx, use_len, has_len_flag);
	    result.has_sign(left.has_sign() || right.has_sign());
	    return result;
      }

	/* If the right expression is a zero value, then the result is
	   filled with 'bx bits. */
      if (right.is_zero()) {
	    verinum result (verinum::Vx, use_len, has_len_flag);
	    result.has_sign(left.has_sign() || right.has_sign());
	    return result;
      }

      verinum result(verinum::Vz, use_len, has_len_flag);
      result.has_sign(left.has_sign() || right.has_sign());

	/* do the operation differently, depending on whether the
	   result is signed or not. */
      if (result.has_sign()) {

	    if (use_len <= (8*sizeof(long) - 1)) {
		  long l = left.as_long();
		  long r = right.as_long();
		  long v = l / r;
		  for (unsigned idx = 0 ;  idx < use_len ;  idx += 1) {
			result.set(idx,  (v & 1)? verinum::V1 : verinum::V0);
			v >>= 1;
		  }

	    } else {
		  verinum use_left, use_right;
		  verinum zero(verinum::V0, 1, false);
		  zero.has_sign(true);
		  bool negative = false;
		  if (left < zero) {
			use_left = zero - left;
			negative = !negative;
		  } else {
			use_left = left;
		  }
		  if (right < zero) {
			use_right = zero - right;
			negative = !negative;
		  } else {
			use_right = right;
		  }
		  result = unsigned_divide(use_left, use_right, true);
		  if (negative) result = zero - result;
	    }

      } else {

	    if (use_len <= 8 * sizeof(unsigned long)) {
		    /* Use native unsigned division to do the work. */

		  unsigned long l = left.as_ulong();
		  unsigned long r = right.as_ulong();
		  unsigned long v = l / r;
		  for (unsigned idx = 0 ;  idx < use_len ;  idx += 1) {
			result.set(idx,  (v & 1)? verinum::V1 : verinum::V0);
			v >>= 1;
		  }

	    } else {
		  result = unsigned_divide(left, right, false);
	    }
      }

      return trim_vnum(result);
}

verinum operator % (const verinum&left, const verinum&right)
{
      const bool has_len_flag = left.has_len() && right.has_len();

      unsigned use_len = left.len();

	/* If either operand is not fully defined, then the entire
	   result is undefined. Create a result that is the right size
	   and is filled with 'bx bits. */
      if (! (left.is_defined() && right.is_defined())) {
	    verinum result (verinum::Vx, use_len, has_len_flag);
	    result.has_sign(left.has_sign() || right.has_sign());
	    return result;
      }

	/* If the right expression is a zero value, then the result is
	   filled with 'bx bits. */
      if (right.as_ulong() == 0) {
	    verinum result (verinum::Vx, use_len, has_len_flag);
	    result.has_sign(left.has_sign() || right.has_sign());
	    return result;
      }

      verinum result(verinum::Vz, use_len, has_len_flag);
      result.has_sign(left.has_sign() || right.has_sign());

      if (result.has_sign()) {
	    if (use_len <= 8*sizeof(long)) {
		    /* Use native signed modulus to do the work. */
		  long l = left.as_long();
		  long r = right.as_long();
		  long v = l % r;
		  for (unsigned idx = 0 ;  idx < use_len ;  idx += 1) {
			result.set(idx,  (v & 1)? verinum::V1 : verinum::V0);
			v >>= 1;
		  }
	    } else {
		  verinum use_left, use_right;
		  verinum zero(verinum::V0, 1, false);
		  zero.has_sign(true);
		  bool negative = false;
		  if (left < zero) {
			use_left = zero - left;
			negative = true;
		  } else {
			use_left = left;
		  }
		  if (right < zero) {
			use_right = zero - right;
		  } else {
			use_right = right;
		  }
		  result = unsigned_modulus(use_left, use_right);
		  result.has_sign(true);
		  if (negative) result = zero - result;
	    }
      } else {
	    if (use_len <= 8*sizeof(unsigned long)) {
		    /* Use native unsigned modulus to do the work. */
		  unsigned long l = left.as_ulong();
		  unsigned long r = right.as_ulong();
		  unsigned long v = l % r;
		  for (unsigned idx = 0 ;  idx < use_len ;  idx += 1) {
			result.set(idx,  (v & 1)? verinum::V1 : verinum::V0);
			v >>= 1;
		  }
	    } else {
		  result = unsigned_modulus(left, right);
	    }
      }

      return trim_vnum(result);
}

verinum concat(const verinum&left, const verinum&right)
{
      if (left.is_string() && right.is_string()) {
	    std::string tmp = left.as_string() + right.as_string();
	    verinum res (tmp);
	    return res;
      }

      verinum res (verinum::V0, left.len() + right.len());
      for (unsigned idx = 0 ;  idx < right.len() ;  idx += 1)
	    res.set(idx, right.get(idx));

      for (unsigned idx = 0 ;  idx < left.len() ;  idx += 1)
	    res.set(idx+right.len(), left.get(idx));

      return res;
}

verinum::V operator ~ (verinum::V l)
{
      switch (l) {
	  case verinum::V0:
	    return verinum::V1;
	  case verinum::V1:
	    return verinum::V0;
	  default:
	    return verinum::Vx;
      }
}

verinum::V operator | (verinum::V l, verinum::V r)
{
      if (l == verinum::V1)
	    return verinum::V1;
      if (r == verinum::V1)
	    return verinum::V1;
      if (l != verinum::V0)
	    return verinum::Vx;
      if (r != verinum::V0)
	    return verinum::Vx;
      return verinum::V0;
}

verinum::V operator & (verinum::V l, verinum::V r)
{
      if (l == verinum::V0)
	    return verinum::V0;
      if (r == verinum::V0)
	    return verinum::V0;
      if (l != verinum::V1)
	    return verinum::Vx;
      if (r != verinum::V1)
	    return verinum::Vx;
      return verinum::V1;
}

verinum::V operator ^ (verinum::V l, verinum::V r)
{
      if (l == verinum::V0)
	    return bit4_z2x(r);
      if (r == verinum::V0)
	    return bit4_z2x(l);
      if ((l == verinum::V1) && (r == verinum::V1))
	    return verinum::V0;

      return verinum::Vx;
}
