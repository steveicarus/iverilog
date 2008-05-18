/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: sys_fileio.c,v 1.10 2007/03/14 04:05:51 steve Exp $"
#endif

# include  "vpi_user.h"
# include  "sys_priv.h"
# include  <assert.h>
# include  <string.h>
# include  <stdio.h>
# include  <stdlib.h>

#define IS_MCD(mcd)     !((mcd)>>31&1)


/*
 * Implement the $fopen system function.
 */
static PLI_INT32 sys_fopen_compiletf(PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item;

      if (argv == 0) {
	    vpi_printf("%s: file name argument missing.\n", name);
	    vpi_control(vpiFinish, 1);
	    return -1;
      }

      item = vpi_scan(argv);
      if (item == 0) {
	    vpi_printf("%s: file name argument missing.\n", name);
	    vpi_control(vpiFinish, 1);
	    return -1;
      }

      item = vpi_scan(argv);
      if (item == 0) {
	      /* The mode argument is optional. It is OK for it
		 to be missing. In this case, there are no more
		 arguments, and we're done. */
	    return 0;
      }

      if (! is_constant(item)) {
	    vpi_printf("ERROR: %s mode argument must be a constant\n", name);
	    vpi_control(vpiFinish, 1);
      }

      if (vpi_get(vpiConstType, item) != vpiStringConst) {
	    vpi_printf("ERROR: %s mode argument must be a string.\n", name);
	    vpi_control(vpiFinish, 1);
      }
 
      item = vpi_scan(argv);
      if (item == 0) {
	      /* There should be no more arguments. */
	    return 0;
      }

      vpi_free_object(argv);
      vpi_printf("%s: Too many arguments to system function.\n", name);
      return 0;
}

static PLI_INT32 sys_fopen_calltf(PLI_BYTE8*name)
{
      s_vpi_value value;
      char *mode_string = 0;

      vpiHandle call_handle = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, call_handle);
      vpiHandle item = vpi_scan(argv);
      vpiHandle mode = vpi_scan(argv);

      assert(item);

      if (mode) {
           value.format = vpiStringVal;
           vpi_get_value(mode, &value);
           mode_string = strdup(value.value.str);

	   vpi_free_object(argv);
      }

	/* Get the string form of the file name from the file name
	   argument. */
      value.format = vpiStringVal;
      vpi_get_value(item, &value);

      if ((value.format != vpiStringVal) || !value.value.str) {
	    vpi_printf("ERROR: %s: File name argument (type=%d)"
		       " does not have a string value\n",
		       name, vpi_get(vpiType, item));
	    if (mode) free(mode_string);
	    return 0;
      }

      value.format = vpiIntVal;
      if (mode) {
	    value.value.integer = vpi_fopen(value.value.str, mode_string);
	    free(mode_string);
      } else
	    value.value.integer = vpi_mcd_open(value.value.str);

      vpi_put_value(call_handle, &value, 0, vpiNoDelay);

      return 0;
}

/*
 * Implement the $fopenr(), $fopenw() and $fopena() system functions
 * from Chris Spear's File I/O for Verilog.
 */
