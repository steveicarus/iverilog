#ifndef IVL_scope_H
#define IVL_scope_H
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

# include  <algorithm>
# include  <list>
# include  <map>
# include  "StringHeap.h"
# include  "entity.h"
# include  "expression.h"
# include  "vsignal.h"

class ActiveScope;
class Architecture;
class ComponentBase;
class Package;
class SubprogramHeader;
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
      explicit ScopeBase(const ActiveScope&ref);
      virtual ~ScopeBase() =0;

      const VType* find_type(perm_string by_name);
      virtual bool find_constant(perm_string by_name, const VType*&typ, Expression*&exp) const;
      Signal* find_signal(perm_string by_name) const;
      Variable* find_variable(perm_string by_name) const;
      virtual const InterfacePort* find_param(perm_string by_name) const;
      const InterfacePort* find_param_all(perm_string by_name) const;
      SubprogramHeader* find_subprogram(perm_string by_name) const;
	// Checks if a string is one of possible enum values. If so, the enum
	// type is returned, otherwise NULL.
      const VTypeEnum* is_enum_name(perm_string name) const;

	// Moves signals, variables and components from another scope to
	// this one. After the transfer new_* maps are cleared in the source scope.
      enum transfer_type_t { SIGNALS = 1, VARIABLES = 2, COMPONENTS = 4, ALL = 0xffff };
      void transfer_from(ScopeBase&ref, transfer_type_t what = ALL);

      inline void bind_subprogram(perm_string name, SubprogramHeader*obj)
      { map<perm_string, SubprogramHeader*>::iterator it;
        if((it = use_subprograms_.find(name)) != use_subprograms_.end() )
            use_subprograms_.erase(it);
        cur_subprograms_[name] = obj;
      }

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

	// The new_*_ maps below are managed only by the ActiveScope
	// derived class. When any scope is constructed from the
	// ActiveScope, the new_*_ and old_*_ maps are merged and
	// installed into the old_*_ maps. Thus, all other derived
	// classes should only use the old_*_ maps.

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
      std::map<perm_string,const VType*> use_types_; //imported types
      std::map<perm_string,const VType*> cur_types_; //current types
	// Constant declarations...
      struct const_t {
        ~const_t() {delete val;}
        const_t(const VType*t, Expression* v) : typ(t), val(v) {};

	    const VType*typ;
	    Expression*val;
      };
      std::map<perm_string, struct const_t*> use_constants_; //imported constants
      std::map<perm_string, struct const_t*> cur_constants_; //current constants

      std::map<perm_string, SubprogramHeader*> use_subprograms_; //imported
      std::map<perm_string, SubprogramHeader*> cur_subprograms_; //current

      std::list<const VTypeEnum*> use_enums_;

      void do_use_from(const ScopeBase*that);
};

class Scope : public ScopeBase {

    public:
      explicit Scope(const ActiveScope&ref);
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
      ActiveScope() : package_header_(0), context_entity_(0) { }
      ActiveScope(ActiveScope*par) : ScopeBase(*par), package_header_(0), context_entity_(0) { }

      ~ActiveScope() { }

      void set_package_header(Package*);

	// Pull items from "that" scope into "this" scope as is
	// defined by a "use" directive. The parser uses this method
	// to implement the "use <pkg>::*" directive.
      void use_from(const Scope*that) { do_use_from(that); }

	// This function returns true if the name is a vectorable
	// name. The parser uses this to distinguish between function
	// calls and array index operations.
      bool is_vector_name(perm_string name) const;

	// Locate the subprogram by name. The subprogram body uses
	// this to locate the subprogram declaration. Note that the
	// subprogram may be in a package header.
      SubprogramHeader* recall_subprogram(perm_string name) const;

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
        if((it = use_types_.find(name)) != use_types_.end() )
            use_types_.erase(it);
        cur_types_[name] = t;
      }

      inline void use_enum(const VTypeEnum* t)
      { use_enums_.push_back(t); }

      inline void use_name(perm_string name, const VType* t)
      { use_types_[name] = t; }

      void bind_name(perm_string name, const VType*obj, Expression*val)
      { map<perm_string, const_t*>::iterator it;
        if((it = use_constants_.find(name)) != use_constants_.end() )
            use_constants_.erase(it);
        cur_constants_[name] = new const_t(obj, val);
      }

      inline void bind_name(perm_string name, SubprogramHeader*obj)
      { map<perm_string, SubprogramHeader*>::iterator it;
        if((it = use_subprograms_.find(name)) != use_subprograms_.end() )
            use_subprograms_.erase(it);
        cur_subprograms_[name] = obj;
      }

      void bind(Entity*ent)
      { context_entity_ = ent; }

      void destroy_global_scope()
      {
          cleanup();
      }

	// Keep track of incomplete types until their proper
	// definition shows up.
      std::map<perm_string,VTypeDef*> incomplete_types;

    private:
	// If this is a package body, then there is a Package header
	// already declared.
      Package*package_header_;

      Entity*context_entity_;
};

#endif /* IVL_scope_H */
