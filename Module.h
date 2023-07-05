#ifndef IVL_Module_H
#define IVL_Module_H
/*
 * Copyright (c) 1998-2021 Stephen Williams (steve@icarus.com)
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


# include  <list>
# include  <map>
# include  <vector>
# include  <utility>
# include  "StringHeap.h"
# include  "HName.h"
# include  "named.h"
# include  "PScope.h"
# include  "PNamedItem.h"
# include  "netlist.h"
# include  "pform_types.h"
class PExpr;
class PEIdent;
class PGate;
class PGenerate;
class PModport;
class PSpecPath;
class PTimingCheck;
class PTask;
class PFunction;
class PWire;
class PProcess;
class Design;
class NetScope;

/*
 * A module is a named container and scope. A module holds a bunch of
 * semantic quantities such as wires and gates. The module is
 * therefore the handle for grasping the described circuit.
 *
 * SystemVerilog introduces program blocks and interfaces. These have
 * much in common with modules, so the Module class is used to represent
 * these containers as well.
 */

class Module : public PScopeExtra, public PNamedItem {

	/* The module ports are in general a vector of port_t
	   objects. Each port has a name and an ordered list of
	   wires. The name is the means that the outside uses to
	   access the port, the wires are the internal connections to
	   the port. In SystemVerilog, input ports may also have a
	   default value. */
    public:
      struct port_t {
	    perm_string name;
	    std::vector<PEIdent*> expr;
	    PExpr*default_value;
      };

    public:
	/* The name passed here is the module name, not the instance
	   name. This name must be a permallocated string. */
      explicit Module(LexicalScope*parent, perm_string name);
      ~Module();

	/* Initially false. This is set to true if the module has been
	   declared as a library module. This makes the module
	   ineligible for being chosen as an implicit root. It has no
	   other effect. */
      bool library_flag;

      bool is_cell;

	/* This is true if the module represents a program block
	   instead of a module/cell. Program blocks have content
	   restrictions and slightly modify scheduling semantics. */
      bool program_block;

	/* This is true if the module represents a interface
	   instead of a module/cell. Interfaces have different
	   content restrictions and some extra allowed items. */
      bool is_interface;

      enum UCDriveType { UCD_NONE, UCD_PULL0, UCD_PULL1 };
      UCDriveType uc_drive;

	/* specparams are simpler than other parameters, in that they
	   can have a range, but not an explicit type. The restrictions
	   are enforced by the parser. */
      std::map<perm_string,param_expr_t*>specparams;

	/* The module also has defparam assignments which don't create
	   new parameters within the module, but may be used to set
	   values within this module (when instantiated) or in other
	   instantiated modules. */
      typedef std::pair<pform_name_t,PExpr*> named_expr_t;
      std::list<named_expr_t>defparms;
      static std::list<named_expr_t>user_defparms;

        /* Parameters may be overridden at instantiation time;
           the overrides do not contain explicit parameter names,
           but rather refer to parameters in the order they
           appear in the instantiated module.  Therefore a
           list of names in module-order is needed to pass from
           a parameter-index to its name. */
      std::list<perm_string> param_names;

	/* This is an array of port descriptors, which is in turn a
	   named array of PEident pointers. */
      std::vector<port_t*> ports;

      std::map<perm_string,PExpr*> attributes;

	/* The module has a list of generate schemes that appear in
	   the module definition. These are used at elaboration time. */
      std::list<PGenerate*> generate_schemes;

	/* Nested modules are placed here, and are not elaborated
	   unless they are instantiated, implicitly or explicitly. */
      std::map<perm_string,Module*> nested_modules;

	/* An interface can contain one or more named modport lists.
           The parser will ensure these don't appear in modules or
           program blocks. */
      std::map<perm_string,PModport*> modports;

	/* List for specify paths and timing checks */
      std::list<PSpecPath*> specify_paths;
      std::list<PTimingCheck*> timing_checks;

	// The mod_name() is the name of the module type.
      perm_string mod_name() const { return pscope_name(); }

      void add_gate(PGate*gate);

      unsigned port_count() const;
      const std::vector<PEIdent*>& get_port(unsigned idx) const;
      unsigned find_port(const char*name) const;

      // Return port name ("" for undeclared port)
      perm_string get_port_name(unsigned idx) const;

      PExpr* get_port_default_value(unsigned idx) const;

      PGate* get_gate(perm_string name);

      const std::list<PGate*>& get_gates() const;

      void dump(std::ostream&out) const;
      bool elaborate(Design*, NetScope*scope) const;

      typedef std::map<perm_string,PExpr*> replace_t;
      bool elaborate_scope(Design*, NetScope*scope, const replace_t&rep);

      bool elaborate_sig(Design*, NetScope*scope) const;

      SymbolType symbol_type() const;

      bool can_be_toplevel() const;

    private:
      void dump_specparams_(std::ostream&out, unsigned indent) const;
      void dump_timingchecks_(std::ostream&out, unsigned indent) const;
      std::list<PGate*> gates_;

    private: // Not implemented
      Module(const Module&);
      Module& operator= (const Module&);
};

#endif /* IVL_Module_H */
