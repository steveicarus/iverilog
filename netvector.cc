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
# include  "compiler.h"
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

netvector_t netvector_t::time_signed (IVL_VT_LOGIC, 63, 0, true);
netvector_t netvector_t::time_unsigned (IVL_VT_LOGIC, 63, 0, false);

static netvector_t* save_integer_type[2];
const netvector_t* netvector_t::integer_type(bool is_signed)
{
      if (save_integer_type[is_signed])
	    return save_integer_type[is_signed];

      save_integer_type[is_signed] = new netvector_t(IVL_VT_LOGIC, integer_width-1, 0, is_signed);
      save_integer_type[is_signed]->set_isint(true);
      return save_integer_type[is_signed];
}

//netvector_t netvector_t::scalar_bool (IVL_VT_BOOL);
netvector_t netvector_t::scalar_logic (IVL_VT_LOGIC);

netvector_t::netvector_t(ivl_variable_type_t type, long msb, long lsb, bool flag)
: type_(type), signed_(flag), isint_(false), implicit_(false)
{
      packed_dims_.push_back(netrange_t(msb,lsb));
}

netvector_t::netvector_t(ivl_variable_type_t type)
: type_(type), signed_(false), isint_(false), implicit_(false)
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

netranges_t netvector_t::slice_dimensions() const
{
      return packed_dims_;
}

bool netvector_t::test_compatibility(ivl_type_t that) const
{
      return packed_type_compatible(that);
}

bool netvector_t::test_equivalence(const ivl_type_t that) const
{
      return packed_types_equivalent(this, that);
}
