#ifndef __PScope_H
#define __PScope_H
/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
# include  <map>

class PEvent;

/*
 * The PScope class is a base representation of an object that
 * represents some sort of compile-time scope. For example, a module,
 * a function/task, a named block is derived from a PScope.
 *
 * NOTE: This is note the same concept as the "scope" of an elaborated
 * hierarchy. That is represented by NetScope objects after elaboration.
 */
class PScope {

    public:
      PScope(perm_string name);
      ~PScope();

      perm_string pscope_name() const { return name_; }

	// Named events in the scope.
      map<perm_string,PEvent*>events;

    private:
      perm_string name_;
};

#endif
