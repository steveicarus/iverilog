/*
 * Copyright (c) 2002 Mike Runyan, Michael Ruff
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
#include <stdio.h>
#include "vpi_user.h"

static PLI_INT32
my_EndOfCompile(p_cb_data cb_data)
{
    vpi_printf ("EndOfCompile %s\n", cb_data->user_data);
    return 0;
}

static PLI_INT32
my_StartOfSimulation(p_cb_data cb_data)
{
    vpi_printf ("StartOfSimulation %s\n", cb_data->user_data);
    return 0;
}

static PLI_INT32
my_EndOfSimulation(p_cb_data cb_data)
{
    vpi_printf ("EndOfSimulation %s\n", cb_data->user_data);
    return 0;
}

static void
my_Register(void)
{
    s_cb_data cb_data;
    cb_data.time = NULL;

    vpi_printf("Registering Callbacks\n");

    // first register
    cb_data.reason = cbEndOfCompile;
    cb_data.cb_rtn = my_EndOfCompile;
    cb_data.user_data = "EOC";
    vpi_register_cb(&cb_data);

    cb_data.reason = cbStartOfSimulation;
    cb_data.cb_rtn = my_StartOfSimulation;
    cb_data.user_data = "SOS";
    vpi_register_cb(&cb_data);

    cb_data.reason = cbEndOfSimulation;
    cb_data.cb_rtn = my_EndOfSimulation;
    cb_data.user_data = "EOS";
    vpi_register_cb(&cb_data);

    // second register
    cb_data.reason = cbEndOfCompile;
    cb_data.cb_rtn = my_EndOfCompile;
    cb_data.user_data = "EOC";
    vpi_register_cb(&cb_data);

    cb_data.reason = cbStartOfSimulation;
    cb_data.cb_rtn = my_StartOfSimulation;
    cb_data.user_data = "SOS";
    vpi_register_cb(&cb_data);

    cb_data.reason = cbEndOfSimulation;
    cb_data.cb_rtn = my_EndOfSimulation;
    cb_data.user_data = "EOS";
    vpi_register_cb(&cb_data);
}

void (*vlog_startup_routines[])(void) = {
        my_Register,
        0
};
