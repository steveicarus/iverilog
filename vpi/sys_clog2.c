/*
 *  Copyright (C) 2008-2021  Cary R. (cygcary@yahoo.com)
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
#include "vpi_user.h"
#include "sv_vpi_user.h"

/*
 * This routine returns 1 if the argument supports has a numeric value,
 * otherwise it returns 0.
 *
 * This is copied from sys_priv.c.
 */
static unsigned is_numeric_obj(vpiHandle obj)
{
    unsigned rtn = 0;

    assert(obj);

    switch(vpi_get(vpiType, obj)) {
      case vpiConstant:
      case vpiParameter:
	  /* These cannot be a string constant. */
	if (vpi_get(vpiConstType, obj) != vpiStringConst) rtn = 1;
	break;

	/* These can have a valid numeric value. */
      case vpiIntegerVar:
      case vpiBitVar:
      case vpiByteVar:
      case vpiShortIntVar:
      case vpiIntVar:
      case vpiLongIntVar:
      case vpiMemoryWord:
      case vpiNet:
      case vpiPartSelect:
      case vpiRealVar:
      case vpiReg:
      case vpiTimeVar:
	rtn = 1;
	break;
    }

    return rtn;
}

/*
 * Check that the function is called with the correct argument.
 */
static PLI_INT32 sys_clog2_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, arg;

      assert(callh != 0);
      argv = vpi_iterate(vpiArgument, callh);
      (void)name;  /* Parameter is not used. */

	/* We must have an argument. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("$clog2 requires one numeric argument.\n");
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The argument must be numeric. */
      arg = vpi_scan(argv);
      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("The first argument to $clog2 must be numeric.\n");
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* We can have a maximum of one argument. */
      if (vpi_scan(argv) != 0) {
	    char msg[64];
	    unsigned argc;

	    snprintf(msg, sizeof(msg), "ERROR: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;

	    argc = 1;
	    while (vpi_scan(argv)) argc += 1;

            vpi_printf("%s $clog2 takes at most one argument.\n", msg);
            vpi_printf("%*s Found %u extra argument%s.\n",
	               (int) strlen(msg), " ", argc, argc == 1 ? "" : "s");
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

static PLI_INT32 sys_clog2_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      s_vpi_vecval vec;
      (void)name;  /* Parameter is not used. */

	/* Get the argument. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);

      vec = vpip_calc_clog2(arg);

      val.format = vpiVectorVal;
      val.value.vector = &vec;
      vpi_put_value(callh, &val, 0, vpiNoDelay);
      return 0;
}

/*
 * Register the function with Verilog.
 */
void sys_clog2_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.calltf      = sys_clog2_calltf;
      tf_data.compiletf   = sys_clog2_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$clog2";
      tf_data.user_data   = 0;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
