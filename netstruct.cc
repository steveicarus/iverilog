/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "netstruct.h"
# include  <iostream>

using namespace std;

netstruct_t::netstruct_t()
: packed_(false)
{
}

netstruct_t::~netstruct_t()
{
}

void netstruct_t::packed(bool flag)
{
      packed_ = flag;
}

void netstruct_t::append_member(const netstruct_t::member_t&val)
{
      members_.push_back(val);
}

const netstruct_t::member_t* netstruct_t::packed_member(perm_string name, unsigned long&off) const
{
      unsigned long count_off = 0;
      for (size_t idx = members_.size() ; idx > 0 ; idx -= 1) {
	    if (members_[idx-1].name == name) {
		  off = count_off;
		  return &members_[idx-1];
	    }
	    count_off += members_[idx-1].width();
      }

      return 0;
}

long netstruct_t::packed_width(void) const
{
      if (! packed_)
	    return -1;

      long res = 0;
      for (size_t idx = 0 ; idx < members_.size() ; idx += 1)
	    res += members_[idx].width();

      return res;
}

ivl_variable_type_t netstruct_t::base_type() const
{
      if (! packed_)
	    return IVL_VT_NO_TYPE;

      for (size_t idx = 0 ;  idx < members_.size() ; idx += 1) {
	    if (members_[idx].data_type() != IVL_VT_BOOL)
		  return members_[idx].data_type();
      }

      return IVL_VT_BOOL;
}
