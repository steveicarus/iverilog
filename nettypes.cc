/*
 * Copyright (c) 2012-2016 Stephen Williams (steve@icarus.com)
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
# include  "netenum.h"
# include  <iostream>
# include  <cassert>

using namespace std;

ivl_type_s::~ivl_type_s()
{
}

/*
 * The derived class may override this to provide a more accurate
 * response.
 */
bool ivl_type_s::packed(void) const
{
      return false;
}

long ivl_type_s::packed_width(void) const
{
      return 1;
}

netranges_t ivl_type_s::slice_dimensions() const
{
      return netranges_t();
}

ivl_variable_type_t ivl_type_s::base_type() const
{
      return IVL_VT_NO_TYPE;
}

bool ivl_type_s::get_signed() const
{
      return false;
}

bool ivl_type_s::get_scalar() const
{
      return false;
}

bool ivl_type_s::type_compatible(ivl_type_t that) const
{
      if (this == that)
	    return true;

      return test_compatibility(that);
}

bool ivl_type_s::test_compatibility(ivl_type_t that) const
{
      return test_equivalence(that);
}

bool ivl_type_s::type_equivalent(ivl_type_t that) const
{
      if (this == that)
	    return true;

      return test_equivalence(that);
}

bool ivl_type_s::test_equivalence(ivl_type_t) const
{
	return false;
}

netarray_t::~netarray_t()
{
}

ivl_variable_type_t netarray_t::base_type() const
{
      return element_type_->base_type();
}

unsigned long netrange_width(const netranges_t &packed,
			     unsigned int base_width)
{
      unsigned wid = base_width;
      for (netranges_t::const_iterator cur = packed.begin()
		 ; cur != packed.end() ; ++cur) {
	    unsigned use_wid = cur->width();
	    wid *= use_wid;
      }

      return wid;
}

bool netrange_equivalent(const netranges_t &a, const netranges_t &b)
{
	if (a.size() != b.size())
		return false;

	for (size_t i = 0; i < a.size(); i++) {
		if (!a[i].equivalent(b[i]))
			return false;
	}

	return true;
}

/*
 * Given a netrange_t list (which represent packed dimensions) and a
 * prefix of calculated index values, calculate the canonical offset
 * and width of the resulting slice. In this case, the "sb" argument
 * is an extra index of the prefix.
 */
bool prefix_to_slice(const netranges_t&dims, const std::list<long>&prefix,
		     long sb, long&loff, unsigned long&lwid)
{
      assert(prefix.size() < dims.size());

	// Figure out the width of the slice, given the number of
	// prefix numbers there are. We don't need to look at the
	// actual values yet, but we do need to know how many there
	// are compared to the actual dimensions of the target. So do
	// this by multiplying the widths of the dims that are NOT
	// accounted for by the prefix or sb indices.
      size_t acc_wid = 1;
      netranges_t::const_iterator pcur = dims.end();
      for (size_t idx = prefix.size()+1 ; idx < dims.size() ; idx += 1) {
	    -- pcur;
	    acc_wid *= pcur->width();
      }

      lwid = acc_wid; // lwid is now the final slice width.

	// pcur is pointing to the dimension AFTER the dimension that
	// we have an index for, so step back one, then this will be
	// used with the sb index. Start accumulating in the acc_off
	// the offset into the n-dimensional vector.
      -- pcur;
      if (sb < pcur->get_msb() && sb < pcur->get_lsb())
	    return false;
      if (sb > pcur->get_msb() && sb > pcur->get_lsb())
	    return false;

      long acc_off = 0;
      if (pcur->get_msb() >= pcur->get_lsb())
	    acc_off += (sb - pcur->get_lsb()) * acc_wid;
      else
	    acc_off += (pcur->get_lsb() - sb) * acc_wid;

	// If there are no more prefix items, we are done.
      if (prefix.empty()) {
	    loff = acc_off;
	    return true;
      }

	// Now similarly go through the prefix numbers, working
	// through the dimensions until we run out. Accumulate a
	// growing slice width (acc_wid) that is used to calculate the
	// growing offset (acc_off).
      list<long>::const_iterator icur = prefix.end();
      do {
	    -- icur;
	    acc_wid *= pcur->width();
	    -- pcur;
	    if (pcur->get_msb() >= pcur->get_lsb())
		  acc_off += (*icur - pcur->get_lsb()) * acc_wid;
	    else
		  acc_off += (pcur->get_lsb() - *icur) * acc_wid;

      } while (icur != prefix.begin());

	// Got our final offset.
      loff = acc_off;

      return true;
}

bool packed_types_equivalent(ivl_type_t a, ivl_type_t b)
{
      if (!a->packed() || !b->packed())
	    return false;

      if (a->base_type() != b->base_type())
	    return false;

      if (a->packed_width() != b->packed_width())
	    return false;

      if (a->get_signed() != b->get_signed())
	    return false;

      // Special case, even though enums are packed they are not equivalent,
      // they are only assignment compatible to other packed types
      if (dynamic_cast<const netenum_t*>(b))
	    return false;

      return true;
}

bool packed_type_compatible(ivl_type_t type)
{
      if (type->packed())
	    return true;

      if (type->base_type() == IVL_VT_REAL)
	    return true;

      return false;
}
