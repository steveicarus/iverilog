#ifndef __PEvent_H
#define __PEvent_H
/*
 * Copyright (c) 2000-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PEvent.h,v 1.9 2004/02/19 06:57:10 steve Exp $"
#endif

# include  "LineInfo.h"
# include  "StringHeap.h"
# include  <string>

class Design;
class NetScope;

/*
 * The PEvent class represents event objects. These are things that
 * are declared in Verilog as ``event foo;'' The name passed to the
 * constructor is the "foo" part of the declaration.
 */
class PEvent : public LineInfo {

    public:
	// The name is a perm-allocated string. It is the simple name
	// of the event, without any scope.
      explicit PEvent(perm_string name);
      ~PEvent();

      perm_string name() const;

      void elaborate_scope(Design*des, NetScope*scope) const;

    private:
      perm_string name_;

    private: // not implemented
      PEvent(const PEvent&);
      PEvent& operator= (const PEvent&);
};

/*
 * $Log: PEvent.h,v $
 * Revision 1.9  2004/02/19 06:57:10  steve
 *  Memory and Event names use perm_string.
 *
 * Revision 1.8  2003/03/01 06:25:30  steve
 *  Add the lex_strings string handler, and put
 *  scope names and system task/function names
 *  into this table. Also, permallocate event
 *  names from the beginning.
 *
 * Revision 1.7  2003/01/30 16:23:07  steve
 *  Spelling fixes.
 *
 * Revision 1.6  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.4  2001/01/16 02:44:18  steve
 *  Use the iosfwd header if available.
 *
 * Revision 1.3  2000/04/09 17:44:30  steve
 *  Catch event declarations during scope elaborate.
 *
 * Revision 1.2  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
 * Revision 1.1  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 */
#endif
