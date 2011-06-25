/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "vpi_config.h"
# include  "vpi_user.h"
# include  <cassert>

/*
 * The $ivlh_attribute_event implements the VHDL <varname>'event
 * attribute. It does this by monitoring value-change events on the
 * operand, and noting the time. If the $ivlh_attribute_event is
 * called at the same simulation time as a value-change, then the
 * function returns logic true. Otherwise it returns false.
 */
struct monitor_data {
      struct t_vpi_time last_event;
};

static PLI_INT32 monitor_events(struct t_cb_data*cb)
{
      struct monitor_data*mon = reinterpret_cast<monitor_data*> (cb->user_data);

      assert(cb->time);
      assert(cb->time->type == vpiSimTime);
      mon->last_event = *(cb->time);

      return 0;
}

static PLI_INT32 ivlh_attribute_event_compiletf(ICARUS_VPI_CONST PLI_BYTE8*)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

	// Check that there is at least 1 argument...
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, sys), vpi_get(vpiLineNo, sys));
	    vpi_printf("Call to %s missing its argument\n", vpi_get_str(vpiName,sys));
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      arg = vpi_scan(argv);
      assert(arg);

      struct monitor_data*monitor_handle = new struct monitor_data;

      struct t_cb_data cb;
      struct t_vpi_time tb;
      tb.type = vpiSimTime;
      cb.reason = cbValueChange;
      cb.cb_rtn = monitor_events;
      cb.obj = arg;
      cb.time = &tb;
      cb.value = 0;
      cb.user_data = reinterpret_cast<char*>(monitor_handle);
      vpi_register_cb(&cb);
      vpi_put_userdata(sys, monitor_handle);

	// check that there is no more then 1 argument
      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, sys), vpi_get(vpiLineNo, sys));
	    vpi_printf("Too many arguments for call to  %s.\n", vpi_get_str(vpiName,sys));
	    vpi_control(vpiFinish, 1);
      }

      return 0;
}

static PLI_INT32 ivlh_attribute_event_calltf(ICARUS_VPI_CONST PLI_BYTE8*)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);

      struct t_vpi_value rval;
      rval.format = vpiScalarVal;

      struct monitor_data*mon = reinterpret_cast<monitor_data*>(vpi_get_userdata(sys));

      if (mon->last_event.type == 0) {
	    rval.value.scalar = vpi0;

      } else {
	    struct t_vpi_time tnow;
	    tnow.type = vpiSimTime;
	    vpi_get_time(0,&tnow);

	    rval.value.scalar = vpi1;
	    if (mon->last_event.high != tnow.high)
		  rval.value.scalar = vpi0;
	    if (mon->last_event.low != tnow.low)
		  rval.value.scalar = vpi0;
      }

      vpi_put_value(sys, &rval, 0, vpiNoDelay);

      return 0;
}

static void vhdl_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$ivlh_attribute_event";
      tf_data.calltf    = ivlh_attribute_event_calltf;
      tf_data.compiletf = ivlh_attribute_event_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ivlh_attribute_event";
      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])() = {
      vhdl_register,
      0
};
