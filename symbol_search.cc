/*
 * Copyright (c) 2003-2024 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012 / Stephen Williams (steve@icarus.com)
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

# include  "netlist.h"
# include  "netclass.h"
# include  "netparray.h"
# include  "netmisc.h"
# include  "compiler.h"
# include  "PPackage.h"
# include  "PWire.h"
# include  "ivl_assert.h"

using namespace std;

/*
 * Search for the hierarchical name. The path may have multiple components. If
 * that's the case, then recursively pull the path apart until we find the
 * first item in the path, look that up, and work our way up. In most cases,
 * the path will be a string of scopes, with an object at the end. But if we
 * find an object before the end, then the tail will have to be figured out by
 * the initial caller.
 */

bool symbol_search(const LineInfo*li, Design*des, NetScope*scope,
		   pform_name_t path, unsigned lexical_pos,
		   struct symbol_search_results*res,
		   NetScope*start_scope, bool prefix_scope)
{
      assert(scope);

      if (debug_elaborate) {
	    cerr << li->get_fileline() << ": symbol_search: "
		 << "scope: " << scope_path(scope) << endl;
	    cerr << li->get_fileline() << ": symbol_search: "
		 << "path: " << path << endl;
	    if (start_scope)
		  cerr << li->get_fileline() << ": symbol_search: "
		       << "start_scope: " << scope_path(start_scope) << endl;
      }

      assert(li);
      ivl_assert(*li, ! path.empty());
      name_component_t path_tail = path.back();
      path.pop_back();

      // If this is a recursive call, then we need to know that so
      // that we can enable the search for scopes. Set the
      // recurse_flag to true if this is a recurse.
      if (start_scope==0)
	    start_scope = scope;

      // If there are components ahead of the tail, symbol_search
      // recursively. Ideally, the result is a scope that we search
      // for the tail key, but there are other special cases as well.
      if (! path.empty()) {
	    bool flag = symbol_search(li, des, scope, path, lexical_pos,
				      res, start_scope, prefix_scope);
	    if (! flag)
		  return false;

	    // The prefix is found to be something besides a scope. Put the
	    // tail into the path_tail of the result, and return success. The
	    // caller needs to deal with that tail bit. Note that the
	    // path_tail is a single item, but we might have been called
	    // recursively, so the complete tail will be built up as we unwind.
	    if (res->is_found() && !res->is_scope()) {
		  if (!path_tail.empty())
			res->path_tail.push_back(path_tail);
		  return true;
	    }

	    // The prefix is found to be a scope, so switch to that
	    // scope, set the hier_path to turn off upwards searches,
	    // and continue our search for the tail.
	    if (res->is_scope()) {
		  scope = res->scope;
		  prefix_scope = true;

		  if (debug_scopes || debug_elaborate) {
			cerr << li->get_fileline() << ": symbol_search: "
			     << "Prefix scope " << scope_path(scope) << endl;
		  }

		  if (scope->is_auto()) {
			cerr << li->get_fileline() << ": error: Hierarchical "
			      "reference to automatically allocated item "
			      "`" << path_tail.name << "' in path `" << path << "'" << endl;
			des->errors += 1;
		  }

	    } else {
		  // Prefix is present, but is NOT a scope. Fail! Actually, this
		  // should not happen, since this is the "not found" case, and we
		  // should have returned already.
		  ivl_assert(*li, 0);
		  return false;
	    }
      }

      bool passed_module_boundary = false;

      // At this point, we've stripped right-most components until the search
      // found the scope part of the path, or there is no scope part of the
      // path. For example, if the path in was s1.s2.x, we found the scope
      // s1.s2, res->is_scope() is true, and path_tail is x. We look for x
      // now. The preceeding code set prefix_scope=true to ease our test below.
      //
      // If the input was x (without prefixes) then we don't know if x is a
      // scope or item. In this case, res->is_found() is false and we may need
      // to scan upwards to find the scope or item.
      while (scope) {
	    if (debug_scopes || debug_elaborate) {
		  cerr << li->get_fileline() << ": symbol_search: "
		       << "Looking for " << path_tail
		       << " in scope " << scope_path(scope)
		       << " prefix_scope=" << prefix_scope << endl;
	    }
            if (scope->genvar_tmp.str() && path_tail.name == scope->genvar_tmp)
                  return false;

	    // These items cannot be seen outside the bounding module where
	    // the search starts. But we continue searching up because scope
	    // names can match. For example:
	    //
	    //    module top;
	    //        int not_ok;
	    //        dut foo(...);
	    //    endmodule
	    //    module dut;
	    //        ... not_ok; // <-- Should NOT match.
	    //        ... top.not_ok; // Matches.
	    //    endmodule
	    if (!passed_module_boundary) {
		  // Special case `super` keyword. Return the `this` object, but
		  // with the type of the base class.
		  if (path_tail.name == "#") {
			if (NetNet *net = scope->find_signal(perm_string::literal(THIS_TOKEN))) {
			      const netclass_t *class_type = dynamic_cast<const netclass_t*>(net->net_type());
			      path.push_back(path_tail);
			      res->scope = scope;
			      res->net = net;
			      res->type = class_type->get_super();
			      res->path_head = path;
			      return true;
			}
			return false;
		  }

		  if (NetNet*net = scope->find_signal(path_tail.name)) {
			if (prefix_scope || (net->lexical_pos() <= lexical_pos)) {
			      path.push_back(path_tail);
			      res->scope = scope;
			      res->net = net;
			      res->type = net->net_type();
			      res->path_head = path;
			      return true;
			} else if (!res->decl_after_use) {
			      res->decl_after_use = net;
			}
		  }

		  if (NetEvent*eve = scope->find_event(path_tail.name)) {
			if (prefix_scope || (eve->lexical_pos() <= lexical_pos)) {
			      path.push_back(path_tail);
			      res->scope = scope;
			      res->eve = eve;
			      res->path_head = path;
			      return true;
			} else if (!res->decl_after_use) {
			      res->decl_after_use = eve;
			}
		  }

		  if (const NetExpr*par = scope->get_parameter(des, path_tail.name, res->type)) {
			if (prefix_scope || (scope->get_parameter_lexical_pos(path_tail.name) <= lexical_pos)) {
			      path.push_back(path_tail);
			      res->scope = scope;
			      res->par_val = par;
			      res->path_head = path;
			      return true;
			} else if (!res->decl_after_use) {
			      res->decl_after_use = par;
			}
		  }

		    // Static items are just normal signals and are found above.
		  if (scope->type() == NetScope::CLASS) {
			const netclass_t *clsnet = scope->class_def();
			int pidx = clsnet->property_idx_from_name(path_tail.name);
			if (pidx >= 0) {
			      // This is a class property being accessed in a
			      // class method. Return `this` for the net and the
			      // property name for the path tail.
			      NetScope *scope_method = find_method_containing_scope(*li, start_scope);
			      ivl_assert(*li, scope_method);
			      res->net = scope_method->find_signal(perm_string::literal(THIS_TOKEN));
			      ivl_assert(*li, res->net);
			      res->scope = scope;
			      ivl_assert(*li, path.empty());
			      res->path_head.push_back(name_component_t(perm_string::literal(THIS_TOKEN)));
			      res->path_tail.push_front(path_tail);
			      res->type = clsnet;
			      return true;
			}
		  }

		    // Finally check the rare case of a signal that hasn't
		    // been elaborated yet.
		  if (PWire*wire = scope->find_signal_placeholder(path_tail.name)) {
			if (prefix_scope || (wire->lexical_pos() <= lexical_pos)) {
			      NetNet*net = wire->elaborate_sig(des, scope);
			      if (!net)
				    return false;
			      path.push_back(path_tail);
			      res->scope = scope;
			      res->net = net;
			      res->type = net->net_type();
			      res->path_head = path;
			      return true;
			}
		  }
	    }

	    // Could not find an object. Maybe this is a child scope name? If
	    // so, evaluate the path components to find the exact scope this
	    // refers to. This item might be:
	    //     <scope>.s
	    //     <scope>.s[n]
	    // etc. The scope->child_byname tests if the name exists, and if
	    // it does, the eval_path_component() evaluates any [n]
	    // expressions to constants to generate an hname_t object for a
	    // more complete scope name search. Note that the index
	    // expressions for scope names must be constant.
	    if (scope->child_byname(path_tail.name)) {
		  bool flag = false;
		  hname_t path_item = eval_path_component(des, start_scope, path_tail, flag);
		  if (flag) {
			cerr << li->get_fileline() << ": XXXXX: Errors evaluating scope index" << endl;
		  } else if (NetScope*chld = scope->child(path_item)) {
			path.push_back(path_tail);
			res->scope = chld;
			res->path_head = path;
			return true;
		  }
	    }

	    // Don't scan up if we are searching within a prefixed scope.
	    if (prefix_scope)
		  break;

	    // Imports are not visible through hierachical names
	    if (NetScope*import_scope = scope->find_import(des, path_tail.name)) {
		  scope = import_scope;
		  continue;
	    }

	    // Special case: We can match the module name of a parent
	    // module. That means if the current scope is a module of type
	    // "mod", then "mod" matches the current scope. This is fairly
	    // obscure, but looks like this:
	    //
	    //  module foo;
	    //    reg x;
	    //    ... foo.x; // This matches x in myself.
	    //  endmodule
	    //
	    // This feature recurses, so code in subscopes of foo can refer to
	    // foo by the name "foo" as well. In general, anything within
	    // "foo" can use the name "foo" to reference it.
	    if (scope->type()==NetScope::MODULE && scope->module_name()==path_tail.name) {
		  path.push_back(path_tail);
		  res->scope = scope;
		  res->path_head = path;
		  return true;
	    }

	    // If there is no prefix, then we are free to scan upwards looking
	    // for a scope name. Note that only scopes can be searched for up
	    // past module boundaries. To handle that, set a flag to indicate
	    // that we passed a module boundary on the way up.
	    if (scope->type()==NetScope::MODULE && !scope->nested_module())
		  passed_module_boundary = true;

	    scope = scope->parent();

	    // Last chance - try the compilation unit. Note that modules may
	    // reference nets/variables in the compilation unit, even if they
	    // cannot reference variables in containing scope.
	    //
	    //    int ok = 1;
	    //    module top;
	    //        int not_ok = 2;
	    //        dut foo();
	    //    endmodule
	    //
	    //    module dut;
	    //        ... = ok; // This reference is OK
	    //        ... = not_ok; // This reference is NOT OK.
	    //    endmodule
	    if (scope == 0 && start_scope != 0) {
		  scope = start_scope->unit();
		  start_scope = 0;
		  passed_module_boundary = false;
	    }
      }


      // Last chance: this is a single name, so it might be the name
      // of a root scope. Ask the design if this is a root
      // scope. This is only possible if there is no prefix.
      if (prefix_scope==false) {
	    hname_t path_item (path_tail.name);
	    scope = des->find_scope(path_item);
	    if (scope) {
		  path.push_back(path_tail);
		  res->scope = scope;
		  res->path_head = path;
		  return true;
	    }
      }

      return false;
}

bool symbol_search(const LineInfo *li, Design *des, NetScope *scope,
		   const pform_scoped_name_t &path, unsigned lexical_pos,
		   struct symbol_search_results *res)
{
      NetScope *search_scope = scope;
      bool prefix_scope = false;

      if (path.package) {
	    search_scope = des->find_package(path.package->pscope_name());
	    if (!search_scope)
		  return false;
	    prefix_scope = true;
      }

      return symbol_search(li, des, search_scope, path.name, lexical_pos,
			   res, search_scope, prefix_scope);
}
