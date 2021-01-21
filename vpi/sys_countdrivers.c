/*
 *  Copyright (C) 2012-2021  Martin Whitaker. (icarus@martin-whitaker.me.uk)
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

#include "sys_priv.h"
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include "ivl_alloc.h"

/*
 * Check to see if an argument is a single bit net.
 */
static void check_net_arg(vpiHandle arg, vpiHandle callh, const char *name)
{
      assert(arg);

      switch (vpi_get(vpiType, arg)) {
	  case vpiPartSelect:
            if (vpi_get(vpiType, vpi_handle(vpiParent, arg)) != vpiNet)
                  break;
	    // fallthrough
	  case vpiNet:
	    if (vpi_get(vpiSize, arg) != 1)
                  break;
            return;
	  default:
            break;
      }
      vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                 (int)vpi_get(vpiLineNo, callh));
      vpi_printf("%s's first argument must be a scalar net or "
                 "a bit-select of a vector net.\n", name);
      vpip_set_return_value(1);
      vpi_control(vpiFinish, 1);
}

/*
 * Check to see if an argument is a variable.
 */
static void check_var_arg(vpiHandle arg, vpiHandle callh, const char *name,
                          const char *arg_name)
{
      assert(arg);

      switch (vpi_get(vpiType, arg)) {
	  case vpiPartSelect:
            if (vpi_get(vpiType, vpi_handle(vpiParent, arg)) == vpiNet)
                  break;
	  case vpiMemoryWord:
	  case vpiBitVar:
	  case vpiReg:
	  case vpiIntegerVar:
	  case vpiIntVar:
	  case vpiLongIntVar:
	  case vpiTimeVar:
	    return;
	  default:
            break;
      }
      vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                 (int)vpi_get(vpiLineNo, callh));
      vpi_printf("%s's %s argument must be a variable.\n",
                 name, arg_name);
      vpip_set_return_value(1);
      vpi_control(vpiFinish, 1);
}

static PLI_INT32 sys_countdrivers_sizetf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    (void)name;  /* Parameter is not used. */
    return 1;
}

/*
 * Check that the given $countdrivers() call has valid arguments.
 */
static PLI_INT32 sys_countdrivers_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      unsigned arg_num;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least one argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument must be a scalar net or a net bit select. */
      arg = vpi_scan(argv);
      check_net_arg(arg, callh, name);

        /* The optional arguments must be variables. */
      for (arg_num = 2; arg_num < 7; arg_num += 1) {
	    const char *arg_name = NULL;
	    switch (arg_num) {
	        case 2: arg_name = "second"; break;
	        case 3: arg_name = "third";  break;
	        case 4: arg_name = "fourth"; break;
	        case 5: arg_name = "fifth";  break;
	        case 6: arg_name = "sixth";  break;
	        default: assert(0);
	    }

	    arg = vpi_scan(argv);
            if (arg == 0)
                  return 0;

            check_var_arg(arg, callh, name, arg_name);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "six arguments", 0);
      return 0;
}

/*
 * The runtime code for $countdrivers().
 */
static PLI_INT32 sys_countdrivers_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      unsigned idx;
      unsigned counts[4];
      unsigned num_drivers;
      s_vpi_value val;

      (void)name;  /* Parameter is not used. */

        /* All returned values are integers. */
      val.format = vpiIntVal;

        /* Get the base net reference and bit select */
      idx = 0;
      arg = vpi_scan(argv);
      assert(arg);
      if (vpi_get(vpiType, arg) == vpiPartSelect) {
            idx = vpi_get(vpiLeftRange, arg);
            arg = vpi_handle(vpiParent, arg);
            assert(arg);
      }

        /* Get the net driver counts from the runtime. */
      vpip_count_drivers(arg, idx, counts);
      num_drivers = counts[0] + counts[1] + counts[2];

        /* Handle optional net_is_forced argument. */
      arg = vpi_scan(argv);
      if (arg == 0) goto args_done;
      val.value.integer = counts[3];
      vpi_put_value(arg, &val, 0, vpiNoDelay);

        /* Handle optional number_of_01x_drivers argument. */
      arg = vpi_scan(argv);
      if (arg == 0) goto args_done;
      val.value.integer = num_drivers;
      vpi_put_value(arg, &val, 0, vpiNoDelay);

        /* Handle optional number_of_0_drivers argument. */
      arg = vpi_scan(argv);
      if (arg == 0) goto args_done;
      val.value.integer = counts[0];
      vpi_put_value(arg, &val, 0, vpiNoDelay);

        /* Handle optional number_of_1_drivers argument. */
      arg = vpi_scan(argv);
      if (arg == 0) goto args_done;
      val.value.integer = counts[1];
      vpi_put_value(arg, &val, 0, vpiNoDelay);

        /* Handle optional number_of_x_drivers argument. */
      arg = vpi_scan(argv);
      if (arg == 0) goto args_done;
      val.value.integer = counts[2];
      vpi_put_value(arg, &val, 0, vpiNoDelay);

        /* Free the argument iterator. */
      vpi_free_object(argv);

args_done:
      val.value.integer = (num_drivers > 1) ? 1 : 0;
      vpi_put_value(callh, &val, 0, vpiNoDelay);
      return 0;
}

/*
 * Routine to register the system tasks/functions provided in this file.
 */
void sys_countdrivers_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSizedFunc;
      tf_data.tfname      = "$countdrivers";
      tf_data.calltf      = sys_countdrivers_calltf;
      tf_data.compiletf   = sys_countdrivers_compiletf;
      tf_data.sizetf      = sys_countdrivers_sizetf;
      tf_data.user_data   = "$countdrivers";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
