/*
 *  Copyright (C) 2018-2021  Cary R. (cygcary@yahoo.com)
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

#include <assert.h>
#include "vpi_user.h"
#include "sys_priv.h"

/*
 * Check that $couintbits() is called with the correct arguments.
 */
static PLI_INT32 countbits_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, arg;
      int cb_count = 1;

      assert(callh != 0);
      argv = vpi_iterate(vpiArgument, callh);
      (void)name;  /* Parameter is not used. */

	/* $countbits() must have arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("$countbits() requires at least two arguments.\n");
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The 1st argument must be numeric. */
      arg = vpi_scan(argv);
      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("The first argument to $countbits() must be numeric.\n");
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* We need one or more numeric control bit arguments. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("$countbits() requires at least one control bit "
	               "argument.\n");
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      do {
	    if (arg && ! is_numeric_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("Control bit argument %d to $countbits() must "
		             "be numeric.\n", cb_count);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
	    ++cb_count;
	    if (arg) arg = vpi_scan(argv);
      } while (arg);

      return 0;
}

/* Count the number of bits in the expression that match the search bits. */
static PLI_INT32 count_bits_in_expr(vpiHandle expr_arg, char search[4])
{
      s_vpi_value val;
      PLI_INT32 result;
      PLI_INT32 size = vpi_get(vpiSize, expr_arg);
      assert(size > 0);

      val.format = vpiVectorVal;
      vpi_get_value(expr_arg, &val);

      result = 0;
      for (unsigned lp = 0; lp < (unsigned)size; ++lp) {
	    unsigned offset = lp / 32;
	    unsigned bit = lp % 32;
	    unsigned abit, bbit;
	    abit = (val.value.vector[offset].aval >> bit) & 0x1;
	    bbit = (val.value.vector[offset].bval >> bit) & 0x1;
	    if (search[(bbit<<1)|abit]) ++result;
      }

      return result;
}

static PLI_INT32 countbits_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle expr_arg = vpi_scan(argv);
      vpiHandle arg;
      char search[4];
      (void)name;  /* Parameter is not used. */

	/* Scan the control bit arguments and mark which control bits to
	 * include in the count. */
      for (unsigned lp = 0; lp < 4 ; ++lp) search[lp] = 0;
      while ((arg = vpi_scan(argv))) {
	    s_vpi_value val;
	    val.format = vpiScalarVal;
	    vpi_get_value(arg, &val);
	    switch (val.value.scalar) {
	      case vpi0:
		  search[0] = 1;
		  break;
	      case vpi1:
		  search[1] = 1;
		  break;
	      case vpiZ:
		  search[2] = 1;
		  break;
	      case vpiX:
		  search[3] = 1;
		  break;
	      default:
		  vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("Unknown scalar control bit argument %d passed "
		             "to $countbits() will be ignored.\n",
		             val.value.scalar);
		  break;
	    }
      }

      put_integer_value(callh, count_bits_in_expr(expr_arg, search));

      return 0;
}

/* Count the number of ones in the expression. */
static PLI_INT32 count_ones_in_expr(vpiHandle expr_arg)
{
      s_vpi_value val;
      PLI_INT32 result;
      PLI_INT32 size = vpi_get(vpiSize, expr_arg);
      assert(size > 0);

      val.format = vpiVectorVal;
      vpi_get_value(expr_arg, &val);

      result = 0;
      size = (size + 31) / 32;
      for (unsigned lp = 0; lp < (unsigned)size; ++lp) {
            PLI_UINT32 ones = ~val.value.vector[lp].bval &
                               val.value.vector[lp].aval;
	    while (ones) {
		  if (ones & 0x1) ++result;
		  ones >>= 1;
	    }
      }

      return result;
}

static PLI_INT32 countones_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle expr_arg = vpi_scan(argv);
      (void)name;  /* Parameter is not used. */

      vpi_free_object(argv);

      put_integer_value(callh, count_ones_in_expr(expr_arg));

      return 0;
}

/* Check to see if the expression is onehot. */
static PLI_INT32 is_onehot(vpiHandle expr_arg, unsigned zero_is_okay)
{
      s_vpi_value val;
      unsigned found_a_one;
      PLI_INT32 size = vpi_get(vpiSize, expr_arg);
      assert(size > 0);

      val.format = vpiVectorVal;
      vpi_get_value(expr_arg, &val);

      found_a_one = 0;
      size = (size + 31) / 32;
      for (unsigned lp = 0; lp < (unsigned)size; ++lp) {
            PLI_UINT32 ones = ~val.value.vector[lp].bval &
                               val.value.vector[lp].aval;
	    while (ones) {
		  if (ones & 0x1) {
			if (found_a_one) return vpi0;
			found_a_one = 1;
		  }
		  ones >>= 1;
	    }
      }

      if (found_a_one) return vpi1;
      else if (zero_is_okay) return vpi1;
      return vpi0;
}

static PLI_INT32 onehot_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle expr_arg = vpi_scan(argv);
      (void)name;  /* Parameter is not used. */

      vpi_free_object(argv);

      put_scalar_value(callh, is_onehot(expr_arg, 0));

      return 0;
}

static PLI_INT32 onehot0_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle expr_arg = vpi_scan(argv);
      (void)name;  /* Parameter is not used. */

      vpi_free_object(argv);

      put_scalar_value(callh, is_onehot(expr_arg, 1));

      return 0;
}

/* Check to see if the expression has an undefined value. */
static PLI_INT32 is_unknown(vpiHandle expr_arg)
{
      s_vpi_value val;
      PLI_INT32 size = vpi_get(vpiSize, expr_arg);
      assert(size > 0);

      val.format = vpiVectorVal;
      vpi_get_value(expr_arg, &val);

      size = (size + 31) / 32;
      for (unsigned lp = 0; lp < (unsigned)size; ++lp) {
            if (val.value.vector[lp].bval) return vpi1;
      }

      return vpi0;
}

static PLI_INT32 isunknown_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle expr_arg = vpi_scan(argv);
      (void)name;  /* Parameter is not used. */

      vpi_free_object(argv);

      put_scalar_value(callh, is_unknown(expr_arg));

      return 0;
}

static PLI_INT32 bit_vec_sizetf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      (void)name;  /* Parameter is not used. */

      return 1;
}

/*
 * Register the functions with Verilog.
 */
void v2009_bitvec_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.calltf      = countbits_calltf;
      tf_data.compiletf   = countbits_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$countbits";
      tf_data.user_data   = 0;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.calltf      = countones_calltf;
      tf_data.compiletf   = sys_one_numeric_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$countones";
      tf_data.user_data   = "$countones";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSizedFunc;
      tf_data.calltf      = onehot_calltf;
      tf_data.compiletf   = sys_one_numeric_arg_compiletf;
      tf_data.sizetf      = bit_vec_sizetf;
      tf_data.tfname      = "$onehot";
      tf_data.user_data   = "$onehot";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSizedFunc;
      tf_data.calltf      = onehot0_calltf;
      tf_data.compiletf   = sys_one_numeric_arg_compiletf;
      tf_data.sizetf      = bit_vec_sizetf;
      tf_data.tfname      = "$onehot0";
      tf_data.user_data   = "$onehot0";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSizedFunc;
      tf_data.calltf      = isunknown_calltf;
      tf_data.compiletf   = sys_one_numeric_arg_compiletf;
      tf_data.sizetf      = bit_vec_sizetf;
      tf_data.tfname      = "$isunknown";
      tf_data.user_data   = "$isunknown";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
