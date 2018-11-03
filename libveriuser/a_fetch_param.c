/*
 * Copyright (c) 2003-2009 Stephen Williams (steve@icarus.com)
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
	    return (double) (intptr_t)val.value.str;

	  default:
	    vpi_printf("XXXX: parameter %s has type %d\n",
		       vpi_get_str(vpiName, object), (int)val.format);
	    assert(0);
	    return 0.0;
      }
}
