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
 * This test verifies deletion of event callbacks
 */

#include <assert.h>
#include "vpi_user.h"

static vpiHandle Handle;

static PLI_INT32
Callback(s_cb_data *data)
{
    s_vpi_time t;
    static int count = 0;

    (void)data;  /* Parameter is not used. */

    t.type = vpiScaledRealTime;
    vpi_get_time(0, &t);

    vpi_printf("Callback @ %.1f\n", t.real);

    if (count>1) {
	vpi_printf("vpi_remove_cb returned %d @ %.1f\n",
	    (int)vpi_remove_cb(Handle), t.real);
    }

    count++;

    return 0;
}

static PLI_INT32
CallbackRegister(s_cb_data *data)
{
    vpiHandle hand;
    s_cb_data cb_data;
    s_vpi_time timerec = { vpiSimTime, 0, 0, 0 };

    (void)data;  /* Parameter is not used. */

    hand = vpi_handle_by_name("test.e", 0);
    assert(hand);

    cb_data.time = &timerec;
    cb_data.value = 0;
    cb_data.user_data = (char *)hand;
    cb_data.obj = hand;
    cb_data.reason = cbValueChange;
    cb_data.cb_rtn = Callback;
    Handle = vpi_register_cb(&cb_data);

    return (0);
}


static void
VPIRegister(void)
{
    s_cb_data cb_data;
    s_vpi_time timerec = { vpiSuppressTime, 0, 0, 0 };

    cb_data.time = &timerec;
    cb_data.value = 0;
    cb_data.user_data = 0;
    cb_data.obj = 0;
    cb_data.reason = cbEndOfCompile;
    cb_data.cb_rtn = CallbackRegister;

    vpi_register_cb(&cb_data);
}

void (*vlog_startup_routines[]) (void) = { VPIRegister, 0};
