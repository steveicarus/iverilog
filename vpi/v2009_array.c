/*
 *  Copyright (C) 2013  Cary R. (cygcary@yahoo.com)
 *  Copyright (C) 2014  Stephen Williams (steve@icarus.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

# include  "sys_priv.h"
# include  <assert.h>

static PLI_INT32 one_array_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;

      argv = vpi_iterate(vpiArgument, callh);
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires an array argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      arg =  vpi_scan(argv);
      if (arg == 0) return 0;

      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s has too many arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      return 0;
}

static PLI_INT32 func_not_implemented_compiletf(ICARUS_VPI_CONST PLI_BYTE8* name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);

      vpi_printf("SORRY: %s:%d: function %s() is not currently implemented.\n",
                 vpi_get_str(vpiFile, callh), (int)vpi_get(vpiLineNo, callh),
                 name);
      vpi_control(vpiFinish, 1);
      return 0;
}

static void high_array(const char*name, vpiHandle callh, vpiHandle arg)
{
      s_vpi_value value;
      int array_type;
      int size;

      switch ( (array_type = vpi_get(vpiArrayType, arg)) ) {
	  case vpiDynamicArray:
	  case vpiQueueArray:
	    size = vpi_get(vpiSize, arg);
	    value.format = vpiIntVal;
	    value.value.integer = size - 1;
	    vpi_put_value(callh, &value, 0, vpiNoDelay);
	    break;

	  default:
	    vpi_printf("SORRY: %s:%d: function %s() argument object code is %d\n",
		       vpi_get_str(vpiFile,callh), (int)vpi_get(vpiLineNo, callh),
		       name, array_type);
	    break;
      }
}

static PLI_INT32 high_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;
      int object_code;

      (void)name; /* Parameter is not used. */

      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      arg = vpi_scan(argv);
      assert(arg);
      vpi_free_object(argv);

      switch ( (object_code = vpi_get(vpiType, arg)) ) {
	  case vpiArrayVar:
	    high_array(name, callh, arg);
	    break;

	  default:
	    vpi_printf("SORRY: %s:%d: function %s() argument object code is %d\n",
		       vpi_get_str(vpiFile,callh), (int)vpi_get(vpiLineNo, callh),
		       name, object_code);
	    return 0;
      }

      return 0;
}

void v2009_array_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.calltf      = 0;
      tf_data.compiletf   = func_not_implemented_compiletf;;
      tf_data.sizetf      = 0;

	/* These functions are not currently implemented. */
      tf_data.tfname      = "$dimensions";
      tf_data.user_data   = "$dimensions";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$unpacked_dimensions";
      tf_data.user_data   = "$unpacked_dimensions";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$left";
      tf_data.user_data   = "$left";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$right";
      tf_data.user_data   = "$right";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$low";
      tf_data.user_data   = "$low";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$increment";
      tf_data.user_data   = "$increment";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$high";
      tf_data.user_data   = "$high";
      tf_data.compiletf   = one_array_arg_compiletf;
      tf_data.calltf      = high_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
