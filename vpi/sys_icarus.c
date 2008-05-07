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

#include "vpi_config.h"
#include <assert.h>
#include <vpi_user.h>


/*
 * Routine to return the width in bits of a CPU word (long).
 */
static PLI_INT32 vvp_cpu_wordsize_calltf(PLI_BYTE8* ud)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    assert(callh != 0);
    s_vpi_value val;
    (void) ud;  /* Not used! */

    /* Calculate the result */
    val.format = vpiIntVal;
    val.value.integer = 8*sizeof(long);

    /* Return the result */
    vpi_put_value(callh, &val, 0, vpiNoDelay);

    return 0;
}

static PLI_INT32 size_32(PLI_BYTE8* ud)
{
    (void) ud;  /* Not used! */

    return 32;
}

/*
 * Register the function with Verilog.
 */
void sys_special_register(void)
{
    s_vpi_systf_data tf_data;

    /* Register the single argument functions. */
    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiIntFunc;
    tf_data.calltf      = vvp_cpu_wordsize_calltf;
    tf_data.compiletf   = 0;
    tf_data.sizetf      = size_32;
    tf_data.tfname      = "$vvp_cpu_wordsize";
    tf_data.user_data   = 0;
    vpi_register_systf(&tf_data);
}
