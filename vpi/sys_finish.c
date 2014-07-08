/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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

#include "sys_priv.h"
#include <string.h>

static PLI_INT32 sys_finish_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv;
      s_vpi_value val;
      long diag_msg = 1;

      /* Get the argument list and look for the diagnostic message level. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      if (argv) {
            vpiHandle arg = vpi_scan(argv);
            vpi_free_object(argv);
            val.format = vpiIntVal;
            vpi_get_value(arg, &val);
            diag_msg = val.value.integer;
      }

      if (strcmp((const char*)name, "$stop") == 0) {
	    vpi_control(vpiStop, diag_msg);
	    return 0;
      }

      vpi_control(vpiFinish, diag_msg);
      return 0;
}

void sys_finish_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$finish";
      tf_data.calltf    = sys_finish_calltf;
      tf_data.compiletf = sys_one_opt_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$finish";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$stop";
      tf_data.calltf    = sys_finish_calltf;
      tf_data.compiletf = sys_one_opt_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$stop";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
