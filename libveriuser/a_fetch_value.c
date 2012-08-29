/*
 * Copyright (c) 2003-2008 Stephen Williams (steve@icarus.com)
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

# include  <acc_user.h>
# include  <vpi_user.h>
# include  "priv.h"
# include  <assert.h>
# include  <string.h>

static char* fetch_struct_value(handle obj, s_acc_value*value)
{
      struct t_vpi_value val;

      switch (value->format) {

	  case accScalarVal:
	    val.format = vpiScalarVal;
	    vpi_get_value(obj, &val);
	    switch (val.value.scalar) {
		case vpi0:
		  value->value.scalar = acc0;
		  break;
		case vpi1:
		  value->value.scalar = acc1;
		  break;
		case vpiX:
		  value->value.scalar = accX;
		  break;
		case vpiZ:
		  value->value.scalar = accZ;
		  break;
		default:
		  assert(0);
	    }

	    if (pli_trace) {
		  fprintf(pli_trace, "acc_fetch_value(<%s>, "
			  "accScalarVal) --> %d\n",
			  vpi_get_str(vpiFullName,obj),
			  value->value.scalar);
	    }
	    break;

	  case accIntVal:
	    val.format = vpiIntVal;
	    vpi_get_value(obj, &val);
	    value->value.integer = val.value.integer;

	    if (pli_trace) {
		  fprintf(pli_trace, "acc_fetch_value(<%s>, "
			  "accIntVal) --> %d\n",
			  vpi_get_str(vpiFullName,obj),
			  value->value.integer);
	    }
	    break;

	  case accRealVal:
	    val.format = vpiRealVal;
	    vpi_get_value(obj, &val);
	    value->value.real = val.value.real;

	    if (pli_trace) {
		  fprintf(pli_trace, "acc_fetch_value(<%s>, "
			  "accRealVal) --> %g\n",
			  vpi_get_str(vpiFullName,obj),
			  value->value.real);
	    }
	    break;

	  default:
	    vpi_printf("XXXX acc_fetch_value(..., \"%%%%\", <%d>);\n",
		       value->format);
	    value->value.str = "<string value>";
	    break;
      }

      return 0;
}

static char* fetch_strength_value(handle obj)
{
      struct t_vpi_value val;
      char str[4];

      val.format = vpiStrengthVal;
      vpi_get_value(obj, &val);

      /* Should this iterate over the bits? It now matches the old code. */
      vpip_format_strength(str, &val, 0);

      if (pli_trace) {
	    fprintf(pli_trace, "acc_fetch_value(<%s>, \"%%v\") --> %s\n",
		    vpi_get_str(vpiFullName,obj), str);
      }

      return __acc_newstring(str);
}

char* acc_fetch_value(handle obj, const char*fmt, s_acc_value*value)
{
      if (strcmp(fmt, "%%") == 0)
	    return fetch_struct_value(obj, value);

      if (strcmp(fmt, "%v") == 0)
	    return fetch_strength_value(obj);

      vpi_printf("XXXX acc_fetch_value(..., \"%s\", ...)\n", fmt);
      return "<acc_fetch_value>";
}
