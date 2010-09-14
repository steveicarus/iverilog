/*
 * Copyright (c) 2003-2010 Stephen Williams (steve@icarus.com)
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

# include  "netlist.h"
# include  "netmisc.h"
# include  <cassert>


/*
 * Search for the hierarchical name.
 */
NetScope*symbol_search(const LineInfo*li, Design*des, NetScope*scope,
                       pform_name_t path,
		       NetNet*&net,
		       const NetExpr*&par,
		       NetEvent*&eve,
		       const NetExpr*&ex1, const NetExpr*&ex2)
{
      assert(scope);
      bool hier_path = false;

	/* Get the tail name of the object we are looking for. */
      perm_string key = peek_tail_name(path);
      path.pop_back();

	/* Initialize output argument to cleared. */
      net = 0;
      par = 0;
      eve = 0;

	/* If the path has a scope part, then search for the specified
	   scope that we are supposed to search. */
      if (! path.empty()) {
	    list<hname_t> path_list = eval_scope_path(des, scope, path);
	    assert(path_list.size() <= path.size());

	      // If eval_scope_path returns a short list, then some
	      // part of the scope was not found. Abort.
	    if (path_list.size() < path.size())
		  return 0;

	    scope = des->find_scope(scope, path_list);

            if (scope && scope->is_auto() && li) {
                  cerr << li->get_fileline() << ": error: Hierarchical "
                        "reference to automatically allocated item "
                        "`" << key << "' in path `" << path << "'" << endl;
                  des->errors += 1;
            }

	    hier_path = true;
      }

      while (scope) {
	    if ( (net = scope->find_signal(key)) )
		  return scope;

	    if ( (eve = scope->find_event(key)) )
		  return scope;

	    if ( (par = scope->get_parameter(key, ex1, ex2)) )
		  return scope;

	      /* We can't look up if we are at the enclosing module scope
	       * or if a hierarchical path was given. */
	    if ((scope->type() == NetScope::MODULE) || hier_path)
		  scope = 0;
	    else
		  scope = scope->parent();
      }

      return 0;
}
