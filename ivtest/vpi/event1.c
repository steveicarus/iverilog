/*
 * Copyright (c) 2003	Michael Ruff (mruff at chiaro.com)
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
 * Test that named events are passed properly to system tasks.
 */
#include <stdio.h>
#include <string.h>
#include "vpi_user.h"

static int num;

static PLI_INT32 CompileTF(PLI_BYTE8 *x)
{
	vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
	vpiHandle argv = vpi_iterate(vpiArgument, sys);
	vpiHandle arg;
	int first = 1;

	(void)x;  /* Parameter is not used. */

	vpi_printf("%s (", vpi_get_str(vpiName, sys));
	while ((arg = vpi_scan(argv))) {
	    if (!first)
		vpi_printf(", ");
	    else
		first = 0;
	    vpi_printf("%s [type = %d]",
		vpi_get_str(vpiFullName, arg), (int)vpi_get(vpiType, arg));
	    if (vpi_get(vpiType, arg) == vpiNamedEvent) num++;
	}
	vpi_printf(")\n");
	if (num == 2) vpi_printf("PASSED\n");
	return 0;
}

static void Register(void)
{
	s_vpi_systf_data tf_data;

	tf_data.type = vpiSysTask;
	tf_data.tfname = "$test";
	tf_data.calltf = 0;
	tf_data.compiletf = CompileTF;
	tf_data.sizetf = 0;
	vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = { Register, 0};
