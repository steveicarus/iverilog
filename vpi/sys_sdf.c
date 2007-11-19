/*
 * Copyright (c) 2007 Stephen Williams (steve@icarus.com)
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

# include "vpi_config.h"

# include  "vpi_user.h"
# include  "sdf_priv.h"
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

static PLI_INT32 sys_sdf_annotate_compiletf(PLI_BYTE8*name)
{
      return 0;
}

static PLI_INT32 sys_sdf_annotate_calltf(PLI_BYTE8*name)
{
      s_vpi_value value;
      vpiHandle sys = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      vpiHandle path = vpi_scan(argv);
      assert(path);

      vpi_free_object(argv);

      value.format = vpiStringVal;
      vpi_get_value(path, &value);

      if ((value.format != vpiStringVal) || !value.value.str) {
	    vpi_printf("ERROR: %s: File name argument (type=%d)"
		       " does not have a string value\n",
		       name, vpi_get(vpiType, path));
	    return 0;
      }

      char*path_str = strdup(value.value.str);
      FILE*sdf_fd = fopen(path_str, "r");
      assert(sdf_fd);

      sdf_process_file(sdf_fd, path_str);

      fclose(sdf_fd);
      free(path_str);
      return 0;
}

void sys_sdf_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$sdf_annotate";
      tf_data.calltf    = sys_sdf_annotate_calltf;
      tf_data.compiletf = sys_sdf_annotate_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$sdf_annotate";
      vpi_register_systf(&tf_data);
}
