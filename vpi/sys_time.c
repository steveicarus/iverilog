/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: sys_time.c,v 1.10 2004/01/21 01:22:53 steve Exp $"
#endif

# include "vpi_config.h"

# include  "vpi_user.h"
# include  <string.h>
# include  <math.h>
# include  <assert.h>

static vpiHandle module_of_function(vpiHandle obj)
{
      while (vpi_get(vpiType, obj) != vpiModule) {
	    obj = vpi_handle(vpiScope, obj);
	    assert(obj);
      }

      return obj;
}

static int sys_time_sizetf(char*x)
{
      return 64;
}

static int sys_stime_sizetf(char*x)
{
      return 32;
}

static int sys_time_calltf(char*name)
{
      s_vpi_value val;
      s_vpi_time  now;
      vpiHandle call_handle;
      vpiHandle mod;
      int units, prec;
      long scale;
      long frac;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

      mod = module_of_function(call_handle);

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
      { long long tmp_now = ((long long)now.high) << 32;
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

static int sys_realtime_calltf(char*name)
{
      s_vpi_value val;
      s_vpi_time  now;
      vpiHandle call_handle;
      vpiHandle mod;
      int units, prec;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

      mod = module_of_function(call_handle);

      now.type = vpiSimTime;
      vpi_get_time(0, &now);

      units = vpi_get(vpiTimeUnit, mod);
      prec  = vpi_get(vpiTimePrecision, 0);

      val.format = vpiRealVal;
      val.value.real = pow(10.0, prec-units) * now.low;
      assert(now.high == 0);

      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

      return 0;
}

void sys_time_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$time";
      tf_data.calltf    = sys_time_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = sys_time_sizetf;
      tf_data.user_data = "$time";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$realtime";
      tf_data.calltf    = sys_realtime_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$realtime";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$stime";
      tf_data.calltf    = sys_time_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = sys_stime_sizetf;
      tf_data.user_data = "$stime";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$simtime";
      tf_data.calltf    = sys_time_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = sys_time_sizetf;
      tf_data.user_data = "$simtime";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_time.c,v $
 * Revision 1.10  2004/01/21 01:22:53  steve
 *  Give the vip directory its own configure and vpi_config.h
 *
 * Revision 1.9  2003/06/18 00:54:28  steve
 *  Account for all 64 bits in results of $time.
 *
 * Revision 1.8  2003/02/07 02:44:25  steve
 *  Properly round inter time values from $time.
 *
 * Revision 1.7  2003/01/28 04:41:55  steve
 *  Use more precise pow function to scale time by units.
 *
 * Revision 1.6  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.5  2002/12/21 00:55:58  steve
 *  The $time system task returns the integer time
 *  scaled to the local units. Change the internal
 *  implementation of vpiSystemTime the $time functions
 *  to properly account for this. Also add $simtime
 *  to get the simulation time.
 *
 * Revision 1.4  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2002/01/11 05:20:59  steve
 *  Add the stime system function.
 *
 * Revision 1.2  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2000/11/01 03:19:36  steve
 *  Add the general $time system function.
 *
 */

