#ifndef __HName_H
#define __HName_H
/*
 * Copyright (c) 2001-2007 Stephen Williams (steve@icarus.com)
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
#ident "$Id: HName.h,v 1.7 2007/06/02 03:42:12 steve Exp $"
#endif

# include  <iostream>
# include  "StringHeap.h"
#ifdef __GNUC__
#if __GNUC__ > 2
using namespace std;
#endif
#endif

/*
 * This class represents a component of a Verilog hierarchical name. A
 * hierarchical component contains a name string (prepresented here
 * with a perm_string) and an optional signed number. This signed
 * number is used if the scope is part of an array, for example an
 * array of module instances or a loop generated scope.
 */

class hname_t {

    public:
      hname_t ();
      explicit hname_t (perm_string text);
      explicit hname_t (perm_string text, int num);
      hname_t (const hname_t&that);
      ~hname_t();

      hname_t& operator= (const hname_t&);

	// Return the string part of the hname_t.
      perm_string peek_name(void) const;

      bool has_number() const;
      int peek_number() const;

    private:
      perm_string name_;
	// If the number is anything other then INT_MIN, then this is
	// the numeric part of the name. Otherwise, it is not part of
	// the name at all.
      int number_;

    private: // not implemented
};

extern bool operator <  (const hname_t&, const hname_t&);
extern bool operator == (const hname_t&, const hname_t&);
extern bool operator != (const hname_t&, const hname_t&);
extern ostream& operator<< (ostream&, const hname_t&);

/*
 * $Log: HName.h,v $
 * Revision 1.7  2007/06/02 03:42:12  steve
 *  Properly evaluate scope path expressions.
 *
 * Revision 1.6  2007/05/16 19:12:33  steve
 *  Fix hname_t use of space for 1 perm_string.
 *
 * Revision 1.5  2007/04/26 03:06:21  steve
 *  Rework hname_t to use perm_strings.
 *
 * Revision 1.4  2002/11/02 03:27:51  steve
 *  Allow named events to be referenced by
 *  hierarchical names.
 *
 * Revision 1.3  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2002/06/14 03:25:51  steve
 *  Compiler portability.
 *
 * Revision 1.1  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 */
#endif
