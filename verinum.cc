/*
 * Copyright (c) 1998-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  "verinum.h"
# include  <iostream>
# include  <cassert>
# include  <climits>
# include  <cmath> // Needed to get pow for as_double().
# include  <cstdio> // Needed to get snprintf for as_string().
# include  <algorithm>

using namespace std;

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
: bits_(0), nbits_(0), has_len_(false), has_sign_(false), is_single_(false), string_flag_(false)
{
}

verinum::verinum(const V*bits, unsigned nbits, bool has_len__)
: has_len_(has_len__), has_sign_(false), is_single_(false), string_flag_(false)
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
		      case 'v':
			res = res + '\v';
			idx += 1;
			break;
		      case 'f':
			res = res + '\f';
			idx += 1;
			break;
		      case 'a':
			res = res + '\a';
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
		      case 'x': {
			    char byte_val = 0;
			    int odx = 1;
			    while (odx < 3 && idx+odx < str_len) {
				  if (str[idx+odx] >= '0' && str[idx+odx] <= '9') {
					byte_val = 16*byte_val + str[idx+odx]-'0';
					odx += 1;
				  } else if  (str[idx+odx] >= 'a' && str[idx+odx] <= 'f') {
					byte_val = 16*byte_val + str[idx+odx]-'a'+10;
					odx += 1;
				  } else if  (str[idx+odx] >= 'A' && str[idx+odx] <= 'F') {
					byte_val = 16*byte_val + str[idx+odx]-'A'+10;
					odx += 1;
				  } else {
					break;
				  }
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
: has_len_(true), has_sign_(false), is_single_(false), string_flag_(true)
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
: has_len_(h), has_sign_(false), is_single_(false), string_flag_(false)
{
      nbits_ = n;
      bits_ = new V[nbits_];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = val;
}

verinum::verinum(uint64_t val, unsigned n)
: has_len_(true), has_sign_(false), is_single_(false), string_flag_(false)
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
verinum::verinum(double val, bool)
: has_len_(false), has_sign_(true), is_single_(false), string_flag_(false)
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

	/* Round to the nearest integer now, as this may increase the
	   number of bits we need to allocate. */
      val = round(val);

	/* Get the exponent and fractional part of the number. */
      fraction = frexp(val, &exponent);
      nbits_ = exponent+1;
      bits_ = new V[nbits_];

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
      } else {
	    for (int wd = nwords; wd >= 0; wd -= 1) {
		  unsigned long bits = (unsigned long) fraction;
		  fraction = fraction - (double) bits;
		  unsigned max_idx = (wd+1)*BITS_IN_LONG;
		  if (max_idx > nbits_) max_idx = nbits_;
		  for (unsigned idx = wd*BITS_IN_LONG; idx < max_idx; idx += 1) {
			bits_[idx] = (bits&1) ? V1 : V0;
			bits >>= 1;
		  }
		  fraction = ldexp(fraction, BITS_IN_LONG);
	    }
      }

	/* Convert a negative number if needed. */
      if (is_neg) {
	    *this = -(*this);
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
      is_single_ = that.is_single_;
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = that.bits_[idx];
}

verinum::verinum(const verinum&that, unsigned nbits)
{
      string_flag_ = that.string_flag_ && (that.nbits_ == nbits);
      nbits_ = nbits;
      bits_ = new V[nbits_];
      has_len_ = true;
      has_sign_ = that.has_sign_;
      is_single_ = false;

      unsigned copy = nbits;
      if (copy > that.nbits_)
	    copy = that.nbits_;
      for (unsigned idx = 0 ;  idx < copy ;  idx += 1)
	    bits_[idx] = that.bits_[idx];

      if (copy < nbits_) {
	    if (has_sign_ || that.is_single_) {
		  for (unsigned idx = copy ;  idx < nbits_ ;  idx += 1)
			bits_[idx] = bits_[idx-1];
	    } else {
		  for (unsigned idx = copy ;  idx < nbits_ ;  idx += 1)
			bits_[idx] = verinum::V0;
	    }
      }
}

verinum::verinum(int64_t that)
: has_len_(false), has_sign_(true), is_single_(false), string_flag_(false)
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
      if (nbits_ != that.nbits_) {
            delete[]bits_;
            nbits_ = that.nbits_;
            bits_ = new V[that.nbits_];
      }
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = that.bits_[idx];

      has_len_ = that.has_len_;
      has_sign_ = that.has_sign_;
      is_single_ = that.is_single_;
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

