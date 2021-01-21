/*
 *  Copyright (C) 2011-2021  Cary R. (cygcary@yahoo.com)
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
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table_mod.h"
#include "ivl_alloc.h"

/*
 * Flag used to enable/disable debug output.
 */
static unsigned table_model_debug = 0;

static p_table_mod *tables = 0;
static unsigned table_count = 0;

/*
 * Routine to cleanup the table model data at the end of simulation.
 */
static PLI_INT32 cleanup_table_mod(p_cb_data cause)
{
      unsigned idx;

      (void)cause; /* Parameter is not used. */

      for (idx = 0; idx < table_count; idx += 1) {
	    free(tables[idx]->indep);
	    free(tables[idx]->indep_val);
	    if (tables[idx]->have_fname) free(tables[idx]->file.name);
	    if (tables[idx]->have_ctl) {
		  free(tables[idx]->control.info.interp);
		  free(tables[idx]->control.info.extrap_low);
		  free(tables[idx]->control.info.extrap_high);
	    }
            free(tables[idx]);
      }
      free(tables);
      tables = 0;
      table_count = 0;

      return 0;
}

/*
 * Create an empty table model object and add it to the list of table
 * model objects.
 */
static p_table_mod create_table(void)
{
	/* Create an empty table model object. */
      p_table_mod obj = (p_table_mod) malloc(sizeof(s_table_mod));
      assert(obj);

	/* Add it to the list of tables. */
      table_count += 1;
      tables = (p_table_mod *) realloc(tables, sizeof(p_table_mod)*table_count);
      tables[table_count-1] = obj;

	/* Initialize and return the table object. */
      obj->indep = 0;
      obj->indep_val = 0;
      obj->have_fname = 0;
      obj->have_ctl = 0;
      obj->control.arg = 0;
      obj->depend = 0;
      obj->dims = 0;
      obj->fields = 0;
      return obj;
}

/*
 * Check to see if this is a constant string. It returns 1 if the argument
 * is a constant string otherwise it returns 0.
 */
static unsigned is_const_string_obj(vpiHandle arg)
{
      unsigned rtn = 0;

      assert(arg);

      switch (vpi_get(vpiType, arg)) {
	case vpiConstant:
	case vpiParameter:
	      /* This must be a constant string. */
	    if (vpi_get(vpiConstType, arg) == vpiStringConst) rtn = 1;
      }

      return rtn;
}

/*
 * Get any command line flags. For now we only have a debug flag.
 */
static void check_command_line_flags(void)
{
      struct t_vpi_vlog_info vlog_info;
      static unsigned command_line_processed = 0;

	/* If we have already processed the arguments then just return. */
      if (command_line_processed) return;

      vpi_get_vlog_info(&vlog_info);

      for (int idx = 0; idx < vlog_info.argc; idx += 1) {
	    if (strcmp(vlog_info.argv[idx], "-table-model-debug") == 0) {
		  table_model_debug = 1;
	    }
      }

      command_line_processed = 1;
}

/*
 * Check that the given $table_model() call has valid arguments.
 */
