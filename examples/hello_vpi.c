/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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

/*
 * This file contains an example VPI module to demonstrate the tools
 * to create vpi modules. To compile this module, use the iverilog-vpi
 * command like so:
 *
 *    iverilog-vpi hello_vpi.c
 *
 * The result is the hello_vpi.vpi module. See the hello_vpi.vl
 * program for example Verilog code to call this module.
 */

# include  <vpi_user.h>

static PLI_INT32 my_hello_calltf(char *xx)
{
      vpi_printf("Hello World, from VPI.\n");
      return 0;
}

static void my_hello_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$my_hello";
      tf_data.calltf    = my_hello_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);

}

/*
 * This is a table of register functions. This table is the external
 * symbol that the simulator looks for when loading this .vpi module.
 */
void (*vlog_startup_routines[])() = {
      my_hello_register,
      0
};
