/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2000 Stephan Boettcher <stephan@nevis.columbia.edu>
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
# include  <assert.h>

static PLI_INT32 sys_deposit_calltf(char *name)
{
  vpiHandle sys, argv, target, value;
  s_vpi_value val;

  sys = vpi_handle(vpiSysTfCall, 0);
  assert(sys);
  argv = vpi_iterate(vpiArgument, sys);
  if (!argv)
    {
      vpi_printf("ERROR: %s requires parameters "
		 "(target, value)\n", name);
      return 0;
    }
  target = vpi_scan(argv);
  assert(target);
  value = vpi_scan(argv);
  assert(value);
  vpi_free_object(argv);

  val.format = vpiIntVal;
  vpi_get_value(value, &val);

  switch (vpi_get(vpiType, target))
    {
    default:
      vpi_printf("ERROR: %s invalid target parameter\n", name);
      break;
    case vpiNet:
    case vpiReg:
      vpi_put_value(target, &val, 0, vpiNoDelay);
      break;
    }

  return 0;
}

void sys_deposit_register()
{
  s_vpi_systf_data tf_data;

  tf_data.type      = vpiSysTask;
  tf_data.tfname    = "$deposit";
  tf_data.calltf    = sys_deposit_calltf;
  tf_data.compiletf = 0;
  tf_data.sizetf    = 0;
  tf_data.user_data = "$deposit";
  vpi_register_systf(&tf_data);
}