static PLI_INT32 sys_table_model_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      p_table_mod table = create_table();

	/* See if there are any table model arguments. */
      check_command_line_flags();

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least two arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first N (dimensions) arguments must be numeric. */
      for (arg = vpi_scan(argv);
           arg && is_numeric_obj(arg);
           arg = vpi_scan(argv)) {
	    table->dims += 1;
	    table->indep = (vpiHandle *)realloc(table->indep,
	                                        sizeof(vpiHandle)*table->dims);
	    table->indep[table->dims-1] = arg;
      }

	/* We must have a data file. */
      if (arg == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a file name argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* For now we only allow a constant string (file name). */
      if (! is_const_string_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s file name argument must be a constant string.\n",
	               name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      table->file.arg = arg;

	/* There may be an optional constant string (control string). */
      arg = vpi_scan(argv);
      if (arg) {
	    if (! is_const_string_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		            (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s control string argument must be a constant "
		             "string.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 0;
	    }
	    check_for_extra_args(argv, callh, name, "N numeric and 1 or 2 "
	                         " constant string arguments", 0);
	    table->control.arg = arg;
      }

	/* Save the table data information. */
      vpi_put_userdata(callh, table);

      return 0;
}

/*
 * Initialize the control string fields with the default value.
 */
static void initialize_control_fields_def(p_table_mod table)
{
      char *val;
      unsigned width = table->dims;

	/* Set the default interpolation for each dimension. */
      val = (char *) malloc(width);
      assert(val);
      table->control.info.interp = memset(val, IVL_LINEAR_INTERP, width);

	/* Set the default low extrapolation for each dimension. */
      val = (char *) malloc(width);
      assert(val);
      table->control.info.extrap_low = memset(val, IVL_LINEAR_EXTRAP, width);

	/* Set the default low extrapolation for each dimension. */
      val = (char *) malloc(width);
      assert(val);
      table->control.info.extrap_high = memset(val, IVL_LINEAR_EXTRAP, width);

	/* By default the dependent data column is the first one after the
	 * independent data column(s). */
      table->depend = 1;

	/* Set the number of control fields. */
      table->fields = width;

	/* We have defined the interpolation/extrapolation fields. */
      table->have_ctl = 1;
}

/*
 * Parse the extrapolation control codes.
 */
static unsigned parse_extrap(vpiHandle callh, p_table_mod table, unsigned idx,
                             char **extrap, char *control)
{
	/* Advance to the first extrapolation code. */
      *extrap += 1;

	/* By default both the low and high extrapolation is set by the
	 * first extrapolation character. We'll override the high value
	 * later if there is a second character. */
      switch (**extrap) {
	  /* Clamp (no) extrapolation. */
	case 'C':
	    table->control.info.extrap_low[idx] = IVL_CONSTANT_EXTRAP;
	    table->control.info.extrap_high[idx] = IVL_CONSTANT_EXTRAP;
	    break;

	  /* Linear extrapolation. */
	case 'L':
	    table->control.info.extrap_low[idx] = IVL_LINEAR_EXTRAP;
	    table->control.info.extrap_high[idx] = IVL_LINEAR_EXTRAP;
	    break;

	  /* Error on extrapolation. */
	case 'E':
	    table->control.info.extrap_low[idx] = IVL_ERROR_EXTRAP;
	    table->control.info.extrap_high[idx] = IVL_ERROR_EXTRAP;
	    break;

	  /* There are no extrapolation characters so use the default
	   * (linear extrapolation). */
	case 0:
	case ',':
	    table->control.info.extrap_low[idx] = IVL_LINEAR_EXTRAP;
	    table->control.info.extrap_high[idx] = IVL_LINEAR_EXTRAP;
	    return 0;

	  /* Anything else is an error. */
	default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Unknown extrapolation code '%c' for dimension "
	               "%u: %s\n", **extrap, idx+1, control);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 1;
      }

	/* Advance to the next (high) extrapolation code. */
      *extrap += 1;

	/* Override the high extrapolation value if needed. */
      switch (**extrap) {
	  /* Clamp (no) extrapolation. */
	case 'C':
	    table->control.info.extrap_high[idx] = IVL_CONSTANT_EXTRAP;
	    break;

	  /* Linear extrapolation. */
	case 'L':
	    table->control.info.extrap_high[idx] = IVL_LINEAR_EXTRAP;
	    break;

	  /* Error on extrapolation. */
	case 'E':
	    table->control.info.extrap_high[idx] = IVL_ERROR_EXTRAP;
	    break;

	  /* There is no high extrapolation character so just return. */
	case 0:
	case ',':
	    return 0;

	  /* Anything else is an error. */
	default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Unknown high extrapolation code '%c' for dimension "
	               "%u: %s\n", **extrap, idx+1, control);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 1;
      }

	/* Advance to the next field. */
      *extrap += 1;

      return 0;
}

