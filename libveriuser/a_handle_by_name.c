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
#ident "$Id: a_handle_by_name.c,v 1.2 2003/06/17 16:55:07 steve Exp $"
#endif

# include  <acc_user.h>
# include  <vpi_user.h>
# include  "priv.h"
# include  <assert.h>
# include  <string.h>

/*
 * acc_handle_by_name implemented using VPI interface
 */
handle acc_handle_by_name(const char*obj_name, handle scope)
{
      vpiHandle sys_h;
      vpiHandle res;

      /* if no scope provided, use tasks scope */
      if (!scope) {
	    sys_h = vpi_handle(vpiSysTfCall, 0 /* NULL */);
	    scope = vpi_handle(vpiScope, sys_h);
      }

      res = vpi_handle_by_name(obj_name, scope);

      if (pli_trace) {
	    fprintf(pli_trace, "acc_handle_by_name(\"%s\", scope=%s) "
		    " --> %p\n", obj_name,
		    vpi_get_str(vpiFullName, scope), res);
      }

      return res;
}

/*
 * $Log: a_handle_by_name.c,v $
 * Revision 1.2  2003/06/17 16:55:07  steve
 *  1) setlinebuf() for vpi_trace
 *  2) Addes error checks for trace file opens
 *  3) removes now extraneous flushes
 *  4) fixes acc_next() bug
 *
 * Revision 1.1  2003/05/24 03:02:04  steve
 *  Add implementation of acc_handle_by_name.
 *
 */
