/*
 * Copyright (c) 2003-2010 Stephen Williams (steve@icarus.com)
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

# include "sys_priv.h"

/*
 * This file contains the implementations of the VCD related
 * functions. These are stub versions
 */

# include  "vpi_user.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  <time.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  "vcd_priv.h"


static FILE*dump_file = 0;

static PLI_INT32 sys_dumpoff_calltf(char*name)
{
      return 0;
}

static PLI_INT32 sys_dumpon_calltf(char*name)
{
      return 0;
}

static PLI_INT32 sys_dumpall_calltf(char*name)
{
      return 0;
}

static void open_dumpfile(const char*path)
{
      dump_file = fopen(path, "w");

      if (dump_file == 0) {
	    vpi_mcd_printf(6,
			   "VCD Error: Unable to open %s for output.\n",
			   path);
	    return;
      } else {
	    fprintf(dump_file, "VCD Dump suppressed.\n");
      }
}

static PLI_INT32 sys_dumpfile_calltf(char*name)
{
      char*path;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item;

      if (argv && (item = vpi_scan(argv))) {
	    s_vpi_value value;

	    if (vpi_get(vpiType, item) != vpiConstant
		|| vpi_get(vpiConstType, item) != vpiStringConst) {
		  vpi_mcd_printf(6,
				 "VCD Error:"
				 " %s parameter must be a string constant\n",
				 name);
		  return 0;
	    }

	    value.format = vpiStringVal;
	    vpi_get_value(item, &value);
	    path = strdup(value.value.str);

	    vpi_free_object(argv);

      } else {
	    path = strdup("dumpfile.vcd");
      }

      if (dump_file) {
	    fclose(dump_file);
	    dump_file = 0;
      }

      assert(dump_file == 0);
      open_dumpfile(path);

      free(path);

      return 0;
}

static PLI_INT32 sys_dumpvars_calltf(char*name)
{
      if (dump_file == 0) {
	    open_dumpfile("dumpfile.vcd");
	    if (dump_file == 0)
		  return 0;
      }

      return 0;
}

void sys_vcdoff_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpall";
      tf_data.calltf    = sys_dumpall_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpall";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpoff";
      tf_data.calltf    = sys_dumpoff_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpoff";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpon";
      tf_data.calltf    = sys_dumpon_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpon";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpfile";
      tf_data.calltf    = sys_dumpfile_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpfile";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpvars";
      tf_data.calltf    = sys_dumpvars_calltf;
      tf_data.compiletf = sys_vcd_dumpvars_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpvars";
      vpi_register_systf(&tf_data);
}
