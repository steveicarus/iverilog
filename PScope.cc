/*
 * Copyright (c) 2008,2010 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "PScope.h"

PScope::PScope(perm_string n, LexicalScope*parent)
: LexicalScope(parent), name_(n)
{
}

PScope::PScope(perm_string n)
: LexicalScope(0), name_(n)
{
}

PScope::~PScope()
{
    for(map<perm_string, data_type_t*>::iterator it = typedefs.begin();
        it != typedefs.end(); ++it)
        delete it->second;
}

PWire* LexicalScope::wires_find(perm_string name)
{
      map<perm_string,PWire*>::const_iterator cur = wires.find(name);
      if (cur == wires.end())
	    return 0;
      else
	    return (*cur).second;
}

PScopeExtra::PScopeExtra(perm_string n, LexicalScope*parent)
: PScope(n, parent)
{
}

PScopeExtra::PScopeExtra(perm_string n)
: PScope(n)
{
}

PScopeExtra::~PScopeExtra()
{
}

