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
#ident "$Id: a_fetch_param.c,v 1.4 2003/06/17 16:55:07 steve Exp $"
#endif

#include  <assert.h>
#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

double acc_fetch_paramval(handle object)
{
      s_vpi_value val;

      val.format = vpiObjTypeVal;
      vpi_get_value(object, &val);

      switch (val.format) {

	  case vpiStringVal:
	    if (pli_trace) {
		  fprintf(pli_trace, "acc_fetch_paramval(%s) --> \"%s\"\n",
			  vpi_get_str(vpiName, object), val.value.str);
	    }
	    return (double) (long)val.value.str;

	  default:
	    vpi_printf("XXXX: parameter %s has type %d\n",
		       vpi_get_str(vpiName, object), val.format);
	    assert(0);
	    return 0.0;
      }
}

/*
 * $Log: a_fetch_param.c,v $
 * Revision 1.4  2003/06/17 16:55:07  steve
 *  1) setlinebuf() for vpi_trace
 *  2) Addes error checks for trace file opens
 *  3) removes now extraneous flushes
 *  4) fixes acc_next() bug
 *
 * Revision 1.3  2003/05/18 00:16:35  steve
 *  Add PLI_TRACE tracing of PLI1 modules.
 *
 *  Add tf_isetdelay and friends, and add
 *  callback return values for acc_vcl support.
 *
 * Revision 1.2  2003/03/14 04:59:54  steve
 *  Better message when asserting funky value type.
 *
 * Revision 1.1  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 */

