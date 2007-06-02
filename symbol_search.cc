/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: symbol_search.cc,v 1.7 2007/06/02 03:42:13 steve Exp $"
#endif

# include  "netlist.h"
# include  "netmisc.h"
# include  <assert.h>


/*
 * Search for the hierarchical name.
 */
NetScope*symbol_search(Design*des, NetScope*scope, pform_name_t path,
		       NetNet*&net,
		       const NetExpr*&par,
		       NetEvent*&eve,
		       const NetExpr*&ex1, const NetExpr*&ex2)
{
      assert(scope);

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
	    assert(path_list.size() == path.size());
	    scope = des->find_scope(scope, path_list);
      }

      while (scope) {
	    if ( (net = scope->find_signal(key)) )
		  return scope;

	    if ( (eve = scope->find_event(key)) )
		  return scope;

	    if ( (par = scope->get_parameter(key, ex1, ex2)) )
		  return scope;

	    if (scope->type() == NetScope::MODULE)
		  scope = 0;
	    else
		  scope = scope->parent();
      }

      return 0;
}


/*
 * $Log: symbol_search.cc,v $
 * Revision 1.7  2007/06/02 03:42:13  steve
 *  Properly evaluate scope path expressions.
 *
 * Revision 1.6  2007/05/24 04:07:12  steve
 *  Rework the heirarchical identifier parse syntax and pform
 *  to handle more general combinations of heirarch and bit selects.
 *
 * Revision 1.5  2007/04/26 03:06:22  steve
 *  Rework hname_t to use perm_strings.
 *
 * Revision 1.4  2007/01/16 05:44:15  steve
 *  Major rework of array handling. Memories are replaced with the
 *  more general concept of arrays. The NetMemory and NetEMemory
 *  classes are removed from the ivl core program, and the IVL_LPM_RAM
 *  lpm type is removed from the ivl_target API.
 *
 * Revision 1.3  2005/11/27 05:56:20  steve
 *  Handle bit select of parameter with ranges.
 *
 * Revision 1.2  2005/07/11 16:56:51  steve
 *  Remove NetVariable and ivl_variable_t structures.
 *
 * Revision 1.1  2003/09/19 03:30:05  steve
 *  Fix name search in elab_lval.
 *
 */

