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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: sys_time.c,v 1.2 2001/07/25 03:10:50 steve Exp $"
#endif

# include "config.h"

# include  "vpi_user.h"
# include  <assert.h>

static int sys_time_sizetf(char*x)
{
      return 64;
}

static int sys_time_calltf(char*name)
{
      s_vpi_value val;
      s_vpi_time  now;
      vpiHandle call_handle;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

      vpi_get_time(0, &now);

      val.format = vpiIntVal;
      val.value.integer = now.low;
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
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_time.c,v $
 * Revision 1.2  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2000/11/01 03:19:36  steve
 *  Add the general $time system function.
 *
 */

