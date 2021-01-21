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

#include "sys_priv.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "version_base.h"

/* Once we have real string objects replace this with a dynamic string. */
#define MAX_STRING_RESULT 1024


/*
 * Check that the routines are called with the correct arguments.
 */
static PLI_INT32 simparam_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name_ext)
{
      vpiHandle callh, argv, arg;

      callh = vpi_handle(vpiSysTfCall, 0);
      assert(callh != 0);
      argv = vpi_iterate(vpiArgument, callh);

	/* We must have at least one argument. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("$simparam%s requires a string argument.\n", name_ext);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument must be a string. */
      arg = vpi_scan(argv);
      if (! is_string_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("The first argument to $simparam%s must be a string.\n",
	                name_ext);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* The second argument (default value) is optional. */
      arg = vpi_scan(argv);
      if (arg == 0) return 0;

	/* For the string version the default must also be a string. */
      if (strcmp(name_ext, "$str") == 0) {
	    if (! is_string_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("When provided, the second argument to $simparam%s"
		             "must be a string.\n", name_ext);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
	/* For the rest the default must be numeric. */
      } else {
	    if (! is_numeric_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("When provided, the second argument to $simparam%s"
		             "must be numeric.\n", name_ext);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
      }

	/* We can have a maximum of two arguments. */
      if (vpi_scan(argv) != 0) {
	    char msg[64];
	    unsigned argc;

	    snprintf(msg, sizeof(msg), "ERROR: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;

	    argc = 1;
	    while (vpi_scan(argv)) argc += 1;

            vpi_printf("%s $simparam%s takes at most two arguments.\n",
	               msg, name_ext);
            vpi_printf("%*s Found %u extra argument%s.\n",
	               (int) strlen(msg), " ", argc, argc == 1 ? "" : "s");
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

static PLI_INT32 simparam_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name_ext)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      char *param;
      unsigned have_def_val = 0;
      double retval, defval = 0.0;

	/* Get the parameter we are looking for. */
      arg = vpi_scan(argv);
      val.format = vpiStringVal;
      vpi_get_value(arg, &val);
      param = strdup(val.value.str);

	/* See if there is a default value. */
      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_free_object(argv);
	    have_def_val = 1;
	    val.format = vpiRealVal;
	    vpi_get_value(arg, &val);
	    defval = val.value.real;
      }

	/* Now check the various things we can return. */
      if (strcmp(param, "gdev") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "gmin") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "imax") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "imelt") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "iteration") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "scale") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "shrink") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "simulatorSubversion") == 0) {
	    retval = VERSION_MINOR;
      } else if (strcmp(param, "simulatorVersion") == 0) {
	    retval = VERSION_MAJOR;
      } else if (strcmp(param, "sourceScaleFactor") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "tnom") == 0) {
	    retval = 0.0; /* Nothing for now. */
      } else if (strcmp(param, "timeUnit") == 0) {
	    retval = pow(10, vpi_get(vpiTimeUnit, sys_func_module(callh)));
      } else if (strcmp(param, "timePrecision") == 0) {
	    retval = pow(10, vpi_get(vpiTimePrecision, sys_func_module(callh)));
      } else if (strcmp(param, "CPUWordSize") == 0) {
	    retval = 8.0*sizeof(long);
      } else {
	    if (! have_def_val) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("$simparam%s unknown parameter name \"%s\".\n",
		              name_ext, param);
	    }
	    retval = defval;
      }

      free(param);

	/* Return the value to the system. */
      val.format = vpiRealVal;
      val.value.real = retval;
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 simparam_str_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name_ext)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      char *param;
      char *retval, *defval = NULL;

	/* Get the parameter we are looking for. */
      arg = vpi_scan(argv);
      val.format = vpiStringVal;
      vpi_get_value(arg, &val);
      param = strdup(val.value.str);

	/* See if there is a default value. */
      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_free_object(argv);
	    val.format = vpiStringVal;
	    vpi_get_value(arg, &val);
	    defval = strdup(val.value.str);
      }

	/* Now check the various things we can return. */
	/* For now we limit the result to 1024 characters. */
      if (strcmp(param, "analysis_name") == 0) {
	    retval = strdup("N/A"); /* Nothing for now. */
      } else if (strcmp(param, "analysis_type") == 0) {
	    retval = strdup("N/A"); /* Nothing for now. */
      } else if (strcmp(param, "cwd") == 0) {
	    char path [MAX_STRING_RESULT];
	    char *ptr = getcwd(path, MAX_STRING_RESULT);
	    if (ptr == NULL) {
		  strcpy(path, "<error getting the cwd, is it too long?>");
	    }
	    retval = strdup(path);
      } else if (strcmp(param, "module") == 0) {
	    retval = strdup(vpi_get_str(vpiDefName, sys_func_module(callh)));
      } else if (strcmp(param, "instance") == 0) {
	    retval = strdup(vpi_get_str(vpiFullName, sys_func_module(callh)));
      } else if (strcmp(param, "path") == 0) {
	    retval = strdup(vpi_get_str(vpiFullName,
	                    vpi_handle(vpiScope,callh)));
      } else {
	    if (defval == NULL) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("$simparam%s unknown parameter name \"%s\".\n",
		             name_ext, param);
		  defval = strdup("<error>");
	    }
	    retval = defval;
      }

      free(param);

	/* Return the value to the system. */
      val.format = vpiStringVal;
      val.value.str = retval;
      vpi_put_value(callh, &val, 0, vpiNoDelay);
      if (defval != retval) free(defval);
      free(retval);

      return 0;
}

static PLI_INT32 simparam_str_sizetf(ICARUS_VPI_CONST PLI_BYTE8 *name_ext)
{
      (void) name_ext;  /* Parameter is not used. */
      return MAX_STRING_RESULT;  /* 128 characters max! */
}

/*
 * Register the function with Verilog.
 */
void vams_simparam_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiRealFunc;
      tf_data.calltf      = simparam_calltf;
      tf_data.compiletf   = simparam_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$simparam";
      tf_data.user_data   = "";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSizedFunc;  /* What should this be? */
      tf_data.calltf      = simparam_str_calltf;
      tf_data.compiletf   = simparam_compiletf;
      tf_data.sizetf      = simparam_str_sizetf;  /* Only 128 characters! */
      tf_data.tfname      = "$simparam$str";
      tf_data.user_data   = "$str";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
