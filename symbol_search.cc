/*
 * Copyright (c) 2003-2012 Stephen Williams (steve@icarus.com)
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
# include  "netmisc.h"
# include  "ivl_assert.h"


/*
 * Search for the hierarchical name.
 */
struct symbol_search_results {
      inline symbol_search_results() {
	    scope = 0;
	    net = 0;
	    par_val = 0;
	    par_msb = 0;
	    par_lsb = 0;
	    eve = 0;
      }

      inline bool is_scope() const {
	    if (net) return false;
	    if (eve) return false;
	    if (par_val) return false;
	    if (scope) return true;
	    return false;
      }

	// Scope where symbol was located. This is set in all cases,
	// assuming the search succeeded.
      NetScope*scope;
	// If this was a net, the signal itself.
      NetNet*net;
	// If this was a parameter, the value expression and the
	// optional value dimensions.
      const NetExpr*par_val;
      const NetExpr*par_msb;
      const NetExpr*par_lsb;
	// If this is a named event, ...
      NetEvent*eve;
};

static bool symbol_search(const LineInfo*li, Design*des, NetScope*scope,
			  pform_name_t path, struct symbol_search_results*res,
			  NetScope*start_scope = 0)
{
      assert(scope);
      bool prefix_scope = false;
      bool recurse_flag = false;

      ivl_assert(*li, ! path.empty());
      name_component_t path_tail = path.back();
      path.pop_back();

	// If this is a recursive call, then we need to know that so
	// that we can enable the search for scopes. Set the
	// recurse_flag to true if this is a recurse.
      if (start_scope==0)
	    start_scope = scope;
      else
	    recurse_flag = true;

	// If there are components ahead of the tail, symbol_search
	// recursively. Ideally, the result is a scope that we search
	// for the tail key, but there are other special cases as well.
      if (! path.empty()) {
	    symbol_search_results recurse;
	    bool flag = symbol_search(li, des, scope, path, &recurse, start_scope);
	    if (! flag)
		  return false;

	      // The prefix is found to be a scope, so switch to that
	      // scope, set the hier_path to turn off upwards searches,
	      // and continue our search for the tail.
	    if (recurse.is_scope()) {
		  scope = recurse.scope;
		  prefix_scope = true;

		  if (scope->is_auto() && li) {
			cerr << li->get_fileline() << ": error: Hierarchical "
			      "reference to automatically allocated item "
			      "`" << path_tail.name << "' in path `" << path << "'" << endl;
			des->errors += 1;
		  }
	    } else {
		    // Prefix is present, but is NOT a scope. Fail!
		  return false;
	    }
      }

      while (scope) {
	    if (path_tail.name == "#") {
		  cerr << li->get_fileline() << ": sorry: "
		       << "Implicit class handle \"super\" not supported." << endl;
		  return false;
	    }

	    if (NetNet*net = scope->find_signal(path_tail.name)) {
		  res->scope = scope;
		  res->net = net;
		  return true;
	    }

	    if (NetEvent*eve = scope->find_event(path_tail.name)) {
		  res->scope = scope;
		  res->eve = eve;
		  return true;
	    }

	    if (const NetExpr*par = scope->get_parameter(des, path_tail.name, res->par_msb, res->par_lsb)) {
		  res->scope = scope;
		  res->par_val = par;
		  return true;
	    }

	    if (recurse_flag) {
		  bool flag = false;
		  hname_t path_item = eval_path_component(des, start_scope, path_tail, flag);
		  if (flag) {
			cerr << li->get_fileline() << ": XXXXX: Errors evaluating scope index" << endl;
		  } else if (NetScope*chld = des->find_scope(scope, path_item)) {
			res->scope = chld;
			return true;
		  }
	    }

	      // Don't scan up past a module boundary.
	    if (scope->type()==NetScope::MODULE && !scope->nested_module())
		  break;
	      // Don't scan up if we are searching within a prefixed scope.
	    if (prefix_scope)
		  break;

	    scope = scope->parent();
      }

	// Last chance: this is a single name, so it might be the name
	// of a root scope. Ask the design if this is a root
	// scope. This is only possible if there is no prefix.
      if (prefix_scope==false) {
	    hname_t path_item (path_tail.name);
	    scope = des->find_scope(path_item);
	    if (scope) {
		  res->scope = scope;
		  return true;
	    }
      }

      return false;
}

/*
 * Compatibility version. Remove me!
 */
NetScope*symbol_search(const LineInfo*li, Design*des, NetScope*scope,
                       pform_name_t path,
		       NetNet*&net,
		       const NetExpr*&par,
		       NetEvent*&eve,
		       const NetExpr*&ex1, const NetExpr*&ex2)
{
      symbol_search_results recurse;
      bool flag = symbol_search(li, des, scope, path, &recurse);
      net = recurse.net;
      par = recurse.par_val;
      ex1 = recurse.par_msb;
      ex2 = recurse.par_lsb;
      eve = recurse.eve;
      if (! flag) {
	    return 0;
      }

      if (recurse.is_scope())
	    return recurse.scope;

      return recurse.scope;
}
