#ifndef IVL_PScope_H
#define IVL_PScope_H
/*
 * Copyright (c) 2008-2014 Stephen Williams (steve@icarus.com)
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

# include  "LineInfo.h"
# include  "StringHeap.h"
# include  "pform_types.h"
# include  "ivl_target.h"
# include  <map>
# include  <set>
# include  <vector>

class PEvent;
class PExpr;
class PFunction;
class PPackage;
class AProcess;
class PProcess;
class PClass;
class PTask;
class PWire;

class Design;
class NetScope;

/*
 * The PScope class is a base representation of an object that
 * represents lexical scope. For example, a module, a function/task, a
 * named block is derived from a PScope.
 *
 * NOTE: This is not the same concept as the "scope" of an elaborated
 * hierarchy. That is represented by NetScope objects after elaboration.
 */

class LexicalScope {

    public:
      explicit LexicalScope(LexicalScope*parent) : parent_(parent) { }
	// A virtual destructor is so that dynamic_cast can work.
      virtual ~LexicalScope() { }

      struct range_t {
	      // True if this is an exclude
	    bool exclude_flag;
	      // lower bound
	      // If low_open_flag is false and low_expr=0, then use -inf
	    bool low_open_flag;
	    PExpr*low_expr;
	      // upper bound
	      // If high_open_flag is false and high_expr=0, then use +inf
	    bool high_open_flag;
	    PExpr*high_expr;
	      // Next range description in list
	    struct range_t*next;
      };

	/* The scope has parameters that are evaluated when the scope
	   is elaborated. During parsing, I put the parameters into
	   this map. */
      struct param_expr_t : public LineInfo {
	    param_expr_t() : type(IVL_VT_NO_TYPE), msb(0), lsb(0), signed_flag(false), expr(0), range(0) { }
	      // Type information
	    ivl_variable_type_t type;
	    PExpr*msb;
	    PExpr*lsb;
	    bool signed_flag;
	      // Value expression
	    PExpr*expr;
	      // If there are range constraints, list them here
	    range_t*range;
      };
      map<perm_string,param_expr_t>parameters;
      map<perm_string,param_expr_t>localparams;

	// Defined types in the scope.
      map<perm_string,data_type_t*>typedefs;

	// Named events in the scope.
      map<perm_string,PEvent*>events;

	// Symbols that are imported. Bind the imported name to the
	// package from which the name is imported.
      std::map<perm_string,PPackage*>imports;

	// Nets and variables (wires) in the scope
      map<perm_string,PWire*>wires;
      PWire* wires_find(perm_string name);

        // Genvars in the scope. These will only be present in module
        // scopes, but are listed here to allow them to be found when
        // creating implicit nets.
      map<perm_string,LineInfo*> genvars;

	// Behaviors (processes) in this scope
      list<PProcess*> behaviors;
      list<AProcess*> analog_behaviors;

	// Enumeration sets.
      std::set<enum_type_t*> enum_sets;

      LexicalScope* parent_scope() const { return parent_; }

    protected:
      void dump_typedefs_(ostream&out, unsigned indent) const;

      void dump_parameters_(ostream&out, unsigned indent) const;

      void dump_localparams_(ostream&out, unsigned indent) const;

      void dump_enumerations_(ostream&out, unsigned indent) const;

      void dump_events_(ostream&out, unsigned indent) const;

      void dump_wires_(ostream&out, unsigned indent) const;

    private:
      LexicalScope*parent_;
};

class PScope : public LexicalScope {

    public:
	// When created, a scope has a name and a parent. The name is
	// the name of the definition. For example, if this is a
	// module declaration, the name is the name after the "module"
	// keyword, and if this is a task scope, the name is the task
	// name. The parent is the lexical parent of this scope. Since
	// modules do not nest in Verilog, the parent must be nil for
	// modules. Scopes for tasks and functions point to their
	// containing module.
      PScope(perm_string name, LexicalScope*parent);
      PScope(perm_string name);
      virtual ~PScope();

      perm_string pscope_name() const { return name_; }

    protected:
      bool elaborate_sig_wires_(Design*des, NetScope*scope) const;

      bool elaborate_behaviors_(Design*des, NetScope*scope) const;

    private:
      perm_string name_;
};

/*
 * Some scopes can carry definitions. These include Modules and PClass
 * scopes. These derive from PScopeExtra so that they hold the maps of
 * extra definitions.
 */
class PScopeExtra : public PScope {

    public:
      PScopeExtra(perm_string, LexicalScope*parent);
      PScopeExtra(perm_string);
      ~PScopeExtra();

	/* Task definitions within this module */
      std::map<perm_string,PTask*> tasks;
      std::map<perm_string,PFunction*> funcs;
	/* class definitions within this module. */
      std::map<perm_string,PClass*> classes;
	/* This is the lexical order of the classes, and is used by
	   elaboration to choose an elaboration order. */
      std::vector<PClass*> classes_lexical;

    protected:
      void dump_classes_(ostream&out, unsigned indent) const;
      void dump_tasks_(ostream&out, unsigned indent) const;
      void dump_funcs_(ostream&out, unsigned indent) const;
};

#endif /* IVL_PScope_H */
