/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: verinum.cc,v 1.18 2000/09/07 22:37:10 steve Exp $"
#endif

# include  "verinum.h"
# include  <iostream>
# include  <cassert>

verinum::verinum()
: bits_(0), nbits_(0), has_len_(false), has_sign_(false), string_flag_(false)
{
}

verinum::verinum(const V*bits, unsigned nbits, bool has_len)
: has_len_(has_len), has_sign_(false), string_flag_(false)
{
      nbits_ = nbits;
      bits_ = new V [nbits];
      for (unsigned idx = 0 ;  idx < nbits ;  idx += 1) {
	    bits_[idx] = bits[idx];
      }
}

verinum::verinum(const string&str)
: has_len_(true), has_sign_(false), string_flag_(true)
{
      nbits_ = str.length() * 8;
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

verinum::verinum(verinum::V val, unsigned n)
: has_len_(true), has_sign_(false), string_flag_(false)
{
      nbits_ = n;
      bits_ = new V[nbits_];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = val;
}

verinum::verinum(unsigned long val, unsigned n)
: has_len_(true), has_sign_(false), string_flag_(false)
{
      nbits_ = n;
      bits_ = new V[nbits_];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1) {
	    bits_[idx] = (val&1) ? V1 : V0;
	    val >>= 1;
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
      assert(nbits <= that.nbits_);
      string_flag_ = false;
      nbits_ = nbits;
      bits_ = new V[nbits_];
      has_len_ = true;
      has_sign_ = false;
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = that.bits_[idx];
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
      return bits_[idx];
}

verinum::V verinum::set(unsigned idx, verinum::V val)
{
      return bits_[idx] = val;
}

unsigned long verinum::as_ulong() const
{
      assert(nbits_ <= 8 * sizeof(unsigned long));

      if (nbits_ == 0)
	    return 0;

      unsigned long val = 0;
      for (unsigned idx = nbits_ ;  idx > 0 ;  idx -= 1) {
	    val <<= 1;
	    switch (bits_[idx-1]) {
		case V0:
		  break;
		case V1:
		  val |= 1;
		  break;
		case Vx:
		case Vz:
		    // what should I do here? Throw an exception?
		  break;
	    }
      }
      return val;
}

signed long verinum::as_long() const
{
      assert(nbits_ <= 8 * sizeof(signed long));

      if (nbits_ == 0)
	    return 0;

      signed long val = 0;

	// Extend the sign bit to fill the long. (But only for signed
	// numbers.)
      if (has_sign_ && (bits_[nbits_-1] == V1))
	    val = -1;

      for (unsigned idx = nbits_ ;  idx > 0 ;  idx -= 1) {
	    val <<= 1;
	    switch (bits_[idx-1]) {
		case V0:
		  break;
		case V1:
		  val |= 1;
		  break;
		case Vx:
		case Vz:
		    // what should I do here? Throw an exception?
		  break;
	    }
      }

      return val;
}

string verinum::as_string() const
{
      assert( nbits_%8 == 0 );
      if (nbits_ == 0)
	    return "";

      char*tmp = new char[nbits_/8+1];
      char*cp = tmp;
      for (unsigned idx = nbits_ ;  idx > 0 ;  idx -= 8, cp += 1) {
	    V*bp = bits_+idx;
	    *cp = 0;
	    if (*(--bp) == V1) *cp |= 0x80;
	    if (*(--bp) == V1) *cp |= 0x40;
	    if (*(--bp) == V1) *cp |= 0x20;
	    if (*(--bp) == V1) *cp |= 0x10;
	    if (*(--bp) == V1) *cp |= 0x08;
	    if (*(--bp) == V1) *cp |= 0x04;
	    if (*(--bp) == V1) *cp |= 0x02;
	    if (*(--bp) == V1) *cp |= 0x01;
      }

      tmp[nbits_/8] = 0;
      string result = string(tmp);
      delete[]tmp;
      return result;
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
 * This operator is used by various dumpers to write the verilog
 * number in a Verilog format.
 */
ostream& operator<< (ostream&o, const verinum&v)
{
      if (v.has_sign()) {
	    o << "+";
      }

	/* If the verinum number has a fixed length, dump all the bits
	   literally. This is how we express the fixed length in the
	   output. */
      if (v.has_len()) {
	    o << v.len() << "'b";
	    if (v.len() == 0) {
		  o << "0";
		  return o;
	    }

	    for (unsigned idx = v.len() ;  idx > 0 ;  idx -= 1)
		  o << v[idx-1];

	    return o;
      }

	/* If the number is fully defined (no x or z) then print it
	   out as a decimal number. */
      if (v.is_defined()) {
	    o << "'d" << v.as_ulong();
	    return o;
      }

	/* Oh, well. Print the minimum to get the value properly
	   displayed. */
      o << "'b";

      if (v.len() == 0) {
	    o << "0";
	    return o;
      }

      verinum::V trim_left = v.get(v.len()-1);
      unsigned idx;

      for (idx = v.len()-1;  idx > 0;  idx -= 1)
	    if (trim_left != v.get(idx-1))
		  break;

      o << trim_left;

      while (idx > 0) {
	    o << v.get(idx-1);
	    idx -= 1;
      }

      return o;
}

verinum::V operator == (const verinum&left, const verinum&right)
{
      if (left.len() != right.len())
	    return verinum::V0;

      for (unsigned idx = 0 ;  idx < left.len() ;  idx += 1)
	    if (left[idx] != right[idx])
		  return verinum::V0;

      return verinum::V1;
}

verinum::V operator <= (const verinum&left, const verinum&right)
{
      unsigned idx;
      for (idx = left.len() ; idx > right.len() ;  idx -= 1) {
	    if (left[idx-1] != verinum::V0) return verinum::V0;
      }

      for (idx = right.len() ; idx > left.len() ;  idx -= 1) {
	    if (right[idx-1] != verinum::V0) return verinum::V0;
      }

      while (idx > 0) {
	    if (left[idx-1] == verinum::Vx) return verinum::Vx;
	    if (left[idx-1] == verinum::Vz) return verinum::Vx;
	    if (right[idx-1] == verinum::Vx) return verinum::Vx;
	    if (right[idx-1] == verinum::Vz) return verinum::Vx;
	    if (left[idx-1] > right[idx-1]) return verinum::V0;
	    idx -= 1;
      }

      return verinum::V1;
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
 * the most significant. The result is signed only if either of the
 * operands is signed.
 */
verinum operator + (const verinum&left, const verinum&right)
{
      unsigned min = left.len();
      if (right.len() < min) min = right.len();

      unsigned max = left.len();
      if (right.len() > max) max = right.len();

      verinum val (verinum::V0, max);
      val.has_sign( left.has_sign() || right.has_sign() );

      verinum::V carry = verinum::V0;
      for (unsigned idx = 0 ;  idx < min ;  idx += 1)
	    val.set(idx, add_with_carry(left[idx], right[idx], carry));

      if (left.len() > right.len()) {
	    for (unsigned idx = min ;  idx < max ;  idx += 1)
		  val.set(idx,add_with_carry(left[idx], verinum::V0, carry));
      } else {
	    for (unsigned idx = min ;  idx < max ;  idx += 1)
		  val.set(idx, add_with_carry(verinum::V0, right[idx], carry));
      }

      return val;
}

verinum operator - (const verinum&left, const verinum&r)
{
      verinum right;
      unsigned min = left.len();
      if (r.len() < min) {
	    right = verinum(verinum::V0, min);
	    for (unsigned idx = 0 ;  idx < r.len() ;  idx += 1)
		  right.set(idx, r[idx]);

      } else {
	    right = r;
      }

      right = v_not(right);

      unsigned max = left.len();
      if (right.len() > max) max = right.len();

      verinum val (verinum::V0, max);

      verinum::V carry = verinum::V1;
      for (unsigned idx = 0 ;  idx < min ;  idx += 1)
	    val.set(idx, add_with_carry(left[idx], right[idx], carry));

      assert(left.len() <= right.len());
      for (unsigned idx = min ;  idx < max ;  idx += 1)
	    val.set(idx, add_with_carry(verinum::V0, right[idx], carry));


      return val;
}

/*
 * $Log: verinum.cc,v $
 * Revision 1.18  2000/09/07 22:37:10  steve
 *  The + operator now preserves signedness.
 *
 * Revision 1.17  2000/06/12 03:56:51  steve
 *  Fix subract of short value form long one.
 *
 * Revision 1.16  2000/02/23 04:43:43  steve
 *  Some compilers do not accept the not symbol.
 *
 * Revision 1.15  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.14  2000/01/07 03:45:49  steve
 *  Initial support for signed constants.
 *
 * Revision 1.13  2000/01/06 05:57:06  steve
 *  Only sign-extend unsized numbers.
 *
 * Revision 1.12  1999/11/06 16:00:17  steve
 *  Put number constants into a static table.
 *
 * Revision 1.11  1999/10/22 23:57:53  steve
 *  do the <= in bits, not numbers.
 *
 * Revision 1.10  1999/10/10 23:29:37  steve
 *  Support evaluating + operator at compile time.
 *
 * Revision 1.9  1999/07/20 05:12:22  steve
 *  Implement the set method.
 *
 * Revision 1.8  1999/05/13 04:02:09  steve
 *  More precise handling of verinum bit lengths.
 *
 * Revision 1.7  1999/05/09 01:38:33  steve
 *  Add implementation of integer to verunum constructor.
 *
 * Revision 1.6  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 * Revision 1.5  1998/11/11 00:01:51  steve
 *  Check net ranges in declarations.
 *
 * Revision 1.4  1998/11/09 19:03:26  steve
 *  Oops, forgot return from operator<<
 *
 * Revision 1.3  1998/11/09 18:55:35  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.2  1998/11/07 17:04:48  steve
 *  Properly dump 0 length numbers.
 *
 * Revision 1.1  1998/11/03 23:29:07  steve
 *  Introduce verilog to CVS.
 *
 */

