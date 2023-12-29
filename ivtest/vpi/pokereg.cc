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
#include <stdlib.h>
#include "vpi_user.h"

static struct str_s {
    int		format;
    const char	*str;
} words[4] = {
    {vpiBinStrVal, "x001x001"},
    {vpiOctStrVal, "0x2"},
    {vpiDecStrVal, "3"},
    {vpiHexStrVal, "x4"}
};

extern "C" PLI_INT32 RegPeek(PLI_BYTE8 *)
{
    vpiHandle	mod_h, iterate, handle;
    vpiHandle	reg_h[5];
    s_vpi_value	value;
    int		index;

    vpi_printf("RegPeek Callback\n");

    // get top module handle
    iterate = vpi_iterate(vpiModule, NULL);
    if (iterate == NULL) return -1;
    mod_h = vpi_scan(iterate);
    vpi_free_object(iterate);

    // Get register
    iterate = vpi_iterate(vpiReg, mod_h);
    if (iterate == NULL) return -1;
    for (index = 0; index < 5; index++) reg_h[index] = NULL;
    while ((handle = vpi_scan(iterate))) {
        if (!strcmp("r_peek_1", vpi_get_str(vpiName, handle))) {
	    reg_h[0] = handle;
        } else if (!strcmp("r_peek_2", vpi_get_str(vpiName, handle))) {
	    reg_h[1] = handle;
        } else if (!strcmp("r_peek_3", vpi_get_str(vpiName, handle))) {
	    reg_h[2] = handle;
        } else if (!strcmp("r_peek_4", vpi_get_str(vpiName, handle))) {
	    reg_h[3] = handle;
        } else if (!strcmp("r_peek_5", vpi_get_str(vpiName, handle))) {
	    reg_h[4] = handle;
        }
    }

    // Get value
    for (index = 0; index < 5; index++) {
	// Print out info
	value.format=vpiBinStrVal;
	vpi_get_value(reg_h[index], &value);
	vpi_printf("%3d: 'b_%s,", index, value.value.str);

	value.format=vpiOctStrVal;
	vpi_get_value(reg_h[index], &value);
	vpi_printf(" 'o_%s,", value.value.str);

	value.format=vpiDecStrVal;
	vpi_get_value(reg_h[index], &value);
	vpi_printf(" 'd_%s,", value.value.str);

	value.format=vpiHexStrVal;
	vpi_get_value(reg_h[index], &value);
	vpi_printf(" 'h_%s\n", value.value.str);
    }

    return 0;
}

extern "C" PLI_INT32 RegPoke(PLI_BYTE8 *)
{
    vpiHandle	mod_h, iterate, handle;
    vpiHandle	reg_h[5];
    s_vpi_value	value;
    int		index;

    vpi_printf("RegPoke Callback\n");

    // get top module handle
    iterate = vpi_iterate(vpiModule, NULL);
    if (iterate == NULL) return -1;
    mod_h = vpi_scan(iterate);
    vpi_free_object(iterate);

    // Get register
    iterate = vpi_iterate(vpiReg, mod_h);
    if (iterate == NULL) return -1;
    for (index = 0; index < 5; index++) reg_h[index] = NULL;
    while ((handle = vpi_scan(iterate))) {
        if (!strcmp("r_poke_1", vpi_get_str(vpiName, handle))) {
            reg_h[0] = handle;
        } else if (!strcmp("r_poke_2", vpi_get_str(vpiName, handle))) {
            reg_h[1] = handle;
        } else if (!strcmp("r_poke_3", vpi_get_str(vpiName, handle))) {
            reg_h[2] = handle;
        } else if (!strcmp("r_poke_4", vpi_get_str(vpiName, handle))) {
            reg_h[3] = handle;
        } else if (!strcmp("r_poke_5", vpi_get_str(vpiName, handle))) {
            reg_h[4] = handle;
        }
    }

    // Poke register using integer and strings
    for (index = 0; index < 5; index++) {
	if (index < 4) {
	    value.format=words[index].format;
	    value.value.str=strdup(words[index].str);
	    vpi_printf("%3d: %s\n", index, value.value.str);
	} else {
	    value.format=vpiIntVal;
	    value.value.integer = 69;
	    vpi_printf("%3d: %d\n", index, (int)value.value.integer);
	}
	vpi_put_value(reg_h[index], &value, NULL, vpiNoDelay);
	if (index < 4) free(value.value.str);
    }

    return 0;
}

extern "C" void
RegisterCallbacks(void)
{
    s_vpi_systf_data tf_data;

    vpi_printf("Registering Callbacks\n");

    tf_data.type = vpiSysTask;
    tf_data.tfname = "$regpoke";
    tf_data.calltf = RegPoke;
    tf_data.compiletf = 0;
    tf_data.sizetf = 0;
    vpi_register_systf(&tf_data);

    tf_data.tfname = "$regpeek";
    tf_data.calltf = RegPeek;
    vpi_register_systf(&tf_data);
}

#ifdef __SUNPRO_CC
extern "C"
#endif
void (*vlog_startup_routines[])() =
{
    RegisterCallbacks,
    0
};
