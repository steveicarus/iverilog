/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: verireal.cc,v 1.6 2001/11/06 06:11:55 steve Exp $"
#endif

# include "config.h"

# include  "verireal.h"
# include  <ctype.h>
# include  <iostream>
# include  <assert.h>

verireal::verireal()
{
      sign_ = false;
      mant_ = 0;
      exp10_  = 0;
}

verireal::verireal(const char*txt)
{
      mant_ = 0;
      sign_ = false;
      exp10_ = 0;

      const char*ptr = txt;
      for (  ; *ptr ;  ptr += 1) {
	    if (*ptr == '.') break;
	    if (*ptr == 'e') break;
	    if (*ptr == 'E') break;
	    if (*ptr == '_') continue;

	    assert(isdigit(*ptr));

	    mant_ *= 10;
	    mant_ += *ptr - '0';
      }

      if (*ptr == '.') {
	    ptr += 1;
	    for ( ; *ptr ; ptr += 1) {
		  if (*ptr == 'e') break;
		  if (*ptr == 'E') break;
		  if (*ptr == '_') continue;

		  assert(isdigit(*ptr));

		  mant_ *= 10;
		  mant_ += *ptr - '0';
		  exp10_ -= 1;
	    }
      }

      if ((*ptr == 'e') || (*ptr == 'E')) {
	    ptr += 1;
	    long tmp = 0;
	    bool sflag = false;
	    if (*ptr == '+') {ptr += 1; sflag = false;}
	    if (*ptr == '-') {ptr += 1; sflag = true;}

	    for ( ; *ptr ;  ptr += 1) {
		  if (*ptr == '_') continue;
		  assert(isdigit(*ptr));
		  tmp *= 10;
		  tmp += *ptr - '0';
	    }

	    exp10_ += sflag? -tmp : +tmp;
      }

      assert(*ptr == 0);
}

verireal::verireal(long val)
{
      sign_  = val < 0;
      mant_  = sign_? -val : +val;
      exp10_ = 0;
}

verireal::~verireal()
{
}

long verireal::as_long(int shift) const
{
      long val = mant_;
      int ex = exp10_ + shift;

      while (ex < 0) {
	    long mod = val % 10;
	    val /= 10;
	    if (mod >= 5)
		  val += 1;

	    ex += 1;
      }

      while (ex > 0) {
	    val *= 10;
	    ex -= 1;
      }

      if (sign_)
	    return -val;
      else
	    return val;
}

verireal operator* (const verireal&l, const verireal&r)
{
      verireal res;
      res.sign_ = l.sign_ != r.sign_;
      res.mant_ = l.mant_ * r.mant_;
      res.exp10_= l.exp10_ + r.exp10_;
      return res;
}

ostream& operator<< (ostream&out, const verireal&v)
{
      out << (v.sign_? "-" : "+") << v.mant_ << "e" << v.exp10_;
      return out;
}

/*
 * $Log: verireal.cc,v $
 * Revision 1.6  2001/11/06 06:11:55  steve
 *  Support more real arithmetic in delay constants.
 *
 * Revision 1.5  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.4  2001/07/07 20:20:10  steve
 *  Pass parameters to system functions.
 *
 * Revision 1.3  2000/12/10 22:01:36  steve
 *  Support decimal constants in behavioral delays.
 *
 * Revision 1.2  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/06/15 02:50:02  steve
 *  Add lexical support for real numbers.
 *
 */

