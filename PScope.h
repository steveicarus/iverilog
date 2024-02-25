#ifndef IVL_PScope_H
#define IVL_PScope_H
/*
 * Copyright (c) 2008-2024 Stephen Williams (steve@icarus.com)
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

# include  "PNamedItem.h"
# include  "StringHeap.h"
# include  "pform_types.h"
# include  "ivl_target.h"
# include  <map>
# include  <set>
# include  <unordered_set>
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
class Statement;
class PCallTask;

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
      enum lifetime_t { INHERITED, STATIC, AUTOMATIC };

      explicit LexicalScope(LexicalScope*parent)
        : default_lifetime(INHERITED), has_parameter_port_list(false),
	  generate_counter(0), parent_(parent) { }
	// A virtual destructor is so that dynamic_cast can work.
      virtual ~LexicalScope() { }

      lifetime_t default_lifetime;

	// Symbols that are defined or declared in this scope.
      std::map<perm_string,PNamedItem*>local_symbols;

	// Symbols that are explicitly imported. This contains the package where
	// the symbol has been decelared. When using exports, this might not be
	// the same as the package where it has been imported from.
      std::map<perm_string,PPackage*>explicit_imports;
        // Symbols that are explicitly imported. This contains the set of
	// packages from which the symbol has been imported. When using exports
	// the same identifier can be imported via multiple packages.
      std::map<perm_string,std::unordered_set<PPackage*>> explicit_imports_from;

	// Packages that are wildcard imported. When identifiers from
	// these packages are referenced, they will be added to the
	// explicit imports (IEEE 1800-2012 26.3).
      std::list<PPackage*>potential_imports;

	// A task or function call may reference a task or function defined
	// later in the scope. So here we stash the potential imports for
	// task and function calls. They will be added to the explicit
	// imports if we don't find a local definition.
      std::map<perm_string,PPackage*>possible_imports;

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
      struct param_expr_t : public PNamedItem {
            inline param_expr_t() : data_type(0), expr(0), range(0),
				    local_flag(false), overridable(true) { }
	      // Type information.
	    data_type_t*data_type;
	      // Value expression
	    PExpr*expr;
	      // If there are range constraints, list them here
	    range_t*range;
	      // Whether it is a local parameter
	    bool local_flag;
	      // Whether the parameter can be overridden
	    bool overridable;
	      // Whether the parameter is a type parameter
	    bool type_flag = false;
	      // The lexical position of the declaration
	    unsigned lexical_pos = 0;

	    SymbolType symbol_type() const;
      };
      std::map<perm_string,param_expr_t*>parameters;
      bool has_parameter_port_list;

	// Defined types in the scope.
      typedef std::map<perm_string,typedef_t*> typedef_map_t;
      typedef_map_t typedefs;

	// Named events in the scope.
      std::map<perm_string,PEvent*>events;

	// Nets and variables (wires) in the scope
      std::map<perm_string,PWire*>wires;
      PWire* wires_find(perm_string name);

        // Genvars in the scope. These will only be present in module
        // scopes, but are listed here to allow them to be found when
        // creating implicit nets.
      std::map<perm_string,LineInfo*> genvars;

	// Variable initializations in this scope
      std::vector<Statement*> var_inits;

	// Behaviors (processes) in this scope
      std::list<PProcess*> behaviors;
      std::list<AProcess*> analog_behaviors;

	// The elaboration tasks in this scope
      std::list<PCallTask*> elab_tasks;

	// Enumeration sets.
      std::vector<enum_type_t*> enum_sets;

        // A count of the generate constructs in this scope. This is
        // used to automatically name unnamed generate blocks, as
        // specified in the LRM.
      unsigned generate_counter;

      LexicalScope* parent_scope() const { return parent_; }

      virtual bool var_init_needs_explicit_lifetime() const;

    protected:
      void dump_typedefs_(std::ostream&out, unsigned indent) const;

      void dump_parameters_(std::ostream&out, unsigned indent) const;

      void dump_enumerations_(std::ostream&out, unsigned indent) const;

      void dump_events_(std::ostream&out, unsigned indent) const;

      void dump_wires_(std::ostream&out, unsigned indent) const;

      void dump_var_inits_(std::ostream&out, unsigned indent) const;

      bool elaborate_var_inits_(Design*des, NetScope*scope) const;

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
      explicit PScope(perm_string name, LexicalScope*parent =0);
      virtual ~PScope();

      perm_string pscope_name() const { return name_; }

	/* These are the timescale for this scope. The value is
	   set by the `timescale directive or, in SystemVerilog,
	   by timeunit and timeprecision statements. */
      int time_unit, time_precision;

	/* Flags used to support warnings about timescales. */
      bool time_unit_is_default;
      bool time_prec_is_default;

      bool has_explicit_timescale() const {
	     return !(time_unit_is_default || time_prec_is_default);
	   }

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
      explicit PScopeExtra(perm_string, LexicalScope*parent =0);
      ~PScopeExtra();

	/* Task definitions within this module */
      std::map<perm_string,PTask*> tasks;
      std::map<perm_string,PFunction*> funcs;
	/* Class definitions within this module. */
      std::map<perm_string,PClass*> classes;
	/* This is the lexical order of the classes, and is used by
	   elaboration to choose an elaboration order. */
      std::vector<PClass*> classes_lexical;

	/* Flags used to support warnings about timescales. */
      bool time_unit_is_local;
      bool time_prec_is_local;

    protected:
      void dump_classes_(std::ostream&out, unsigned indent) const;
      void dump_tasks_(std::ostream&out, unsigned indent) const;
      void dump_funcs_(std::ostream&out, unsigned indent) const;
};

#endif /* IVL_PScope_H */
