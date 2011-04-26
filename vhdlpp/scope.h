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

# include  <list>
# include  <map>
# include  "StringHeap.h"

class Architecture;
class ComponentBase;
class Entity;
class Expression;
class Signal;
class VType;

class ScopeBase {

    public:
      ScopeBase() { }
      explicit ScopeBase(const ScopeBase&ref);
      virtual ~ScopeBase() =0;

      const VType* find_type(perm_string by_name);
      bool find_constant(perm_string by_name, const VType*&typ, Expression*&exp);
    protected:
	// Signal declarations...
      std::map<perm_string,Signal*> signals_;
	// Component declarations...
      std::map<perm_string,ComponentBase*> components_;
	// Type declarations...
      std::map<perm_string,const VType*> types_;
	// Constant declarations...
      struct const_t {
	    const VType*typ;
	    Expression*val;
      };
      std::map<perm_string, struct const_t> constants_;

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
      ActiveScope(ActiveScope*par) : ScopeBase(*par) { }

      ~ActiveScope() { }

      void use_from(const ScopeBase*that) { do_use_from(that); }

      void bind_name(perm_string name, Signal*obj)
      { signals_[name] = obj; }

      void bind_name(perm_string name, ComponentBase*obj)
      { components_[name] = obj; }

      void bind_name(perm_string name, const VType*obj)
      { types_[name] = obj; }

      void bind_name(perm_string name, const VType*obj, Expression*val)
      {
	    const_t&tmp = constants_[name];
	    tmp.typ = obj;
	    tmp.val = val;
      }
};

#endif
