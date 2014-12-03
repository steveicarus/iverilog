/*
 *  Copyright (C) 2013  Cary R. (cygcary@yahoo.com)
 *  Copyright (C) 2014  Stephen Williams (steve@icarus.com)
 *  Copyright (C) 2014  CERN
 *  @author Maciej Suminski (maciej.suminski@cern.ch)
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

# include  "sys_priv.h"
# include  <assert.h>
# include  <math.h>
# include  <stdlib.h>
# include  <string.h>

static PLI_INT32 one_array_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;

      argv = vpi_iterate(vpiArgument, callh);
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires an array argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      arg =  vpi_scan(argv);
      if (arg == 0) return 0;

      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s has too many arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      return 0;
}

static PLI_INT32 func_not_implemented_compiletf(ICARUS_VPI_CONST PLI_BYTE8* name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);

      vpi_printf("SORRY: %s:%d: function %s() is not currently implemented.\n",
                 vpi_get_str(vpiFile, callh), (int)vpi_get(vpiLineNo, callh),
                 name);
      vpi_control(vpiFinish, 1);
      return 0;
}

static void high_array(const char*name, vpiHandle callh, vpiHandle arg)
{
      s_vpi_value value;
      int array_type;
      int size;

      switch ( (array_type = vpi_get(vpiArrayType, arg)) ) {
	  case vpiDynamicArray:
	  case vpiQueueArray:
	    size = vpi_get(vpiSize, arg);
	    value.format = vpiIntVal;
	    value.value.integer = size - 1;
	    vpi_put_value(callh, &value, 0, vpiNoDelay);
	    break;

	  default:
	    vpi_printf("SORRY: %s:%d: function %s() argument object code is %d\n",
		       vpi_get_str(vpiFile,callh), (int)vpi_get(vpiLineNo, callh),
		       name, array_type);
	    break;
      }
}

static PLI_INT32 high_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;
      int object_code;

      (void)name; /* Parameter is not used. */

      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      arg = vpi_scan(argv);
      assert(arg);
      vpi_free_object(argv);

      switch ( (object_code = vpi_get(vpiType, arg)) ) {
	  case vpiArrayVar:
	    high_array(name, callh, arg);
	    break;

	  default:
	    vpi_printf("SORRY: %s:%d: function %s() argument object code is %d\n",
		       vpi_get_str(vpiFile,callh), (int)vpi_get(vpiLineNo, callh),
		       name, object_code);
	    return 0;
      }

      return 0;
}

static void low_array(const char*name, vpiHandle callh, vpiHandle arg)
{
      s_vpi_value value;
      int array_type;

      switch ( (array_type = vpi_get(vpiArrayType, arg)) ) {
	  case vpiDynamicArray:
	  case vpiQueueArray:
	    value.format = vpiIntVal;
	    value.value.integer = 0;
	    vpi_put_value(callh, &value, 0, vpiNoDelay);
	    break;

	  default:
	    vpi_printf("SORRY: %s:%d: function %s() argument object code is %d\n",
		       vpi_get_str(vpiFile,callh), (int)vpi_get(vpiLineNo, callh),
		       name, array_type);
	    break;
      }
}

static PLI_INT32 low_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;
      int object_code;

      (void)name; /* Parameter is not used. */

      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      arg = vpi_scan(argv);
      assert(arg);
      vpi_free_object(argv);

      switch ( (object_code = vpi_get(vpiType, arg)) ) {
	  case vpiArrayVar:
	    low_array(name, callh, arg);
	    break;

	  default:
	    vpi_printf("SORRY: %s:%d: function %s() argument object code is %d\n",
		       vpi_get_str(vpiFile,callh), (int)vpi_get(vpiLineNo, callh),
		       name, object_code);
	    return 0;
      }

      return 0;
}

static PLI_INT32 to_vec_compiletf(ICARUS_VPI_CONST PLI_BYTE8*user_data)
{
      (void) user_data; /* Parameter is not used. */

      vpiHandle systf_handle, arg_iterator, arg_handle;
      PLI_INT32 arg_type[2];

      /* obtain a handle to the system task instance */
      systf_handle = vpi_handle(vpiSysTfCall, NULL);
      if (systf_handle == NULL) {
          vpi_printf("ERROR: $ivl_darray_method$to_vec failed to obtain systf handle\n");
          vpi_control(vpiFinish,0); /* abort simulation */
          return 0;
      }

      /* obtain handles to system task arguments */
      arg_iterator = vpi_iterate(vpiArgument, systf_handle);
      if (arg_iterator == NULL) {
          vpi_printf("ERROR: $ivl_darray_method$to_vec requires 2 arguments\n");
          vpi_control(vpiFinish, 0);
          return 0;
      }

      /* check the type of object in system task arguments */
      arg_handle = vpi_scan(arg_iterator);
      for(int i = 0; i < 2; ++i) {
          arg_type[i] = vpi_get(vpiType, arg_handle);
          arg_handle = vpi_scan(arg_iterator);
      }

      if (arg_handle != NULL) {       /* are there more arguments? */
          vpi_printf("ERROR: $ivl_darray_method$to_vec can only have 2 arguments\n");
          vpi_free_object(arg_iterator);
          vpi_control(vpiFinish, 0);
          return 0;
      }

      if ((arg_type[0] != vpiRegArray) ||
          (arg_type[1] != vpiNet && arg_type[1] != vpiReg && arg_type[1] != vpiBitVar)) {
          vpi_printf("ERROR: $ivl_darray_method$to_vec value arguments must be a dynamic array and a net or reg\n");
          vpi_free_object(arg_iterator);
          vpi_control(vpiFinish, 0);
          return 0;
      }

      return 0;
}

