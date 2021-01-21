/*
 * Copyright (c) 2010-2021 Stephen Williams (steve@icarus.com)
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

# include "vpi_config.h"
# include "sv_vpi_user.h"
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

/*
 * The compiletf routine for the enumeration next() and prev() methods.
 */
static PLI_INT32 ivl_enum_method_next_prev_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg_enum, arg_var, arg_count;

	/* These routines must have arguments. */
      if (argv == 0) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("No arguments given for enum method %s().\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check for the enumeration type argument. */
      arg_enum = vpi_scan(argv);
      if (arg_enum == 0) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("No enumeration type argument given for enum "
	               "method %s().\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      if (vpi_get(vpiType, arg_enum) != vpiEnumTypespec) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("The first argument to enum method %s() must be an "
	               "enumeration type, found a %s.\n", name,
	               vpi_get_str(vpiType, arg_enum));
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Check for the enumeration variable. */
      arg_var = vpi_scan(argv);
      if (arg_var == 0) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("No enumeration variable argument given for enum "
	               "method %s().\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      switch(vpi_get(vpiType, arg_var)) {
	case vpiBitVar:
	case vpiByteVar:
	case vpiIntegerVar:
	case vpiIntVar:
	case vpiLongIntVar:
	case vpiReg:
	case vpiShortIntVar:
	    break;
	default:
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("The second argument to enum method %s() must be an "
	               "enumeration variable, found a %s.\n", name,
	               vpi_get_str(vpiType, arg_var));
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* We can have an optional numeric count value. */
      arg_count = vpi_scan(argv);
      if (arg_count) {
	    switch(vpi_get(vpiType, arg_count)) {
		case vpiBitVar:
		case vpiByteVar:
		case vpiIntegerVar:
		case vpiIntVar:
		case vpiLongIntVar:
		case vpiMemoryWord:
		case vpiNet:
		case vpiPartSelect:
		case vpiRealVar:
		case vpiReg:
		case vpiShortIntVar:
		case vpiTimeVar:
		  break;
		case vpiConstant:
		case vpiParameter:
		  if (vpi_get(vpiConstType, arg_count) != vpiStringConst) break;
		  // fallthrough
		default:
		  vpi_printf("%s:%d: compiler error: ",
		             vpi_get_str(vpiFile, sys),
		             (int) vpi_get(vpiLineNo,sys));
		  vpi_printf("The second argument to enum method %s() must be "
		             "numeric, found a %s.\n", name,
		             vpi_get_str(vpiType, arg_count));
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
      }

	/* If we have a count argument then check to see if any extra
	 * arguments were given. */
      if (arg_count && (vpi_scan(argv) != 0)) {
	    vpi_free_object(argv);
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("Extra argument(s) given to enum method %s().\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* The method return value and the enum variable must be the
	 * same size */
      if (vpi_get(vpiSize, sys) != vpi_get(vpiSize, arg_var)) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("The enum method %s() return width (%d) must match "
	               "the enum variable width (%d).\n", name,
	               (int) vpi_get(vpiSize, sys),
	               (int) vpi_get(vpiSize, arg_var));
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      return 0;
}

/*
 * Compare it two values are equal to each other.
 */
static int compare_value_eequal(s_vpi_value*ref1, s_vpi_value*ref2,
                                PLI_INT32 wid)
{
	/* Two integer values are easy. */
      if (ref1->format == vpiIntVal && ref2->format == vpiIntVal)
	    return ref1->value.integer == ref2->value.integer;

	/* For two vectors compare them word by word. */
      if (ref1->format == vpiVectorVal && ref2->format == vpiVectorVal) {
	    PLI_INT32 words = (wid-1)/32 + 1;
	    PLI_INT32 idx;

	    for (idx = 0 ; idx < words ; idx += 1) {
		  if (ref1->value.vector[idx].aval !=
		      ref2->value.vector[idx].aval) return 0;
		  if (ref1->value.vector[idx].bval !=
		      ref2->value.vector[idx].bval) return 0;
	    }
	    return 1;
      }

	/* Swap the order so the code below can be used. */
      if (ref1->format == vpiVectorVal && ref2->format == vpiIntVal) {
	    s_vpi_value*tmp = ref1;
	    ref1 = ref2;
	    ref2 = tmp;
      }

	/* Compare an integer to a vector. */
      if (ref1->format == vpiIntVal && ref2->format == vpiVectorVal) {
	    PLI_INT32 aval = ref1->value.integer;
	    PLI_INT32 words = (wid-1)/32 + 1;
	    PLI_INT32 idx;

	    for (idx = 0 ; idx < words ; idx += 1) {
		  if (aval != ref2->value.vector[idx].aval) return 0;
		  if (ref2->value.vector[idx].bval) return 0;

		  aval = 0;
	    }
	    return 1;
      }

	/* Unsupported types. */
      vpi_printf("XXXX formats are: %d vs %d\n", ref1->format, ref2->format);
      assert(0);
      return 0;
}

/*
 * If the current value is not found in the enumeration. Then we need to
 * return X/0 for the next()/prev() value.
 */
static void fill_handle_with_init(vpiHandle arg, int is_two_state)
{
      s_vpi_value val;
      PLI_INT32 words = ((vpi_get(vpiSize, arg) - 1) / 32) + 1;
      PLI_INT32 idx;
      p_vpi_vecval val_ptr = (p_vpi_vecval) malloc(words*sizeof(s_vpi_vecval));
      PLI_INT32 fill = (is_two_state) ? 0x0 : 0xffffffff;

      assert(val_ptr);

	/* Fill the vector with X. */
      for (idx=0; idx < words; idx += 1) {
	    val_ptr[idx].aval = fill;
	    val_ptr[idx].bval = fill;
      }

	/* Put the vector to the argument. */
      val.format = vpiVectorVal;
      val.value.vector = val_ptr;
      vpi_put_value(arg, &val, 0, vpiNoDelay);
      free(val_ptr);
}

/*
 * Implement the next()/prev() enumeration methods.
 */
static PLI_INT32 ivl_enum_method_next_prev_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg_enum = vpi_scan(argv);
      vpiHandle arg_var = vpi_scan(argv);
      vpiHandle arg_count = vpi_scan(argv);

      vpiHandle enum_list;
      vpiHandle cur;
      PLI_INT32 var_width = vpi_get(vpiSize, arg_var);
      PLI_UINT32 enum_size = vpi_get(vpiSize, arg_enum);
      PLI_UINT32 count = 1;
      PLI_UINT32 loc = 0;

      s_vpi_value cur_val, var_val;

	/* Get the count value if one was given. */
      if (arg_count) {
	    s_vpi_value count_val;

	      /* Get the count value. */
	    count_val.format = vpiIntVal;
	    vpi_get_value(arg_count, &count_val);
	    count = count_val.value.integer;
	      /* Remove any multiple loops around the enumeration. */
	    count %= enum_size;
	      /* Free the argument iterator. */
	    vpi_free_object(argv);
      }

	/* Get the current value. */
      var_val.format = vpiObjTypeVal;
      vpi_get_value(arg_var, &var_val);

	/* If the count is zero then just return the current value. */
      if (count == 0) {
	    vpi_put_value(sys, &var_val, 0, vpiNoDelay);
	    return 0;
      }

	/* If the current value is a vector, then make a safe copy of
	   it so that other vpi_get_value() calls don't trash the value. */
      if (var_val.format == vpiVectorVal) {
	    PLI_INT32 idx;
	    PLI_INT32 words = (var_width - 1)/32 + 1;
	    p_vpi_vecval nvec = malloc(words*sizeof(s_vpi_vecval));
	    for (idx = 0 ; idx < words ; idx += 1) {
		  nvec[idx].aval = var_val.value.vector[idx].aval;
		  nvec[idx].bval = var_val.value.vector[idx].bval;
	    }
	    var_val.value.vector = nvec;
      }

	/* Search for the current value in the enumeration list. */
      enum_list = vpi_iterate(vpiEnumConst, arg_enum);
      assert(enum_list);
      do {
	    cur = vpi_scan(enum_list);
	    if (cur == 0) break;

	    cur_val.format = vpiObjTypeVal;
	    vpi_get_value(cur, &cur_val);
	    assert(var_width == vpi_get(vpiSize, cur));
	    loc += 1;
      } while (! compare_value_eequal(&cur_val, &var_val, var_width));

	/* If the variable is a vector then free the copy we created above. */
      if (var_val.format == vpiVectorVal) free(var_val.value.vector);

	/* The current value was not found in the list so return X/0. */
      if (cur == 0) {
	      /* This only works correctly since we don't really define the
	       * the correct base typespec. */
	    int is_two_state = vpi_get(vpiBaseTypespec, arg_enum) != vpiReg;
	    fill_handle_with_init(sys, is_two_state);
      } else {
	    if (strcmp(name, "$ivl_enum_method$next") == 0) {
		    /* Check to see if we need to wrap to the beginning. */
		  if (loc + count > enum_size) {
			  /* Free the current iterator before creating a new
			   * one that is at the beginning of the list. */
			vpi_free_object(enum_list);
			enum_list = vpi_iterate(vpiEnumConst, arg_enum);
			assert(enum_list);
			count -= (enum_size - loc);
		  }
	    } else if (strcmp(name, "$ivl_enum_method$prev") == 0) {
		    /* Because of wrapping the count could be either before
		     * or after the current element. */
		  if (loc <= count ) {
			  /* The element we want is after the current
			   * element. */
			count = enum_size - count;
		  } else {
			  /* The element we want is before the current
			   * element (at the beginning of the list). Free
			   * the current iterator before creating a new
			   * one that is at the beginning of the list. */
			vpi_free_object(enum_list);
			enum_list = vpi_iterate(vpiEnumConst, arg_enum);
			assert(enum_list);
			count = loc - count;
		  }
	    } else assert(0);

	      /* Find the correct element. */
	    while (count) {
		  cur = vpi_scan(enum_list);
		  count -= 1;
	    }
	    assert(cur != 0);

	      /* Free the iterator. */
	    vpi_free_object(enum_list);

	      /* Get the value and return it. */
	    cur_val.format = vpiObjTypeVal;
	    vpi_get_value(cur, &cur_val);
	    vpi_put_value(sys, &cur_val, 0, vpiNoDelay);
      }

      return 0;
}

/*
 * The compiletf routine for the enumeration name() method.
 */
static PLI_INT32 ivl_enum_method_name_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg_enum, arg_var;

	/* This routine must have arguments. */
      if (argv == 0) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("No arguments given for enum method %s().\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check for the enumeration type argument. */
      arg_enum = vpi_scan(argv);
      if (arg_enum == 0) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("No enumeration type argument given for enum "
	               "method %s().\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      if (vpi_get(vpiType, arg_enum) != vpiEnumTypespec) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("The first argument to enum method %s() must be an "
	               "enumeration type, found a %s.\n", name,
	               vpi_get_str(vpiType, arg_enum));
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Check for the enumeration variable. */
      arg_var = vpi_scan(argv);
      if (arg_var == 0) {
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("No enumeration variable argument given for enum "
	               "method %s().\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      switch(vpi_get(vpiType, arg_var)) {
	case vpiBitVar:
	case vpiByteVar:
	case vpiIntegerVar:
	case vpiIntVar:
	case vpiLongIntVar:
	case vpiReg:
	case vpiShortIntVar:
	    break;
	default:
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("The second argument to enum method %s() must be an "
	               "enumeration variable, found a %s.\n", name,
	               vpi_get_str(vpiType, arg_var));
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Only two arguments are supported. */
      if (vpi_scan(argv) != 0) {
	    vpi_free_object(argv);
	    vpi_printf("%s:%d: compiler error: ", vpi_get_str(vpiFile, sys),
	               (int) vpi_get(vpiLineNo,sys));
	    vpi_printf("Extra argument(s) given to enum method %s().\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      return 0;
}

/*
 * Implement the name() enumeration method.
 */
static PLI_INT32 ivl_enum_method_name_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg_enum = vpi_scan(argv);
      vpiHandle arg_var = vpi_scan(argv);

      vpiHandle enum_list;
      vpiHandle cur;
      PLI_INT32 var_width = vpi_get(vpiSize, arg_var);

      s_vpi_value cur_val, var_val;

      (void)name; /* Parameter is not used. */

	/* Free the argument iterator. */
      vpi_free_object(argv);

	/* Get the current value. */
      var_val.format = vpiObjTypeVal;
      vpi_get_value(arg_var, &var_val);

	/* If the current value is a vector, then make a safe copy of
	   it so that other vpi_get_value() calls don't trash the value. */
      if (var_val.format == vpiVectorVal) {
	    PLI_INT32 idx;
	    PLI_INT32 words = (var_width - 1)/32 + 1;
	    p_vpi_vecval nvec = malloc(words*sizeof(s_vpi_vecval));
	    for (idx = 0 ; idx < words ; idx += 1) {
		  nvec[idx].aval = var_val.value.vector[idx].aval;
		  nvec[idx].bval = var_val.value.vector[idx].bval;
	    }
	    var_val.value.vector = nvec;
      }

	/* Search for the current value in the enumeration list. */
      enum_list = vpi_iterate(vpiEnumConst, arg_enum);
      assert(enum_list);
      do {
	    cur = vpi_scan(enum_list);
	    if (cur == 0) break;

	    cur_val.format = vpiObjTypeVal;
	    vpi_get_value(cur, &cur_val);
	    assert(var_width == vpi_get(vpiSize, cur));
      } while (! compare_value_eequal(&cur_val, &var_val, var_width));

	/* If the variable is a vector then free the copy we created above. */
      if (var_val.format == vpiVectorVal) free(var_val.value.vector);

	/* The current value was not found in the list so return an empty
	 * string. */
      cur_val.format = vpiStringVal;
      if (cur == 0) {
	    cur_val.value.str = "";
      } else {
	    cur_val.value.str = vpi_get_str(vpiName, cur);

	      /* Free the iterator. */
	    vpi_free_object(enum_list);
      }

	/* Return the appropriate string value. */
      vpi_put_value(sys, &cur_val, 0, vpiNoDelay);
      return 0;
}

void v2009_enum_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiOtherFunc;
      tf_data.calltf      = ivl_enum_method_name_calltf;
      tf_data.compiletf   = ivl_enum_method_name_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$ivl_enum_method$name";
      tf_data.user_data   = "$ivl_enum_method$name";
      res = vpi_register_systf(&tf_data);
	/* This is not a user defined system function so hide it. */
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiOtherFunc;
      tf_data.calltf      = ivl_enum_method_next_prev_calltf;
      tf_data.compiletf   = ivl_enum_method_next_prev_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$ivl_enum_method$next";
      tf_data.user_data   = "$ivl_enum_method$next";
      res = vpi_register_systf(&tf_data);
	/* This is not a user defined system function so hide it. */
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiOtherFunc;
      tf_data.calltf      = ivl_enum_method_next_prev_calltf;
      tf_data.compiletf   = ivl_enum_method_next_prev_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$ivl_enum_method$prev";
      tf_data.user_data   = "$ivl_enum_method$prev";
      res = vpi_register_systf(&tf_data);
	/* This is not a user defined system function so hide it. */
      vpip_make_systf_system_defined(res);
}
