/* vi:sw=6
 * Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
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
#ident "$Id: a_fetch_tfarg.c,v 1.4 2003/03/13 04:35:09 steve Exp $"
#endif

#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

/*
 * acc_fetch_tfarg routines implemented using VPI interface
 */
int acc_fetch_tfarg_int(int n)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value value;
      int rtn;

      sys_h = vpi_handle(vpiSysTfCall, 0);
      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* scan to nth argument */
      while (n > 0 && (arg_h = vpi_scan(sys_i))) n--;

      if (arg_h) {
	    value.format=vpiIntVal;
	    vpi_get_value(arg_h, &value);
	    rtn = value.value.integer; 
      } else {
	    rtn = 0;
      }

      return rtn;
}

char *acc_fetch_tfarg_str(int n)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value value;
      char *rtn;
      int idx = n;

      sys_h = vpi_handle(vpiSysTfCall, 0);
      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* scan to nth argument */
      while (idx > 0 && (arg_h = vpi_scan(sys_i)))
	    idx -= 1;

      if (arg_h) {
	    value.format=vpiStringVal;
	    vpi_get_value(arg_h, &value);
	    rtn = __acc_newstring(value.value.str);
	    vpi_free_object(sys_i);
      } else {
	    rtn = (char *) 0;
      }

      return rtn;
}

/*
 * $Log: a_fetch_tfarg.c,v $
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
