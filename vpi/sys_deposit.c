/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "sys_priv.h"
# include  <assert.h>
# include  <string.h>

static PLI_INT32 sys_deposit_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle target, value;

      /* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there are at least two arguments. */
      target = vpi_scan(argv);  /* This should never be zero. */
      value = vpi_scan(argv);
      if (value == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      assert(target);

      /* Check the targets type. It must be a net or a register. */
      switch (vpi_get(vpiType, target)) {
            case vpiNet:
            case vpiReg:
                  break;
            default:
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("invalid target type (%s) for %s.\n",
		             vpi_get_str(vpiType, target), name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
      }

      /* Check that there is at most two arguments. */
      check_for_extra_args(argv, callh, name, "two arguments", 0);

      return 0;
}

static PLI_INT32 sys_deposit_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, target, value;
      s_vpi_value val;

      (void)name; /* Parameter is not used. */

      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      target = vpi_scan(argv);
      assert(target);
      value = vpi_scan(argv);
      assert(value);

      val.format = vpiIntVal;
      vpi_get_value(value, &val);

      vpi_put_value(target, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

void sys_deposit_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$deposit";
      tf_data.calltf    = sys_deposit_calltf;
      tf_data.compiletf = sys_deposit_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$deposit";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}