void verinum::set(unsigned off, const verinum&val)
{
      assert(off + val.len() <= nbits_);
      for (unsigned idx = 0 ; idx < val.len() ; idx += 1)
	    bits_[off+idx] = val[idx];
}

unsigned verinum::as_unsigned() const
{
      if (nbits_ == 0)
	    return 0;

      if (!is_defined())
	    return 0;

      unsigned val = 0;
      unsigned mask = 1;
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1, mask <<= 1)
	    if (bits_[idx] == V1) {
		  if (mask == 0) return ~mask;
		  val |= mask;
	    }

      return val;
}

unsigned long verinum::as_ulong() const
{
      if (nbits_ == 0)
	    return 0;

      if (!is_defined())
	    return 0;

      unsigned long val = 0;
      unsigned long mask = 1;
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1, mask <<= 1)
	    if (bits_[idx] == V1) {
		  if (mask == 0) return ~mask;
		  val |= mask;
	    }

      return val;
}

uint64_t verinum::as_ulong64() const
{
      if (nbits_ == 0)
	    return 0;

      if (!is_defined())
	    return 0;

      uint64_t val = 0;
      uint64_t mask = 1;
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1, mask <<= 1)
	    if (bits_[idx] == V1) {
		  if (mask == 0) return ~mask;
		  val |= mask;
	    }

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
#define IVLLBITS (8 * sizeof(long) - 1)
      if (nbits_ == 0)
	    return 0;

      if (!is_defined())
	    return 0;

      signed long val = 0;
      unsigned diag_top = 0;

      unsigned top = nbits_;
      if (top > IVLLBITS) {
	    diag_top = top;
	    top = IVLLBITS;
      }
      int lost_bits=0;

      if (has_sign_ && (bits_[nbits_-1] == V1)) {
	    val = -1;
	    signed long mask = ~1L;
	    for (unsigned idx = 0 ;  idx < top ;  idx += 1) {
		  if (bits_[idx] == V0) val &= mask;
		  mask = (mask << 1) | 1L;
	    }
	    if (diag_top) {
		  for (unsigned idx = top; idx < diag_top; idx += 1) {
			if (bits_[idx] == V0) lost_bits=1;
		  }
	    }
      } else {
	    signed long mask = 1;
	    for (unsigned idx = 0 ;  idx < top ;  idx += 1, mask <<= 1) {
		  if (bits_[idx] == V1) val |= mask;
	    }
	    if (diag_top) {
		  for (unsigned idx = top; idx < diag_top; idx += 1) {
			if (bits_[idx] == V1) lost_bits=1;
		  }
	    }
      }

      if (lost_bits) cerr << "warning: verinum::as_long() truncated " <<
	  diag_top << " bits to " << IVLLBITS << ", returns " << val << endl;
      return val;
#undef IVLLBITS
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

	    if (char_val == '"' || char_val == '\\') {
		  char tmp[5];
		  snprintf(tmp, sizeof tmp, "\\%03o", char_val);
		  res = res + tmp;
	    } else if (isprint(char_val)) {
		  res = res + char_val;
	    } else {
		  char tmp[5];
		  snprintf(tmp, sizeof tmp, "\\%03o", (unsigned char)char_val);
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

unsigned verinum::significant_bits() const
{
      unsigned sbits = nbits_;

      if (has_sign_) {
	    V sgn_bit = bits_[sbits-1];
	    while ((sbits > 1) && (bits_[sbits-2] == sgn_bit))
		  sbits -= 1;
      } else {
	    while ((sbits > 1) && (bits_[sbits-1] == verinum::V0))
		  sbits -= 1;
      }
      return sbits;
}

void verinum::cast_to_int2()
{
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1) {
	    if (bits_[idx] == Vx || bits_[idx] == Vz)
		  bits_[idx] = V0;
      }
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
      if (pad==verinum::V1 && !that.has_sign() && !that.is_single())
	    pad = verinum::V0;
      if (that.has_len() && !that.has_sign() && !that.is_single()) {
	    if (pad==verinum::Vx)
		  pad = verinum::V0;
	    if (pad==verinum::Vz)
		  pad = verinum::V0;
      }

      verinum val(pad, width, that.has_len());

      for (unsigned idx = 0 ;  idx < that.len() ;  idx += 1)
	    val.set(idx, that[idx]);

      val.has_sign(that.has_sign());
      if (that.is_string() && (width % 8) == 0) {
	    val = verinum(val.as_string());
      }
      return val;
}

