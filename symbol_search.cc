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
#ident "$Id: symbol_search.cc,v 1.1 2003/09/19 03:30:05 steve Exp $"
#endif

# include  "netlist.h"
# include  <assert.h>


/*
 * Search for the hierarchical name.
 */
NetScope*symbol_search(Design*des, NetScope*scope, hname_t path,
		       NetNet*&net,
		       NetMemory*&mem,
		       NetVariable*&var,
		       const NetExpr*&par,
		       NetEvent*&eve)
{
      assert(scope);

	/* Get the tail name of the object we are looking for. */
      char*key = path.remove_tail_name();

	/* Initialize output argument to cleared. */
      net = 0;
      mem = 0;
      var = 0;
      par = 0;
      eve = 0;

	/* If the path has a scope part, then search for the specified
	   scope that we are supposed to search. */
      if (path.peek_name(0))
	    scope = des->find_scope(scope, path);

      while (scope) {
	    if ( (net = scope->find_signal(key)) ) {
		  delete key;
		  return scope;
	    }

	    if ( (mem = scope->find_memory(key)) ) {
		  delete key;
		  return scope;
	    }

	    if ( (var = scope->find_variable(key)) ) {
		  delete key;
		  return scope;
	    }

	    if ( (eve = scope->find_event(key)) ) {
		  delete key;
		  return scope;
	    }

	    if ( (par = scope->get_parameter(key)) ) {
		  delete key;
		  return scope;
	    }

	    if (scope->type() == NetScope::MODULE)
		  scope = 0;
	    else
		  scope = scope->parent();
      }

      delete key;
      return 0;
}

/*
 * $Log: symbol_search.cc,v $
 * Revision 1.1  2003/09/19 03:30:05  steve
 *  Fix name search in elab_lval.
 *
 */

