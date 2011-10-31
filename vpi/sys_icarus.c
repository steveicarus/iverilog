/*
 *  Copyright (C) 2008-2011  Cary R. (cygcary@yahoo.com)
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
#include <assert.h>

static PLI_INT32 finish_and_return_calltf(PLI_BYTE8* name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle arg;
    s_vpi_value val;
    (void) name;  /* Not used! */

    /* Get the return value. */
    arg = vpi_scan(argv);
    vpi_free_object(argv);
    val.format = vpiIntVal;
    vpi_get_value(arg, &val);

    /* Set the return value. */
    vpip_set_return_value(val.value.integer);

    /* Now finish. */
    vpi_control(vpiFinish, 1);
    return 0;
}

static PLI_INT32 task_not_implemented_compiletf(PLI_BYTE8* name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);

      vpi_printf("SORRY: %s:%d: task %s() is not currently implemented.\n",
                 vpi_get_str(vpiFile, callh), (int)vpi_get(vpiLineNo, callh),
                 name);
      vpi_control(vpiFinish, 1);
      return 0;
}

/*
 * This is used to warn the user that the specified optional system
 * task/function is not available (from Annex C 1364-2005).
 */
static PLI_INT32 missing_optional_compiletf(PLI_BYTE8* name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);

      vpi_printf("SORRY: %s:%d: %s() is not available in Icarus verilog.\n",
                 vpi_get_str(vpiFile, callh), (int)vpi_get(vpiLineNo, callh),
                 name);
      vpi_control(vpiFinish, 1);
      return 0;
}

/*
 * Register the function with Verilog.
 */
void sys_special_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type        = vpiSysTask;
      tf_data.calltf      = finish_and_return_calltf;
      tf_data.compiletf   = sys_one_numeric_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$finish_and_return";
      tf_data.user_data   = "$finish_and_return";
      vpi_register_systf(&tf_data);

	/* These tasks are not currently implemented. */
      tf_data.type        = vpiSysTask;
      tf_data.calltf      = 0;
      tf_data.sizetf      = 0;
      tf_data.compiletf   = task_not_implemented_compiletf;

      tf_data.tfname      = "$fmonitor";
      tf_data.user_data   = "$fmonitor";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fmonitorb";
      tf_data.user_data   = "$fmonitorb";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fmonitoro";
      tf_data.user_data   = "$fmonitoro";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fmonitorh";
      tf_data.user_data   = "$fmonitorh";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$and$array";
      tf_data.user_data   = "$async$and$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$nand$array";
      tf_data.user_data   = "$async$nand$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$or$array";
      tf_data.user_data   = "$async$or$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$nor$array";
      tf_data.user_data   = "$async$nor$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$and$plane";
      tf_data.user_data   = "$async$and$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$nand$plane";
      tf_data.user_data   = "$async$nand$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$or$plane";
      tf_data.user_data   = "$async$or$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$nor$plane";
      tf_data.user_data   = "$async$nor$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$and$array";
      tf_data.user_data   = "$sync$and$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$nand$array";
      tf_data.user_data   = "$sync$nand$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$or$array";
      tf_data.user_data   = "$sync$or$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$nor$array";
      tf_data.user_data   = "$sync$nor$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$and$plane";
      tf_data.user_data   = "$sync$and$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$nand$plane";
      tf_data.user_data   = "$sync$nand$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$or$plane";
      tf_data.user_data   = "$sync$or$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$nor$plane";
      tf_data.user_data   = "$sync$nor$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dumpports";
      tf_data.user_data   = "$dumpports";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dumpportsoff";
      tf_data.user_data   = "$dumpportsoff";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dumpportson";
      tf_data.user_data   = "$dumpportson";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dumpportsall";
      tf_data.user_data   = "$dumpportsall";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dumpportslimit";
      tf_data.user_data   = "$dumpportslimit";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dumpportsflush";
      tf_data.user_data   = "$dumpportsflush";
      vpi_register_systf(&tf_data);

	/* The following optional system tasks/functions are not implemented
	 * in Icarus Verilog (from Annex C 1364-2005). */
      tf_data.type        = vpiSysTask;
      tf_data.calltf      = 0;
      tf_data.sizetf      = 0;
      tf_data.compiletf   = missing_optional_compiletf;

      tf_data.tfname      = "$input";
      tf_data.user_data   = "$input";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$key";
      tf_data.user_data   = "$key";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$nokey";
      tf_data.user_data   = "$nokey";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$list";
      tf_data.user_data   = "$list";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$log";
      tf_data.user_data   = "$log";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$nolog";
      tf_data.user_data   = "$nolog";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$save";
      tf_data.user_data   = "$save";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$restart";
      tf_data.user_data   = "$restart";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$incsave";
      tf_data.user_data   = "$incsave";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$scope";
      tf_data.user_data   = "$scope";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$showscopes";
      tf_data.user_data   = "$showscopes";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$showvars";
      tf_data.user_data   = "$showvars";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sreadmemb";
      tf_data.user_data   = "$sreadmemb";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sreadmemh";
      tf_data.user_data   = "$sreadmemh";
      vpi_register_systf(&tf_data);

	/* Optional functions. */
      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;

      tf_data.tfname      = "$countdrivers";
      tf_data.user_data   = "$countdrivers";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$getpattern";
      tf_data.user_data   = "$getpattern";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$scale";
      tf_data.user_data   = "$scale";
      vpi_register_systf(&tf_data);
}
