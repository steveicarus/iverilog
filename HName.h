#ifndef __HName_H
#define __HName_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: HName.h,v 1.4 2002/11/02 03:27:51 steve Exp $"
#endif

# include  <iostream>
#ifdef __GNUC__
#if __GNUC__ > 2
using namespace std;
#endif
#endif

/*
 * This class represents a Verilog hierarchical name. A hierarchical
 * name is an ordered list of simple names.
 */

class hname_t {

    public:
      hname_t ();
      explicit hname_t (const char*text);
      hname_t (const hname_t&that);
      ~hname_t();

	// This method adds a name to the end of the hierarchical
	// path. This becomes a new base name.
      void append(const char*text);

	// This method adds a name to the *front* of the hierarchical
	// path. The base name remains the same, unless this is the
	// only component.
      void prepend(const char*text);

	// This method removes the tail name from the hierarchy, and
	// returns a pointer to that tail name. That tail name now
	// must be removed by the caller.
      char* remove_tail_name();

	// Return the given component in the hierarchical name. If the
	// idx is too large, return 0.
      const char*peek_name(unsigned idx) const;
      const char*peek_tail_name() const;

	// Return the number of components in the hierarchical
	// name. If this is a simple name, this will return 1.
      unsigned component_count() const;

      friend ostream& operator<< (ostream&, const hname_t&);

    private:
      union {
	    char**array_;
	    char* item_;
      };
      unsigned count_;

    private: // not implemented
      hname_t& operator= (const hname_t&);
};

extern bool operator <  (const hname_t&, const hname_t&);
extern bool operator == (const hname_t&, const hname_t&);

/*
 * $Log: HName.h,v $
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
