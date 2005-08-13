#ifndef __verireal_H
#define __verireal_H
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
#ident "$Id: verireal.h,v 1.10.2.2 2005/08/13 00:45:55 steve Exp $"
#endif

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif

using namespace std;

class verinum;

/*
 * This class holds a floating point decimal number. The number is
 * stored as an integer mantissa and a power of 10. The mantissa is an
 * integer so that decimal numbers in the source (which are decimal)
 * can be stored exactly.
 */

class verireal {

      friend std::ostream& operator<< (std::ostream&, const verireal&);
      friend verireal operator* (const verireal&, const verireal&);
      friend verireal operator/ (const verireal&, const verireal&);
      friend verireal operator/ (const verireal&, const verinum&);
      friend verireal operator% (const verireal&, const verireal&);
      friend verireal operator% (const verireal&, const verinum&);

	// Unary minus.
      friend verireal operator- (const verireal&);

    public:
      explicit verireal();
      explicit verireal(const char*text);
      explicit verireal(long val);
      ~verireal();

	/* Return the value of the floating point number as an
	   integer, rounded as needed. The shift is the power of 10 to
	   multiply the value before calculating the result. So for
	   example if the value is 2.5 and shift == 1, the result
	   is 25. */
      long as_long(int shift =0) const;

      double as_double() const;

    private:
      double value_;
};

extern std::ostream& operator<< (std::ostream&, const verireal&);
extern verireal operator* (const verireal&, const verireal&);
extern verireal operator/ (const verireal&, const verireal&);
extern verireal operator/ (const verireal&, const verinum&);
extern verireal operator% (const verireal&, const verireal&);
extern verireal operator% (const verireal&, const verinum&);
extern verireal operator- (const verireal&);

/*
 * $Log: verireal.h,v $
 * Revision 1.10.2.2  2005/08/13 00:45:55  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.10.2.1  2005/06/14 15:33:54  steve
 *  Fix gcc4 build issues.
 *
 * Revision 1.10  2004/06/04 23:33:51  steve
 *  Add unary minus as operator supported by verireal.
 *
 * Revision 1.9  2003/02/07 06:13:44  steve
 *  Store real values as native double.
 *
 * Revision 1.8  2003/02/07 02:48:43  steve
 *  NetEBDiv handles real value constant expressions.
 *
 * Revision 1.7  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.6  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2001/11/06 06:11:55  steve
 *  Support more real arithmetic in delay constants.
 *
 * Revision 1.4  2001/01/16 02:44:18  steve
 *  Use the iosfwd header if available.
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
#endif
