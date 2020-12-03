/*
 * Copyright (c) 2002-2020 Michael Ruff (mruff at chiaro.com)
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
		    vpi_get_str(vpiName, obj), (int)n, obj, rtn);
      }

      return rtn;
}

double acc_fetch_tfarg(PLI_INT32 n)
{
      return acc_fetch_itfarg_int(n, cur_instance);
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
		    vpi_get_str(vpiName, obj), (int)n, obj, rtn);
      }

      return rtn;
}

PLI_INT32 acc_fetch_tfarg_int(PLI_INT32 n)
{
      return acc_fetch_itfarg_int(n, cur_instance);
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
		    (int)n, obj, rtn? rtn : "");
      }

      return rtn;
}

char *acc_fetch_tfarg_str(PLI_INT32 n)
{
      return acc_fetch_itfarg_str(n, cur_instance);
}
