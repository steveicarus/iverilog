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

#endif
