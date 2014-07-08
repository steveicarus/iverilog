/*
 * Copyright (c) 2000-2011 Stephen Williams (steve@icarus.com)
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

#include "sys_priv.h"
#include <string.h>
#include <math.h>
#include <assert.h>

static PLI_INT32 sys_time_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_value val;
      s_vpi_time  now;
      vpiHandle call_handle;
      vpiHandle mod;
      int units, prec;
      long scale;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

      mod = sys_func_module(call_handle);

      now.type = vpiSimTime;
      vpi_get_time(0, &now);

	/* All the variants but $simtime return the time in units of
	   the local scope. The $simtime function returns the
	   simulation time. */
      if (strcmp(name, "$simtime") == 0)
	    units = vpi_get(vpiTimePrecision, 0);
      else
	    units = vpi_get(vpiTimeUnit, mod);

      prec  = vpi_get(vpiTimePrecision, 0);
      scale = 1;
      while (units > prec) {
	    scale *= 10;
	    units -= 1;
      }

      assert(8*sizeof(long long) >= 64);
      { long frac;
        long long tmp_now = ((long long)now.high) << 32;
        tmp_now += (long long)now.low;
	frac = tmp_now % (long long)scale;
	tmp_now /= (long long)scale;

	  /* Round to the nearest integer, which may be up. */
	if ((scale > 1) && (frac >= scale/2))
	      tmp_now += 1;

	now.low = tmp_now & 0xffffffff;
	now.high = tmp_now >> 32LL;
      }

      val.format = vpiTimeVal;
      val.value.time = &now;

      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

      return 0;
}

/* This also supports $abstime() from VAMS-2.3. */
static PLI_INT32 sys_realtime_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_value val;
      s_vpi_time  now;
      vpiHandle callh;
      vpiHandle mod;

      callh = vpi_handle(vpiSysTfCall, 0);
      assert(callh);

      mod = sys_func_module(callh);

      now.type = vpiScaledRealTime;
      vpi_get_time(mod, &now);

	/* For $abstime() we return the time in second. */
      if (strcmp(name, "$abstime") == 0) {
	    PLI_INT32 scale = vpi_get(vpiTimeUnit, mod);
	    if (scale >= 0) now.real *= pow(10.0, scale);
	    else now.real /= pow(10.0, -scale);
      }

      val.format = vpiRealVal;
      val.value.real = now.real;
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

void sys_time_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.tfname      = "$time";
      tf_data.sysfunctype = vpiTimeFunc;
      tf_data.calltf      = sys_time_calltf;
      tf_data.compiletf   = sys_no_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$time";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.tfname      = "$realtime";
      tf_data.sysfunctype = vpiRealFunc;
      tf_data.calltf      = sys_realtime_calltf;
      tf_data.compiletf   = sys_no_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$realtime";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.tfname      = "$stime";
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.calltf      = sys_time_calltf;
      tf_data.compiletf   = sys_no_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$stime";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.tfname      = "$simtime";
      tf_data.sysfunctype = vpiTimeFunc;
      tf_data.calltf      = sys_time_calltf;
      tf_data.compiletf   = sys_no_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$simtime";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.tfname      = "$abstime";
      tf_data.sysfunctype = vpiRealFunc;
      tf_data.calltf      = sys_realtime_calltf;
      tf_data.compiletf   = sys_no_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$abstime";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