static PLI_INT32 to_vec_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */
      const unsigned int PLI_INT32_bits = sizeof(PLI_INT32) * 8;

      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, darr, darr_word, vec;
      s_vpi_value darr_val;
      s_vpi_vecval*vec_val;

      /* Fetch arguments */
      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      darr = vpi_scan(argv);
      assert(darr);
      vec = vpi_scan(argv);
      assert(vec);
      vpi_free_object(argv);

      int darr_length = vpi_get(vpiSize, darr);
      darr_word = vpi_handle_by_index(darr, 0);
      int darr_word_bit_size = vpi_get(vpiSize, darr_word);
      int darr_bit_size = darr_length * darr_word_bit_size;

      int vec_size = vpi_get(vpiSize, vec);
      if(darr_length <= 0) {
	    vpi_printf("ERROR: Cannot cast empty dynamic array");
            vpi_control(vpiFinish, 0);
            return 0;
      }

      if(vec_size != darr_bit_size) {
	    vpi_printf("ERROR: Dynamic array and vector size do not match");
            vpi_control(vpiFinish, 0);
            return 0;
      }

      /* Conversion part */
      int vec_number = ceil((double)darr_bit_size / PLI_INT32_bits);
      vec_val = calloc(vec_number, sizeof(s_vpi_vecval));

      int darr_number = ceil((double)darr_word_bit_size / PLI_INT32_bits);

      darr_val.format = vpiVectorVal;
      unsigned int offset = 0;
      s_vpi_vecval*vec_val_ptr = vec_val;
      vec_val_ptr->aval = 0;
      vec_val_ptr->bval = 0;

      /* We have to reverse the order of the dynamic array, no memcpy here */
      for(int i = darr_length - 1; i >= 0; --i) {
            unsigned int bits_to_copy = darr_word_bit_size;
            darr_word = vpi_handle_by_index(darr, i);
            vpi_get_value(darr_word, &darr_val);
            assert(darr_val.value.vector);

            for(int j = 0; j < darr_number; ++j) {
                PLI_INT32 aval = darr_val.value.vector->aval;
                PLI_INT32 bval = darr_val.value.vector->bval;

                if(offset < PLI_INT32_bits) {
                    vec_val_ptr->aval |= (aval << offset);
                    vec_val_ptr->bval |= (bval << offset);
                }

                offset += bits_to_copy > PLI_INT32_bits ? PLI_INT32_bits : bits_to_copy;

                if(offset >= PLI_INT32_bits) {
                    ++vec_val_ptr;
                    vec_val_ptr->aval = 0;
                    vec_val_ptr->bval = 0;

                    // is the current word crossing the s_vpi_vecval boundary?
                    if(offset > PLI_INT32_bits) {
                        // this assert is to warn you, that the following
                        // part could not be tested at the moment of writing
                        // (dynamic arrays work with vectors of 8, 16, 32, 64
                        // bits, so there is no chance that one of the vectors
                        // will cross the s_vpi_vecval boundary)
                        // it *may* work, but it is better to check first
                        assert(0);

                        // copy the remainder that did not fit in the previous s_vpi_vecval
                        offset -= PLI_INT32_bits;
                        vec_val_ptr->aval |= (aval >> (darr_word_bit_size - offset));
                        vec_val_ptr->bval |= (bval >> (darr_word_bit_size - offset));
                    } else {
                        offset = 0;
                    }
                }

                bits_to_copy -= PLI_INT32_bits;
                darr_val.value.vector++;
            }
      }

      darr_val.format = vpiVectorVal;
      darr_val.value.vector = vec_val;
      vpi_put_value(vec, &darr_val, 0, vpiNoDelay);

      free(vec_val);

      return 0;
}

void v2009_array_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysTask;
      tf_data.sysfunctype = 0;
      tf_data.tfname    = "$ivl_darray_method$to_vec";
      tf_data.calltf    = to_vec_calltf;
      tf_data.compiletf = to_vec_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ivl_darray_method$to_vec";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.calltf      = 0;
      tf_data.compiletf   = func_not_implemented_compiletf;;
      tf_data.sizetf      = 0;

	/* These functions are not currently implemented. */
      tf_data.tfname      = "$dimensions";
      tf_data.user_data   = "$dimensions";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$unpacked_dimensions";
      tf_data.user_data   = "$unpacked_dimensions";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$left";
      tf_data.user_data   = "$left";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$right";
      tf_data.user_data   = "$right";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$increment";
      tf_data.user_data   = "$increment";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$high";
      tf_data.user_data   = "$high";
      tf_data.compiletf   = one_array_arg_compiletf;
      tf_data.calltf      = high_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.tfname      = "$low";
      tf_data.user_data   = "$low";
      tf_data.compiletf   = one_array_arg_compiletf;
      tf_data.calltf      = low_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
