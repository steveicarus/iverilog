/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: verinum.cc,v 1.4 1998/11/09 19:03:26 steve Exp $"
#endif

# include  "verinum.h"
# include  <iostream>
# include  <cassert>

verinum::verinum()
: bits_(0), nbits_(0), string_flag_(false)
{
}

verinum::verinum(const V*bits, unsigned nbits)
: string_flag_(false)
{
      nbits_ = nbits;
      bits_ = new V [nbits];
      for (unsigned idx = 0 ;  idx < nbits ;  idx += 1) {
	    bits_[idx] = bits[idx];
      }
}

verinum::verinum(const string&str)
: string_flag_(true)
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

verinum::verinum(const verinum&that)
{
      string_flag_ = that.string_flag_;
      nbits_ = that.nbits_;
      bits_ = new V[nbits_];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    bits_[idx] = that.bits_[idx];
}

verinum::~verinum()
{
      delete[]bits_;
}

verinum::V verinum::get(unsigned idx) const
{
      return bits_[idx];
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

	// Extend the sign bit to fill the long.
      if (bits_[nbits_-1] == V1)
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

ostream& operator<< (ostream&o, const verinum&v)
{
      o << v.len() << "'b";

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

/*
 * $Log: verinum.cc,v $
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

