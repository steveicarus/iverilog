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
#ident "$Id: a_handle_object.c,v 1.2 2003/12/17 15:45:07 steve Exp $"
#endif

#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

static vpiHandle search_scope = 0;

handle acc_handle_object(const char*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle scope = search_scope? search_scope : vpi_handle(vpiScope, sys);
      vpiHandle res = vpi_handle_by_name(name, scope);

      if (pli_trace) {
	    fprintf(pli_trace, "acc_handle_object(%s <scope=%s>) --> .\n",
		    name, acc_fetch_fullname(scope));
      }

      return res;
}

char* acc_set_scope(handle ref, ...)
{
      char*name;
      search_scope = ref;

      name = acc_fetch_fullname(search_scope);
      if (pli_trace) {
	    fprintf(pli_trace, "acc_set_scope(<scope=%s>)\n", name);
      }

      return acc_fetch_fullname(ref);
}

/*
 * $Log: a_handle_object.c,v $
 * Revision 1.2  2003/12/17 15:45:07  steve
 *  Add acc_set_scope function.
 *
 * Revision 1.1  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 */

