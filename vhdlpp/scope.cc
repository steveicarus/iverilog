/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  "scope.h"
# include  "package.h"
# include  "subprogram.h"
# include  "entity.h"
# include  "std_funcs.h"
# include  "std_types.h"
# include  <algorithm>
# include  <iostream>
# include  <iterator>
# include  <cassert>

using namespace std;

/*
 * If the merge_flag is passed in, then the new scope is a merge of
 * the parent scopes. This brings in all of the parent scopes into the
 * "old_*_" variables. This clears up the "new_*_" variables to
 * accumulate new scope values.
 */
ScopeBase::ScopeBase(const ActiveScope&ref)
: use_constants_(ref.use_constants_), cur_constants_(ref.cur_constants_)
{
    merge(ref.old_signals_.begin(), ref.old_signals_.end(),
          ref.new_signals_.begin(), ref.new_signals_.end(),
          insert_iterator<map<perm_string, Signal*> >(
              old_signals_, old_signals_.end())
    );
    merge(ref.old_variables_.begin(), ref.old_variables_.end(),
          ref.new_variables_.begin(), ref.new_variables_.end(),
          insert_iterator<map<perm_string, Variable*> >(
              old_variables_, old_variables_.end())
    );
    merge(ref.old_components_.begin(), ref.old_components_.end(),
          ref.new_components_.begin(), ref.new_components_.end(),
          insert_iterator<map<perm_string, ComponentBase*> >(
              old_components_, old_components_.end())
    );
    use_types_ = ref.use_types_;
    cur_types_ = ref.cur_types_;

    use_subprograms_ = ref.use_subprograms_;
    cur_subprograms_ = ref.cur_subprograms_;

    use_enums_ = ref.use_enums_;

      // This constructor is invoked when the parser is finished with
      // an active scope and is making the actual scope. At this point
      // we know that "this" is the parent scope for the subprograms,
      // so set it now.
    for (map<perm_string,SubprogramHeader*>::iterator cur = cur_subprograms_.begin()
	       ; cur != cur_subprograms_.end(); ++cur) {
	cur->second->set_parent(this);
    }
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
    delete_all(cur_types_);
    delete_all(cur_constants_);
    delete_all(cur_subprograms_);
}

const VType*ScopeBase::find_type(perm_string by_name)
{
      map<perm_string,const VType*>::const_iterator cur = cur_types_.find(by_name);
      if (cur == cur_types_.end()) {
        cur = use_types_.find(by_name);
        if (cur == use_types_.end())
          return 0;
        else
          return cur->second;
      } else
	    return cur->second;
}

bool ScopeBase::find_constant(perm_string by_name, const VType*&typ, Expression*&exp) const
{
      typ = NULL;
      exp = NULL;

      map<perm_string,struct const_t*>::const_iterator cur = cur_constants_.find(by_name);
      if (cur == cur_constants_.end()) {
        cur = use_constants_.find(by_name);
        if (cur == use_constants_.end())
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

      return false;
}

Signal* ScopeBase::find_signal(perm_string by_name) const
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

Variable* ScopeBase::find_variable(perm_string by_name) const
{
      map<perm_string,Variable*>::const_iterator cur = new_variables_.find(by_name);
      if (cur == new_variables_.end()) {
	    cur = old_variables_.find(by_name);
	    if (cur == old_variables_.end())
		  return 0;
	    else
		  return cur->second;
      } else {
	    return cur->second;
      }
}

const InterfacePort* ScopeBase::find_param(perm_string) const
{
      return NULL;
}

const InterfacePort* ScopeBase::find_param_all(perm_string by_name) const
{
      for(map<perm_string,SubprogramHeader*>::const_iterator it = use_subprograms_.begin();
              it != use_subprograms_.end(); ++it) {
          if(const InterfacePort*port = it->second->find_param(by_name))
              return port;
      }

      for(map<perm_string,SubprogramHeader*>::const_iterator it = cur_subprograms_.begin();
              it != cur_subprograms_.end(); ++it) {
          if(const InterfacePort*port = it->second->find_param(by_name))
              return port;
      }

      return NULL;
}

SubprogramHeader* ScopeBase::find_subprogram(perm_string name) const
{
      map<perm_string,SubprogramHeader*>::const_iterator cur;

      cur = cur_subprograms_.find(name);
      if (cur != cur_subprograms_.end())
	    return cur->second;

      cur = use_subprograms_.find(name);
      if (cur != use_subprograms_.end())
	  return cur->second;

      return find_std_subprogram(name);
}

const VTypeEnum* ScopeBase::is_enum_name(perm_string name) const
{
    for(list<const VTypeEnum*>::const_iterator it = use_enums_.begin();
            it != use_enums_.end(); ++it) {
        if((*it)->has_name(name))
            return *it;
    }

    return find_std_enum_name(name);
}

/*
 * This method is only used by the ActiveScope derived class to import
 * definition from another scope.
 */
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

      for (map<perm_string,SubprogramHeader*>::const_iterator cur = that->cur_subprograms_.begin()
		 ; cur != that->cur_subprograms_.end() ; ++ cur) {
	    if (cur->second == 0)
		  continue;
	    use_subprograms_[cur->first] = cur->second;
      }


      for (map<perm_string,const VType*>::const_iterator cur = that->cur_types_.begin()
		 ; cur != that->cur_types_.end() ; ++ cur) {
	    if (cur->second == 0)
		  continue;
	    use_types_[cur->first] = cur->second;
      }

      for (map<perm_string,const_t*>::const_iterator cur = that->cur_constants_.begin()
		 ; cur != that->cur_constants_.end() ; ++ cur) {
	    use_constants_[cur->first] = cur->second;
      }

      use_enums_ = that->use_enums_;
}

void ScopeBase::transfer_from(ScopeBase&ref, transfer_type_t what)
{
    if(what & SIGNALS) {
        std::copy(ref.new_signals_.begin(), ref.new_signals_.end(),
            insert_iterator<map<perm_string, Signal*> >(
                new_signals_, new_signals_.end())
        );
        ref.new_signals_.clear();
    }

    if(what & VARIABLES) {
        std::copy(ref.new_variables_.begin(), ref.new_variables_.end(),
            insert_iterator<map<perm_string, Variable*> >(
                new_variables_, new_variables_.end())
        );
        ref.new_variables_.clear();
    }

    if(what & COMPONENTS) {
        std::copy(ref.new_components_.begin(), ref.new_components_.end(),
            insert_iterator<map<perm_string, ComponentBase*> >(
                new_components_, new_components_.end())
        );
        ref.new_components_.clear();
    }
}

void ActiveScope::set_package_header(Package*pkg)
{
      assert(package_header_ == 0);
      package_header_ = pkg;
}

SubprogramHeader* ActiveScope::recall_subprogram(perm_string name) const
{
      if (SubprogramHeader*tmp = find_subprogram(name))
	    return tmp;

      if (package_header_)
	    return package_header_->find_subprogram(name);

      return 0;
}

bool ActiveScope::is_vector_name(perm_string name) const
{
      if (find_signal(name))
	    return true;
      if (find_variable(name))
	    return true;

      const VType*dummy_type;
      Expression*dummy_exp;
      if (find_constant(name, dummy_type, dummy_exp))
	    return true;

      if (context_entity_ && context_entity_->find_port(name))
	    return true;

      return false;
}

Scope::Scope(const ActiveScope&ref)
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
