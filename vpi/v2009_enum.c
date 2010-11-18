/*
 * Copyright (c) 2010 Stephen Williams (steve@icarus.com)
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

# include "vpi_config.h"
# include "sv_vpi_user.h"
# include  <assert.h>

static void missing_arguments(vpiHandle sys)
{
      vpi_printf("%s:%d: error: Invalid/missing arguments next/prev method\n",
		 vpi_get_str(vpiFile, sys), vpi_get(vpiLineNo,sys));
      vpi_control(vpiFinish, 1);
}

static PLI_INT32 ivl_method_next_prev_compiletf(ICARUS_VPI_CONST PLI_BYTE8*data)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg_enum, arg_item, arg_extra;

      if (argv == 0) {
	    missing_arguments(sys);
	    return 0;
      }

      arg_enum = vpi_scan(argv);
      if (arg_enum == 0) {
	    missing_arguments(sys);
	    return 0;
      }

      arg_item = vpi_scan(argv);
      if (arg_item == 0) {
	    missing_arguments(sys);
	    return 0;
      }

	/* Make sure there are no excess arguments */
      arg_extra = vpi_scan(argv);
      if (arg_extra != 0) {
	    missing_arguments(sys);
	    vpi_free_object(argv);
	    return 0;
      }

	/* The first argument must be an enum typespec */
      if (vpi_get(vpiType, arg_enum) != vpiEnumTypespec) {
	    missing_arguments(sys);
	    return 0;
      }

	/* The return value and input value must be the same size */
      if (vpi_get(vpiSize,sys) != vpi_get(vpiSize,arg_item)) {
	    missing_arguments(sys);
	    return 0;
      }
      return 0;
}

static PLI_INT32 ivl_method_next2_calltf(PLI_BYTE8*data)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg_enum = vpi_scan(argv);
      vpiHandle arg_item = vpi_scan(argv);
      vpiHandle arg_extra = vpi_scan(argv);

      vpiHandle enum_list = 0;
      vpiHandle memb = 0, first_memb = 0;

      s_vpi_value memb_value, item_value;

      assert(arg_extra == 0);

      item_value.format = vpiIntVal;
      vpi_get_value(arg_item, &item_value);

      enum_list = vpi_iterate(vpiMember, arg_enum);
      assert(enum_list);

	/* Search for the current value in the member list. */
      do {
	    memb = vpi_scan(enum_list);
	    if (first_memb == 0) first_memb = memb;
	    if (memb == 0) break;
	    memb_value.format = vpiIntVal;
	    vpi_get_value(memb, &memb_value);
      } while (memb_value.value.integer != item_value.value.integer);

      if (memb != 0);
	    memb = vpi_scan(enum_list);

      if (memb != 0)
	    vpi_free_object(enum_list);

      if (memb == 0) {
	    memb = first_memb;
	    memb_value.format = vpiIntVal;
      }

      vpi_get_value(memb, &memb_value);
      vpi_put_value(sys, &memb_value, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 ivl_method_prev2_calltf(PLI_BYTE8*data)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg_enum = vpi_scan(argv);
      vpiHandle arg_item = vpi_scan(argv);
      vpiHandle arg_extra = vpi_scan(argv);

      vpiHandle enum_list = 0;
      vpiHandle memb = 0, prev = 0, last_memb = 0;

      s_vpi_value memb_value, item_value;

      assert(arg_extra == 0);

      item_value.format = vpiIntVal;
      vpi_get_value(arg_item, &item_value);

      enum_list = vpi_iterate(vpiMember, arg_enum);
      assert(enum_list);

	/* Search for the current value in the member list. */
      do {
	    prev = memb;
	    memb = vpi_scan(enum_list);
	    if (memb == 0) break;
	    last_memb = memb;
	    memb_value.format = vpiIntVal;
	    vpi_get_value(memb, &memb_value);
      } while (memb_value.value.integer != item_value.value.integer);

      while (memb) {
	    last_memb = memb;
	    memb = vpi_scan(enum_list);
      }

      if (prev == 0)
	    prev = last_memb;

      vpi_get_value(prev, &memb_value);
      vpi_put_value(sys, &memb_value, 0, vpiNoDelay);

      return 0;
}

void v2009_enum_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type        = vpiSysFunc;
      tf_data.calltf      = ivl_method_next2_calltf;
      tf_data.compiletf   = ivl_method_next_prev_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$ivl_method$next";
      tf_data.user_data   = "$ivl_method$next";
      vpi_register_systf(&tf_data);

      tf_data.type        = vpiSysFunc;
      tf_data.calltf      = ivl_method_prev2_calltf;
      tf_data.compiletf   = 0;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$ivl_method$prev";
      tf_data.user_data   = "$ivl_method$prev";
      vpi_register_systf(&tf_data);
}
