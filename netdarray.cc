/*
 * Copyright (c) 2012-2020 Stephen Williams (steve@icarus.com)
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

# include  "netdarray.h"
# include  "netqueue.h"
# include  <iostream>

using namespace std;

netdarray_t::netdarray_t(ivl_type_t vec)
: netarray_t(vec)
{
}

netdarray_t::~netdarray_t()
{
}

ivl_variable_type_t netdarray_t::base_type(void) const
{
      return IVL_VT_DARRAY;
}

bool netdarray_t::test_equivalence(ivl_type_t that) const
{
      // Queues and dynamic arrays are not equivalent, so check for the base
      // type to make sure both are either dynamic array or queue.
      if (base_type() != that->base_type())
	    return false;

      return test_compatibility(that);
}

bool netdarray_t::test_compatibility(ivl_type_t that) const
{
      // This will match both queues and dynamic arrays
      const netdarray_t *that_da = dynamic_cast<const netdarray_t*>(that);
      if (!that_da)
	    return false;

      return element_type()->type_equivalent(that_da->element_type());
}
