/*
 * Copyright (c) 2012-2013 Stephen Williams (steve@icarus.com)
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

# include  "netvector.h"
# include  <iostream>

using namespace std;

netvector_t netvector_t::atom2s64 (IVL_VT_BOOL, 63, 0, true);
netvector_t netvector_t::atom2u64 (IVL_VT_BOOL, 63, 0, false);
netvector_t netvector_t::atom2s32 (IVL_VT_BOOL, 31, 0, true);
netvector_t netvector_t::atom2u32 (IVL_VT_BOOL, 31, 0, false);
netvector_t netvector_t::atom2s16 (IVL_VT_BOOL, 15, 0, true);
netvector_t netvector_t::atom2u16 (IVL_VT_BOOL, 15, 0, false);
netvector_t netvector_t::atom2s8  (IVL_VT_BOOL,  7, 0, true);
netvector_t netvector_t::atom2u8  (IVL_VT_BOOL,  7, 0, false);

//netvector_t netvector_t::scalar_bool (IVL_VT_BOOL);
netvector_t netvector_t::scalar_logic (IVL_VT_LOGIC);

netvector_t::netvector_t(ivl_variable_type_t type, long msb, long lsb, bool flag)
: type_(type), signed_(flag), isint_(false), is_scalar_(false)
{
      packed_dims_.push_back(netrange_t(msb,lsb));
}

netvector_t::netvector_t(ivl_variable_type_t type)
: type_(type), signed_(false), isint_(false), is_scalar_(false)
{
}

netvector_t::~netvector_t()
{
}

ivl_variable_type_t netvector_t::base_type() const
{
      return type_;
}

/*
 * vectors are by definition packed.
 */
bool netvector_t::packed(void) const
{
      return true;
}

long netvector_t::packed_width() const
{
      return netrange_width(packed_dims_);
}

vector<netrange_t> netvector_t::slice_dimensions() const
{
      return packed_dims_;
}

bool netvector_t::test_compatibility(ivl_type_t that) const
{
      const netvector_t*that_st = dynamic_cast<const netvector_t*>(that);
      if (that_st == 0)
	    return false;

      if (type_ != that_st->type_)
	    return false;
      if (packed_dims_.size() != that_st->packed_dims_.size())
	    return false;

      for (size_t idx = 0 ; idx < packed_dims_.size() ; idx += 1) {
	    if (packed_dims_[idx] != that_st->packed_dims_[idx])
		  return false;
      }

      return true;
}