verinum cast_to_width(const verinum&that, unsigned width)
{
      if (that.has_len() && (that.len() == width))
            return that;

      if (that.len() >= width)
            return verinum(that, width);

      if (that.len() == 0) {
	    verinum val (verinum::V0, width, true);
	    val.has_sign(that.has_sign());
	    return val;
      }

      verinum::V pad = that[that.len()-1];
      if (pad==verinum::V1 && !that.has_sign() && !that.is_single())
	    pad = verinum::V0;
      if (that.has_len() && !that.has_sign() && !that.is_single()) {
	    if (pad==verinum::Vx)
		  pad = verinum::V0;
	    if (pad==verinum::Vz)
		  pad = verinum::V0;
      }

      verinum val(pad, width, true);

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
      unsigned dec_len = 8*sizeof(int);  /* avoid 32/64 bit differences. */
      if (! v.has_sign()) dec_len -= 1;  /* an unsigned number. */
      if (v.is_defined() && v.len() <= dec_len) {
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
		  return verinum::V0;
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
      bool signed_calc = left.has_sign() && right.has_sign();
      if (signed_calc) {
	    left_pad = left.get(left.len()-1);
	    right_pad = right.get(right.len()-1);

	    if (left_pad == verinum::V1 && right_pad == verinum::V0)
		  return verinum::V1;
	    if (left_pad == verinum::V0 && right_pad == verinum::V1)
		  return verinum::V0;
      }

      unsigned idx;
      for (idx = left.len() ; idx > right.len() ;  idx -= 1) {
	    if (left[idx-1] != right_pad) {
		      // A change of padding for a negative left argument
		      // denotes the left value is less than the right.
		   return (signed_calc &&
		           (left_pad == verinum::V1)) ? verinum::V1 :
		                                        verinum::V0;
	    }
      }

      for (idx = right.len() ; idx > left.len() ;  idx -= 1) {
	    if (right[idx-1] != left_pad) {
		      // A change of padding for a negative right argument
		      // denotes the left value is not less than the right.
		   return (signed_calc &&
		           (right_pad == verinum::V1)) ? verinum::V0 :
		                                         verinum::V1;
	    }
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
      bool signed_calc = left.has_sign() && right.has_sign();
      if (signed_calc) {
	    left_pad = left.get(left.len()-1);
	    right_pad = right.get(right.len()-1);

	    if (left_pad == verinum::V1 && right_pad == verinum::V0)
		  return verinum::V1;
	    if (left_pad == verinum::V0 && right_pad == verinum::V1)
		  return verinum::V0;
      }

      unsigned idx;
      for (idx = left.len() ; idx > right.len() ;  idx -= 1) {
	    if (left[idx-1] != right_pad) {
		      // A change of padding for a negative left argument
		      // denotes the left value is less than the right.
		   return (signed_calc &&
		           (left_pad == verinum::V1)) ? verinum::V1 :
		                                        verinum::V0;
	    }
      }

      for (idx = right.len() ; idx > left.len() ;  idx -= 1) {
	    if (right[idx-1] != left_pad) {
		      // A change of padding for a negative right argument
		      // denotes the left value is not less than the right.
		   return (signed_calc &&
		           (right_pad == verinum::V1)) ? verinum::V0 :
		                                         verinum::V1;
	    }
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

verinum operator ~ (const verinum&left)
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
 * Addition and subtraction works a bit at a time, from the least
 * significant up to the most significant. The result is signed only
 * if both of the operands are signed. If either operand is unsized,
 * the result is expanded as needed to prevent overflow.
 */

verinum operator + (const verinum&left, const verinum&right)
{
      const bool has_len_flag = left.has_len() && right.has_len();
      const bool signed_flag = left.has_sign() && right.has_sign();

      unsigned min_len = min(left.len(), right.len());
      unsigned max_len = max(left.len(), right.len());

	// If either the left or right values are undefined, the
	// entire result is undefined.
      if (!left.is_defined() || !right.is_defined()) {
	    unsigned len = has_len_flag ? max_len : 1;
	    verinum result (verinum::Vx, len, has_len_flag);
	    result.has_sign(signed_flag);
	    return result;
      }

      verinum::V*val_bits = new verinum::V[max_len+1];

      verinum::V carry = verinum::V0;
      for (unsigned idx = 0 ;  idx < min_len ;  idx += 1)
	    val_bits[idx] = add_with_carry(left[idx], right[idx], carry);

      verinum::V rpad = sign_bit(right);
      verinum::V lpad = sign_bit(left);

      if (left.len() > right.len()) {

	    for (unsigned idx = min_len ;  idx < max_len ;  idx += 1)
		  val_bits[idx] = add_with_carry(left[idx], rpad, carry);

      } else {

	    for (unsigned idx = min_len ;  idx < max_len ;  idx += 1)
		  val_bits[idx] = add_with_carry(lpad, right[idx], carry);
      }

      unsigned len = max_len;
      if (!has_len_flag) {
	    val_bits[max_len] = add_with_carry(lpad, rpad, carry);
	    if (signed_flag) {
		  if (val_bits[max_len] != val_bits[max_len-1]) len += 1;
	    } else {
		  if (val_bits[max_len] != verinum::V0) len += 1;
	    }
      }
      verinum result (val_bits, len, has_len_flag);
      result.has_sign(signed_flag);

      delete[]val_bits;

      return result;
}

verinum operator - (const verinum&left, const verinum&right)
{
      const bool has_len_flag = left.has_len() && right.has_len();
      const bool signed_flag = left.has_sign() && right.has_sign();

      unsigned min_len = min(left.len(), right.len());
      unsigned max_len = max(left.len(), right.len());

	// If either the left or right values are undefined, the
	// entire result is undefined.
      if (!left.is_defined() || !right.is_defined()) {
	    unsigned len = has_len_flag ? max_len : 1;
	    verinum result (verinum::Vx, len, has_len_flag);
	    result.has_sign(signed_flag);
	    return result;
      }

      verinum::V*val_bits = new verinum::V[max_len+1];

      verinum::V carry = verinum::V1;
      for (unsigned idx = 0 ;  idx < min_len ;  idx += 1)
	    val_bits[idx] = add_with_carry(left[idx], ~right[idx], carry);

      verinum::V rpad = sign_bit(right);
      verinum::V lpad = sign_bit(left);

      if (left.len() > right.len()) {

	    for (unsigned idx = min_len ;  idx < max_len ;  idx += 1)
		  val_bits[idx] = add_with_carry(left[idx], ~rpad, carry);

      } else {

	    for (unsigned idx = min_len ;  idx < max_len ;  idx += 1)
		  val_bits[idx] = add_with_carry(lpad, ~right[idx], carry);
      }

      unsigned len = max_len;
      if (signed_flag && !has_len_flag) {
	    val_bits[max_len] = add_with_carry(lpad, ~rpad, carry);
	    if (val_bits[max_len] != val_bits[max_len-1]) len += 1;
      }
      verinum result (val_bits, len, has_len_flag);
      result.has_sign(signed_flag);

      delete[]val_bits;

      return result;
}

verinum operator - (const verinum&right)
{
      const bool has_len_flag = right.has_len();
      const bool signed_flag = right.has_sign();

      unsigned len = right.len();

	// If either the left or right values are undefined, the
	// entire result is undefined.
      if (!right.is_defined()) {
	    verinum result (verinum::Vx, has_len_flag ? len : 1, has_len_flag);
	    result.has_sign(signed_flag);
	    return result;
      }

      verinum::V*val_bits = new verinum::V[len+1];

      verinum::V carry = verinum::V1;
      for (unsigned idx = 0 ;  idx < len ;  idx += 1)
	    val_bits[idx] = add_with_carry(verinum::V0, ~right[idx], carry);

      if (signed_flag && !has_len_flag) {
	    val_bits[len] = add_with_carry(verinum::V0, ~sign_bit(right), carry);
	    if (val_bits[len] != val_bits[len-1]) len += 1;
      }
      verinum result (val_bits, len, has_len_flag);
      result.has_sign(signed_flag);

      delete[]val_bits;

      return result;
}

/*
 * This operator multiplies the left number by the right number. The
 * result is signed only if both of the operands are signed. If either
 * operand is unsized, the resulting number is as large as the sum of
 * the sizes of the operands.
 *
 * The algorithm used is successive shift and add operations,
 * implemented as the nested loops.
 */
verinum operator * (const verinum&left, const verinum&right)
{
      const bool has_len_flag = left.has_len() && right.has_len();
      const bool signed_flag = left.has_sign() && right.has_sign();

      const unsigned l_len = left.len();
      const unsigned r_len = right.len();

      unsigned len = has_len_flag ? max(l_len, r_len) : l_len + r_len;

	// If either the left or right values are undefined, the
	// entire result is undefined.
      if (!left.is_defined() || !right.is_defined()) {
	    verinum result (verinum::Vx, has_len_flag ? len : 1, has_len_flag);
	    result.has_sign(signed_flag);
	    return result;
      }

      verinum result(verinum::V0, len, has_len_flag);
      result.has_sign(signed_flag);

      verinum::V r_sign = sign_bit(right);
      for (unsigned rdx = 0 ;  rdx < len ;  rdx += 1) {

	    verinum::V r_bit = rdx < r_len ? right.get(rdx) : r_sign;
	    if (r_bit == verinum::V0)
		  continue;

	    verinum::V l_sign = sign_bit(left);
	    verinum::V carry = verinum::V0;
	    for (unsigned ldx = 0 ;  ldx < (len - rdx) ;  ldx += 1) {
		  verinum::V l_bit = ldx < l_len ? left[ldx] : l_sign;
		  result.set(ldx+rdx, add_with_carry(l_bit,
						     result[rdx+ldx],
						     carry));
	    }
      }

      return trim_vnum(result);
}

static verinum make_p_one(unsigned len, bool has_len, bool has_sign)
{
      verinum tmp (verinum::V0, has_len ? len : 2, has_len);
      tmp.set(0, verinum::V1);
      tmp.has_sign(has_sign);
      return tmp;
}

static verinum make_m_one(unsigned len, bool has_len, bool has_sign)
{
      verinum tmp (verinum::V1, has_len ? len : 1, has_len);
      tmp.has_sign(has_sign);
      return tmp;
}

static verinum recursive_pow(const verinum&left, verinum&right)
{
        // If the exponent is zero, return a value of 1
      if (right.is_zero()) {
            return make_p_one(left.len(), left.has_len(), left.has_sign());
      }

      verinum result;
      if (right.get(0) == 1) {
              // The exponent is odd, so subtract 1 from it and recurse.
	      // We know it's odd, so the subtraction is easy.
	    right.set(0, verinum::V0);
	    result = pow(left, right);
	    result = left * result;
      } else {
              // The exponent is even, so divide it by 2 and recurse
            right = right >> 1;
            result = pow(left, right);
            result = result * result;
      }
      return result;
}

verinum pow(const verinum&left, const verinum&right)
{
      verinum result;

	// We need positive and negative one in a few places.
      verinum p_one = make_p_one(left.len(), left.has_len(), left.has_sign());
      verinum m_one = make_m_one(left.len(), left.has_len(), left.has_sign());

	// If either the left or right values are undefined, the
	// entire result is undefined.
      if (!left.is_defined() || !right.is_defined()) {
	    result = verinum(verinum::Vx, left.len(), left.has_len());
            result.has_sign(left.has_sign());

	// If the right value is zero we need to set the result to 1.
      } else if (right.is_zero()) {
	    result = p_one;

      } else if (right.is_negative()) {

	      // 0 ** <negative> is 'bx.
	    if (left.is_zero()) {
		  result = verinum(verinum::Vx, left.len(), left.has_len());
                  result.has_sign(left.has_sign());

	      // -1 ** <negative> is 1 or -1. Note that this must be done
              // before testing for +1 in case 'left' has a width of 1.
	    } else if (left.has_sign() && left == m_one) {
		  if (right.get(0) == verinum::V0) {
			result = p_one;
		  } else {
			result = m_one;
		  }

	      // 1 ** <negative> is 1.
	    } else if (left == p_one) {
		  result = p_one;

	      // Everything else is 0.
	    } else {
		  result = verinum(verinum::V0, left.len(), left.has_len());
                  result.has_sign(left.has_sign());
	    }

      } else {
            verinum exponent = right;
            result = recursive_pow(left, exponent);
      }

      return trim_vnum(result);
}

verinum operator << (const verinum&that, unsigned shift)
{
      bool has_len_flag = that.has_len();

      unsigned len = that.len();
      if (!has_len_flag) len += shift;

      verinum result(verinum::V0, len, has_len_flag);
      result.has_sign(that.has_sign());

      for (unsigned idx = shift ;  idx < len ;  idx += 1)
	    result.set(idx, that.get(idx - shift));

      return trim_vnum(result);
}

verinum operator >> (const verinum&that, unsigned shift)
{
      bool has_len_flag = that.has_len();

      unsigned len = that.len();

      verinum::V sgn_bit = that.has_sign() ? that.get(len-1) : verinum::V0;

      if (shift >= len) {
	    if (!has_len_flag) len = 1;
	    verinum result(sgn_bit, len, has_len_flag);
	    result.has_sign(that.has_sign());
	    return result;
      }

      if (!has_len_flag) len -= shift;
      verinum result(sgn_bit, len, has_len_flag);
      result.has_sign(that.has_sign());

      for (unsigned idx = shift ;  idx < that.len() ;  idx += 1)
	    result.set(idx-shift, that.get(idx));

      return trim_vnum(result);
}

static verinum unsigned_divide(verinum num, verinum den, bool signed_result)
{
	// We need the following calculations to be lossless. The
	// result will be cast to the required width by the caller.
      num.has_len(false);
      den.has_len(false);

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
	// We need the following calculations to be lossless. The
	// result will be cast to the required width by the caller.
      num.has_len(false);
      den.has_len(false);

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
 * This operator divides the left number by the right number. The result
 * is signed only if both of the operands are signed.
 */
verinum operator / (const verinum&left, const verinum&right)
{
      const bool has_len_flag = left.has_len() && right.has_len();
      const bool signed_flag = left.has_sign() && right.has_sign();

      unsigned use_len = left.len();

	// If either the left or right values are undefined, or the
	// right value is zero, the entire result is undefined.
      if (!left.is_defined() || !right.is_defined() || right.is_zero()) {
	    verinum result (verinum::Vx, use_len, has_len_flag);
	    result.has_sign(signed_flag);
	    return result;
      }

      verinum result(verinum::Vz, use_len, has_len_flag);

	/* do the operation differently, depending on whether the
	   result is signed or not. */
      if (signed_flag) {

	    if (use_len <= (8*sizeof(long) - 1)) {
		  long l = left.as_long();
		  long r = right.as_long();
		  bool overflow = (l == LONG_MIN) && (r == -1);
		  long v = overflow ? LONG_MIN : l / r;
		  for (unsigned idx = 0 ;  idx < use_len ;  idx += 1) {
			result.set(idx,  (v & 1)? verinum::V1 : verinum::V0);
			v >>= 1;
		  }

	    } else {
		  verinum use_left, use_right;
		  bool negative = false;
		  if (left.is_negative()) {
			use_left = -left;
			negative = !negative;
		  } else {
			use_left = left;
		  }
		  use_left.has_sign(false);
		  if (right.is_negative()) {
			use_right = -right;
			negative = !negative;
		  } else {
			use_right = right;
		  }
		  use_right.has_sign(false);
		  result = unsigned_divide(use_left, use_right, true);
		  if (negative) result = -result;
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

      if (has_len_flag)
	    result = cast_to_width(result, use_len);

      result.has_sign(signed_flag);
      return trim_vnum(result);
}

verinum operator % (const verinum&left, const verinum&right)
{
      const bool has_len_flag = left.has_len() && right.has_len();
      const bool signed_flag = left.has_sign() && right.has_sign();

      unsigned use_len = left.len();

	// If either the left or right values are undefined, or the
	// right value is zero, the entire result is undefined.
      if (!left.is_defined() || !right.is_defined() || right.is_zero()) {
	    verinum result (verinum::Vx, use_len, has_len_flag);
	    result.has_sign(signed_flag);
	    return result;
      }

      verinum result(verinum::Vz, use_len, has_len_flag);

      if (signed_flag) {
	    if (use_len <= 8*sizeof(long)) {
		    /* Use native signed modulus to do the work. */
		  long l = left.as_long();
		  long r = right.as_long();
		  bool overflow = (l == LONG_MIN) && (r == -1);
		  long v = overflow ? 0 : l % r;
		  for (unsigned idx = 0 ;  idx < use_len ;  idx += 1) {
			result.set(idx,  (v & 1)? verinum::V1 : verinum::V0);
			v >>= 1;
		  }
	    } else {
		  verinum use_left, use_right;
		  bool negative = false;
		  if (left.is_negative()) {
			use_left = -left;
			negative = true;
		  } else {
			use_left = left;
		  }
		  use_left.has_sign(false);
		  if (right.is_negative()) {
			use_right = -right;
		  } else {
			use_right = right;
		  }
		  use_right.has_sign(false);
		  result = unsigned_modulus(use_left, use_right);
		  if (negative) result = -result;
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

      if (has_len_flag)
	    result = cast_to_width(result, use_len);

      result.has_sign(signed_flag);
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
