/*
 * Copyright (c) 2002-2013 Michael Ruff (mruff at chiaro.com)
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

/*
 * acc_object_of_type implemented using VPI interface
 */
int acc_object_of_type(handle object, PLI_INT32 type)
{
      int vtype;
      int rtn = 0;	/* false */

      if (pli_trace) {
	    fprintf(pli_trace, "acc_object_of_type(%p \"%s\", %d)",
		object, vpi_get_str(vpiName, object), (int)type);
	    fflush(pli_trace);
      }

      /* get VPI type of object */
      vtype = vpi_get(vpiType, object);

      switch (type) {
	  case accModule: rtn = vtype == vpiModule; break;

	  case accScope:
	    if (vtype == vpiModule || vtype == vpiNamedBegin ||
		vtype == vpiNamedFork || vtype == vpiTask ||
		vtype == vpiFunction || vtype == vpiGenScope) rtn = 1;
	    break;

	  case accNet: rtn = vtype == vpiNet; break;
	  case accReg: rtn = vtype == vpiReg; break;

	  case accRealParam:
	    if (vtype == vpiNamedEvent &&
	        vpi_get(vpiConstType, object) == vpiRealConst)
		  rtn = 1;
	    break;

	  case accParameter: rtn = vtype == vpiParameter; break;
	  case accNamedEvent: rtn = vtype == vpiNamedEvent; break;
	  case accIntegerVar: rtn = vtype == vpiIntegerVar; break;
	  case accRealVar: rtn = vtype == vpiRealVar; break;
	  case accTimeVar: rtn = vtype == vpiTimeVar; break;

	  case accScalar:
	    if (vtype == vpiReg || vtype == vpiNet)
		  rtn = vpi_get(vpiSize, object) == 1;
	    break;

	  case accVector:
	    if (vtype == vpiReg || vtype == vpiNet)
		  rtn = vpi_get(vpiSize, object) > 1;
	    break;

	    default:
		  vpi_printf("acc_object_of_type: Unknown type %d\n",
		             (int)type);
		  rtn = 0;
      }

      if (pli_trace) fprintf(pli_trace, " --> %d\n", rtn);

      return rtn;
}

int acc_object_in_typelist(handle object, PLI_INT32*typelist)
{
      while (typelist[0] != 0) {
	    int rtn = acc_object_of_type(object, typelist[0]);
	    if (rtn)
		  return rtn;

	    typelist += 1;
      }

      return 0;
}
