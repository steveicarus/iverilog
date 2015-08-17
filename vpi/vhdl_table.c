/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "vpi_config.h"
# include  "vpi_user.h"
# include  <assert.h>
# include  "ivl_alloc.h"

/*
 * The $ivlh_attribute_event implements the VHDL <varname>'event
 * attribute. It does this by monitoring value-change events on the
 * operand, and noting the time. If the $ivlh_attribute_event is
 * called at the same simulation time as a value-change, then the
 * function returns logic true. Otherwise it returns false.
 *
 * $ivlh_{rising,falling}_edge implement the VHDL rising_edge() and
 * falling_edge() system functions.
 */
struct monitor_data {
      struct t_vpi_time last_event;
      struct t_vpi_value last_value;
};

static struct monitor_data **mdata = 0;
static unsigned mdata_count = 0;

typedef enum { EVENT = 0, RISING_EDGE = 1, FALLING_EDGE = 2 } event_type_t;
static const char* func_names[] = {
      "$ivlh_attribute_event",
      "$ivlh_rising_edge",
      "$ivlh_falling_edge"
};

/* To keep valgrind happy free the allocated memory. */
static PLI_INT32 cleanup_mdata(p_cb_data cause)
{
      unsigned idx;

      (void) cause;  /* Parameter is not used. */

      for (idx= 0; idx < mdata_count; idx += 1) {
	    free(mdata[idx]);
      }
      free(mdata);
      mdata = 0;
      mdata_count = 0;

      return 0;
}

static PLI_INT32 monitor_events(struct t_cb_data*cb)
{
      struct monitor_data*mon = (struct monitor_data*)(cb->user_data);

      assert(cb->time);
      assert(cb->time->type == vpiSimTime);

      mon->last_event = *(cb->time);
      mon->last_value = *(cb->value);

      return 0;
}

static PLI_INT32 ivlh_attribute_event_compiletf(ICARUS_VPI_CONST PLI_BYTE8*data)
{
      event_type_t type = (event_type_t) data;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;
      struct monitor_data*mon;
      struct t_cb_data cb;
      struct t_vpi_time tb;
      struct t_vpi_value vb;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, sys),
	               (int)vpi_get(vpiLineNo, sys));
	    vpi_printf("(compiler error) %s requires a single argument.\n",
	               func_names[type]);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Icarus either returns 0 above or has one argument. */
      arg = vpi_scan(argv);
      assert(arg);

      mon = malloc(sizeof(struct monitor_data));
	/* Add this to the list of data. */
      mdata_count += 1;
      mdata = (struct monitor_data **) realloc(mdata,
                                               sizeof(struct monitor_data **) *
                                               mdata_count);
      mdata[mdata_count-1] = mon;

      tb.type = vpiSimTime;
      vb.format = vpiScalarVal;
      cb.reason = cbValueChange;
      cb.cb_rtn = monitor_events;
      cb.obj = arg;
      cb.time = &tb;
      cb.value = &vb;
      cb.user_data = (char*) (mon);
      vpi_register_cb(&cb);
      vpi_put_userdata(sys, mon);

	/* Check that there is no more than one argument. */
      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, sys),
	               (int)vpi_get(vpiLineNo, sys));
	    vpi_printf("(compiler error) %s only takes a single argument.\n",
	               func_names[type]);
	    vpi_free_object(argv);
	    vpi_control(vpiFinish, 1);
      }

      return 0;
}

static PLI_INT32 ivlh_attribute_event_calltf(ICARUS_VPI_CONST PLI_BYTE8*data)
{
      event_type_t type = (event_type_t) data;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      struct t_vpi_value rval;
      struct monitor_data*mon;

      rval.format = vpiScalarVal;

      mon = (struct monitor_data*) (vpi_get_userdata(sys));

      if (mon->last_event.type == 0) {
	    rval.value.scalar = vpi0;

      } else {
	    struct t_vpi_time tnow;
	    tnow.type = vpiSimTime;
	    vpi_get_time(0, &tnow);

	    rval.value.scalar = vpi1;

	    // Detect if change occurred in this moment
	    if (mon->last_event.high != tnow.high)
		  rval.value.scalar = vpi0;
	    if (mon->last_event.low != tnow.low)
		  rval.value.scalar = vpi0;

	    // Determine the edge, if required
	    if (type == RISING_EDGE && mon->last_value.value.scalar != vpi1)
		  rval.value.scalar = vpi0;
	    else if (type == FALLING_EDGE && mon->last_value.value.scalar != vpi0)
		  rval.value.scalar = vpi0;
      }

      vpi_put_value(sys, &rval, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 ivlh_attribute_event_sizetf(ICARUS_VPI_CONST PLI_BYTE8*type)
{
      (void) type;  /* Parameter is not used. */
      return 1;
}

static void vhdl_register(void)
{
      s_vpi_systf_data tf_data;
      s_cb_data cb;
      vpiHandle res;

      tf_data.type         = vpiSysFunc;
      tf_data.sysfunctype  = vpiSizedFunc;
      tf_data.calltf       = ivlh_attribute_event_calltf;
      tf_data.compiletf    = ivlh_attribute_event_compiletf;
      tf_data.sizetf       = ivlh_attribute_event_sizetf;
      tf_data.tfname       = func_names[EVENT];
      tf_data.user_data    = (PLI_BYTE8*) EVENT;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type         = vpiSysFunc;
      tf_data.sysfunctype  = vpiSizedFunc;
      tf_data.calltf       = ivlh_attribute_event_calltf;
      tf_data.compiletf    = ivlh_attribute_event_compiletf;
      tf_data.sizetf       = ivlh_attribute_event_sizetf;
      tf_data.tfname       = func_names[RISING_EDGE];
      tf_data.user_data    = (PLI_BYTE8*) RISING_EDGE;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type         = vpiSysFunc;
      tf_data.sysfunctype  = vpiSizedFunc;
      tf_data.calltf       = ivlh_attribute_event_calltf;
      tf_data.compiletf    = ivlh_attribute_event_compiletf;
      tf_data.sizetf       = ivlh_attribute_event_sizetf;
      tf_data.tfname       = func_names[FALLING_EDGE];
      tf_data.user_data    = (PLI_BYTE8*) FALLING_EDGE;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

	/* Create a callback to clear the monitor data memory when the
	 * simulator finishes. */
      cb.time = NULL;
      cb.reason = cbEndOfSimulation;
      cb.cb_rtn = cleanup_mdata;
      cb.user_data = NULL;
      cb.obj = NULL;

      vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {
      vhdl_register,
      0
};
