/*
 * Copyright (c) 2011-2016 Stephen Williams (steve@icarus.com)
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
# include  "compiler.h"
# include  <algorithm>
# include  <iostream>
# include  <iterator>
# include  <cstdio>
# include  <cstring>
# include  <cassert>
# include  <StringHeap.h>

using namespace std;

static int scope_counter = 0;

ScopeBase::ScopeBase(const ActiveScope&ref)
: old_signals_(ref.old_signals_), new_signals_(ref.new_signals_),
    old_variables_(ref.old_variables_), new_variables_(ref.new_variables_),
    old_components_(ref.old_components_), new_components_(ref.new_components_),
    use_types_(ref.use_types_), cur_types_(ref.cur_types_),
    use_constants_(ref.use_constants_), cur_constants_(ref.cur_constants_),
    use_subprograms_(ref.use_subprograms_), cur_subprograms_(ref.cur_subprograms_),
    scopes_(ref.scopes_), use_enums_(ref.use_enums_),
    initializers_(ref.initializers_), finalizers_(ref.finalizers_),
    package_header_(ref.package_header_), name_(ref.name_)
{
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
    delete_all(new_variables_);
    delete_all(new_components_);
    delete_all(cur_types_);
    delete_all(cur_constants_);
    for (map<perm_string,SubHeaderList>::iterator cur = cur_subprograms_.begin()
                ; cur != cur_subprograms_.end() ; ++cur) {
        delete_all(cur->second);
    }
}

ScopeBase*ScopeBase::find_scope(perm_string name) const
{
    map<perm_string, ScopeBase*>::const_iterator it = scopes_.find(name);

    if(it != scopes_.end())
        return it->second;

    return NULL;
}

const VType*ScopeBase::find_type(perm_string by_name)
{
      map<perm_string,const VType*>::const_iterator cur = cur_types_.find(by_name);
      if (cur == cur_types_.end()) {
        cur = use_types_.find(by_name);
        if (cur == use_types_.end())
          return NULL;     // nothing found
      }

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
          return false;     // nothing found
      }

      typ = cur->second->typ;
      exp = cur->second->val;
      return true;
}

Signal* ScopeBase::find_signal(perm_string by_name) const
{
      map<perm_string,Signal*>::const_iterator cur = new_signals_.find(by_name);
      if (cur == new_signals_.end()) {
        cur = old_signals_.find(by_name);
        if (cur == old_signals_.end())
            return NULL;        // nothing found
      }

      return cur->second;
}

Variable* ScopeBase::find_variable(perm_string by_name) const
{
      map<perm_string,Variable*>::const_iterator cur = new_variables_.find(by_name);
      if (cur == new_variables_.end()) {
	    cur = old_variables_.find(by_name);
	    if (cur == old_variables_.end())
		return 0;     // nothing found
      }

      return cur->second;
}

const InterfacePort* ScopeBase::find_param(perm_string) const
{
      return NULL;
}

const InterfacePort* ScopeBase::find_param_all(perm_string by_name) const
{
      for(map<perm_string,SubHeaderList>::const_iterator cur = use_subprograms_.begin();
              cur != use_subprograms_.end(); ++cur) {
            const SubHeaderList& subp_list = cur->second;

            for(SubHeaderList::const_iterator it = subp_list.begin();
                        it != subp_list.end(); ++it) {
                if(const InterfacePort*port = (*it)->find_param(by_name))
                    return port;
            }
      }

      for(map<perm_string,SubHeaderList>::const_iterator cur = cur_subprograms_.begin();
              cur != cur_subprograms_.end(); ++cur) {
            const SubHeaderList& subp_list = cur->second;

            for(SubHeaderList::const_iterator it = subp_list.begin();
                        it != subp_list.end(); ++it) {
                if(const InterfacePort*port = (*it)->find_param(by_name))
                    return port;
            }
      }

      return NULL;
}

SubHeaderList ScopeBase::find_subprogram(perm_string name) const
{
      map<perm_string,SubHeaderList>::const_iterator cur;

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

      for (map<perm_string,SubHeaderList>::const_iterator cur = that->cur_subprograms_.begin()
		 ; cur != that->cur_subprograms_.end() ; ++ cur) {
	    if (cur->second.empty())
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

SubprogramHeader*ScopeBase::match_subprogram(perm_string name,
                                             const list<const VType*>*params) const
{
    int req_param_count = params ? params->size() : 0;

    // Find all subprograms with matching name
    SubHeaderList l = find_std_subprogram(name);
    map<perm_string,SubHeaderList>::const_iterator cur;

    cur = use_subprograms_.find(name);
    if (cur != use_subprograms_.end())
        copy(cur->second.begin(), cur->second.end(),
                front_insert_iterator<SubHeaderList>(l));

    cur = cur_subprograms_.find(name);
    if(cur != cur_subprograms_.end())
        copy(cur->second.begin(), cur->second.end(),
                front_insert_iterator<SubHeaderList>(l));

    // Find the matching one
    for(SubHeaderList::iterator it = l.begin(); it != l.end(); ++it) {
        SubprogramHeader*subp = *it;

        if(req_param_count != subp->param_count())
            continue;

        // Do not check the return type here, it might depend on the arguments

        if(params) {
            list<const VType*>::const_iterator p = params->begin();
            bool ok = true;

            for(int i = 0; i < req_param_count; ++i) {
                const VType*param_type = subp->peek_param_type(i);

                if(*p && param_type && !param_type->type_match(*p)) {
                    ok = false;
                    break;
                }

                ++p;
            }

            if(!ok)
                continue;   // check another function
        }

        // Yay, we have a match!
        return subp;
    }

    return NULL;
}

void ScopeBase::generate_name()
{
    char buf[64];

    // Generate a name for the scope
    snprintf(buf, sizeof(buf), "__scope_%d", scope_counter++);
    name_ = gen_strings.make(buf);
}

SubprogramHeader* ActiveScope::recall_subprogram(const SubprogramHeader*subp) const
{
      list<const VType*> arg_types;
      SubprogramHeader*tmp;

      for(int i = 0; i < subp->param_count(); ++i)
          arg_types.push_back(subp->peek_param_type(i));

      if ((tmp = match_subprogram(subp->name(), &arg_types))) {
            assert(!tmp->body());
            return tmp;
      }

      if (package_header_) {
            tmp = package_header_->match_subprogram(subp->name(), &arg_types);
            assert(!tmp || !tmp->body());
            return tmp;
      }

      return NULL;
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

ActiveScope::ActiveScope(const ActiveScope*par)
: ScopeBase(*par), context_entity_(par->context_entity_)
{
    generate_name();

    // Move all the objects available in higher level scopes to use*/old* maps.
    // This way we can store the new items in now empty cur*/new* maps.
    merge(par->old_signals_.begin(), par->old_signals_.end(),
          par->new_signals_.begin(), par->new_signals_.end(),
          insert_iterator<map<perm_string, Signal*> >(
              old_signals_, old_signals_.end())
    );
    merge(par->old_variables_.begin(), par->old_variables_.end(),
          par->new_variables_.begin(), par->new_variables_.end(),
          insert_iterator<map<perm_string, Variable*> >(
              old_variables_, old_variables_.end())
    );
    merge(par->old_components_.begin(), par->old_components_.end(),
          par->new_components_.begin(), par->new_components_.end(),
          insert_iterator<map<perm_string, ComponentBase*> >(
              old_components_, old_components_.end())
    );
    merge(par->use_types_.begin(), par->use_types_.end(),
          par->cur_types_.begin(), par->cur_types_.end(),
          insert_iterator<map<perm_string, const VType*> >(
              use_types_, use_types_.end())
    );
    merge(par->use_subprograms_.begin(), par->use_subprograms_.end(),
          par->cur_subprograms_.begin(), par->cur_subprograms_.end(),
          insert_iterator<map<perm_string, SubHeaderList> >(
              use_subprograms_, use_subprograms_.end())
    );
}
