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
#ident "$Id: spname.c,v 1.4 2003/06/17 16:55:08 steve Exp $"
#endif

#include  <assert.h>
#include  <vpi_user.h>
#include  <veriuser.h>
#include  "priv.h"

char* tf_spname(void)
{
      char*rtn;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle scope = vpi_handle(vpiScope, sys);

      rtn = __acc_newstring(vpi_get_str(vpiFullName, scope));

      if (pli_trace) {
	    fprintf(pli_trace, "%s: tf_spname() --> %s\n",
		    vpi_get_str(vpiName,sys), rtn);
      }

      return rtn;
}

char *tf_imipname(void *obj)
{
      return vpi_get_str(vpiFullName, vpi_handle(vpiScope, (vpiHandle)obj));
}

char *tf_mipname(void)
{
      return tf_imipname(vpi_handle(vpiSysTfCall,0));
}

/*
 * $Log: spname.c,v $
 * Revision 1.4  2003/06/17 16:55:08  steve
 *  1) setlinebuf() for vpi_trace
 *  2) Addes error checks for trace file opens
 *  3) removes now extraneous flushes
 *  4) fixes acc_next() bug
 *
 * Revision 1.3  2003/05/29 03:46:21  steve
 *  Add tf_getp/putp support for integers
 *  and real valued arguments.
 *
 *  Add tf_mipname function.
 *
 * Revision 1.2  2003/05/18 00:16:35  steve
 *  Add PLI_TRACE tracing of PLI1 modules.
 *
 *  Add tf_isetdelay and friends, and add
 *  callback return values for acc_vcl support.
 *
 * Revision 1.1  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 */

