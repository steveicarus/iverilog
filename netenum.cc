/*
 * Copyright (c) 2010 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "netenum.h"
# include  <cassert>

netenum_t::netenum_t(ivl_variable_type_t btype, bool signed_flag, long msb, long lsb)
: base_type_(btype), signed_flag_(signed_flag), msb_(msb), lsb_(lsb)
{
}

netenum_t::~netenum_t()
{
}

bool netenum_t::insert_name(perm_string name, const verinum&val)
{
      std::pair<std::map<perm_string,verinum>::iterator, bool> res;

      assert(val.has_len() && val.len() == (msb_-lsb_+1));

      res = names_map_.insert( make_pair(name,val) );
	// Only add the name to the list if it is not there already.
      if (res.second) names_.push_back(name);

      return res.second;
}

netenum_t::iterator netenum_t::find_name(perm_string name) const
{
      return names_map_.find(name);
}

netenum_t::iterator netenum_t::end_name() const
{
      return names_map_.end();
}

netenum_t::iterator netenum_t::first_name() const
{
      return names_map_.find(names_.front());
}

netenum_t::iterator netenum_t::last_name() const
{
      return names_map_.find(names_.back());
}
