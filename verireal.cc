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
#ident "$Id: verireal.cc,v 1.2 2000/02/23 02:56:56 steve Exp $"
#endif

# include  "verireal.h"
# include  <ctype.h>
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


verireal::~verireal()
{
}

/*
 * $Log: verireal.cc,v $
 * Revision 1.2  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/06/15 02:50:02  steve
 *  Add lexical support for real numbers.
 *
 */

