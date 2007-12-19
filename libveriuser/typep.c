/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: typep.c,v 1.2 2003/03/13 05:07:46 steve Exp $"
#endif

#include  <assert.h>
#include  <veriuser.h>
#include  <vpi_user.h>

PLI_INT32 tf_typep(PLI_INT32 narg)
{
      vpiHandle sys_h, argv, arg_h = 0;
      int rtn;

      assert(narg > 0);

      /* get task/func handle */
      sys_h = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, sys_h);

      /* find nth arg */
      while (narg > 0) {
	      /* Watch that the argument is not out of range. */
	    if (!(arg_h = vpi_scan(argv)))
		  return TF_NULLPARAM;
	    narg -= 1;
      }

      switch (vpi_get(vpiType, arg_h)) {
	  case vpiConstant:
	    switch (vpi_get(vpiConstType, arg_h)) {
		case vpiStringConst:
		  rtn = TF_STRING;
		  break;
		case vpiRealConst:
		  rtn = TF_READONLYREAL;
		  break;
		default:
		  rtn = TF_READONLY;
		  break;
	    }
	    break;

	  case vpiIntegerVar:
	  case vpiReg:
	  case vpiMemoryWord:
	    rtn = TF_READWRITE;
	    break;

	  case vpiRealVar:
	    rtn = TF_READWRITEREAL;
	    break;

	  default:
	    rtn = TF_READONLY;
	    break;
      }

      vpi_free_object(argv);
      return rtn;
}
