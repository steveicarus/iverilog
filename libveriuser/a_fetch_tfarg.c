/* vi:sw=6
 * Copyright (c) 2002,2003 Michael Ruff (mruff at chiaro.com)
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
#ident "$Id: a_fetch_tfarg.c,v 1.10 2004/10/04 01:10:56 steve Exp $"
#endif

#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

/*
 * acc_fetch_tfarg and friends implemented using VPI interface
 */
double acc_fetch_itfarg(PLI_INT32 n, handle obj)
{
      vpiHandle iter, hand = 0;
      s_vpi_value value;
      int idx = n;
      double rtn;

      iter = vpi_iterate(vpiArgument, obj);

      /* scan to nth argument */
      while (idx > 0 && (hand = vpi_scan(iter))) idx--;

      if (hand) {
	    value.format=vpiRealVal;
	    vpi_get_value(hand, &value);
	    rtn = value.value.real;
	    vpi_free_object(iter);
      } else {
	    rtn = 0.0;
      }

      if (pli_trace) {
	    fprintf(pli_trace, "%s: acc_fetch_itfarg(%d, %p) --> %f\n",
		    vpi_get_str(vpiName, obj), n, obj, rtn);
      }

      return rtn;
}

double acc_fetch_tfarg(PLI_INT32 n)
{
      return acc_fetch_itfarg_int(n, vpi_handle(vpiSysTfCall,0));
}


PLI_INT32 acc_fetch_itfarg_int(PLI_INT32 n, handle obj)
{
      vpiHandle iter, hand = 0;
      s_vpi_value value;
      int idx = n;
      int rtn;

      iter = vpi_iterate(vpiArgument, obj);

      /* scan to nth argument */
      while (idx > 0 && (hand = vpi_scan(iter))) idx--;

      if (hand) {
	    value.format=vpiIntVal;
	    vpi_get_value(hand, &value);
	    rtn = value.value.integer;
	    vpi_free_object(iter);
      } else {
	    rtn = 0;
      }

      if (pli_trace) {
	    fprintf(pli_trace, "%s: acc_fetch_itfarg_int(%d, %p) --> %d\n",
		    vpi_get_str(vpiName, obj), n, obj, rtn);
      }

      return rtn;
}

PLI_INT32 acc_fetch_tfarg_int(PLI_INT32 n)
{
      return acc_fetch_itfarg_int(n, vpi_handle(vpiSysTfCall,0));
}


char *acc_fetch_itfarg_str(PLI_INT32 n, handle obj)
{
      vpiHandle iter, hand = 0;
      s_vpi_value value;
      int idx = n;
      char *rtn;

      iter = vpi_iterate(vpiArgument, obj);

      /* scan to nth argument */
      while (idx > 0 && (hand = vpi_scan(iter))) idx -= 1;

      if (hand) {
	    value.format=vpiStringVal;
	    vpi_get_value(hand, &value);
	    rtn = __acc_newstring(value.value.str);
	    vpi_free_object(iter);
      } else {
	    rtn = (char *) 0;
      }

      if (pli_trace) {
	    fprintf(pli_trace, "%s: acc_fetch_itfarg_str(%d, %p) --> \"%s\"\n",
		    vpi_get_str(vpiName, obj),
		    n, obj, rtn? rtn : "");
      }

      return rtn;
}

char *acc_fetch_tfarg_str(PLI_INT32 n)
{
      return acc_fetch_itfarg_str(n, vpi_handle(vpiSysTfCall,0));
}

/*
 * $Log: a_fetch_tfarg.c,v $
 * Revision 1.10  2004/10/04 01:10:56  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.9  2004/02/18 02:51:59  steve
 *  Fix type mismatches of various VPI functions.
 *
 * Revision 1.8  2003/06/14 01:16:17  steve
 *  ihand is system task, not scope.
 *
 * Revision 1.7  2003/06/13 19:23:42  steve
 *  Add a bunch more PLI1 routines.
 *
 * Revision 1.6  2003/05/18 00:16:35  steve
 *  Add PLI_TRACE tracing of PLI1 modules.
 *
 *  Add tf_isetdelay and friends, and add
 *  callback return values for acc_vcl support.
 *
 * Revision 1.5  2003/03/14 04:58:50  steve
 *  Free the iterator when Im done.
 *
 * Revision 1.4  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 * Revision 1.3  2003/02/17 06:39:47  steve
 *  Add at least minimal implementations for several
 *  acc_ functions. Add support for standard ACC
 *  string handling.
 *
 *  Add the _pli_types.h header file to carry the
 *  IEEE1364-2001 standard PLI type declarations.
 *
 * Revision 1.2  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/06/07 02:58:58  steve
 *  Add a bunch of acc/tf functions. (mruff)
 *
 */
