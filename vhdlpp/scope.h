#ifndef __scope_H
#define __scope_H
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

# include  <algorithm>
# include  <list>
# include  <map>
# include  "StringHeap.h"
# include "entity.h"
# include "expression.h"
# include "vsignal.h"

class Architecture;
class ComponentBase;
class VType;

template<typename T>
struct delete_object{
    void operator()(T* item) { delete item; }
};

template<typename T>
struct delete_pair_second{
    void operator()(pair<perm_string, T*> item){ delete item.second; }
};

class ScopeBase {

    public:
      ScopeBase() { }
      explicit ScopeBase(const ScopeBase&ref);
      virtual ~ScopeBase() =0;

      const VType* find_type(perm_string by_name);
      bool find_constant(perm_string by_name, const VType*&typ, Expression*&exp);
      Signal* find_signal(perm_string by_name) const;
      Variable* find_variable(perm_string by_name) const;

    protected:
      void cleanup();

      //containers' cleaning helper functions
      template<typename T> void delete_all(list<T*>& c)
      {
          for_each(c.begin(), c.end(), ::delete_object<T>());
      }
      template<typename T> void delete_all(map<perm_string, T*>& c)
      {
          for_each(c.begin(), c.end(), ::delete_pair_second<T>());
      }

	// Signal declarations...
      std::map<perm_string,Signal*> old_signals_; //previous scopes
      std::map<perm_string,Signal*> new_signals_; //current scope
	// Variable declarations...
      std::map<perm_string,Variable*> old_variables_; //previous scopes
      std::map<perm_string,Variable*> new_variables_; //current scope
	// Component declarations...
      std::map<perm_string,ComponentBase*> old_components_; //previous scopes
      std::map<perm_string,ComponentBase*> new_components_; //current scope
	// Type declarations...
      std::map<perm_string,const VType*> old_types_; //previous scopes
      std::map<perm_string,const VType*> new_types_; //current scope
	// Constant declarations...
      struct const_t {
        ~const_t() {delete typ; delete val;}
        const_t(const VType*t, Expression* v) : typ(t), val(v) {};

	    const VType*typ;
	    Expression*val;
      };
      std::map<perm_string, struct const_t*> old_constants_; //previous scopes
      std::map<perm_string, struct const_t*> new_constants_; //current scope

      void do_use_from(const ScopeBase*that);
};

class Scope : public ScopeBase {

    public:
      Scope(const ScopeBase&ref);
      ~Scope();

      ComponentBase* find_component(perm_string by_name);

    public:
      void dump_scope(ostream&out) const;

    protected:
	// Helper method for emitting signals in the scope.
      int emit_signals(ostream&out, Entity*ent, Architecture*arc);
      int emit_variables(ostream&out, Entity*ent, Architecture*arc);
};

/*
 * The active_scope object accumulates declarations for the scope that
 * is in the process of being parsed. When the declarations are over,
 * they are transferred over to the specific scope. The ActiveScope is
 * used by the parser to build up scopes.
 */
class ActiveScope : public ScopeBase {

    public:
      ActiveScope() { }
      ActiveScope(ActiveScope*par) : ScopeBase(*par), context_entity_(0) { }

      ~ActiveScope() { }

      void use_from(const ScopeBase*that) { do_use_from(that); }

	// This function returns true if the name is a vectorable
	// name. The parser uses this to distinguish between function
	// calls and array index operations.
      bool is_vector_name(perm_string name) const;

      /* All bind_name function check if the given name was present
       * in previous scopes. If it is found, it is erased (but the pointer
       * is not freed), in order to implement name shadowing. The pointer
       * be freed only in the scope where the object was defined. This is
       * done in ScopeBase::cleanup() function .*/

      void bind_name(perm_string name, Signal*obj)
      { map<perm_string, Signal*>::iterator it;
        if((it = old_signals_.find(name)) != old_signals_.end() )
            old_signals_.erase(it);
        new_signals_[name] = obj;
      }

      void bind_name(perm_string name, Variable*obj)
      { map<perm_string, Variable*>::iterator it;
        if((it = old_variables_.find(name)) != old_variables_.end() )
            old_variables_.erase(it);
        new_variables_[name] = obj;
      }

      void bind_name(perm_string name, ComponentBase*obj)
      { map<perm_string, ComponentBase*>::iterator it;
        if((it = old_components_.find(name)) != old_components_.end() )
            old_components_.erase(it);
        new_components_[name] = obj;
      }

      void bind_name(perm_string name, const VType* t)
      { map<perm_string, const VType*>::iterator it;
        if((it = old_types_.find(name)) != old_types_.end() )
            old_types_.erase(it);
        new_types_[name] = t;
      }

      void bind_name(perm_string name, const VType*obj, Expression*val)
      { map<perm_string, const_t*>::iterator it;
        if((it = old_constants_.find(name)) != old_constants_.end() )
            old_constants_.erase(it);
        new_constants_[name] = new const_t(obj, val);
      }

      void bind(Entity*ent)
      { context_entity_ = ent; }

      void destroy_global_scope()
      {
          cleanup();
      }

    private:
      Entity*context_entity_;
};

#endif
