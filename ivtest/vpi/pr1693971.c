/**********************************************************************
 * $my_pow example -- PLI application using VPI routines
 *
 * C source to calculate the result of a number to the power of an
 * exponent. The result is returned as a 32-bit integer.
 *
 * Usage: result = $my_pow(<base>,<exponent>);
 *
 * For the book, "The Verilog PLI Handbook" by Stuart Sutherland
 *  Copyright 1999 & 2001, Kluwer Academic Publishers, Norwell, MA, USA
 *   Contact: www.wkap.il
 *  Example copyright 1998, Sutherland HDL Inc, Portland, Oregon, USA
 *   Contact: www.sutherland-hdl.com
 *********************************************************************/

#define VPI_1995 0 /* set to non-zero for Verilog-1995 compatibility */

#include <stdlib.h>    /* ANSI C standard library */
#include <stdio.h>     /* ANSI C standard input/output library */
#include <stdarg.h>    /* ANSI C standard arguments library */
#include "vpi_user.h"  /* IEEE 1364 PLI VPI routine library  */

#if VPI_1995
#include "../vpi_1995_compat.h"  /* kludge new Verilog-2001 routines */
#endif

/* prototypes of PLI application routine names */
static PLI_INT32 PLIbook_PowSizetf(PLI_BYTE8 *user_data);
static PLI_INT32 PLIbook_PowCalltf(PLI_BYTE8 *user_data);
static PLI_INT32 PLIbook_PowCompiletf(PLI_BYTE8 *user_data);
static PLI_INT32 PLIbook_PowStartOfSim(s_cb_data *callback_data);

/**********************************************************************
 * $my_pow Registration Data
 * (add this function name to the vlog_startup_routines array)
 *********************************************************************/
void PLIbook_pow_register(void)
{
  s_vpi_systf_data tf_data;
  s_cb_data        cb_data_s;
  vpiHandle        callback_handle;

  tf_data.type        = vpiSysFunc;
  tf_data.sysfunctype = vpiSysFuncSized;
  tf_data.tfname      = "$my_pow";
  tf_data.calltf      = PLIbook_PowCalltf;
  tf_data.compiletf   = PLIbook_PowCompiletf;
  tf_data.sizetf      = PLIbook_PowSizetf;
  tf_data.user_data   = NULL;
  vpi_register_systf(&tf_data);

  cb_data_s.reason    = cbStartOfSimulation;
  cb_data_s.cb_rtn    = PLIbook_PowStartOfSim;
  cb_data_s.obj       = NULL;
  cb_data_s.time      = NULL;
  cb_data_s.value     = NULL;
  cb_data_s.user_data = NULL;
  callback_handle = vpi_register_cb(&cb_data_s);
  vpi_free_object(callback_handle);  /* don't need callback handle */
}

/**********************************************************************
 * Sizetf application
 *********************************************************************/
static PLI_INT32 PLIbook_PowSizetf(PLI_BYTE8 *user_data)
{
  (void)user_data;  /* Parameter is not used. */
  //vpi_printf("\n$my_pow PLI sizetf function.\n\n");
  return(32);   /* $my_pow returns 32-bit values */
}

/**********************************************************************
 * compiletf application to verify valid systf args.
 *********************************************************************/
static PLI_INT32 PLIbook_PowCompiletf(PLI_BYTE8 *user_data)
{
  vpiHandle systf_handle, arg_itr, arg_handle;
  PLI_INT32 tfarg_type;
  int       err_flag = 0;

  (void)user_data;  /* Parameter is not used. */

  vpi_printf("\n$my_pow PLI compiletf function.\n\n");

  do { /* group all tests, so can break out of group on error */
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    arg_itr = vpi_iterate(vpiArgument, systf_handle);
    if (arg_itr == NULL) {
      vpi_printf("ERROR: $my_pow requires 2 arguments; has none\n");
      err_flag = 1;
      break;
    }
    arg_handle = vpi_scan(arg_itr);
    tfarg_type = vpi_get(vpiType, arg_handle);
    if ( (tfarg_type != vpiReg) &&
         (tfarg_type != vpiIntegerVar) &&
         (tfarg_type != vpiConstant)   ) {
      vpi_printf("ERROR: $my_pow arg1 must be number, variable or net\n");
      err_flag = 1;
      break;
    }
    arg_handle = vpi_scan(arg_itr);
    if (arg_handle == NULL) {
      vpi_printf("ERROR: $my_pow requires 2nd argument\n");
      err_flag = 1;
      break;
    }
    tfarg_type = vpi_get(vpiType, arg_handle);
    if ( (tfarg_type != vpiReg) &&
         (tfarg_type != vpiIntegerVar) &&
         (tfarg_type != vpiConstant)   ) {
      vpi_printf("ERROR: $my_pow arg2 must be number, variable or net\n");
      err_flag = 1;
      break;
    }
    if (vpi_scan(arg_itr) != NULL) {
      vpi_printf("ERROR: $my_pow requires 2 arguments; has too many\n");
      vpi_free_object(arg_itr);
      err_flag = 1;
      break;
    }
  } while (0 == 1); /* end of test group; only executed once */

  if (err_flag) {
    vpi_control(vpiFinish, 1);  /* abort simulation */
  }
  return(0);
}

/**********************************************************************
 * calltf to calculate base to power of exponent and return result.
 *********************************************************************/
#include <math.h>
static PLI_INT32 PLIbook_PowCalltf(PLI_BYTE8 *user_data)
{
  s_vpi_value value_s;
  vpiHandle   systf_handle, arg_itr, arg_handle;
  PLI_INT32   base, expo;
  double      result;

  (void)user_data;  /* Parameter is not used. */

  //vpi_printf("\n$my_pow PLI calltf function.\n\n");

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_itr = vpi_iterate(vpiArgument, systf_handle);
  if (arg_itr == NULL) {
    vpi_printf("ERROR: $my_pow failed to obtain systf arg handles\n");
    return(0);
  }

  /* read base from systf arg 1 (compiletf has already verified) */
  arg_handle = vpi_scan(arg_itr);
  value_s.format = vpiIntVal;
  vpi_get_value(arg_handle, &value_s);
  base = value_s.value.integer;

  /* read exponent from systf arg 2 (compiletf has already verified) */
  arg_handle = vpi_scan(arg_itr);
  vpi_free_object(arg_itr); /* not calling scan until returns null */
  vpi_get_value(arg_handle, &value_s);
  expo = value_s.value.integer;

  /* calculate result of base to power of exponent */
  result = pow( (double)base, (double)expo );

  /* write result to simulation as return value $my_pow */
  value_s.value.integer = (PLI_INT32)result;
  vpi_put_value(systf_handle, &value_s, NULL, vpiNoDelay);
  return(0);
}

/**********************************************************************
 * Start-of-simulation application
 *********************************************************************/
static PLI_INT32 PLIbook_PowStartOfSim(s_cb_data *callback_data)
{
  (void)callback_data;  /* Parameter is not used. */
  vpi_printf("\n$my_pow StartOfSim callback.\n\n");
  return(0);
}
/*********************************************************************/


void (*vlog_startup_routines[])(void) =
{
    /*** add user entries here ***/
  PLIbook_pow_register,
  //PLIbook_test_user_data_register,
  0 /*** final entry must be 0 ***/
};
