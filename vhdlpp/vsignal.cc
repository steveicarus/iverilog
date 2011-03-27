/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "vsignal.h"
# include  "vtype.h"
# include  <iostream>

using namespace std;

Signal::Signal(perm_string nam, const VType*typ)
: name_(nam), type_(typ)
{
}

Signal::~Signal()
{
}

int Signal::emit(ostream&out, Entity*, Architecture*)
{
      int errors = 0;

      VType::decl_t decl;
      type_->elaborate(decl);
      errors += decl.emit(out, name_);
      return errors;
}
