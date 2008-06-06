/*
 *  Copyright (C) 2008  Cary R. (cygcary@yahoo.com)
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

#include <assert.h>
#include <vpi_user.h>
#include "sys_priv.h"

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
}
