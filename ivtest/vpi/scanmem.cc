/*
 * Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
 *                    Michael Runyan (mrunyan at chiaro.com)
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
#include <string.h>
#include "vpi_user.h"

extern "C" PLI_INT32 MemPeek(PLI_BYTE8 *)
{
    vpiHandle	mod_h, mem_h, iterate, handle;
    s_vpi_value	value;

    vpi_printf("MemPeek Callback\n");

    // get top module handle
    iterate = vpi_iterate(vpiModule, NULL);
    if (iterate == NULL) return -1;
    mod_h = vpi_scan(iterate);
    vpi_free_object(iterate);

    // Get memory
    mem_h = NULL;
    iterate = vpi_iterate(vpiMemory, mod_h);
    if (iterate != NULL) {
        while ((handle = vpi_scan(iterate))) {
            if (!strcmp("m_peek", vpi_get_str(vpiName, handle))) {
		vpiHandle memw_iter = vpi_iterate(vpiMemoryWord, handle);
		vpi_printf("  Found %s (%d deep x %d bits)\n",
		    vpi_get_str(vpiName, handle),
		    (int)vpi_get(vpiSize, handle),
		    (int)vpi_get(vpiSize, vpi_scan(memw_iter)));
		vpi_free_object(memw_iter);
		mem_h = handle;
		vpi_free_object(iterate);
                break;
            }
        }
    }

    // Invert read memory
    iterate = vpi_iterate(vpiMemoryWord, mem_h);
    while ((handle = vpi_scan(iterate))) {
	// Get current value
	value.format=vpiIntVal;
	vpi_get_value(handle, &value);

	// Store inverted
	value.value.integer ^= 0xffffffff;
	if (vpi_get(vpiSize, handle) < 32) {
	    value.value.integer &= ~((1 << vpi_get(vpiSize, handle)) - 1);
	}
	vpi_put_value(handle, &value, NULL, vpiNoDelay);
    }

    return 0;
}

#define REP4(x) \
    (((x) & 0xff) << 24 | ((x) & 0xff) << 16 | ((x) & 0xff) << 8 | ((x) & 0xff))

extern "C" PLI_INT32 MemPoke(PLI_BYTE8 *)
{
    vpiHandle	mod_h, mem_h, iterate, handle;
    s_vpi_value	value;

    vpi_printf("MemPoke Callback\n");

    // get top module handle
    iterate = vpi_iterate(vpiModule, NULL);
    if (iterate == NULL) return -1;
    mod_h = vpi_scan(iterate);
    vpi_free_object(iterate);

    // Get memory
    mem_h = NULL;
    iterate = vpi_iterate(vpiMemory, mod_h);
    if (iterate != NULL) {
        while ((handle = vpi_scan(iterate))) {
            if (!strcmp("m_poke", vpi_get_str(vpiName, handle))) {
		vpiHandle memw_iter = vpi_iterate(vpiMemoryWord, handle);
		vpi_printf("  Found %s (%d deep x %d bits)\n",
		    vpi_get_str(vpiName, handle),
		    (int)vpi_get(vpiSize, handle),
		    (int)vpi_get(vpiSize, vpi_scan(memw_iter)));
		vpi_free_object(memw_iter);
		mem_h = handle;
		vpi_free_object(iterate);
                break;
            }
        }
    }

    // Poke memory using integers
    iterate = vpi_iterate(vpiMemoryWord, mem_h);
    while ((handle = vpi_scan(iterate))) {
	value.format = vpiIntVal;
	vpi_get_value(vpi_handle(vpiIndex, handle), &value);
	value.value.integer = REP4(1 + value.value.integer);
	if (vpi_get(vpiSize, handle) < 32) {
	    value.value.integer &= ~((1 << vpi_get(vpiSize, handle)) - 1);
	}
	vpi_put_value(handle, &value, NULL, vpiNoDelay);
    }

    return 0;
}

extern "C" void
RegisterCallbacks(void)
{
    s_vpi_systf_data tf_data;

    vpi_printf("Registering Callbacks\n");

    tf_data.type = vpiSysTask;
    tf_data.tfname = "$mempoke";
    tf_data.calltf = MemPoke;
    tf_data.compiletf = 0;
    tf_data.sizetf = 0;
    vpi_register_systf(&tf_data);

    tf_data.tfname = "$mempeek";
    tf_data.calltf = MemPeek;
    vpi_register_systf(&tf_data);
}

#ifdef __SUNPRO_CC
extern "C"
#endif
void (*vlog_startup_routines[]) () = {
    RegisterCallbacks,
    0
};
