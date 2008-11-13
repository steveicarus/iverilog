/*
 * Copyright (c) 2003-2008 Stephen Williams (steve@icarus.com)
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

# include  "vpi_user.h"
# include  "sys_priv.h"
# include  <assert.h>
# include  <ctype.h>
# include  <string.h>
# include  <stdio.h>
# include  <stdlib.h>

#define IS_MCD(mcd)     !((mcd)>>31&1)


/*
 * Implement the $fopen system function.
 */
static PLI_INT32 sys_fopen_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;
      assert(callh != 0);
      argv = vpi_iterate(vpiArgument, callh);

	/* Check that there is a file name argument and that it is a string. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a string file name argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      if (! is_string_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's file name argument must be a string.\n", name);
	    vpi_control(vpiFinish, 1);
      }

	/* The type argument is optional. */
      arg = vpi_scan(argv);
      if (arg == 0) return 0;

	/* When provided, the type argument must be a string. */
      if (! is_string_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's type argument must be a string.\n", name);
	    vpi_control(vpiFinish, 1);
      }

	/* Make sure there are no extra arguments. */
      if (vpi_scan(argv) != 0) {
	    char msg [64];
	    unsigned argc;

	    snprintf(msg, 64, "ERROR: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));

	    argc = 1;
	    while (vpi_scan(argv)) argc += 1;

	    vpi_printf("%s %s takes at most two string arguments.\n",
	               msg, name);
	    vpi_printf("%*s Found %u extra argument%s.\n",
	               (int) strlen(msg), " ", argc, argc == 1 ? "" : "s");
	    vpi_control(vpiFinish, 1);
      }

      return 0;
}

static PLI_INT32 sys_fopen_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value val;
      int fail = 0;
      char *mode_string = 0;
      unsigned idx;
      vpiHandle item = vpi_scan(argv);
      vpiHandle mode = vpi_scan(argv);
      unsigned len;

	/* Get the mode handle if it exists. */
      if (mode) {
            val.format = vpiStringVal;
            vpi_get_value(mode, &val);
	      /* Verify that we have a string and that it is not NULL. */
            if (val.format != vpiStringVal || !*(val.value.str)) {
		  vpi_printf("WARNING: %s:%d: ",
		             vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's mode argument is not a valid string.\n",
		             name);
		  fail = 1;
	    }

	      /* Make sure the mode string is correct. */
	    if (strlen(val.value.str) > 3) {
		  vpi_printf("WARNING: %s:%d: ",
		             vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's mode argument (%s) is too long.\n",
		             name, val.value.str);
		  fail = 1;
	    } else {
		  unsigned bin = 0, plus = 0;
		  switch (val.value.str[0]) {
		      case 'r':
		      case 'w':
		      case 'a':
			for (idx = 1; idx < 3 ; idx++) {
			      if (val.value.str[idx] == '\0') break;
			      switch (val.value.str[idx]) {
				    case 'b':
				      if (bin) fail = 1;
				      bin = 1;
				      break;
				    case '+':
				      if (plus) fail = 1;
				      plus = 1;
				      break;
				    default:
				      fail = 1;
				      break;
			      }
			}
			if (! fail) break;

		      default:
			vpi_printf("WARNING: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s's mode argument (%s) is invalid.\n",
			name, val.value.str);
			fail = 1;
			break;
		  }
	    }

            mode_string = strdup(val.value.str);

	    vpi_free_object(argv);
      }

	/* Get the string form of the file name from the file name
	   argument. */
      val.format = vpiStringVal;
      vpi_get_value(item, &val);

	/* Verify that we have a string and that it is not NULL. */
      if (val.format != vpiStringVal || !*(val.value.str)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's file name argument is not a valid string.\n",
	                name);
	    fail = 1;
	    if (mode) free(mode_string);
      }

	/*
	 * Verify that the file name is composed of only printable
	 * characters.
	 */
      len = strlen(val.value.str);
      for (idx = 0; idx < len; idx++) {
	    if (! isprint(val.value.str[idx])) {
		  char msg [64];
		  snprintf(msg, 64, "WARNING: %s:%d:",
		           vpi_get_str(vpiFile, callh),
		           (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s %s's file name argument contains non-"
		             "printable characters.\n", msg, name);
		  vpi_printf("%*s \"%s\"\n", (int) strlen(msg), " ", val.value.str);
		  fail = 1;
		  if (mode) free(mode_string);
	    }
      }

	/* If either the mode or file name are not valid just return. */
      if (fail) return 0;

      val.format = vpiIntVal;
      if (mode) {
	    val.value.integer = vpi_fopen(val.value.str, mode_string);
	    free(mode_string);
      } else
	    val.value.integer = vpi_mcd_open(val.value.str);

      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

/*
 * Implement the $fopenr(), $fopenw() and $fopena() system functions
 * from Chris Spear's File I/O for Verilog.
 */

static PLI_INT32 sys_fopenrwa_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle file = vpi_scan(argv);
      s_vpi_value val;
      char *mode;
      unsigned idx, len;

      vpi_free_object(argv);

	/* Get the mode. */
      mode = name + strlen(name) - 1;

	/* Get the filename. */
      val.format = vpiStringVal;
      vpi_get_value(file, &val);

	/* Verify that we have a string and that it is not NULL. */
      if (val.format != vpiStringVal || !*(val.value.str)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's file name argument is not a valid string.\n",
	                name);
	    return 0;
      }

	/*
	 * Verify that the file name is composed of only printable
	 * characters.
	 */
      len = strlen(val.value.str);
      for (idx = 0; idx < len; idx++) {
	    if (! isprint(val.value.str[idx])) {
		  char msg [64];
		  snprintf(msg, 64, "WARNING: %s:%d:",
		           vpi_get_str(vpiFile, callh),
		           (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s %s's file name argument contains non-"
		             "printable characters.\n", msg, name);
		  vpi_printf("%*s \"%s\"\n", (int) strlen(msg), " ", val.value.str);
		  return 0;
	    }
      }

	/* Open the file and return the result. */
      val.format = vpiIntVal;
      val.value.integer = vpi_fopen(val.value.str, mode);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

/*
 * Implement $fclose system function
 */
static PLI_INT32 sys_fclose_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle fd = vpi_scan(argv);
      s_vpi_value val;
      PLI_UINT32 fd_mcd;

      vpi_free_object(argv);
      (void) name;  /* Not used! */

      val.format = vpiIntVal;
      vpi_get_value(fd, &val);
      fd_mcd = val.value.integer;

      vpi_mcd_close(fd_mcd);

      return 0;
}

