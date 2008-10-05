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
#ident "$Id: a_fetch_type.c,v 1.8 2004/10/04 01:10:56 steve Exp $"
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

	  case vpiNamedEvent:
	    return accNamedEvent;

	  case vpiNet:
	    return accNet;

	  case vpiParameter:
	    return accParameter;

	  case vpiReg:
	    return accReg;

	  case vpiIntegerVar:
	    return accIntegerVar;

	  case vpiModule:
	    return accModule;

	  default:
	    vpi_printf("acc_fetch_type: vpiType %d is what accType?\n",
		       vpi_get(vpiType, obj));
	    return accUnknown;
      }

      return 0;
}

PLI_INT32 acc_fetch_fulltype(handle obj)
{
      int type = vpi_get(vpiType, obj);

      switch (type) {
	  case vpiNet: {
	    type = vpi_get(vpiNetType, obj);
	    switch(type) {
		case vpiWire: return accWire;
		default:
		vpi_printf("acc_fetch_fulltype: vpiNetType %d unknown?\n",
			   type);
		return accUnknown;
	    }
	  }

	  case vpiConstant:
	      /* see acc_fetch_type */
	    if (vpi_get(vpiConstType, obj) == vpiStringConst)
		  return accStringParam;
	    else
		  return accConstant;

	  case vpiIntegerVar: return accIntegerVar;

	  case vpiModule:
	    if (!vpi_handle(vpiScope, obj))
		return accTopModule;
	    else
		return accModuleInstance;
	    /* FIXME accCellInstance */

	  case vpiNamedEvent: return accNamedEvent;

	  case vpiParameter:
	    switch(vpi_get(vpiConstType, obj)) {
		case vpiRealConst: return accRealParam;
		case vpiStringConst: return accStringParam;
		default: return accIntegerParam;
	    }

	  case vpiReg: return accReg;

	  default:
	    vpi_printf("acc_fetch_fulltype: vpiType %d unknown?\n",
		       type);
	    return accUnknown;
      }
}

/*
 * $Log: a_fetch_type.c,v $
 * Revision 1.8  2004/10/04 01:10:56  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.7  2003/06/04 01:56:20  steve
 * 1) Adds configure logic to clean up compiler warnings
 * 2) adds acc_compare_handle, acc_fetch_range, acc_next_scope and
 *    tf_isetrealdelay, acc_handle_scope
 * 3) makes acc_next reentrant
 * 4) adds basic vpiWire type support
 * 5) fills in some acc_object_of_type() and acc_fetch_{full}type()
 * 6) add vpiLeftRange/RigthRange to signals
 *
 * Revision 1.6  2003/05/30 04:18:31  steve
 *  Add acc_next function.
 *
 * Revision 1.5  2003/05/29 02:35:41  steve
 *  acc_fetch_type supports module.
 *
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

