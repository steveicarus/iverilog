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

#include <assert.h>
#include <math.h>
#include <string.h>
#include <vpi_user.h>
#include "sys_priv.h"

/*
 * Check that the function is called with the correct argument.
 */
static PLI_INT32 sys_clog2_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      assert(callh != 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      (void) name;  // Not used!

	/* We must have an argument. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s line %d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("$clog2 requires one numeric argument.\n");
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The argument must be numeric. */
      arg = vpi_scan(argv);
      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s line %d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("The first argument to $clog2 must be numeric.\n");
	    vpi_control(vpiFinish, 1);
      }

	/* We can have a maximum of one argument. */
      if (vpi_scan(argv) != 0) {
	    char msg [64];
	    snprintf(msg, 64, "ERROR: %s line %d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));

	    unsigned argc = 1;
	    while (vpi_scan(argv)) argc += 1;

            vpi_printf("%s $clog2 takes at most one argument.\n", msg);
            vpi_printf("%*s Found %u extra argument%s.\n",
	               strlen(msg), " ", argc, argc == 1 ? "" : "s");
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

static PLI_INT32 sys_clog2_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      (void) name;  // Not used!/

	/* Get the argument. */
      arg = vpi_scan(argv);
      val.format = vpiRealVal;
      vpi_get_value(arg, &val);
      vpi_free_object(argv);

	/* For now we don't support a negative value! */
      if (val.value.real < 0.0) {
	    vpi_printf("SORRY: %s line %d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("$clog2 does not currently support negative values.\n");
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Return the value to the system. */
      val.format = vpiIntVal;
      if (val.value.real == 0.0)
	    val.value.integer = 0;
      else
	    val.value.integer = ceil(log(floor(val.value.real+0.5))/M_LN2);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

/*
 * Register the function with Verilog.
 */
void sys_clog2_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.calltf      = sys_clog2_calltf;
      tf_data.compiletf   = sys_clog2_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$clog2";
      tf_data.user_data   = 0;
      vpi_register_systf(&tf_data);
}
