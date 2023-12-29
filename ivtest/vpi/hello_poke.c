/*
 * Copyright (c) 2001 Picture Elements, Inc.
 *    Stephen Williams (steve@picturel.com)
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

# include  <vpi_user.h>
# include  <assert.h>

static PLI_INT32 my_hello_calltf(PLI_BYTE8 *xx)
{
      s_vpi_value value;

      (void)xx;  /* Parameter is not used. */

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      if (argv == 0) {
	    vpi_printf("ERROR: $hello_poke requires one argument\n");
	    vpi_sim_control(vpiFinish, 1);
	    return 0;
      }

      arg = vpi_scan(argv);
      assert(arg != 0);

      value.format = vpiStringVal;
      value.value.str = "Hello";

      vpi_put_value(arg, &value, 0, vpiNoDelay);

      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: too many arguments to $hello_poke\n");
	    vpi_sim_control(vpiFinish, 1);
      }

      return 0;
}

static void my_hello_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$hello_poke";
      tf_data.calltf    = my_hello_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);
}

/*
 * This is a table of register functions. This table is the external
 * symbol that the simulator looks for when loading this .vpi module.
 */
void (*vlog_startup_routines[])(void) = {
      my_hello_register,
      0
};
