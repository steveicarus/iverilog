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
# include  "netlist.h"
# include  <iostream>

using namespace std;

netclass_t::netclass_t(perm_string name)
: name_(name), class_scope_(0)
{
}

netclass_t::~netclass_t()
{
}

bool netclass_t::set_property(perm_string pname, ivl_type_s*ptype)
{
      map<perm_string,size_t>::const_iterator cur;
      cur = properties_.find(pname);
      if (cur != properties_.end())
	    return false;

      prop_t tmp;
      tmp.name = pname;
      tmp.type = ptype;
      property_table_.push_back(tmp);

      properties_[pname] = property_table_.size()-1;
      return true;
}

void netclass_t::set_class_scope(NetScope*class_scope)
{
      assert(class_scope_ == 0);
      class_scope_ = class_scope;
}

ivl_variable_type_t netclass_t::base_type() const
{
      return IVL_VT_CLASS;
}

const ivl_type_s* netclass_t::get_property(perm_string pname) const
{
      map<perm_string,size_t>::const_iterator cur;
      cur = properties_.find(pname);
      if (cur == properties_.end())
	    return 0;

      assert(property_table_.size() > cur->second);
      return property_table_[cur->second].type;
}

int netclass_t::property_idx_from_name(perm_string pname) const
{
      map<perm_string,size_t>::const_iterator cur;
      cur = properties_.find(pname);
      if (cur == properties_.end())
	    return -1;

      return cur->second;
}

const char*netclass_t::get_prop_name(size_t idx) const
{
      assert(idx < property_table_.size());
      return property_table_[idx].name;
}

ivl_type_t netclass_t::get_prop_type(size_t idx) const
{
      assert(idx < property_table_.size());
      return property_table_[idx].type;
}

NetScope*netclass_t::method_from_name(perm_string name) const
{
      NetScope*task = class_scope_->child( hname_t(name) );
      if (task == 0) return 0;
      return task;

}
