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

# include  "netclass.h"
# include  <iostream>

using namespace std;

netclass_t::netclass_t(perm_string name)
: name_(name)
{
}

netclass_t::~netclass_t()
{
}

bool netclass_t::set_property(perm_string pname, ivl_type_s*ptype)
{
      map<perm_string,ivl_type_s*>::const_iterator cur;
      cur = properties_.find(pname);
      if (cur != properties_.end())
	    return false;

      properties_[pname] = ptype;
      property_table_.push_back(pname);
      return true;
}

ivl_variable_type_t netclass_t::base_type() const
{
      return IVL_VT_CLASS;
}

const ivl_type_s* netclass_t::get_property(perm_string pname) const
{
      map<perm_string,ivl_type_s*>::const_iterator cur;
      cur = properties_.find(pname);
      if (cur == properties_.end())
	    return 0;
      else
	    return cur->second;
}

const char*netclass_t::get_prop_name(size_t idx) const
{
      assert(idx < property_table_.size());
      return property_table_[idx];
}

ivl_type_t netclass_t::get_prop_type(size_t idx) const
{
      assert(idx < property_table_.size());
      return get_property(property_table_[idx]);
}
