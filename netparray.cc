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
      long cur_width = element_type()->packed_width();

      for (vector<netrange_t>::const_iterator cur = static_dimensions().begin()
		 ; cur != static_dimensions().end() ; ++cur) {
	    cur_width *= cur->width();
      }

      return cur_width;
}

vector<netrange_t> netparray_t::slice_dimensions() const
{
      const vector<netrange_t>&packed_dims = static_dimensions();

      vector<netrange_t> elem_dims = element_type()->slice_dimensions();

      vector<netrange_t> res (packed_dims.size() + elem_dims.size());

      for (size_t idx = 0 ; idx < packed_dims.size() ; idx += 1)
	    res[idx] = packed_dims[idx];
      for (size_t idx = 0 ; idx < elem_dims.size() ; idx += 1)
	    res[idx+packed_dims.size()] = elem_dims[idx];

      return res;
}

netuarray_t::~netuarray_t()
{
}

vector<netrange_t> netuarray_t::slice_dimensions() const
{
      return static_dimensions();
}