static PLI_INT32 sys_fflush_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;
      (void) name;  /* Not used! */

	/* If we have no argument then flush all the streams. */
      if (argv == 0) {
	    fflush(NULL);
	    return 0;
      }

	/* Get the file/MC descriptor. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

      if (IS_MCD(fd_mcd)) {
	    vpi_mcd_flush(fd_mcd);
      } else {
	      /* If we have a valid file descriptor flush the file. */
	    fp = vpi_get_file(fd_mcd);
	    if (fp) fflush(fp);
      }

      return 0;
}

static PLI_INT32 sys_fputc_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;
      unsigned char chr;
      (void) name;  /* Not used! */

	/* Get the character. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      chr = val.value.integer;


	/* Get the file/MC descriptor. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Put the character and return the result. */
      fp = vpi_get_file(fd_mcd);
      val.format = vpiIntVal;
      if (!fp || IS_MCD(fd_mcd)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n", fd_mcd,
	               name);
	    val.value.integer = EOF;
      } else
	    val.value.integer = fputc(chr, fp);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_fgets_compiletf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/*
	 * Check that there are two arguments and that the first is a
	 * register and that the second is numeric.
	 */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (vpi_get(vpiType, vpi_scan(argv)) != vpiReg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be a reg.\n", name);
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
      if (vpi_scan(argv) != 0) {
	    char msg [64];
	    unsigned argc;

	    snprintf(msg, 64, "ERROR: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));

	    argc = 1;
	    while (vpi_scan(argv)) argc += 1;

	    vpi_printf("%s %s takes two arguments.\n", msg, name);
	    vpi_printf("%*s Found %u extra argument%s.\n",
	               (int) strlen(msg), " ", argc, argc == 1 ? "" : "s");
	    vpi_control(vpiFinish, 1);
      }

      return 0;
}

static PLI_INT32 sys_fgets_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle regh;
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;
      PLI_INT32 reg_size;
      char*text;
      (void) name;  /* Not used! */

	/* Get the register handle. */
      regh = vpi_scan(argv);

	/* Get the file/MCD descriptor. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Return zero if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp || IS_MCD(fd_mcd)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n", fd_mcd,
	               name);
	    val.format = vpiIntVal;
	    val.value.integer = 0;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Get the register size in bytes and allocate the buffer. */
      reg_size = vpi_get(vpiSize, regh) / 8;
      text = malloc(reg_size + 1);

	/* Read in the bytes. Return 0 if there was an error. */
      if (fgets(text, reg_size+1, fp) == 0) {
	    val.format = vpiIntVal;
	    val.value.integer = 0;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    free(text);
	    return 0;
      }

	/* Return the number of character read. */
      val.format = vpiIntVal;
      val.value.integer = strlen(text);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

	/* Return the characters to the register. */
      val.format = vpiStringVal;
      val.value.str = text;
      vpi_put_value(regh, &val, 0, vpiNoDelay);
      free(text);

      return 0;
}

