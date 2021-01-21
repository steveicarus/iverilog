/*
 * Copyright (c) 2003-2021 Stephen Williams (steve@icarus.com)
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

#include "sys_priv.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "ivl_alloc.h"

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
      unsigned idx, cur, cnt, len = strlen(arg) + 1;
      char *res = (char *) malloc(sizeof(char) * len);
      cur = 0;
      cnt = len - 1;
      for (idx = 0; idx < cnt; idx++) {
	    if (isprint((int)arg[idx])) {
		  res[cur] = arg[idx];
		  cur += 1;
	    } else {
		  len += 3;
		  res = (char *) realloc(res, sizeof(char) * len);
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
char *get_filename(vpiHandle callh, const char *name, vpiHandle file)
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

char* get_filename_with_suffix(vpiHandle callh, const char *name, vpiHandle file, const char*suff)
{
      char*path = get_filename(callh, name, file);
      if (path == 0) return 0;

	/* If the name already has a suffix, then don't replace it or
	   add another suffix. Just return this path. */
      char*tailp = strrchr(path, '.');
      if (tailp != 0) return path;

	/* The name doesn't have a suffix, so append the passed in
	   suffix to the file name. */
      char*new_path = malloc(strlen(path) + strlen(suff) + 2);
      strcpy(new_path, path);
      strcat(new_path, ".");
      strcat(new_path, suff);
      free(path);
      return new_path;
}

void check_for_extra_args(vpiHandle argv, vpiHandle callh, const char *name,
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
	    vpip_set_return_value(1);
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
      case vpiBitVar:
      case vpiByteVar:
      case vpiShortIntVar:
      case vpiIntVar:
      case vpiLongIntVar:
      case vpiMemoryWord:
      case vpiNet:
      case vpiPartSelect:
      case vpiReg:
      case vpiTimeVar:
      case vpiStringVar:
	rtn = 1;
	break;
    }

    return rtn;
}


/*
 * Check if the file descriptor or MCD is valid.
 * For a MCD check that every bit set is a valid file and
 * for a FD make sure it exists.
 */
unsigned is_valid_fd_mcd(PLI_UINT32 fd_mcd)
{
      assert(fd_mcd); // Should already be handled!

      if (IS_MCD(fd_mcd)){
	    if (vpi_mcd_printf(fd_mcd, "%s", "") == EOF) return 0;
      } else {
	    if (vpi_get_file(fd_mcd) == NULL) return 0;
      }

      return 1;
}

/*
 * Get a FD/MCD from the given argument and check if it is valid. Return the
 * FD/MCD in the fd_mcd argument and return 0 if it is valid and 1 if it is
 * invalid. We do not print a warning mesage if the FD/MCD is zero.
 */
unsigned get_fd_mcd_from_arg(PLI_UINT32 *fd_mcd, vpiHandle arg,
                             vpiHandle callh, const char *name)
{
      s_vpi_value val;

      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      *fd_mcd = val.value.integer;

      if (*fd_mcd == 0) return 1;

      errno = 0;
      if (! is_valid_fd_mcd(*fd_mcd)) {
            vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("invalid %s (0x%x) given to %s().\n",
                       IS_MCD(*fd_mcd) ? "MCD" : "file descriptor",
                       (unsigned int)*fd_mcd,
                       name);
            errno = EBADF;
            return 1;
      }

      return 0;
}


/*
 * Find the enclosing module. If there is no enclosing module (which can be
 * the case in SystemVerilog), return the highest enclosing scope.
 */
vpiHandle sys_func_module(vpiHandle obj)
{
      assert(obj);

      while (vpi_get(vpiType, obj) != vpiModule) {
	    vpiHandle scope = vpi_handle(vpiScope, obj);
	    if (scope == 0)
		  break;
	    obj = scope;
      }

      return obj;
}

/*
 * Standard compiletf routines.
 */

/* For system tasks/functions that do not take an argument. */
PLI_INT32 sys_no_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
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
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

/* For system tasks/functions that take a single numeric argument. */
PLI_INT32 sys_one_numeric_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* Check that there is an argument and that it is numeric. */
      if (argv == 0) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s requires a single numeric argument.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      if (! is_numeric_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's argument must be numeric.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "a single numeric argument", 0);

      return 0;
}

/* For system tasks/functions that take a single optional numeric argument. */
PLI_INT32 sys_one_opt_numeric_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* The argument is optional so just return if none are found. */
      if (argv == 0) return 0;

      if (! is_numeric_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's argument must be numeric.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "one numeric argument", 1);

      return 0;
}

/* For system tasks/functions that take two numeric arguments. */
PLI_INT32 sys_two_numeric_args_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

      /* Check that there are two argument and that they are numeric. */
      if (argv == 0) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s requires two numeric arguments.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      if (! is_numeric_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's first argument must be numeric.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      arg = vpi_scan(argv);
      if (! arg) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s requires a second (numeric) argument.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      if (! is_numeric_obj(arg)) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's second argument must be numeric.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "two numeric arguments", 0);

      return 0;
}

/* For system tasks/functions that take a single string argument. */
PLI_INT32 sys_one_string_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* Check that there is an argument and that it is a string. */
      if (argv == 0) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s requires a single string argument.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
            return 0;
      }
      if (! is_string_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's argument must be a string.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "a single string argument", 0);

      return 0;
}

/* Return an integer value to the caller. */
void put_integer_value(vpiHandle callh, PLI_INT32 result)
{
      s_vpi_value val;

      val.format = vpiIntVal;
      val.value.integer = result;
      vpi_put_value(callh, &val, 0, vpiNoDelay);
}

/* Return a scalar value to the caller. */
void put_scalar_value(vpiHandle callh, PLI_INT32 result)
{
      s_vpi_value val;

      val.format = vpiScalarVal;
      val.value.scalar = result;
      vpi_put_value(callh, &val, 0, vpiNoDelay);
}
