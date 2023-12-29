/*
 * Copyright (c) 2003  Michael Ruff (mruff at chiaro.com)
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

/*
 * This test verifies vpiScaledRealTime
 */

#include <assert.h>
#include "vpi_user.h"

s_vpi_time get_time = { vpiScaledRealTime, 0, 0, 0 };

static PLI_INT32 calltf(PLI_BYTE8 *data)
{
    vpiHandle hand, iter;

    (void)data;  /* Parameter is not used. */

    hand = vpi_handle(vpiSysTfCall, 0);
    iter = vpi_iterate(vpiArgument, hand);
    hand = vpi_scan(iter);
    vpi_free_object(iter);

    vpi_printf("calltf from %s", vpi_get_str(vpiName, hand));

    vpi_get_time(0, &get_time);
    vpi_printf(" %f,",  get_time.real);

    vpi_get_time(hand, &get_time);
    vpi_printf(" %f\n", get_time.real);

    return 0;
}

static void
VPIRegister(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type = vpiSysTask;
    tf_data.tfname = "$test";
    tf_data.calltf = calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf = 0;

    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = { VPIRegister, 0};
