/*
 * Copyright (c) 2012-2015 Stephen Williams (steve@icarus.com)
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


# include  "sys_priv.h"
# include  <assert.h>
# include  <math.h>
# include  <stdlib.h>
# include  <string.h>

static PLI_INT32 one_darray_arg_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;

      argv = vpi_iterate(vpiArgument, callh);
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a string argument.\n", name);
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

static PLI_INT32 size_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;

      (void)name; /* Parameter is not used. */

      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      arg = vpi_scan(argv);
      assert(arg);
      vpi_free_object(argv);

      int res = vpi_get(vpiSize, arg);

      s_vpi_value value;
      value.format = vpiIntVal;
      value.value.integer = res;

      vpi_put_value(callh, &value, 0, vpiNoDelay);

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

static PLI_INT32 from_vec_compiletf(ICARUS_VPI_CONST PLI_BYTE8*user_data)
{
      (void) user_data; /* Parameter is not used. */

      vpiHandle systf_handle, arg_iterator, arg_handle;
      PLI_INT32 arg_type[2];

      /* obtain a handle to the system task instance */
      systf_handle = vpi_handle(vpiSysTfCall, NULL);
      if (systf_handle == NULL) {
          vpi_printf("ERROR: $ivl_darray_method$from_vec failed to obtain systf handle\n");
          vpi_control(vpiFinish,0); /* abort simulation */
          return 0;
      }

      /* obtain handles to system task arguments */
      arg_iterator = vpi_iterate(vpiArgument, systf_handle);
      if (arg_iterator == NULL) {
          vpi_printf("ERROR: $ivl_darray_method$from_vec requires 2 arguments\n");
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
          vpi_printf("ERROR: $ivl_darray_method$from_vec can only have 2 arguments\n");
          vpi_free_object(arg_iterator);
          vpi_control(vpiFinish, 0);
          return 0;
      }

      if ((arg_type[1] != vpiNet && arg_type[1] != vpiReg && arg_type[1] != vpiBitVar) ||
              (arg_type[0] != vpiRegArray)) {
          vpi_printf("ERROR: $ivl_darray_method$from_vec value arguments must be "\
                      "a net or reg and a dynamic array\n");
          vpi_free_object(arg_iterator);
          vpi_control(vpiFinish, 0);
          return 0;
      }

      return 0;
}

static PLI_INT32 from_vec_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */
      const int PLI_INT32_bits = sizeof(PLI_INT32) * 8;

      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, darr, darr_word, vec;
      s_vpi_value darr_val, vec_val;
      s_vpi_vecval*vector;

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
      if(vec_size <= 0) {
	    vpi_printf("ERROR: Cannot cast empty vector");
            vpi_control(vpiFinish, 0);
            return 0;
      }

      if(vec_size != darr_bit_size) {
	    vpi_printf("ERROR: Dynamic array and vector size do not match");
            vpi_control(vpiFinish, 0);
            return 0;
      }

      /* Conversion part */
      int darr_number = ceil((double)darr_word_bit_size / PLI_INT32_bits);
      vector = calloc(darr_number, sizeof(s_vpi_vecval));

      vec_val.format = vpiVectorVal;
      vpi_get_value(vec, &vec_val);
      s_vpi_vecval*darr_val_ptr;
      int offset = 0;       // offset in bits

      /* We have to reverse the order of the dynamic array, no memcpy here */
      for(int i = darr_length - 1; i >= 0; --i) {
            int bits_to_copy = darr_word_bit_size;
            darr_word = vpi_handle_by_index(darr, i);
            assert(darr_val.value.vector);
            darr_val_ptr = vector;

            while(bits_to_copy > 0) {
                int copied_bits = bits_to_copy > PLI_INT32_bits ? PLI_INT32_bits : bits_to_copy;
                PLI_INT32 aval = vec_val.value.vector[offset / PLI_INT32_bits].aval;
                PLI_INT32 bval = vec_val.value.vector[offset / PLI_INT32_bits].bval;

                if(offset % PLI_INT32_bits != 0) {
                    unsigned int rem_bits = offset % 32;
                    aval >>= rem_bits;
                    aval |= vec_val.value.vector[offset / PLI_INT32_bits + 1].aval << (PLI_INT32_bits - rem_bits);
                    bval >>= rem_bits;
                    bval |= vec_val.value.vector[offset / PLI_INT32_bits + 1].bval << (PLI_INT32_bits - rem_bits);
                }

                offset += copied_bits;
                darr_val_ptr->aval = aval;
                darr_val_ptr->bval = bval;
                darr_val_ptr++;
                bits_to_copy -= copied_bits;
            }

            darr_val.format = vpiVectorVal;
            darr_val.value.vector = vector;
            vpi_put_value(darr_word, &darr_val, 0, vpiNoDelay);
      }

      free(vector);

      return 0;
}

void sys_darray_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$size";
      tf_data.calltf    = size_calltf;
      tf_data.compiletf = one_darray_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$size";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.sysfunctype = 0;
      tf_data.tfname    = "$ivl_darray_method$to_vec";
      tf_data.calltf    = to_vec_calltf;
      tf_data.compiletf = to_vec_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ivl_darray_method$to_vec";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.sysfunctype = 0;
      tf_data.tfname    = "$ivl_darray_method$from_vec";
      tf_data.calltf    = from_vec_calltf;
      tf_data.compiletf = from_vec_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ivl_darray_method$from_vec";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

}
