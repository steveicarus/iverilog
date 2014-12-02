/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
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

static PLI_INT32 one_string_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;

      argv = vpi_iterate(vpiArgument, callh);
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a string argument.\n", name);
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

static PLI_INT32 len_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;

      (void)name; /* Parameter is not used. */

      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      arg = vpi_scan(argv);
      assert(arg);
      vpi_free_object(argv);

      int res = vpi_get(vpiSize, arg);

      s_vpi_value value;
      value.format = vpiIntVal;
      value.value.integer = res;

      vpi_put_value(callh, &value, 0, vpiNoDelay);

      return 0;
}


static PLI_INT32 to_vec_compiletf(ICARUS_VPI_CONST PLI_BYTE8*user_data)
{
    (void) user_data; /* Parameter is not used. */

    vpiHandle systf_handle, arg_iterator, arg_handle;
    PLI_INT32 arg_type[2];

    /* obtain a handle to the system task instance */
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    if (systf_handle == NULL) {
        vpi_printf("ERROR: $ivl_string_method$to_vec failed to obtain systf handle\n");
        vpi_control(vpiFinish,0); /* abort simulation */
        return 0;
    }

    /* obtain handles to system task arguments */
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    if (arg_iterator == NULL) {
        vpi_printf("ERROR: $ivl_string_method$to_vec requires 2 arguments\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    /* check the type of object in system task arguments */
    arg_handle = vpi_scan(arg_iterator);
    for(int i = 0; i < 2; ++i) {
        arg_type[i] = vpi_get(vpiType, arg_handle);
        arg_handle = vpi_scan(arg_iterator);
    }

    if (arg_handle != NULL) {       /* are there more arguments? */
        vpi_printf("ERROR: $ivl_string_method$to_vec can only have 2 arguments\n");
        vpi_free_object(arg_iterator);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    if ((arg_type[0] != vpiStringVar) ||
        (arg_type[1] != vpiNet && arg_type[1] != vpiReg)) {
        vpi_printf("ERROR: $ivl_string_method$to_vec value arguments must be a string and a net or reg\n");
        vpi_free_object(arg_iterator);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    return 0;
}

static PLI_INT32 to_vec_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */

      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, str, vec;
      s_vpi_value str_val;
      s_vpi_vecval*vec_val;

      /* Fetch arguments */
      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      str = vpi_scan(argv);
      assert(str);
      vec = vpi_scan(argv);
      assert(vec);
      vpi_free_object(argv);

      int str_size = vpi_get(vpiSize, str);
      int vec_size = vpi_get(vpiSize, vec);
      if(str_size <= 0) {
	    vpi_printf("ERROR: Cannot cast empty string");
            vpi_control(vpiFinish, 0);
            return 0;
      }

      if(vec_size != str_size * 8) {
	    vpi_printf("ERROR: String and vector size do not match");
            vpi_control(vpiFinish, 0);
            return 0;
      }

      str_val.format = vpiStringVal;
      vpi_get_value(str, &str_val);
      assert(str_val.value.str);

      /* Conversion part */
      int vec_number = ceil((double)str_size / sizeof(PLI_INT32));
      vec_val = calloc(vec_number, sizeof(s_vpi_vecval));
      PLI_BYTE8*str_ptr = &str_val.value.str[str_size - 1];

      /* We have to reverse the order of string, no memcpy here */
      for(int i = 0; i < vec_number; ++i) {
	    int copy_size = str_size > (int)sizeof(PLI_INT32) ?
                                    (int)sizeof(PLI_INT32) : str_size;

            /* Clear the part responsible for X & Z values */
	    memset(&vec_val[i].bval, 0x00, sizeof(PLI_INT32));
	    PLI_BYTE8*dest = (PLI_BYTE8*)&vec_val[i].aval;

	    for(int j = 0; j < copy_size; ++j)
		*dest++ = *str_ptr--;

	    str_size -= copy_size;
      }

      str_val.format = vpiVectorVal;
      str_val.value.vector = vec_val;
      vpi_put_value(vec, &str_val, 0, vpiNoDelay);

      free(vec_val);

      return 0;
}

void v2009_string_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$ivl_string_method$len";
      tf_data.calltf    = len_calltf;
      tf_data.compiletf = one_string_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ivl_string_method$len";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.sysfunctype = 0;
      tf_data.tfname    = "$ivl_string_method$to_vec";
      tf_data.calltf    = to_vec_calltf;
      tf_data.compiletf = to_vec_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ivl_string_method$to_vec";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
