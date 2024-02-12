/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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

# include  "vpi_user.h"
# include  <assert.h>

static PLI_INT32 next_sim_time_callback(struct t_cb_data*cb)
{
      vpiHandle obj = (vpiHandle)cb->user_data;
      s_vpi_value val;
      s_vpi_time  tim;

      val.format = vpiIntVal;
      vpi_get_value(obj, &val);

      tim.type = vpiSimTime;
      vpi_get_time(obj, &tim);

      vpi_printf("Callback time=%d %s=%d\n", (int)tim.low,
		 vpi_get_str(vpiName, obj),
		 (int)val.value.integer);
      return 0;
}

static PLI_INT32 test_next_compiletf(PLI_BYTE8 *name)
{
      (void)name;  /* Parameter is not used. */
      return 0;
}

static PLI_INT32 test_next_calltf(PLI_BYTE8 *name)
{
      vpiHandle sys, argv, value;
      static s_vpi_time dummy_time = {vpiSimTime, 0, 0, 0};

      (void)name;  /* Parameter is not used. */

      sys = vpi_handle(vpiSysTfCall, 0);
      assert(sys);

      argv = vpi_iterate(vpiArgument, sys);
      assert(argv);

      for (value = vpi_scan(argv) ;  value ;  value = vpi_scan(argv)) {
	    s_cb_data cb;
	    cb.reason = cbNextSimTime;
	    cb.time = &dummy_time;
	    cb.cb_rtn = next_sim_time_callback;
	    cb.user_data = (char*)value;
	    vpi_register_cb(&cb);
      }

      return 0;
}

static void register_functions(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$test_next_sim_time";
      tf_data.calltf    = test_next_calltf;
      tf_data.compiletf = test_next_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "";

      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
      register_functions,
      0
};
