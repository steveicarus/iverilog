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
#ident "$Id: a_fetch_type.c,v 1.4 2003/04/24 18:57:06 steve Exp $"
#endif

# include  <acc_user.h>
# include  <vpi_user.h>
# include  <assert.h>

PLI_INT32 acc_fetch_size(handle obj)
{
      return vpi_get(vpiSize, obj);
}

PLI_INT32 acc_fetch_type(handle obj)
{
      switch (vpi_get(vpiType, obj)) {

	  case vpiConstant:
	      /*XXXX SWIFT PLI tasks seem to assume that string
		constants show up an accParameter, instead of
		accConstant. */
	    if (vpi_get(vpiConstType, obj) == vpiStringConst)
		  return accParameter;
	    else
		  return accConstant;

	  case vpiNet:
	    return accNet;

	  case vpiParameter:
	    return accParameter;

	  case vpiReg:
	    return accReg;

	  case vpiIntegerVar:
	    return accIntegerVar;

	  default:
	    vpi_printf("acc_fetch_type: vpiType %d is what accType?\n",
		       vpi_get(vpiType, obj));
	    return 0;
      }

      return 0;
}

PLI_INT32 acc_fetch_fulltype(handle obj)
{
      switch (vpi_get(vpiType, obj)) {
	  case vpiConstant:
	      /* see acc_fetch_type */
	    if (vpi_get(vpiConstType, obj) == vpiStringConst)
		  return accStringParam;
	    else
		  return accConstant;

	  case vpiIntegerVar:
	    return accIntegerVar;

	  default:
	    vpi_printf("acc_fetch_fulltype: vpiType %d is what accType?\n",
		       vpi_get(vpiType, obj));
	    return 0;
      }
}

/*
 * $Log: a_fetch_type.c,v $
 * Revision 1.4  2003/04/24 18:57:06  steve
 *  Add acc_fetch_fulltype function.
 *
 * Revision 1.3  2003/04/12 18:57:14  steve
 *  More acc_ function stubs.
 *
 * Revision 1.2  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 * Revision 1.1  2003/02/17 06:39:47  steve
 *  Add at least minimal implementations for several
 *  acc_ functions. Add support for standard ACC
 *  string handling.
 *
 *  Add the _pli_types.h header file to carry the
 *  IEEE1364-2001 standard PLI type declarations.
 *
 */

