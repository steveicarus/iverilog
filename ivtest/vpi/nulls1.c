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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA
 */

#include "vpi_user.h"
#include <stdlib.h>

PLI_INT32
ValueChange(p_cb_data cb_data)
{
	static s_vpi_time get_time = { vpiSimTime, 0, 0, 0 };

	(void)cb_data;  /* Parameter is not used. */

	vpi_get_time(NULL,&get_time);
	vpi_printf("%6d: Value Change\n", (int)get_time.low);
	return(0);
}

PLI_INT32 CompileTF(PLI_BYTE8 *user_data)
{
	s_cb_data cb_data;
	vpiHandle call_h=vpi_handle(vpiSysTfCall,NULL);
	vpiHandle arg_i,arg_h;

	(void)user_data;  /* Parameter is not used. */

	// Get First Argument and Setup Value Change Callback
	arg_i=vpi_iterate(vpiArgument,call_h);
	arg_h=vpi_scan(arg_i);
	vpi_free_object(arg_i);

	cb_data.reason    = cbValueChange;
	cb_data.cb_rtn    = ValueChange;
	cb_data.value     = NULL;
	cb_data.time      = NULL;
	cb_data.user_data = NULL;
	cb_data.obj       = arg_h;
	vpi_register_cb(&cb_data);

	return(0);
}

static void my_Register(void)
{
  s_vpi_systf_data tf_data;

	vpi_printf("Registering Callbacks\n");

  // Register the $Verbench call
  tf_data.type        = vpiSysTask;
  tf_data.user_data   = 0;
  tf_data.sizetf      = NULL;
  tf_data.tfname      = "$vpi_call";
  tf_data.calltf      = NULL;
  tf_data.compiletf   = CompileTF;
  vpi_register_systf(&tf_data);

}

void (*vlog_startup_routines[]) (void) = {
my_Register, 0};