static PLI_INT32 sys_ungetc_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;
      unsigned char chr;
      (void) name;  /* Not used! */

	/* Get the character. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      chr = val.value.integer;

	/* Get the file/MC descriptor. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Return EOF if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp || IS_MCD(fd_mcd)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n", fd_mcd,
	               name);
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* ungetc the character and return the result. */
      val.format = vpiIntVal;
      val.value.integer = ungetc(chr, fp);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_fseek_compiletf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* Check that there are three numeric arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires three arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check that the first argument is numeric. */
      if (! is_numeric_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be numeric.\n", name);
	    vpi_control(vpiFinish, 1);
      }

	/* Check that the second argument exists and is numeric. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second (numeric) argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (!arg || !is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be numeric.\n", name);
	    vpi_control(vpiFinish, 1);
      }

	/* Check that the third argument exists and is numeric. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a third (numeric) argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (!arg || !is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's third argument must be numeric.\n", name);
	    vpi_control(vpiFinish, 1);
      }

	/* Make sure there are no extra arguments. */
      if (vpi_scan(argv) != 0) {
	    char msg [64];
	    unsigned argc;

	    snprintf(msg, 64, "ERROR: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));

	    argc = 1;
	    while (vpi_scan(argv)) argc += 1;

	    vpi_printf("%s %s takes three arguments.\n", msg, name);
	    vpi_printf("%*s Found %u extra argument%s.\n",
	               (int) strlen(msg), " ", argc, argc == 1 ? "" : "s");
	    vpi_control(vpiFinish, 1);
      }

      return 0;
}

static PLI_INT32 sys_fseek_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      PLI_INT32 offset, oper;
      FILE *fp;


	/* Get the file pointer. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Get the offset. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      offset = val.value.integer;

	/* Get the operation. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      oper = val.value.integer;

	/* Check that the operation is in the valid range. */
      if ((oper < 0) || (oper > 2)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's operation must be 0, 1 or 2 given %d.\n",
	               name, oper);
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Return EOF if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp || IS_MCD(fd_mcd)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n", fd_mcd,
	               name);
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

      val.format = vpiIntVal;
      val.value.integer = fseek(fp, offset, oper);
      vpi_put_value(callh, &val, 0 , vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_common_fd_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;

	/* Get the file pointer. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Return EOF if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp || IS_MCD(fd_mcd)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n", fd_mcd,
	               name);
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

      val.format = vpiIntVal;
      switch (name[4]) {
	  case 'l':  /* $ftell() */
	    val.value.integer = ftell(fp);
	    break;
	  case 'f':  /* $feof() is from 1264-2005*/
	    val.value.integer = feof(fp);
	    break;
	  case 'i':  /* $rewind() */
	    val.value.integer = fseek(fp, 0L, SEEK_SET);
	    break;
	  case 't':  /* $fgetc() */
	    val.value.integer = fgetc(fp);
	    break;
	  default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s cannot be processed with this routine.\n", name);
	    assert(0);
	    break;
      }
      vpi_put_value(callh, &val, 0 , vpiNoDelay);

      return 0;
}

void sys_fileio_register()
{
      s_vpi_systf_data tf_data;

      /*============================== fopen */
      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$fopen";
      tf_data.calltf      = sys_fopen_calltf;
      tf_data.compiletf   = sys_fopen_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$fopen";
      vpi_register_systf(&tf_data);

      /*============================== fopenr */
      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$fopenr";
      tf_data.calltf      = sys_fopenrwa_calltf;
      tf_data.compiletf   = sys_one_string_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$fopenr";
      vpi_register_systf(&tf_data);

      /*============================== fopenw */
      tf_data.tfname      = "$fopenw";
      tf_data.user_data   = "$fopenw";
      vpi_register_systf(&tf_data);

      /*============================== fopena */
      tf_data.tfname      = "$fopena";
      tf_data.user_data   = "$fopena";
      vpi_register_systf(&tf_data);

      /*============================== fclose */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fclose";
      tf_data.calltf    = sys_fclose_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fclose";
      vpi_register_systf(&tf_data);

      /*============================== fflush */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fflush";
      tf_data.calltf    = sys_fflush_calltf;
      tf_data.compiletf = sys_one_opt_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fflush";
      vpi_register_systf(&tf_data);

      /*============================== fputc */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fputc";
      tf_data.calltf    = sys_fputc_calltf;
      tf_data.compiletf = sys_two_numeric_args_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fputc";
      vpi_register_systf(&tf_data);

      /*============================== fgetc */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$fgetc";
      tf_data.calltf    = sys_common_fd_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fgetc";
      vpi_register_systf(&tf_data);

      /*============================== fgets */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$fgets";
      tf_data.calltf    = sys_fgets_calltf;
      tf_data.compiletf = sys_fgets_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fgets";
      vpi_register_systf(&tf_data);

      /*============================== ungetc */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$ungetc";
      tf_data.calltf    = sys_ungetc_calltf;
      tf_data.compiletf = sys_two_numeric_args_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ungetc";
      vpi_register_systf(&tf_data);

      /*============================== ftell */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$ftell";
      tf_data.calltf    = sys_common_fd_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ftell";
      vpi_register_systf(&tf_data);

      /*============================== fseek */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$fseek";
      tf_data.calltf    = sys_fseek_calltf;
      tf_data.compiletf = sys_fseek_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fseek";
      vpi_register_systf(&tf_data);

      /*============================== rewind */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$rewind";
      tf_data.calltf    = sys_common_fd_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$rewind";
      vpi_register_systf(&tf_data);

/* $feof() is from 1364-2005. */
      /*============================== feof */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$feof";
      tf_data.calltf    = sys_common_fd_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$feof";
      vpi_register_systf(&tf_data);

}
