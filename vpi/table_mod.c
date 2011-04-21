/*
 *  Copyright (C) 2011  Cary R. (cygcary@yahoo.com)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table_mod.h"
#include "ivl_alloc.h"

/*
 * This structure is saved for each table model instance.
 */
typedef struct t_table_mod {
      vpiHandle *indep;     /* Independent variable arguments. */
      double *indep_val;    /* Current independent variable values. */
      union {               /* Data file or argument to get the data file. */
	    char *name;
	    vpiHandle arg;
      } file;
      unsigned have_fname;  /* Has the file name been allocated? */
      vpiHandle control;    /* Control string argument. */
// HERE need a pointer to the dependent data and the final table.
      unsigned dims;        /* The number of independent variables. */
} s_table_mod, *p_table_mod;

static p_table_mod *tables = 0;
static unsigned table_count = 0U;

/*
 * Routine to cleanup the table model data at the end of simulation.
 */
static PLI_INT32 cleanup_table_mod(p_cb_data cause)
{
      unsigned idx;
      (void) cause;  /* Unused argument. */

      for (idx = 0; idx < table_count; idx += 1) {
	    free(tables[idx]->indep);
	    free(tables[idx]->indep_val);
	    if (tables[idx]->have_fname) free(tables[idx]->file.name);
            free(tables[idx]);
      }
      free(tables);
      tables = 0;
      table_count = 0U;

      return 0;
}

/*
 * Create an empty table model object and add it to the list of table
 * model objects.
 */
static p_table_mod create_table()
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
      obj->control = 0;
      obj->dims = 0;
      return obj;
}

/*
 * Check to see if this is a constant string. It returns 1 if the argument
 * is a constant string otherwise it returns 0.
 */
unsigned is_const_string_obj(vpiHandle arg)
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
 * Check that the given $table_model() call has valid arguments.
 */
static PLI_INT32 sys_table_model_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      p_table_mod table = create_table();

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least two arguments.\n", name);
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
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* For now we only allow a constant string (file name). */
      if (! is_const_string_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s file name argument must be a constant string.\n",
	               name);
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
		  vpi_control(vpiFinish, 1);
		  return 0;
	    }
	    check_for_extra_args(argv, callh, name, "N numeric and 1 or 2 "
	                         " constant string arguments", 0);
	    table->control = arg;
      }

	/* Save the table data information. */
      vpi_put_userdata(callh, table);

      return 0;
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
      FILE *fp;

	/* Get the table file name. */
      table->file.name = get_filename(callh, name, table->file.arg);
      table->have_fname = 1;
      if (table->file.name == 0) return 1U;
      fprintf(stderr, "Building table \"%s\" with %u variables\n",
              table->file.name, table->dims);

	/* Open the file. */
      fp = fopen(table->file.name, "r");
      if (fp == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() cannot open file \"%s\" for reading (%s).\n",
	               name, table->file.name, strerror(errno));
	    return 1U;
      }
// HERE: Need to process the control string.

	/* Parse the given file and produce the basic data structure. */
// HERE: This is going to return a pointer to the data.
//       We need to update the minimum cols when the control string has
//       been processed.
      if (! parse_table_model(fp, table->file.name, callh, table->dims+1)) {
	    return 1U;
      }

	/* Close the file now that we have loaded all the data. */
      if (fclose(fp)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() cannot close file \"%s\" (%s).\n",
	               name, table->file.name, strerror(errno));
	    return 1U;
      }

	/* Allocate space for the current argument values. */
      table->indep_val = (double*) malloc(sizeof(double)*table->dims);
      assert(table->indep_val);

      return 0U;
}

static double eval_table_model(vpiHandle callh, p_table_mod table)
{
      unsigned idx;
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
void table_model_register()
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
