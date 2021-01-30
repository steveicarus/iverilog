/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
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

# include  "nettypes.h"
# include  "netscalar.h"
# include  "netvector.h"

ivl_type_t make_ivl_type(ivl_variable_type_t vt,
			 const std::vector<netrange_t>&packed_dimensions,
			 bool signed_flag, bool isint_flag)
{
      netvector_t*vec;

      if (packed_dimensions.size() > 0) {
	    vec = new netvector_t(packed_dimensions, vt);
	    vec->set_signed(signed_flag);
	    return vec;
      }

      switch (vt) {
	  case IVL_VT_REAL:
	    return &netreal_t::type_real;
	  case IVL_VT_STRING:
	    return &netstring_t::type_string;
	  default:
	    vec = new netvector_t(packed_dimensions, vt);
	    vec->set_signed(signed_flag);
	    vec->set_isint(isint_flag);
	    return vec;
      }
}
