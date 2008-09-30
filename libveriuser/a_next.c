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
#ident "$Id: a_next.c,v 1.4 2006/10/30 22:45:37 steve Exp $"
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
      vpiHandle iter, hand = 0;

      /* trace */
      if (pli_trace) {
	    PLI_INT32 *ip;
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

/*
 * $Log: a_next.c,v $
 * Revision 1.4  2006/10/30 22:45:37  steve
 *  Updates for Cygwin portability (pr1585922)
 *
 * Revision 1.3  2003/06/17 16:55:07  steve
 *  1) setlinebuf() for vpi_trace
 *  2) Addes error checks for trace file opens
 *  3) removes now extraneous flushes
 *  4) fixes acc_next() bug
 *
 * Revision 1.2  2003/06/04 01:56:20  steve
 * 1) Adds configure logic to clean up compiler warnings
 * 2) adds acc_compare_handle, acc_fetch_range, acc_next_scope and
 *    tf_isetrealdelay, acc_handle_scope
 * 3) makes acc_next reentrant
 * 4) adds basic vpiWire type support
 * 5) fills in some acc_object_of_type() and acc_fetch_{full}type()
 * 6) add vpiLeftRange/RigthRange to signals
 *
 * Revision 1.1  2003/05/30 04:18:31  steve
 *  Add acc_next function.
 *
 */
