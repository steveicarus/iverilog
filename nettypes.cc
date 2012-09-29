/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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
# include  <cassert>

using namespace std;

ivl_type_s::~ivl_type_s()
{
}

long ivl_type_s::packed_width(void) const
{
      return 1;
}

vector<netrange_t> ivl_type_s::slice_dimensions() const
{
      return vector<netrange_t>();
}

ivl_variable_type_t ivl_type_s::base_type() const
{
      return IVL_VT_NO_TYPE;
}

bool ivl_type_s::get_signed() const
{
      return false;
}

unsigned long netrange_width(const vector<netrange_t>&packed)
{
      unsigned wid = 1;
      for (vector<netrange_t>::const_iterator cur = packed.begin()
		 ; cur != packed.end() ; ++cur) {
	    unsigned use_wid = cur->width();
	    wid *= use_wid;
      }

      return wid;
}

/*
 * Given a netrange_t list (which represent packed dimensions) and a
 * prefix of calculated index values, calculate the canonical offset
 * and width of the resulting slice. In this case, the "sb" argument
 * is an extra index of the prefix.
 */
bool prefix_to_slice(const std::vector<netrange_t>&dims,
		     const std::list<long>&prefix, long sb,
		     long&loff, unsigned long&lwid)
{
      assert(prefix.size() < dims.size());

      size_t acc_wid = 1;
      vector<netrange_t>::const_iterator pcur = dims.end();
      for (size_t idx = prefix.size()+1 ; idx < dims.size() ; idx += 1) {
	    -- pcur;
	    acc_wid *= pcur->width();
      }

      lwid = acc_wid;

      -- pcur;
      if (sb < pcur->get_msb() && sb < pcur->get_lsb())
	    return false;
      if (sb > pcur->get_msb() && sb > pcur->get_lsb())
	    return false;

      long acc_off = 0;
      if (pcur->get_msb() >= pcur->get_lsb())
	    acc_off += (sb - pcur->get_lsb()) * acc_wid;
      else
	    acc_off += (sb - pcur->get_msb()) * acc_wid;

      if (prefix.empty()) {
	    loff = acc_off;
	    return true;
      }

      lwid *= pcur->width();

      list<long>::const_iterator icur = prefix.end();
      do {
	    -- pcur;
	    -- icur;
	    acc_wid *= pcur->width();
	    if (pcur->get_msb() >= pcur->get_lsb())
		  acc_off += (*icur - pcur->get_lsb()) * acc_wid;
	    else
		  acc_off += (*icur - pcur->get_msb()) * acc_wid;

      } while (icur != prefix.begin());

      loff = acc_off;

      return true;
}
