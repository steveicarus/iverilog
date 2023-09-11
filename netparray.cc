/*
 * Copyright (c) 2012 Picture Elements, Inc.
 *    Stephen Williams (steve@icarus.com)
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

# include  "netparray.h"

using namespace std;

netsarray_t::~netsarray_t()
{
}

netparray_t::~netparray_t()
{
}

/*
 * The packed width of a packed array is the packed width of the
 * element times the dimension width of the array itself.
 */

bool netparray_t::packed(void) const
{
      return true;
}

long netparray_t::packed_width(void) const
{
      return netrange_width(static_dimensions(),
			    element_type()->packed_width());
}

netranges_t netparray_t::slice_dimensions() const
{
      const netranges_t&packed_dims = static_dimensions();

      netranges_t elem_dims = element_type()->slice_dimensions();

      netranges_t res (packed_dims.size() + elem_dims.size());

      for (size_t idx = 0 ; idx < packed_dims.size() ; idx += 1)
	    res[idx] = packed_dims[idx];
      for (size_t idx = 0 ; idx < elem_dims.size() ; idx += 1)
	    res[idx+packed_dims.size()] = elem_dims[idx];

      return res;
}

bool netparray_t::test_compatibility(ivl_type_t that) const
{
      return packed_type_compatible(that);
}

bool netparray_t::test_equivalence(ivl_type_t that) const
{
      return packed_types_equivalent(this, that);
}

netuarray_t::~netuarray_t()
{
}

netranges_t netuarray_t::slice_dimensions() const
{
      return static_dimensions();
}

bool netuarray_t::test_equivalence(ivl_type_t that) const
{
      const netuarray_t *that_a = dynamic_cast<const netuarray_t *>(that);
      if (!that_a)
	    return false;

      if (!netrange_equivalent(static_dimensions(), that_a->static_dimensions()))
	    return false;

      return element_type()->type_equivalent(that_a->element_type());
}
