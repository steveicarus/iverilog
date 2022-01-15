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
 * This test verifies vpiPureTransportDelay functionality
 */

#include <assert.h>
#include "vpi_user.h"

static PLI_INT32
EndOfCompile(s_cb_data *data)
{
    vpiHandle hand;
    s_vpi_time timerec = { vpiSimTime, 0, 0, 0 };
    s_vpi_value val;
    int i;

    (void)data;  /* Parameter is not used. */

    hand = vpi_handle_by_name("test.r", 0);
    assert(hand);

    // Get current state
    val.format = vpiIntVal;
    vpi_get_value(hand, &val);

    // Add a few transitions
    for (i = 0; i < 6; i++) {

	if (i < 3) {
	    // delay 10+i time units
	    timerec.low = 1000 * (i + 1);
	} else {
	    timerec.type = vpiScaledRealTime;
	    timerec.low = 0;
	    timerec.real = 10000.0 * (i+1);
	}

	// Toggle state
	val.value.integer ^= 1;

	// Put new state
	vpi_put_value(hand, &val, &timerec, vpiPureTransportDelay);
    }

    return 0;
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
    cb_data.cb_rtn = EndOfCompile;

    vpi_register_cb(&cb_data);
}

void (*vlog_startup_routines[]) (void) = { VPIRegister, 0};
