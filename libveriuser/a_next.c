/* vi:sw=6
 * Copyright (c) 2003 Michael Ruff (mruff at chiaro.com)
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
#ident "$Id: a_next.c,v 1.1 2003/05/30 04:18:31 steve Exp $"
#endif

#include  <stdio.h>
#include  <acc_user.h>
#include  <vpi_user.h>
#include  "priv.h"

/*
 * acc_next and friends implemented using VPI
 */
handle acc_next(PLI_INT32 *type, handle scope, handle prev)
{
      vpiHandle hand;

      /* preserved state */
      static vpiHandle pscope = 0;
      static vpiHandle iter = 0;
      static vpiHandle rtn = 0;

      /* trace */
      if (pli_trace) {
	    int *ip;
	    fprintf(pli_trace, "acc_next(%p <", type);
	    for (ip = type; *ip; ip++) {
		  fprintf(pli_trace, "%s%d", ip != type ? "," : "", *ip);
	    }
	    fprintf(pli_trace, ">, %p", scope);
	    if (scope)
		  fprintf(pli_trace, " \"%s\"", vpi_get_str(vpiName, scope));
	    fprintf(pli_trace, ", %p", prev);
	    if (prev)
		  fprintf(pli_trace, " \"%s\"", vpi_get_str(vpiName, prev));
	    else
		  fprintf(pli_trace, ")");
	    fflush(pli_trace);
      }

      /* initialize or reset if detect user dain bramaged */
      if (!prev || !*type || scope != pscope || prev != rtn) {
	    /* if iterator still valid, free it */
	    if (iter) vpi_free_object(iter);

	    /* iterate across all things in specified scope */
	    if (!*type) {
		  pscope = iter = rtn = 0;
		  goto err;
	    } else {
		  pscope = scope;
		  iter = vpi_iterate(vpiScope, pscope);
	    }
      }

      /* scan iterator */
      rtn = 0;
      while ((hand = vpi_scan(iter))) {
	    if (acc_object_in_typelist(hand, type)) {
		  rtn = hand;
		  break;
	    }
      }

      /* if we exhausted iterator, cleanup */
      if (!hand) { pscope = iter = 0; }

err:
      /* trace */
      if (pli_trace) {
	    fprintf(pli_trace, " --> %p", rtn);
	    if (rtn)
		  fprintf(pli_trace, " \"%s\"\n", vpi_get_str(vpiName, rtn));
	    else
		  fprintf(pli_trace, "\n");

      }

      return rtn;
}

/*
 * $Log: a_next.c,v $
 * Revision 1.1  2003/05/30 04:18:31  steve
 *  Add acc_next function.
 *
 */
