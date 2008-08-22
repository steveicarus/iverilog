/*
 *  Copyright (C) 2008  Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <vpi_user.h>

static PLI_INT32 task_not_implemented_compiletf(char* name)
{
      vpi_printf("SORRY: task %s() is not currently implemented in V0.8.\n",
                 name);
      vpi_control(vpiFinish, 1);
      return 0;
}

static PLI_INT32 function_not_implemented_compiletf(char* name)
{
      vpi_printf("SORRY: function %s() is not currently implemented in V0.8.\n",
                 name);
      vpi_control(vpiFinish, 1);
      return 0;
}

/*
 * Register the function with Verilog.
 */
void sys_missing_register(void)
{
      s_vpi_systf_data tf_data;

	/* These tasks are not currently implemented. */
      tf_data.type        = vpiSysTask;
      tf_data.calltf      = 0;
      tf_data.sizetf      = 0;
      tf_data.compiletf   = task_not_implemented_compiletf;

      tf_data.tfname      = "$fmonitor";
      tf_data.user_data   = "$fmonitor";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fmonitorb";
      tf_data.user_data   = "$fmonitorb";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fmonitoro";
      tf_data.user_data   = "$fmonitoro";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fmonitorh";
      tf_data.user_data   = "$fmonitorh";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$and$array";
      tf_data.user_data   = "$async$and$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$nand$array";
      tf_data.user_data   = "$async$nand$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$or$array";
      tf_data.user_data   = "$async$or$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$nor$array";
      tf_data.user_data   = "$async$nor$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$and$plane";
      tf_data.user_data   = "$async$and$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$nand$plane";
      tf_data.user_data   = "$async$nand$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$or$plane";
      tf_data.user_data   = "$async$or$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$async$nor$plane";
      tf_data.user_data   = "$async$nor$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$and$array";
      tf_data.user_data   = "$sync$and$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$nand$array";
      tf_data.user_data   = "$sync$nand$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$or$array";
      tf_data.user_data   = "$sync$or$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$nor$array";
      tf_data.user_data   = "$sync$nor$array";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$and$plane";
      tf_data.user_data   = "$sync$and$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$nand$plane";
      tf_data.user_data   = "$sync$nand$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$or$plane";
      tf_data.user_data   = "$sync$or$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sync$nor$plane";
      tf_data.user_data   = "$sync$nor$plane";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$q_initialize";
      tf_data.user_data   = "$q_initialize";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$q_add";
      tf_data.user_data   = "$q_add";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$q_remove";
      tf_data.user_data   = "$q_remove";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$q_full";
      tf_data.user_data   = "$q_full";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$q_exam";
      tf_data.user_data   = "$q_exam";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fwriteb";
      tf_data.user_data   = "$fwriteb";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fwriteo";
      tf_data.user_data   = "$fwriteo";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fwriteh";
      tf_data.user_data   = "$fwriteh";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$swrite";
      tf_data.user_data   = "$swrite";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$swriteb";
      tf_data.user_data   = "$swriteb";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$swriteo";
      tf_data.user_data   = "$swriteo";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$swriteh";
      tf_data.user_data   = "$swriteh";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sformat";
      tf_data.user_data   = "$format";
      vpi_register_systf(&tf_data);

	/* These functions are not currently implemented. */
      tf_data.compiletf   = function_not_implemented_compiletf;
      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;

      tf_data.tfname      = "$ferror";
      tf_data.user_data   = "$ferror";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fread";
      tf_data.user_data   = "$fread";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$rewind";
      tf_data.user_data   = "$rewind";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fscanf";
      tf_data.user_data   = "$fscanf";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$sscanf";
      tf_data.user_data   = "$sscanf";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$fseek";
      tf_data.user_data   = "$fseek";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$ftell";
      tf_data.user_data   = "$ftell";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dist_chi_square";
      tf_data.user_data   = "$dist_chi_square";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dist_erlang";
      tf_data.user_data   = "$dist_erlang";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dist_exponential";
      tf_data.user_data   = "$dist_exponential";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dist_normal";
      tf_data.user_data   = "$dist_normal";
      vpi_register_systf(&tf_data);

      tf_data.tfname      = "$dist_t";
      tf_data.user_data   = "$dist_t";
      vpi_register_systf(&tf_data);
}
