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

/*
 * This program tests change callbacks on real variables.
 */
# include  <vpi_user.h>
# include  <assert.h>

static PLI_INT32 watchreal_cb(p_cb_data cb)
{
      s_vpi_value value;
      vpiHandle arg = (vpiHandle) (cb->user_data);

      value.format = vpiRealVal;
      vpi_get_value(arg, &value);
      assert(value.format == vpiRealVal);

      vpi_printf("watchreal: %s = %f\n",
		 vpi_get_str(vpiName, arg),
		 value.value.real);

      return 0;
}

static PLI_INT32 my_watchreal_calltf(PLI_BYTE8 *xx)
{
      struct t_cb_data cb;
      struct t_vpi_time timerec;

      (void)xx;  /* Parameter is not used. */

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      vpiHandle arg;

      timerec.type = vpiSimTime;
      timerec.low = 0;
      timerec.high = 0;

      while (0 != (arg = vpi_scan(argv))) {

	    assert(vpi_get(vpiType, arg) == vpiRealVar);

	    cb.reason = cbValueChange;
	    cb.cb_rtn = watchreal_cb;
	    cb.time = &timerec;
	    cb.obj = arg;
	    cb.value = 0;
	    cb.user_data = (char*)arg;
	    vpi_register_cb(&cb);

      }

      return 0;
}

static void my_watchreal_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$my_watchreal";
      tf_data.calltf    = my_watchreal_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
      my_watchreal_register,
      0
};
