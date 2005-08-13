/*
 * Copyright (c) 1999-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: verireal.cc,v 1.15.2.1 2005/08/13 00:45:55 steve Exp $"
#endif

# include "config.h"

# include  "verireal.h"
# include  "verinum.h"
# include  <stdlib.h>
# include  <ctype.h>
# include  <iostream>
# include  <math.h>
# include  <assert.h>

verireal::verireal()
{
      value_ = 0.0;
}

verireal::verireal(const char*txt)
{
      char*tmp = new char[strlen(txt)+1];
      char*cp = tmp;
      for (unsigned idx = 0 ;  txt[idx] ;  idx += 1) {
	    if (txt[idx] == '_')
		  continue;

	    *cp++ = txt[idx];
      }
      cp[0] = 0;

      value_ = strtod(tmp, 0);
      delete[]tmp;
}

verireal::verireal(long val)
{
      value_ = (double)val;
}

verireal::~verireal()
{
}

long verireal::as_long(int shift) const
{
      double out = value_ * pow(10.0,shift);
      double outf;

      if (out >= 0.0) {
	    outf = floor(out);
	    if (out >= (outf + 0.5))
		  outf += 1.0;
      } else {
	    outf = ceil(out);
	    if (out <= (outf - 0.5))
		  outf -= 1.0;
      }
      return (long) outf;
}

double verireal::as_double() const
{
      return value_;
}

verireal operator* (const verireal&l, const verireal&r)
{
      verireal res;
      res.value_ = l.value_ * r.value_;
      return res;
}

verireal operator/ (const verireal&l, const verireal&r)
{
      verireal res;
      res.value_ = l.value_ / r.value_;
      return res;
}

verireal operator/ (const verireal&l, const verinum&r)
{
      verireal res;
      res.value_ = l.value_ / (double)r.as_long();
      return res;
}

verireal operator% (const verireal&l, const verireal&r)
{
      verireal res;
      assert(0);
      return res;
}

verireal operator% (const verireal&l, const verinum&r)
{
      verireal res;
      assert(0);
      return res;
}

verireal operator- (const verireal&l)
{
      verireal res;
      res.value_ = - l.value_;
      return res;
}

std::ostream& operator<< (std::ostream&out, const verireal&v)
{
      out << v.value_;
      return out;
}

/*
 * $Log: verireal.cc,v $
 * Revision 1.15.2.1  2005/08/13 00:45:55  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.15  2004/06/04 23:33:51  steve
 *  Add unary minus as operator supported by verireal.
 *
 * Revision 1.14  2003/03/07 06:10:13  steve
 *  Use strtod to convert text to doubles.
 *
 * Revision 1.13  2003/03/05 03:45:01  steve
 *  Restore verireal constructor to match vvp processing of reals.
 *
 * Revision 1.11  2003/02/07 06:13:44  steve
 *  Store real values as native double.
 *
 * Revision 1.10  2003/02/07 02:48:43  steve
 *  NetEBDiv handles real value constant expressions.
 *
 * Revision 1.9  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.8  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.7  2002/06/15 02:35:49  steve
 *  Rounding error.
 *
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

