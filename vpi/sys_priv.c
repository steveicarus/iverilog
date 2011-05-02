/*
 * Copyright (c) 2003-2011 Stephen Williams (steve@icarus.com)
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

#include "sys_priv.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

PLI_UINT64 timerec_to_time64(const struct t_vpi_time*timerec)
{
      PLI_UINT64 tmp;

      tmp = timerec->high;
      tmp <<= 32;
      tmp |= (PLI_UINT64) timerec->low;

      return tmp;
}

char *as_escaped(char *arg)
{
      unsigned idx, cur, cnt, len = strlen(arg);
      char *res = (char *) malloc(sizeof(char *) * len);
      cur = 0;
      cnt = len;
      for (idx = 0; idx < cnt; idx++) {
	    if (isprint((int)arg[idx])) {
		  res[cur] = arg[idx];
		  cur += 1;
	    } else {
		  len += 3;
		  res = (char *) realloc(res, sizeof(char *) * len);
		  sprintf(&(res[cur]), "\\%03o", arg[idx]);
		  cur += 4;
	    }
      }
      res[cur] = 0;
      return res;
}

/*
 * Generic routine to get the filename from the given handle.
 * The result is duplicated so call free when the name is no
 * longer needed. Returns 0 (NULL) for an error.
 */
char *get_filename(vpiHandle callh, char *name, vpiHandle file)
{
      s_vpi_value val;
      unsigned len, idx;

	/* Get the filename. */
      val.format = vpiStringVal;
      vpi_get_value(file, &val);

	/* Verify that we have a string and that it is not NULL. */
      if (val.format != vpiStringVal || !*(val.value.str)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's file name argument (%s) is not a valid string.\n",
	                name, vpi_get_str(vpiType, file));
	    return 0;
      }

	/*
	 * Verify that the file name is composed of only printable
	 * characters.
	 */
      len = strlen(val.value.str);
      for (idx = 0; idx < len; idx++) {
	    if (! isprint((int)val.value.str[idx])) {
		  char msg[64];
		  char *esc_fname = as_escaped(val.value.str);
		  snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
		           vpi_get_str(vpiFile, callh),
		           (int)vpi_get(vpiLineNo, callh));
		  msg[sizeof(msg)-1] = 0;
		  vpi_printf("%s %s's file name argument contains non-"
		             "printable characters.\n", msg, name);
		  vpi_printf("%*s \"%s\"\n", (int) strlen(msg), " ", esc_fname);
		  free(esc_fname);
		  return 0;
	    }
      }

      return strdup(val.value.str);
}

void check_for_extra_args(vpiHandle argv, vpiHandle callh, char *name,
                          const char *arg_str, unsigned opt)
{
	/* Check that there are no extra arguments. */
      if (vpi_scan(argv) != 0) {
            char msg[64];
            unsigned argc;

            snprintf(msg, sizeof(msg), "ERROR: %s:%d:",
                     vpi_get_str(vpiFile, callh),
                     (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;

            argc = 1;
            while (vpi_scan(argv)) argc += 1;

            vpi_printf("%s %s takes %s%s.\n", msg, name,
	               opt ? "at most ": "", arg_str);
            vpi_printf("%*s Found %u extra argument%s.\n",
                       (int) strlen(msg), " ", argc, argc == 1 ? "" : "s");
            vpi_control(vpiFinish, 1);
      }
}

/*
 * This routine returns 1 if the argument is a constant value,
 * otherwise it returns 0.
 *
 * This routine was also copied to sys_clog2.c since it is not
 * part of the standard system functions.
 */
unsigned is_constant_obj(vpiHandle obj)
{
      unsigned rtn = 0;

      assert(obj);

      switch(vpi_get(vpiType, obj)) {
	case vpiConstant:
	case vpiParameter:
	    rtn = 1;
	    break;
      }

      return rtn;
}

/*
 * This routine returns 1 if the argument supports has a numeric value,
 * otherwise it returns 0.
 */
unsigned is_numeric_obj(vpiHandle obj)
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
 * This routine returns 1 if the argument supports a valid string value,
 * otherwise it returns 0.
 */
unsigned is_string_obj(vpiHandle obj)
{
    unsigned rtn = 0;

    assert(obj);

    switch(vpi_get(vpiType, obj)) {
      case vpiConstant:
      case vpiParameter: {
	  /* These must be a string or binary constant. */
	PLI_INT32 ctype = vpi_get(vpiConstType, obj);
	if (ctype == vpiStringConst || ctype == vpiBinaryConst) rtn = 1;
	break;
      }

	/* These can have a valid string value. */
      case vpiIntegerVar:
      case vpiMemoryWord:
      case vpiNet:
      case vpiPartSelect:
      case vpiReg:
      case vpiTimeVar:
	rtn = 1;
	break;
    }

    return rtn;
}


/*
 * Find the enclosing module.
 */
vpiHandle sys_func_module(vpiHandle obj)
{
      assert(obj);

      while (vpi_get(vpiType, obj) != vpiModule) {
	    obj = vpi_handle(vpiScope, obj);
	    assert(obj);
      }

      return obj;
}

/*
 * Standard compiletf routines.
 */

/* For system tasks/functions that do not take an argument. */
PLI_INT32 sys_no_arg_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* Make sure there are no arguments. */
      if (argv != 0) {
	    char msg[64];
	    unsigned argc;

	    snprintf(msg, sizeof(msg), "ERROR: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;

	    argc = 0;
	    while (vpi_scan(argv)) argc += 1;

            vpi_printf("%s %s does not take an argument.\n", msg, name);
            vpi_printf("%*s Found %u extra argument%s.\n",
	               (int) strlen(msg), " ", argc, argc == 1 ? "" : "s");
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

/* For system tasks/functions that take a single numeric argument. */
PLI_INT32 sys_one_numeric_arg_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* Check that there is an argument and that it is numeric. */
      if (argv == 0) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s requires a single numeric argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      if (! is_numeric_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's argument must be numeric.\n", name);
            vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "a single numeric argument", 0);

      return 0;
}

/* For system tasks/functions that take a single optional numeric argument. */
PLI_INT32 sys_one_opt_numeric_arg_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* The argument is optional so just return if none are found. */
      if (argv == 0) return 0;

      if (! is_numeric_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's argument must be numeric.\n", name);
            vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "one numeric argument", 1);

      return 0;
}

/* For system tasks/functions that take two numeric arguments. */
PLI_INT32 sys_two_numeric_args_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

      /* Check that there are two argument and that they are numeric. */
      if (argv == 0) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s requires two numeric arguments.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      if (! is_numeric_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's first argument must be numeric.\n", name);
            vpi_control(vpiFinish, 1);
      }

      arg = vpi_scan(argv);
      if (! arg) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s requires a second (numeric) argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      if (! is_numeric_obj(arg)) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's second argument must be numeric.\n", name);
            vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "two numeric arguments", 0);

      return 0;
}

/* For system tasks/functions that take a single string argument. */
PLI_INT32 sys_one_string_arg_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* Check that there is an argument and that it is a string. */
      if (argv == 0) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s requires a single string argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }
      if (! is_string_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's argument must be a string.\n", name);
            vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "a single string argument", 0);

      return 0;
}
