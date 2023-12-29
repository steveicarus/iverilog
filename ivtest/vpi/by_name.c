/*
 * Copyright (c) 2002	Michael Ruff (mruff at chiaro.com)
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
 * This test verifies some functionality of vpi_handle_by_name()
 */
#include <stdio.h>
#include <string.h>
#include "vpi_user.h"

static void FindHandleByName(void)
{
    int i;
    vpiHandle hand;
    char *s[] = {
	"top0",
	"top0.t_my",
	"top0.f_my",
	"top0.r",
	"top0.t",
	"top0.e",
	"top0.i",
	"top0.init",
	"top0.lvl1_0",
	"top0.lvl1_1",
	"top0.lvl1_0.lvl2.t_my",
	"top0.lvl1_0.lvl2.f_my",
	"top0.lvl1_0.lvl2.r",
	"top0.lvl1_0.lvl2.t",
	"top0.lvl1_0.lvl2.e",
	"top0.lvl1_0.lvl2.i",
	"top0.lvl1_0.lvl2.init",
	"top1",
	"noexsist",
	"top1.noexsist",
	"top1.lvl1.noexsist",
	0
    };

    for (i=0; s[i]; i++) {
	vpi_printf("Looking up %s: ", s[i]);
	hand = vpi_handle_by_name(s[i], NULL);
	if (hand)
	    vpi_printf("Found name = %s, type = %d\n",
		vpi_get_str(vpiName, hand), (int)vpi_get(vpiType, hand));
	else
	    vpi_printf("*** Not found ***\n");
    }
}

static PLI_INT32 CompileTF(PLI_BYTE8 *x)
{
	(void)x;  /* Parameter is not used. */
	FindHandleByName();
	return 0;
}

static void my_Register(void)
{
	s_vpi_systf_data tf_data;

	tf_data.type = vpiSysTask;
	tf_data.tfname = "$test";
	tf_data.calltf = 0;
	tf_data.compiletf = CompileTF;
	tf_data.sizetf = 0;
	vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = { my_Register, 0};
