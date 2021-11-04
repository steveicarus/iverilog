/*
 * Copyright (c) 2013-2021 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  "vtype.h"
# include  "expression.h"

using namespace std;

int VType::elaborate(Entity*, ScopeBase*) const
{
      return 0;
}

int VTypeArray::elaborate(Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      errors += etype_->elaborate(ent, scope);

      for (vector<range_t>::const_iterator cur = ranges_.begin()
		 ; cur != ranges_.end() ; ++ cur) {

	    Expression*tmp = cur->msb();
	    if (tmp) errors += tmp->elaborate_expr(ent, scope, 0);

	    tmp = cur->lsb();
	    if (tmp) errors += tmp->elaborate_expr(ent, scope, 0);
      }

      return errors;
}

int VTypeRangeExpr::elaborate(Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      errors += base_->elaborate(ent, scope);
      errors += start_->elaborate_expr(ent, scope, 0);
      errors += end_->elaborate_expr(ent, scope, 0);

      return errors;
}
