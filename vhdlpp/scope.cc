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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "scope.h"
# include  <iostream>

using namespace std;

ScopeBase::ScopeBase(const ScopeBase&ref)
{
      constants_ = ref.constants_;
      signals_ = ref.signals_;
      components_ = ref.components_;
      types_ = ref.types_;
}

ScopeBase::~ScopeBase()
{
}

const VType*ScopeBase::find_type(perm_string by_name)
{
      map<perm_string,const VType*>::const_iterator cur = types_.find(by_name);
      if (cur == types_.end())
	    return 0;
      else
	    return cur->second;
}

bool ScopeBase::find_constant(perm_string by_name, const VType*&typ, Expression*&exp)
{
      map<perm_string,const_t>::const_iterator cur = constants_.find(by_name);
      if (cur == constants_.end())
	    return false;

      typ = cur->second.typ;
      exp = cur->second.val;
      return true;
}

void ScopeBase::do_use_from(const ScopeBase*that)
{
      for (map<perm_string,ComponentBase*>::const_iterator cur = that->components_.begin()
		 ; cur != that->components_.end() ; ++ cur) {
	    if (cur->second == 0)
		  continue;
	    components_[cur->first] = cur->second;
      }

      for (map<perm_string,const VType*>::const_iterator cur = that->types_.begin()
		 ; cur != that->types_.end() ; ++ cur) {
	    if (cur->second == 0)
		  continue;
	    types_[cur->first] = cur->second;
      }

      for (map<perm_string,const_t>::const_iterator cur = that->constants_.begin()
		 ; cur != that->constants_.end() ; ++ cur) {
	    constants_[cur->first] = cur->second;
      }
}

Scope::Scope(const ScopeBase&ref)
: ScopeBase(ref)
{
}

Scope::~Scope()
{
}

ComponentBase* Scope::find_component(perm_string by_name)
{
      map<perm_string,ComponentBase*>::const_iterator cur = components_.find(by_name);
      if (cur == components_.end())
	    return 0;
      else
	    return cur->second;
}

Signal* Scope::find_signal(perm_string by_name)
{
      map<perm_string,Signal*>::const_iterator cur = signals_.find(by_name);
      if (cur == signals_.end())
	    return 0;
      else
	    return cur->second;
}