static PLI_INT32 sys_fopenrwa_compiletf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle file;

      /* Check that there are arguments. */
      if (argv == 0) {
            vpi_printf("ERROR: %s requires a single argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      file = vpi_scan(argv); /* This should never be zero. */

      /* These functions take at most one argument. */
      file = vpi_scan(argv);
      if (file != 0) {
            vpi_printf("ERROR: %s takes only a single argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* vpi_scan returning 0 (NULL) has already freed argv. */
      return 0;
}

static PLI_INT32 sys_fopenrwa_calltf(PLI_BYTE8*name)
{
      s_vpi_value val;
      char *mode;
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle file = vpi_scan(argv);
      vpi_free_object(argv);

      /* Get the mode. */
      mode = name + strlen(name) - 1;

      /* Get the filename. */
      val.format = vpiStringVal;
      vpi_get_value(file, &val);
      if ((val.format != vpiStringVal) || !val.value.str) {
	    vpi_printf("ERROR: %s's file name argument must be a string.\n",
                        name);
            vpi_control(vpiFinish, 1);
	    return 0;
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
      unsigned int mcd;
      int type;
      s_vpi_value value;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item == 0) {
	    vpi_printf("%s: mcd parameter missing.\n", name);
	    return 0;
      }
      type = vpi_get(vpiType, item);
      switch (type) {
	    case vpiReg:
	    case vpiRealVal:
	    case vpiIntegerVar:
	      break;
	    default:
	      vpi_printf("ERROR: %s mcd parameter must be of integral type",
		name);
	      vpi_printf(", got vpiType=%d\n", type);
	      vpi_free_object(argv);
	      return 0;
      }

      value.format = vpiIntVal;
      vpi_get_value(item, &value);
      mcd = value.value.integer;

      vpi_mcd_close(mcd);
      return 0;
}

static PLI_INT32 sys_fflush_compiletf(PLI_BYTE8 *ud)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;
      PLI_INT32 type;

      /* The argument is optional. */
      if (argv == 0) {
	    return 0;
      }
      /* Check that the file/MC descriptor is the right type. */
      item = vpi_scan(argv);
      type = vpi_get(vpiType, item);
      switch (type) {
	    case vpiReg:
	    case vpiRealVal:
	    case vpiIntegerVar:
	      break;
	    default:
	      vpi_printf("ERROR: %s fd parameter must be integral", ud);
	      vpi_printf(", got vpiType=%d\n", type);
	      vpi_control(vpiFinish, 1);
	      return 0;
      }

      /* Check that there is at most one argument. */
      item = vpi_scan(argv);
      if (item != 0) {
	    vpi_printf("ERROR: %s takes at most a single argument.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* vpi_scan returning 0 (NULL) has already freed argv. */
      return 0;
}

static PLI_INT32 sys_fflush_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;
      s_vpi_value val;
      PLI_INT32 fd_mcd;
      FILE *fp;

      /* If we have no argument then flush all the streams. */
      if (argv == 0) {
	    fflush(NULL);
	    return 0;
      }

      /* Get the file/MC descriptor. */
      item = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(item, &val);
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
      unsigned int mcd;
      int type;
      unsigned char x;
      s_vpi_value value, xvalue;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);
      FILE *fp;

      if (item == 0) {
	    vpi_printf("%s: mcd parameter missing.\n", name);
	    return 0;
      }

      type = vpi_get(vpiType, item);
      switch (type) {
	    case vpiReg:
	    case vpiRealVal:
	    case vpiIntegerVar:
	      break;
	    default:
	      vpi_printf("ERROR: %s mcd parameter must be of integral", name);
	      vpi_printf(", got vpiType=%d\n", type);
	      vpi_free_object(argv);
	      return 0;
      }

      value.format = vpiIntVal;
      vpi_get_value(item, &value);
      mcd = value.value.integer;

      if (IS_MCD(mcd)) return EOF;

      item = vpi_scan(argv);

      xvalue.format = vpiIntVal;
      vpi_get_value(item, &xvalue);
      x = xvalue.value.integer;

      fp = vpi_get_file(mcd);
      if (!fp) return EOF;

      return fputc(x, fp);
}

static PLI_INT32 sys_fgetc_calltf(PLI_BYTE8*name)
{
      unsigned int mcd;
      int type;
      s_vpi_value value, rval;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);
      FILE *fp;

      if (item == 0) {
	    vpi_printf("%s: mcd parameter missing.\n", name);
	    return 0;
      }

      type = vpi_get(vpiType, item);
      switch (type) {
	    case vpiReg:
	    case vpiRealVal:
	    case vpiIntegerVar:
	      break;
	    default:
	      vpi_printf("ERROR: %s mcd parameter must be of integral", name);
	      vpi_printf(", got vpiType=%d\n", type);
	      vpi_free_object(argv);
	      return 0;
      }

      value.format = vpiIntVal;
      vpi_get_value(item, &value);
      mcd = value.value.integer;

      rval.format = vpiIntVal;

      fp = vpi_get_file(mcd);
      if (!fp || IS_MCD(mcd))
	  rval.value.integer = EOF;
      else
	  rval.value.integer = fgetc(fp);

      vpi_put_value(sys, &rval, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_fgets_compiletf(PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);
      int type;

      if (item == 0) {
	    vpi_printf("%s: string parameter missing.\n", name);
	    return 0;
      }

      type = vpi_get(vpiType, item);

      if (type != vpiReg) {
	    vpi_printf("%s: string parameter must be a reg.\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      item = vpi_scan(argv);
      if (item == 0) {
	    vpi_printf("%s: mcd parameter missing.\n", name);
	    return 0;
      }

	/* That should be all the arguments. */
      item = vpi_scan(argv);
      assert(item == 0);

      return 0;
}

static PLI_INT32 sys_fgets_calltf(PLI_BYTE8*name)
{
      unsigned int mcd;
      FILE*fd;
      s_vpi_value value, rval;

      char*txt;
      unsigned txt_len;

      vpiHandle sys  = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle str  = vpi_scan(argv);
      vpiHandle mch  = vpi_scan(argv);

      value.format = vpiIntVal;
      vpi_get_value(mch, &value);
      mcd = value.value.integer;

      fd = vpi_get_file(mcd);
      if (!fd || IS_MCD(mcd)) {
	    rval.format = vpiIntVal;
	    rval.value.integer = 0;
	    vpi_put_value(sys, &rval, 0, vpiNoDelay);
	    return 0;
      }

      txt_len = vpi_get(vpiSize, str) / 8;
      txt = malloc(txt_len + 1);

      if (fgets(txt, txt_len+1, fd) == 0) {
	    rval.format = vpiIntVal;
	    rval.value.integer = 0;
	    vpi_put_value(sys, &rval, 0, vpiNoDelay);
	    free(txt);
	    return 0;
      }

      rval.format = vpiIntVal;
      rval.value.integer = strlen(txt);
      vpi_put_value(sys, &rval, 0, vpiNoDelay);

      value.format = vpiStringVal;
      value.value.str = txt;
      vpi_put_value(str, &value, 0, vpiNoDelay);

      free(txt);

      return 0;
}

static PLI_INT32 sys_ungetc_compiletf(PLI_BYTE8*name)
{
      int type;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item == 0) {
	    vpi_printf("%s: character parameter missing.\n", name);
	    return 0;
      }
      type = vpi_get(vpiType, item);
      switch (type) {
	    case vpiReg:
	    case vpiRealVal: // Is this correct?
	    case vpiIntegerVar:
	      break;
	    default:
	      vpi_printf("ERROR: %s character parameter must be ", name);
	      vpi_printf("integral, got vpiType=%d\n", type);
	      vpi_free_object(argv);
	      return 0;
      }

      item = vpi_scan(argv);
      type = vpi_get(vpiType, item);
      switch (type) {
	    case vpiReg:
	    case vpiRealVal: // Is this correct?
	    case vpiIntegerVar:
	      break;
	    default:
	      vpi_printf("ERROR: %s mcd parameter must be integral, ", name);
	      vpi_printf("got vpiType=%d\n", type);
	      vpi_free_object(argv);
	      return 0;
      }

	/* That should be all the arguments. */
      item = vpi_scan(argv);
      assert(item == 0);

      return 0;
}

static PLI_INT32 sys_ungetc_calltf(PLI_BYTE8*name)
{
      unsigned int mcd;
      unsigned char x;
      s_vpi_value val, rval;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);
      FILE *fp;

      rval.format = vpiIntVal;

      val.format = vpiIntVal;
      vpi_get_value(item, &val);
      x = val.value.integer;

      item = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(item, &val);
      mcd = val.value.integer;

      if (IS_MCD(mcd)) {
	    rval.value.integer = EOF;
	    vpi_put_value(sys, &rval, 0, vpiNoDelay);
	    return 0;
      }

      fp = vpi_get_file(mcd);
      if ( !fp ) {
	    rval.value.integer = EOF;
	    vpi_put_value(sys, &rval, 0, vpiNoDelay);
	    return 0;
      }

      ungetc(x, fp);

      rval.value.integer = 0;
      vpi_put_value(sys, &rval, 0, vpiNoDelay);
      return 0;
}

static PLI_INT32 sys_check_fd_compiletf(PLI_BYTE8 *ud)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;
      PLI_INT32 type;

      /* Check that there is an argument. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s requires an argument.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      /* Check that the file descriptor is the right type. */
      item = vpi_scan(argv);
      type = vpi_get(vpiType, item);
      switch (type) {
	    case vpiReg:
	    case vpiRealVal:
	    case vpiIntegerVar:
	      break;
	    default:
	      vpi_printf("ERROR: %s fd parameter must be integral", ud);
	      vpi_printf(", got vpiType=%d\n", type);
	      vpi_control(vpiFinish, 1);
	      return 0;
      }

      /* Check that there is at most one argument. */
      item = vpi_scan(argv);
      if (item != 0) {
	    vpi_printf("ERROR: %s takes a single argument.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* vpi_scan returning 0 (NULL) has already freed argv. */
      return 0;
}

static PLI_INT32 sys_rewind_calltf(PLI_BYTE8 *ud)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item = vpi_scan(argv);
      s_vpi_value val;
      PLI_INT32 fd;
      FILE *fp;

      /* Get the file pointer. */
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(item, &val);
      fd = val.value.integer;
      if (IS_MCD(fd)) {
	    vpi_printf("ERROR: %s cannot be used with a MCD.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      fp = vpi_get_file(fd);

      /* If we have a valid file descriptor rewind the file. */
      if (!fp) {
	    val.value.integer = EOF;
      } else {
	    rewind(fp);
	    val.value.integer = 0;
      }
      vpi_put_value(callh, &val, 0 , vpiNoDelay);

      return 0;
}

/* $feof() is from 1364-2005. */
static PLI_INT32 sys_ftell_feof_calltf(PLI_BYTE8 *ud)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item = vpi_scan(argv);
      s_vpi_value val;
      PLI_INT32 fd;
      FILE *fp;

      /* Get the file pointer. */
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(item, &val);
      fd = val.value.integer;
      if (IS_MCD(fd)) {
	    vpi_printf("ERROR: %s cannot be used with a MCD.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      fp = vpi_get_file(fd);

      /* If we do not have a valid file descriptor return EOF, otherwise
       * return that value. */
      if (!fp) {
	    val.value.integer = EOF;
      } else {
	    if (ud[2] == 'e') {
	          val.value.integer = feof(fp);
	    } else {
	          val.value.integer = ftell(fp);
	    }
      }
      vpi_put_value(callh, &val, 0 , vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_fseek_compiletf(PLI_BYTE8 *ud)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;
      PLI_INT32 type;

      /* Check that there is an argument. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s requires three arguments.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      /* Check that the file descriptor is the right type. */
      item = vpi_scan(argv);
      type = vpi_get(vpiType, item);
      switch (type) {
	    case vpiReg:
	    case vpiRealVal:
	    case vpiIntegerVar:
	      break;
	    default:
	      vpi_printf("ERROR: %s fd parameter must be integral", ud);
	      vpi_printf(", got vpiType=%d\n", type);
	      vpi_control(vpiFinish, 1);
	      return 0;
      }

      /* Check that there is an offset argument. */
      item = vpi_scan(argv);
      if (item == 0) {
	    vpi_printf("ERROR: %s is missing an offset and operation "
	               "argument.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there is an operation argument. */
      item = vpi_scan(argv);
      if (item == 0) {
	    vpi_printf("ERROR: %s is missing an operation argument.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there is at most one argument. */
      item = vpi_scan(argv);
      if (item != 0) {
	    vpi_printf("ERROR: %s takes at most three argument.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* vpi_scan returning 0 (NULL) has already freed argv. */

      return 0;
}

static PLI_INT32 sys_fseek_calltf(PLI_BYTE8 *ud)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;
      s_vpi_value val;
      PLI_INT32 fd, offset, oper;
      FILE *fp;

      val.format = vpiIntVal;

      /* Get the file pointer. */
      item = vpi_scan(argv);
      vpi_get_value(item, &val);
      fd = val.value.integer;
      if (IS_MCD(fd)) {
	    vpi_printf("ERROR: %s cannot be used with a MCD.\n", ud);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      fp = vpi_get_file(fd);

      /* Get the offset. */
      item = vpi_scan(argv);
      vpi_get_value(item, &val);
      offset = val.value.integer;

      /* Get the operation. */
      item = vpi_scan(argv);
      vpi_get_value(item, &val);
      oper = val.value.integer;
      /* Should this translate to the SEEK_??? codes? */
      /* What about verifying offset value vs operation ($fseek(fd, -1, 0))? */
      if ((oper < 0) || (oper > 2)) {
	    vpi_printf("ERROR: %s's operation must be 0, 1 or 2 given %d.\n",
	               ud, oper);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* If we do not have a valid file descriptor return EOF, otherwise
       * return that value. */
      if (!fp) {
	    val.value.integer = EOF;
      } else {
	    fseek(fp, offset, oper);
	    val.value.integer = 0;
      }
      vpi_put_value(callh, &val, 0 , vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_integer_sizetf(PLI_BYTE8 *ud)
{
      return 32;
}

void sys_fileio_register()
{
      s_vpi_systf_data tf_data;

      //============================== fopen
      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$fopen";
      tf_data.calltf    = sys_fopen_calltf;
      tf_data.compiletf = sys_fopen_compiletf;
      tf_data.sizetf    = sys_integer_sizetf;
      tf_data.user_data = "$fopen";
      vpi_register_systf(&tf_data);

      //============================== fopenr
      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$fopenr";
      tf_data.calltf    = sys_fopenrwa_calltf;
      tf_data.compiletf = sys_fopenrwa_compiletf;
      tf_data.sizetf    = sys_integer_sizetf;
      tf_data.user_data = "$fopenr";
      vpi_register_systf(&tf_data);

      //============================== fopenw
      tf_data.tfname    = "$fopenw";
      tf_data.user_data = "$fopenw";
      vpi_register_systf(&tf_data);

      //============================== fopena
      tf_data.tfname    = "$fopena";
      tf_data.user_data = "$fopena";
      vpi_register_systf(&tf_data);

      //============================== fclose
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fclose";
      tf_data.calltf    = sys_fclose_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fclose";
      vpi_register_systf(&tf_data);

      //============================== fflush
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fflush";
      tf_data.calltf    = sys_fflush_calltf;
      tf_data.compiletf = sys_fflush_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fflush";
      vpi_register_systf(&tf_data);

      //============================== fputc
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fputc";
      tf_data.calltf    = sys_fputc_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fputc";
      vpi_register_systf(&tf_data);

      //============================== fgetc
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname    = "$fgetc";
      tf_data.calltf    = sys_fgetc_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = sys_integer_sizetf;
      tf_data.user_data = "$fgetc";
      vpi_register_systf(&tf_data);

      //============================== fgets
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname    = "$fgets";
      tf_data.calltf    = sys_fgets_calltf;
      tf_data.compiletf = sys_fgets_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fgets";
      vpi_register_systf(&tf_data);

      //============================== ungetc
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname    = "$ungetc";
      tf_data.calltf    = sys_ungetc_calltf;
      tf_data.compiletf = sys_ungetc_compiletf;
      tf_data.sizetf    = sys_integer_sizetf;
      tf_data.user_data = "$ungetc";
      vpi_register_systf(&tf_data);

      //============================== ftell
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname    = "$ftell";
      tf_data.calltf    = sys_ftell_feof_calltf;
      tf_data.compiletf = sys_check_fd_compiletf;
      tf_data.sizetf    = sys_integer_sizetf;
      tf_data.user_data = "$ftell";
      vpi_register_systf(&tf_data);

      //============================== fseek
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname    = "$fseek";
      tf_data.calltf    = sys_fseek_calltf;
      tf_data.compiletf = sys_fseek_compiletf;
      tf_data.sizetf    = sys_integer_sizetf;
      tf_data.user_data = "$fseek";
      vpi_register_systf(&tf_data);

      //============================== rewind
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname    = "$rewind";
      tf_data.calltf    = sys_rewind_calltf;
      tf_data.compiletf = sys_check_fd_compiletf;
      tf_data.sizetf    = sys_integer_sizetf;
      tf_data.user_data = "$rewind";
      vpi_register_systf(&tf_data);

/* $feof() is from 1364-2005. */
      //============================== feof
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname    = "$feof";
      tf_data.calltf    = sys_ftell_feof_calltf;
      tf_data.compiletf = sys_check_fd_compiletf;
      tf_data.sizetf    = sys_integer_sizetf;
      tf_data.user_data = "$feof";
      vpi_register_systf(&tf_data);

}