/*
 * Load the control string fields for the given string. Return 0 if there
 * is a problem parsing the control string.
 */
static unsigned initialize_control_fields(vpiHandle callh, p_table_mod table,
                                          char *control)
{
      unsigned num_fields, num_ignore, idx;
      char *cp;

	/* If we need to fail these must be defined to zero until memory
	 * has actually been allocated. */
      table->control.info.interp = 0;
      table->control.info.extrap_low = 0;
      table->control.info.extrap_high = 0;

	/* We have defined the interpolation/extrapolation fields. */
      table->have_ctl = 1;

	/* Look for the dependent data field. */
      cp = strchr(control, ';');
      if (cp) {
	    size_t len;
	    unsigned long val;
	    char *end;
	      /* NULL terminate the independent part of the string. */
	    *cp = 0;
	    cp += 1;
	    len = strlen(cp);
	      /* If the length is zero or not all digits then the string
	       * is invalid. */
// HERE: Do we need to support/ignore an underscore? What about in the file?
	    if ((! len) || (len != strspn(cp, "0123456789"))) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("Control string dependent selector is "
		             "invalid: %s.\n", cp);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 1;
	    }
	    val = strtoul(cp, &end, 10);
	    assert(*end == 0);
	      /* If the value is too big also report an error. */
	    if (val > UINT_MAX) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("Control string dependent value is "
		             "to large: %lu.\n", val);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 1;
	    }
	    table->depend = (unsigned) val;
      } else table->depend = 1;

	/* Find the number of interpolation/extrapolation control fields. */
      num_fields = 1;
      cp = control;
      while ((cp = strchr(cp, ','))) {
	    cp += 1;
	    num_fields += 1;
      }

	/* We must have at least as many control fields as dimensions. */
      if (num_fields < table->dims) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Not enough control field(s) (%u) for the dimension(s) "
	               "(%u).", num_fields, table->dims);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 1;
      }

	/* Allocate space for the interpolation/extrapolation values. */
      table->control.info.interp = (char *) malloc(num_fields);
      assert(table->control.info.interp);
      table->control.info.extrap_low = (char *) malloc(num_fields);
      assert(table->control.info.extrap_low);
      table->control.info.extrap_high = (char *) malloc(num_fields);
      assert(table->control.info.extrap_high);

	/* Parse the individual dimension control string. */
      num_ignore = 0;
      cp = control;
      for (idx = 0; idx < num_fields; idx += 1) {
	    switch (*cp) {
		/* Closest point interpolation. */
	      case 'D':
		  table->control.info.interp[idx] = IVL_CLOSEST_POINT;
		  if (parse_extrap(callh, table, idx, &cp, control)) return 0;
		  break;

		/* Linear interpolation. */
	      case '1':
		  table->control.info.interp[idx] = IVL_LINEAR_INTERP;
		  if (parse_extrap(callh, table, idx, &cp, control)) return 0;
		  break;

		/* Quadratic interpolation. */
	      case '2':
		  table->control.info.interp[idx] = IVL_QUADRATIC_INTERP;
		  if (parse_extrap(callh, table, idx, &cp, control)) return 0;
		  break;

		/* Cubic interpolation. */
	      case '3':
		  table->control.info.interp[idx] = IVL_CUBIC_INTERP;
		  if (parse_extrap(callh, table, idx, &cp, control)) return 0;
		  break;

		/* Ignore column. No extrapolation codes are allowed. */
	      case 'I':
		  table->control.info.interp[idx] = IVL_IGNORE_COLUMN;
		  table->control.info.extrap_low[idx] = IVL_ERROR_EXTRAP;
		  table->control.info.extrap_high[idx] = IVL_ERROR_EXTRAP;
		  cp += 1;
		  num_ignore += 1;
		  break;

		/* This field is not specified so use the default values.
		 * Linear interpolation and extrapolation. */
	      case 0:
		  assert(idx == num_fields-1);
	      case ',':
		  table->control.info.interp[idx] = IVL_LINEAR_INTERP;
		  table->control.info.extrap_low[idx] = IVL_LINEAR_EXTRAP;
		  table->control.info.extrap_high[idx] = IVL_LINEAR_EXTRAP;
		  break;

		/* Anything else is an error. */
	      default:
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("Unknown interpolation code '%c' for dimension "
		             "%u: %s\n", *cp, idx+1, control);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 1;
	    }

	      /* Verify that there is no extra junk at the end of the field. */
	    if (*cp && (*cp != ',')) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("Extra control characters found for dimension "
		             "%u: %s\n", idx+1, control);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 1;
	    }

	      /* Advance to the next field if we are not at the end of the
	       * control string. */
	    if (*cp == ',') cp += 1;
      }

	/* We must have a usable control field for each dimensions. */
      if ((num_fields - num_ignore) != table->dims) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Usable control field(s) (%u) do not match dimension(s) "
	               "(%u).", (num_fields - num_ignore), table->dims);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 1;
      }

	/* Set the number of control fields. */
      table->fields = num_fields;

      return 0;
}

