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
#if !defined(WINNT)
#ident "$Id: a_object_of_type.c,v 1.1 2002/06/07 02:58:58 steve Exp $"
#endif

#include  <assert.h>
#include  <vpi_user.h>
#include  <acc_user.h>

/*
 * acc_object_of_type implemented using VPI interface
 */
int acc_object_of_type(handle object, int type)
{
      int vtype;
      int rtn = 0;	/* false */

      /* get VPI type of object */
      vtype = vpi_get(vpiType, object);

      switch (type) {
	    case accScope:
		  if (vtype == vpiModule || vtype == vpiNamedBegin ||
		      vtype == vpiNamedFork || vtype == vpiTask ||
		      vtype == vpiFunction) rtn = 1;
		  break;
	    case accScalar:
		  if (vtype == vpiReg || vtype == vpiNet)
			rtn = vpi_get(vpiSize, object) == 1;
		  break;
	    case accVector:
		  if (vtype == vpiReg || vtype == vpiNet)
			rtn = vpi_get(vpiSize, object) > 1;
		  break;
	    default:
		  vpi_printf("acc_object_of_type: Unknown type %d\n", type);
		  assert(0);
      }

      return rtn;
}

/*
 * $Log: a_object_of_type.c,v $
 * Revision 1.1  2002/06/07 02:58:58  steve
 *  Add a bunch of acc/tf functions. (mruff)
 *
 */
