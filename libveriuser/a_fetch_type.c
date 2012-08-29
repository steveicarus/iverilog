/*
 * Copyright (c) 2003-2010 Stephen Williams (steve@icarus.com)
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
      }

      vpi_printf("acc_fetch_type: vpiType %d is what accType?\n",
                 (int)vpi_get(vpiType, obj));
      return accUnknown;
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