/*
 * Print out the interpolation code.
 */
static void dump_interp(char interp)
{
      switch (interp) {
	case IVL_CLOSEST_POINT:    fprintf(stderr, "D"); break;
	case IVL_LINEAR_INTERP:    fprintf(stderr, "1"); break;
	case IVL_QUADRATIC_INTERP: fprintf(stderr, "2"); break;
	case IVL_CUBIC_INTERP:     fprintf(stderr, "3"); break;
	case IVL_IGNORE_COLUMN:    fprintf(stderr, "I"); break;
	default: fprintf(stderr, "<%d>", interp); break;
      }
}

/*
 * Print out the extrapolation code.
 */
static void dump_extrap(char extrap)
{
      switch (extrap) {
	case IVL_CONSTANT_EXTRAP: fprintf(stderr, "C"); break;
	case IVL_LINEAR_EXTRAP:   fprintf(stderr, "L"); break;
	case IVL_ERROR_EXTRAP:    fprintf(stderr, "E"); break;
	default: fprintf(stderr, "<%d>", extrap); break;
      }
}

/*
 * Print some diagnostic information about the $table_model() call.
 */
static void dump_diag_info(vpiHandle callh, p_table_mod table)
{
      unsigned idx;
      char msg[64];

      snprintf(msg, sizeof(msg), "DEBUG: %s:%d:", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
      msg[sizeof(msg)-1] = 0;
      fprintf(stderr, "%s Building %u variable table using \"%s\" and\n",
              msg, table->dims, table->file.name);
      fprintf(stderr, "%*s the following control string: ",
              (int) strlen(msg), " ");
      dump_interp(table->control.info.interp[0]);
      dump_extrap(table->control.info.extrap_low[0]);
      dump_extrap(table->control.info.extrap_high[0]);
      for (idx = 1; idx < table->fields; idx += 1) {
	    fprintf(stderr, ",");
	    dump_interp(table->control.info.interp[idx]);
	    dump_extrap(table->control.info.extrap_low[idx]);
	    dump_extrap(table->control.info.extrap_high[idx]);
      }
      fprintf(stderr, ";%u\n", table->depend);
      fprintf(stderr, "%*s The data file must have at least %u columns.\n",
              (int) strlen(msg), " ", table->fields+table->depend);
}

/*
 * Initialize the table model data structure.
 *
 * The first time $table_model() is called we need to evaluate the file name
 * and control strings. Then we must load the data from the file.
 */
static unsigned initialize_table_model(vpiHandle callh, const char *name,
                                       p_table_mod table)
{
      s_vpi_value val;
      FILE *fp;

	/* Get the table file name. */
      table->file.name = get_filename(callh, name, table->file.arg);
      table->have_fname = 1;
      if (table->file.name == 0) return 1;

	/* Open the file. */
      fp = fopen(table->file.name, "r");
      if (fp == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() cannot open file \"%s\" for reading (%s).\n",
	               name, table->file.name, strerror(errno));
	    return 1;
      }

	/* Check to see if there is a control string. */
      if (table->control.arg) {
	    val.format = vpiStringVal;
	    vpi_get_value(table->control.arg, &val);
	    if (*(val.value.str) == 0) {
		   /* If there is an empty control string field then we just
		    * use the default values for interpolation/extrapolation.
		    * We also assume that there only dimensions + 1 fields. */
		  initialize_control_fields_def(table);
	    } else {
		  if (initialize_control_fields(callh, table, val.value.str)) {
			return 1;
		  }
	    }
      } else {
	      /* If there is no control string field then we just use the
	       * default values for interpolation/extrapolation. We also
	       * assume that there only dimensions + 1 fields. */
	    initialize_control_fields_def(table);
      }

	/* If requested dump some diagnostic information. */
      if (table_model_debug) dump_diag_info(callh, table);

	/* Parse the given file and produce the basic data structure. We
	 * need to have columns for each control string field and for the
	 * dependent data. */
      if (parse_table_model(fp, callh, table)) return 1;
// HERE: once we are done with the control string should we build a table
//       of function pointer for the interp/extrap?

	/* Close the file now that we have loaded all the data. */
      if (fclose(fp)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() cannot close file \"%s\" (%s).\n",
	               name, table->file.name, strerror(errno));
	    return 1;
      }

	/* Allocate space for the current argument values. */
      table->indep_val = (double*) malloc(sizeof(double)*table->dims);
      assert(table->indep_val);

      return 0;
}

