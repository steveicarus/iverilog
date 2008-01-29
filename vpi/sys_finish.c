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
#ifdef HAVE_CVS_IDENT
#ident "$Id: sys_finish.c,v 1.11 2007/04/09 22:49:33 steve Exp $"
#endif

# include "vpi_config.h"

# include  "vpi_user.h"
# include  <string.h>

static PLI_INT32 sys_finish_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

      /* The argument is optional. */
      if (argv == 0) return 0;
      arg = vpi_scan(argv);

      /* A string diagnostic message level makes no sense. */
      if (vpi_get(vpiType, arg) == vpiConstant &&
          vpi_get(vpiConstType, arg) == vpiStringConst) {
            vpi_printf("Error: %s does not take a string argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* These functions take at most one argument (diagnostic message). */
      arg = vpi_scan(argv);
      if (arg != 0) {
            vpi_printf("Error: %s takes at most one argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* vpi_scan returning 0 (NULL) has already freed argv. */
      return 0;
}

static PLI_INT32 sys_finish_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, arg;
      s_vpi_value val;
      long diag_msg = 1;

      /* Get the argument list and look for the diagnostic message level. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      if (argv) {
            arg = vpi_scan(argv);
            vpi_free_object(argv);
            val.format = vpiIntVal;
            vpi_get_value(arg, &val);
            diag_msg = val.value.integer;
      }

      if (strcmp((char*)name, "$stop") == 0) {
	    vpi_control(vpiStop, diag_msg);
	    return 0;
      }

      vpi_control(vpiFinish, diag_msg);
      return 0;
}

void sys_finish_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$finish";
      tf_data.calltf    = sys_finish_calltf;
      tf_data.compiletf = sys_finish_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = (PLI_BYTE8*)"$finish";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$stop";
      tf_data.calltf    = sys_finish_calltf;
      tf_data.compiletf = sys_finish_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = (PLI_BYTE8*)"$stop";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_finish.c,v $
 * Revision 1.11  2007/04/09 22:49:33  steve
 *  More strict use of PLI_BYTE8 type.
 *
 * Revision 1.10  2006/10/30 22:45:37  steve
 *  Updates for Cygwin portability (pr1585922)
 *
 * Revision 1.9  2004/01/21 01:22:53  steve
 *  Give the vip directory its own configure and vpi_config.h
 *
 * Revision 1.8  2003/02/21 03:24:03  steve
 *  Make the $stop system task really vpiStop.
 *
 * Revision 1.7  2002/08/12 01:35:04  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.5  2001/01/01 19:33:44  steve
 *  Add $stop that does a finish.
 *
 * Revision 1.4  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.3  1999/08/28 02:10:44  steve
 *  Call the right vpiFinish code.
 *
 * Revision 1.2  1999/08/19 02:51:03  steve
 *  Add vpi_sim_control
 *
 * Revision 1.1  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 */

