/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: sys_vcd.c,v 1.1 1999/11/07 20:33:30 steve Exp $"
#endif

/*
 * This file contains the implementations of the VCD related
 * funcitons.
 */

# include  <vpi_user.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

static FILE*dump_file = 0;

struct vcd_info {
      vpiHandle item;
      vpiHandle cb;
      struct t_vpi_time time;
      char*ident;
      struct vcd_info*next;
};

static struct vcd_info*vcd_list = 0;
unsigned long vcd_cur_time = 0;

static int variable_cb(p_cb_data cause)
{
      unsigned long now = cause->time->low;
      s_vpi_value value;
      struct t_cb_data cb;
      struct vcd_info*info = (struct vcd_info*)cause->user_data;

	/* Reschedule this event so that it happens for the next
	   trigger on this variable. */
      cb = *cause;
      vpi_register_cb(&cb);

      if (now != vcd_cur_time) {
	    fprintf(dump_file, "#%lu\n", now);
	    vcd_cur_time = now;
      }

      if (vpi_get(vpiSize, info->item) == 1) {
	    value.format = vpiBinStrVal;
	    vpi_get_value(info->item, &value);
	    fprintf(dump_file, "%s%s\n", value.value.str, info->ident);
      } else {
	    value.format = vpiBinStrVal;
	    vpi_get_value(info->item, &value);
	    fprintf(dump_file, "b%s %s\n", value.value.str, info->ident);
      }

      return 0;
}

static int sys_dumpall_calltf(char*name)
{
      return 0;
}

static int sys_dumpfile_calltf(char*name)
{
      char*path;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item) {
	    s_vpi_value value;

	    if (vpi_get(vpiType, item) != vpiConstant) {
		  vpi_printf("ERROR: %s parameter must be a constant\n", name);
		  return 0;
	    }

	    if (vpi_get(vpiConstType, item) != vpiStringConst) {
		  vpi_printf("ERROR: %s parameter must be a constant\n", name);
		  return 0;
	    }

	    value.format = vpiStringVal;
	    vpi_get_value(item, &value);
	    path = strdup(value.value.str);

	    vpi_free_object(argv);

      } else {
	    path = strdup("dumpfile.vcd");
      }

      assert(dump_file == 0);
      dump_file = fopen(path, "w");
      if (dump_file == 0) {
	    vpi_printf("ERROR: Unable to open %s for output.\n", path);
	    return 0;
      }

      free(path);

      return 0;
}

static int sys_dumpvars_calltf(char*name)
{
      struct t_cb_data cb;
      struct vcd_info*info;
      char ident[64];
      unsigned nident = 0;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item == 0) {
	    vpi_printf("SORRY: %s requires arguments\n", name);
	    return 0;
      }

      assert(dump_file);

      cb.reason = cbValueChange;
      cb.cb_rtn = variable_cb;

      for (item = vpi_scan(argv) ;  item ;  item = vpi_scan(argv)) {

	    switch (vpi_get(vpiType, item)) {
		case vpiNet:
		case vpiReg: {
		      const char*type = "wire";
		      if (vpi_get(vpiType, item) == vpiReg)
			    type = "reg";

		      sprintf(ident, "<%u", nident++);
		      info = malloc(sizeof(*info));
		      info->time.type = vpiSimTime;
		      cb.time = &info->time;
		      cb.user_data = (char*)info;
		      cb.obj = item;
		      info->item  = item;
		      info->ident = strdup(ident);
		      info->cb    = vpi_register_cb(&cb);
		      info->next = vcd_list;
		      vcd_list   = info;
		      fprintf(dump_file, "$var %s %u %s %s $end\n",
			      type, vpi_get(vpiSize, item), ident,
			      vpi_get_str(vpiFullName, item));
		      break;
		}

		default:
		  vpi_printf("ERROR: (%s): Unsupported parameter type\n",
			     name);
	    }
      }

      fprintf(dump_file, "$enddefinitions $end\n");
      fprintf(dump_file, "#0\n");

      return 0;
}

void sys_vcd_register()
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
      tf_data.tfname    = "$dumpfile";
      tf_data.calltf    = sys_dumpfile_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpfile";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpvars";
      tf_data.calltf    = sys_dumpvars_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpvars";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_vcd.c,v $
 * Revision 1.1  1999/11/07 20:33:30  steve
 *  Add VCD output and related system tasks.
 *
 */