/*
 * Routine to evaluate the table model using the current input values.
 */
static double eval_table_model(vpiHandle callh, p_table_mod table)
{
      unsigned idx;

      (void)callh; /* Parameter is not used. */
fprintf(stderr, "Evaluating table \"%s\" with %u variables\n",
        table->file.name, table->dims);
      for (idx = 0; idx < table->dims; idx += 1) {
fprintf(stderr, "  Arg %u: %#g\n", idx, table->indep_val[idx]);
      }

      return 0.0;
}

/*
 * Runtime routine for the $table_model().
 */
static PLI_INT32 sys_table_model_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      s_vpi_value val;
      p_table_mod table;
      unsigned idx;
      double result;

	/* Retrieve the table data. */
      table = vpi_get_userdata(callh);

	/* If this is the first call then build the data structure. */
      if ((table->have_fname == 0) &&
          initialize_table_model(callh, name, table)) {
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Load the current argument values into the table structure. */
      for (idx = 0; idx < table->dims; idx += 1) {
	    val.format = vpiRealVal;
	    vpi_get_value(table->indep[idx], &val);
	    table->indep_val[idx] = val.value.real;
      }

	/* Interpolate/extrapolate the data structure to find the value. */
      result = eval_table_model(callh, table);

	/* Return the calculated value. */
      val.format = vpiRealVal;
      val.value.real = result;
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

/*
 * Routine to register the system function provided in this file.
 */
void table_model_register(void)
{
      s_vpi_systf_data tf_data;
      s_cb_data cb;
      vpiHandle res;

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncReal;
      tf_data.tfname = "$table_model";
      tf_data.calltf = sys_table_model_calltf;
      tf_data.compiletf = sys_table_model_compiletf;
      tf_data.sizetf = 0;  /* Not needed for a vpiSysFuncReal. */
      tf_data.user_data = "$table_model";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

	/* Create a callback to clear all the table model memory when the
	 * simulator finishes. */
      cb.time = NULL;
      cb.reason = cbEndOfSimulation;
      cb.cb_rtn = cleanup_table_mod;
      cb.user_data = 0x0;
      cb.obj = 0x0;

      vpi_register_cb(&cb);
}
