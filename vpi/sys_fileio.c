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
#ident "$Id: sys_fileio.c,v 1.2 2003/11/07 19:40:05 steve Exp $"
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
static int sys_fopen_calltf(char *name)
{
      s_vpi_value value;
      unsigned char *mode_string = 0;

      vpiHandle call_handle = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, call_handle);
      vpiHandle item = argv ? vpi_scan(argv) : 0;
      vpiHandle mode = item ? vpi_scan(argv) : 0;

      if (item == 0) {
	    vpi_printf("%s: file name parameter missing.\n", name);
	    return 0;
      }

      if (mode == 0) {
	    argv = 0;
      }

      if (! is_constant(item)) {
	    vpi_printf("ERROR: %s parameter must be a constant\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      if (vpi_get(vpiConstType, item) != vpiStringConst) {
	    vpi_printf("ERROR: %s parameter must be a string.\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      if (mode) {
	    if (! is_constant(mode)) {
		vpi_printf("ERROR: %s parameter must be a constant\n", name);
		vpi_free_object(argv);
	        return 0;
	    }

           if (vpi_get(vpiConstType, mode) != vpiStringConst) {
               vpi_printf("ERROR: %s parameter must be a string.\n", name);
               vpi_free_object(argv);
               return 0;
           }
           value.format = vpiStringVal;
           vpi_get_value(mode, &value);
           mode_string = strdup(value.value.str);
      }

      value.format = vpiStringVal;
      vpi_get_value(item, &value);

      value.format = vpiIntVal;
      if (mode) {
	    value.value.integer = vpi_fopen(value.value.str, mode_string);
	    free(mode_string);
      } else
	    value.value.integer = vpi_mcd_open(value.value.str);

      vpi_put_value(call_handle, &value, 0, vpiNoDelay);

      return 0;
}

static int sys_fopen_sizetf(char*x)
{
      return 32;
}

/*
 * Implement $fclose system function
 */
static int sys_fclose_calltf(char *name)
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

static int sys_fflush_calltf(char *name)
{
      fflush(0);
      return 0;
}


static int sys_fputc_calltf(char *name)
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

static int sys_fgetc_calltf(char *name)
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

static int sys_fgetc_sizetf(char*x)
{
      return 32;
}


static int sys_ungetc_compiletf(char*name)
{
      int type;
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
	      vpi_printf("ERROR: %s mcd parameter must be of integral", name);
	      vpi_printf(", got vpiType=%d\n", type);
	      vpi_free_object(argv);
	      return 0;
      }

      return 0;
}

static int sys_ungetc_calltf(char *name)
{
      unsigned int mcd;
      unsigned char x;
      s_vpi_value value, xvalue, rval;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);
      FILE *fp;

      rval.format = vpiIntVal;

      assert(item);

      value.format = vpiIntVal;
      vpi_get_value(item, &value);
      mcd = value.value.integer;

      if (IS_MCD(mcd)) {
	    rval.value.integer = EOF;
	    vpi_put_value(sys, &rval, 0, vpiNoDelay);
	    return 0;
      }

      item = vpi_scan(argv);

      xvalue.format = vpiIntVal;
      vpi_get_value(item, &xvalue);
      x = xvalue.value.integer;

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

static int sys_ungetc_sizetf(char*x)
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
      tf_data.compiletf = 0;
      tf_data.sizetf    = sys_fopen_sizetf;
      tf_data.user_data = "$fopen";
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
      tf_data.compiletf = 0;
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
      tf_data.tfname    = "$fgetc";
      tf_data.calltf    = sys_fgetc_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = sys_fgetc_sizetf;
      tf_data.user_data = "$fgetc";
      vpi_register_systf(&tf_data);

      //============================== ungetc
      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$ungetc";
      tf_data.calltf    = sys_ungetc_calltf;
      tf_data.compiletf = sys_ungetc_compiletf;
      tf_data.sizetf    = sys_ungetc_sizetf;
      tf_data.user_data = "$ungetc";
      vpi_register_systf(&tf_data);

}

/*
 * $Log: sys_fileio.c,v $
 * Revision 1.2  2003/11/07 19:40:05  steve
 *  Implement basic fflush.
 *
 * Revision 1.1  2003/10/30 03:43:20  steve
 *  Rearrange fileio functions, and add ungetc.
 *
 */

