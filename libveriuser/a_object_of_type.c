/* vi:sw=6
 * Copyright (c) 2002, 2003 Michael Ruff (mruff at chiaro.com)
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
#ident "$Id: a_object_of_type.c,v 1.4 2003/05/30 04:18:31 steve Exp $"
#endif

#include  <assert.h>
#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

/*
 * acc_object_of_type implemented using VPI interface
 */
PLI_INT32 acc_object_of_type(handle object, PLI_INT32 type)
{
      int vtype;
      int rtn = 0;	/* false */

      if (pli_trace) {
	    fprintf(pli_trace, "acc_object_of_type(%p \"%s\", %d)",
	    	object, vpi_get_str(vpiName, object), type);
	    fflush(pli_trace);
      }

      /* get VPI type of object */
      vtype = vpi_get(vpiType, object);

      switch (type) {
	  case accModule:
	    if (vtype == vpiModule)
		  rtn = 1;
	    break;

	  case accScope:
	    if (vtype == vpiModule || vtype == vpiNamedBegin ||
		vtype == vpiNamedFork || vtype == vpiTask ||
		vtype == vpiFunction) rtn = 1;
	    break;

	  case accNet:
	    if (vtype == vpiNet)
		  rtn = 1;
	    break;

	  case accReg:
	    if (vtype == vpiReg)
		  rtn = 1;
	    break;

	  case accScalar:
	    if (vtype == vpiReg || vtype == vpiNet)
		  rtn = vpi_get(vpiSize, object) == 1;
	    break;

	  case accVector:
	    if (vtype == vpiReg || vtype == vpiNet)
		  rtn = vpi_get(vpiSize, object) > 1;
	    break;

	  case accParameter:
	    if (vtype == vpiParameter)
		  rtn = 1;
	    break;

	    default:
		  vpi_printf("acc_object_of_type: Unknown type %d\n", type);
		  rtn = 0;
      }

      if (pli_trace) fprintf(pli_trace, " --> %d\n", rtn);

      return rtn;
}

PLI_INT32 acc_object_in_typelist(handle object, PLI_INT32*typelist)
{
      while (typelist[0] != 0) {
	    int rtn = acc_object_of_type(object, typelist[0]);
	    if (rtn)
		  return rtn;

	    typelist += 1;
      }

      return 0;
}

/*
 * $Log: a_object_of_type.c,v $
 * Revision 1.4  2003/05/30 04:18:31  steve
 *  Add acc_next function.
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
