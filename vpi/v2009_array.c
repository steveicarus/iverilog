/*
 *  Copyright (C) 2013  Cary R. (cygcary@yahoo.com)
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

#include "sys_priv.h"

static PLI_INT32 func_not_implemented_compiletf(ICARUS_VPI_CONST PLI_BYTE8* name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);

      vpi_printf("SORRY: %s:%d: function %s() is not currently implemented.\n",
                 vpi_get_str(vpiFile, callh), (int)vpi_get(vpiLineNo, callh),
                 name);
      vpi_control(vpiFinish, 1);
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

      tf_data.tfname      = "$high";
      tf_data.user_data   = "$high";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$increment";
      tf_data.user_data   = "$increment";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$size";
      tf_data.user_data   = "$size";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
