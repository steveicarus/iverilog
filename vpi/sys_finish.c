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
#ident "$Id: sys_finish.c,v 1.9 2004/01/21 01:22:53 steve Exp $"
#endif

# include "vpi_config.h"

# include  "vpi_user.h"
# include  <string.h>

static int sys_finish_calltf(char *name)
{
      if (strcmp(name,"$stop") == 0) {
	    vpi_sim_control(vpiStop, 0);
	    return 0;
      }

      vpi_sim_control(vpiFinish, 0);
      return 0;
}

void sys_finish_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$finish";
      tf_data.calltf    = sys_finish_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$finish";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$stop";
      tf_data.calltf    = sys_finish_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$stop";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_finish.c,v $
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

