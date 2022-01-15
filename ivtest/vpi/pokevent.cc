/*
 * Copyright (c) 2002	Michael Ruff (mruff at chiaro.com)
 *			Michael Runyan (mrunyan at chiaro.com)
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
 * This test verifies named events can be peeked and poked.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vpi_user.h"

extern "C" PLI_INT32
CallbackPeek(s_cb_data *data) {

    static s_vpi_time	zero_delay = { vpiNoDelay, 0, 0, 0 };

    vpi_printf(" callback\n");

    // Toggle poke event
    vpiHandle poke_e = *(vpiHandle *)data->user_data;
    vpi_put_value(poke_e, NULL, &zero_delay, vpiInertialDelay);

    return 0;
}

static vpiHandle
FindPoke(const char *name)
{
    vpiHandle	module, iterate, handle;

    // get top module handle
    iterate = vpi_iterate(vpiModule, NULL);
    if (iterate == NULL) return NULL;
    module = vpi_scan(iterate);
    vpi_free_object(iterate);

    // find named event
    handle = NULL;
    iterate = vpi_iterate(vpiNamedEvent, module);
    if (iterate != NULL) {
	while ((handle = vpi_scan(iterate))) {
	    if (!strcmp(name, vpi_get_str(vpiName, handle))) {
		vpi_free_object(iterate);
		break;
	    }
	}
    }

    return handle;
}

static void
RegisterPeek(const char *name, vpiHandle poke)
{
    vpiHandle	module, iterate, handle;
    s_cb_data	vc_cb_data;
    static vpiHandle	user_data = poke;

    // get top module handle
    iterate = vpi_iterate(vpiModule, NULL);
    if (iterate == NULL) return;
    module = vpi_scan(iterate);
    vpi_free_object(iterate);

    // find named event
    handle = NULL;
    iterate = vpi_iterate(vpiNamedEvent, module);
    if (iterate != NULL) {
	while ((handle = vpi_scan(iterate))) {
	    if (!strcmp(name, vpi_get_str(vpiName, handle))) {
		vpi_free_object(iterate);
		break;
	    }
	}
    }

    // Register callback
    vc_cb_data.time = NULL;
    vc_cb_data.value = NULL;
    vc_cb_data.user_data = (char *)&user_data;
    vc_cb_data.obj = handle;
    vc_cb_data.reason = cbValueChange;
    vc_cb_data.cb_rtn = CallbackPeek;
    vpi_register_cb(&vc_cb_data);
}

extern "C" PLI_INT32
EndofCompile(s_cb_data * /*cb_data*/)
{
    RegisterPeek("e_Peek", FindPoke("e_Poke"));
    return 0;
}

extern "C" void my_Register(void)
{
        s_cb_data cb_data;

        vpi_printf("!!!C++:     Registering Callbacks\n");

        cb_data.time = NULL;
        cb_data.value = NULL;
        cb_data.user_data = (char *) NULL;
        cb_data.obj = NULL;
        cb_data.reason = cbEndOfCompile;
        cb_data.cb_rtn = EndofCompile;
        vpi_register_cb(&cb_data);
}

#ifdef __SUNPRO_CC
extern "C"
#endif
void (*vlog_startup_routines[]) () = {
  my_Register,
  0
};
