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
# include  <algorithm>
# include  <iostream>
# include  <iterator>

using namespace std;

ScopeBase::ScopeBase(const ScopeBase&ref)
{
    merge(ref.old_constants_.begin(), ref.old_constants_.end(),
          ref.new_constants_.begin(), ref.new_constants_.end(),
          insert_iterator<map<perm_string, struct const_t*> >(
              old_constants_, old_constants_.end())
    );
    merge(ref.old_signals_.begin(), ref.old_signals_.end(),
          ref.new_signals_.begin(), ref.new_signals_.end(),
          insert_iterator<map<perm_string, Signal*> >(
              old_signals_, old_signals_.end())
    );
    merge(ref.old_components_.begin(), ref.old_components_.end(),
          ref.new_components_.begin(), ref.new_components_.end(),
          insert_iterator<map<perm_string, ComponentBase*> >(
              old_components_, old_components_.end())
    );
    merge(ref.old_types_.begin(), ref.old_types_.end(),
          ref.new_types_.begin(), ref.new_types_.end(),
          insert_iterator<map<perm_string, const VType*> >(
              old_types_, old_types_.end())
    );
}

ScopeBase::~ScopeBase()
{
    //freeing of member objects is performed by child classes
}

void ScopeBase::cleanup()
{
    /*
     * A parent scope is destroyed only if all child scopes
     * were previously destroyed. There for we can delete all
     * objects that were defined in this scope, leaving
     * objects from the other scopes untouched.
     */
    delete_all(new_signals_);
    delete_all(new_components_);
    delete_all(new_types_);
    delete_all(new_constants_);
}

const VType*ScopeBase::find_type(perm_string by_name)
{
      map<perm_string,const VType*>::const_iterator cur = new_types_.find(by_name);
      if (cur == new_types_.end()) {
        cur = old_types_.find(by_name);
        if (cur == old_types_.end())
          return 0;
        else
          return cur->second;
      } else
	    return cur->second;
}

bool ScopeBase::find_constant(perm_string by_name, const VType*&typ, Expression*&exp)
{
      map<perm_string,struct const_t*>::const_iterator cur = new_constants_.find(by_name);
      if (cur == new_constants_.end()) {
        cur = old_constants_.find(by_name);
        if (cur == old_constants_.end())
          return false;
        else {
          typ = cur->second->typ;
          exp = cur->second->val;
          return true;
        }
      } else {
        typ = cur->second->typ;
        exp = cur->second->val;
        return true;
      }
}

void ScopeBase::do_use_from(const ScopeBase*that)
{
      for (map<perm_string,ComponentBase*>::const_iterator cur = that->old_components_.begin()
		 ; cur != that->old_components_.end() ; ++ cur) {
	    if (cur->second == 0)
		  continue;
	    old_components_[cur->first] = cur->second;
      }
       for (map<perm_string,ComponentBase*>::const_iterator cur = that->new_components_.begin()
         ; cur != that->new_components_.end() ; ++ cur) {
        if (cur->second == 0)
          continue;
        old_components_[cur->first] = cur->second;
      }

      for (map<perm_string,const VType*>::const_iterator cur = that->old_types_.begin()
		 ; cur != that->old_types_.end() ; ++ cur) {
	    if (cur->second == 0)
		  continue;
	    old_types_[cur->first] = cur->second;
      }
      for (map<perm_string,const VType*>::const_iterator cur = that->new_types_.begin()
         ; cur != that->new_types_.end() ; ++ cur) {
        if (cur->second == 0)
          continue;
        old_types_[cur->first] = cur->second;
      }

      for (map<perm_string,const_t*>::const_iterator cur = that->old_constants_.begin()
		 ; cur != that->old_constants_.end() ; ++ cur) {
	    old_constants_[cur->first] = cur->second;
      }
      for (map<perm_string,const_t*>::const_iterator cur = that->new_constants_.begin()
         ; cur != that->new_constants_.end() ; ++ cur) {
        old_constants_[cur->first] = cur->second;
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
      map<perm_string,ComponentBase*>::const_iterator cur = new_components_.find(by_name);
      if (cur == new_components_.end()) {
        cur = old_components_.find(by_name);
        if (cur == old_components_.end())
            return 0;
        else
            return cur->second;
      } else
	    return cur->second;
}

Signal* Scope::find_signal(perm_string by_name)
{
      map<perm_string,Signal*>::const_iterator cur = new_signals_.find(by_name);
      if (cur == new_signals_.end()) {
        cur = old_signals_.find(by_name);
        if (cur == old_signals_.end())
            return 0;
        else
            return cur->second;
      } else {
	    return cur->second;
      }
}
