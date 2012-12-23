/*
 * Copyright (c) 2003-2009 Michael Ruff (mruff at chiaro.com)
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

#include  <stdio.h>
#include  <acc_user.h>
#include  <vpi_user.h>
#include  "priv.h"

/*
 * acc_next and friends implemented using VPI
 */
handle acc_next(PLI_INT32 *type, handle scope, handle prev)
{
      vpiHandle iter, hand = 0;

      /* trace */
      if (pli_trace) {
	    PLI_INT32 *ip;
	    fprintf(pli_trace, "acc_next(%p <", type);
	    for (ip = type; *ip; ip++) {
		  fprintf(pli_trace, "%s%d", ip != type ? "," : "", (int)*ip);
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

      /*
       * The acc_next_* functions need to be reentrant, so we need to
       * rescan all the items up to the previous one, then return
       * the next one.
      */
      iter = vpi_iterate(vpiScope, scope);	/* ICARUS extension */
      if (prev) {
	    while ((hand = vpi_scan(iter))) {
		  if (hand == prev) break;
	    }
      }

      /* scan for next */
      if (!prev || hand) {
	    while ((hand = vpi_scan(iter))) {
		  if (acc_object_in_typelist(hand, type))
			break;
	    }
      }

      /* don't leak iterators */
      if (hand) vpi_free_object(iter);

      /* trace */
      if (pli_trace) {
	    fprintf(pli_trace, " --> %p", hand);
	    if (hand)
		  fprintf(pli_trace, " \"%s\"\n", vpi_get_str(vpiName, hand));
	    else
		  fprintf(pli_trace, "\n");

      }

      return hand;
}

handle acc_next_scope(handle scope, handle prev)
{
      PLI_INT32 type[2] = {accScope, 0};
      return acc_next(type, scope, prev);
}
