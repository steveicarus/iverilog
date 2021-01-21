/*
 * Copyright (c) 2012-2021 Stephen Williams (steve@icarus.com)
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


# include  "sys_priv.h"
# include  <assert.h>
# include  <math.h>
# include  <stdlib.h>
# include  <string.h>
static PLI_INT32 dobject_size_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, arg;

      argv = vpi_iterate(vpiArgument, callh);
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a dynamic array, queue or string "
	               "argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      arg =  vpi_scan(argv);  /* This should never be zero. */
      assert(arg);

	/* The argument must be a dynamic array, queue or string. */
      switch (vpi_get(vpiType, arg)) {
	case vpiStringVar:
	    break;
	case vpiArrayVar:
	    switch(vpi_get(vpiArrayType, arg)) {
	      case vpiDynamicArray:
	      case vpiQueueArray:
		  break;
	      default:
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s argument must be a dynamic array, queue or "
		             "string.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
	    break;
	default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s argument must be a dynamic array, queue or string, "
	               "given a %s.\n", name, vpi_get_str(vpiType, arg));
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s has too many arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    vpi_free_object(argv);
      }

      return 0;
}

static PLI_INT32 dobject_size_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg = vpi_scan(argv);

      (void)name; /* Parameter is not used. */

      vpi_free_object(argv);

      s_vpi_value value;
      value.format = vpiIntVal;
      value.value.integer = vpi_get(vpiSize, arg);

      vpi_put_value(callh, &value, 0, vpiNoDelay);

      return 0;
}

void sys_darray_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$size";
      tf_data.calltf    = dobject_size_calltf;
      tf_data.compiletf = dobject_size_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$size";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
