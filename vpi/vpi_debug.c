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

/*
 * The $vpi_tree(...) system task dumps the scopes listed in the
 * arguments. If no arguments are given, then dump the scope that
 * contains the call to the $vpi_tree function.
 */
# include  "vpi_user.h"
# include  <string.h>
# include  <stdlib.h>

static void dump_object(vpiHandle item)
{
      PLI_INT32 item_type = vpi_get(vpiType, item);
      vpiHandle argv, cur;

      vpi_printf("%s: ", vpi_get_str(vpiFullName, item));
      vpi_printf("vpiType=%s (%d)\n", vpi_get_str(vpiType, item), item_type);

      switch (item_type) {

	      /* These types are themselves scopes and have objects within. */
	  case vpiModule:
	  case vpiFunction:
	  case vpiTask:
	  case vpiNamedBegin:
	  case vpiNamedFork:
	    argv = vpi_iterate(vpiScope, item);
	    for (cur = vpi_scan(argv) ; cur ; cur = vpi_scan(argv))
		  dump_object(cur);
	    break;
#if 0
	  case vpiRegArray:
	  case vpiNetArray:
	    vpi_printf("%s: ", vpi_get_str(vpiFullName, item));
	    vpi_printf("vpiType=%s (%d)\n", vpi_get_str(vpiType, item), item_type);
	    argv = vpi_iterate(vpiMember, item);
	    for (cur = vpi_scan(argv) ; cur ; cur = vpi_scan(argv))
		  dump_object(cur);
	    break;
#endif
	      /* vpiMemory contains words. */
	  case vpiMemory:
	    argv = vpi_iterate(vpiMemoryWord, item);
	    for (cur = vpi_scan(argv) ; cur ; cur = vpi_scan(argv))
		  dump_object(cur);

	    break;

	  default:
	    break;
      }
}

static PLI_INT32 vpi_tree_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      return 0;
}

static PLI_INT32 vpi_tree_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;

      if (argv == 0) {
	    item = vpi_handle(vpiScope, callh);
	    dump_object(item);
	    return 0;
      }

      for (item = vpi_scan(argv) ; item ; item = vpi_scan(argv))
	    dump_object(item);

      return 0;
}

void sys_register()
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$vpi_tree";
      tf_data.calltf    = vpi_tree_calltf;
      tf_data.compiletf = vpi_tree_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$vpi_tree";
      res = vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])() = {
      sys_register,
      0
};
