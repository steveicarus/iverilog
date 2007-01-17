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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: hello_vpi.c,v 1.5 2007/01/17 05:35:48 steve Exp $"
#endif

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
/*
 * $Log: hello_vpi.c,v $
 * Revision 1.5  2007/01/17 05:35:48  steve
 *  Fix typo is hello_vpi.c example.
 *
 * Revision 1.4  2006/10/30 22:46:25  steve
 *  Updates for Cygwin portability (pr1585922)
 *
 * Revision 1.3  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 * Revision 1.1  2002/04/18 03:25:16  steve
 *  More examples.
 *
 */

