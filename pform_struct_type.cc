/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
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

# include  "pform.h"
# include  "parse_misc.h"
# include  "ivl_assert.h"

using namespace std;

ivl_variable_type_t struct_type_t::figure_packed_base_type(void) const
{
      if (! packed_flag)
	    return IVL_VT_NO_TYPE;

      if (members.get() == 0)
	    return IVL_VT_NO_TYPE;

      ivl_variable_type_t base_type = IVL_VT_BOOL;

      ivl_assert(*this, members.get());
      for (list<struct_member_t*>::iterator cur = members->begin()
		 ; cur != members->end() ; ++ cur) {

	    struct_member_t*tmp = *cur;

	    ivl_variable_type_t tmp_type = IVL_VT_NO_TYPE;
	    if (tmp->type.get())
		  tmp_type = tmp->type->figure_packed_base_type();

	    if (tmp_type == IVL_VT_BOOL) {
		  continue;
	    }

	    if (tmp_type == IVL_VT_LOGIC) {
		  base_type = IVL_VT_LOGIC;
		  continue;
	    }

	      // Oh no! Member is not a packable type!
	    return IVL_VT_NO_TYPE;
      }

      return base_type;
}
