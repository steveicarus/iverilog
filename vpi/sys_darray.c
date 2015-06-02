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
static PLI_INT32 dobject_size_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, arg;

      argv = vpi_iterate(vpiArgument, callh);
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a dynamic array, queue or string "
	               "argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      arg =  vpi_scan(argv);  /* This should never be zero. */
      assert(arg);

	/* The argument must be a dynamic array, queue or string. */
      switch (vpi_get(vpiType, arg)) {
	case vpiStringVar:
	    break;
	case vpiArrayVar:
	    switch(vpi_get(vpiArrayType, arg)) {
	      case vpiDynamicArray:
	      case vpiQueueArray:
		  break;
	      default:
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s argument must be a dynamic array, queue or "
		             "string.\n", name);
		  vpi_control(vpiFinish, 1);
	    }
	    break;
	default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s argument must be a dynamic array, queue or string, "
	               "given a %s.\n", name, vpi_get_str(vpiType, arg));
	    vpi_control(vpiFinish, 1);
      }

      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s has too many arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    vpi_free_object(argv);
      }

      return 0;
}

static PLI_INT32 dobject_size_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg = vpi_scan(argv);

      (void)name; /* Parameter is not used. */

      vpi_free_object(argv);

      s_vpi_value value;
      value.format = vpiIntVal;
      value.value.integer = vpi_get(vpiSize, arg);

      vpi_put_value(callh, &value, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 to_from_vec_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, arg;

      argv = vpi_iterate(vpiArgument, callh);
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument must be a dynamic array. */
      arg =  vpi_scan(argv);  /* This should never be zero. */
      assert(arg);
      if (vpi_get(vpiType, arg) != vpiArrayVar) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s first argument must be a dynamic array, "
	               "given a %s.\n", name, vpi_get_str(vpiType, arg));
	    vpi_control(vpiFinish, 1);
      }
      if (vpi_get(vpiArrayType, arg) != vpiDynamicArray) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s first argument must be a dynamic array.\n", name);
	    vpi_control(vpiFinish, 1);
      }
// HERE: Still need to verify that this is not a real or string array.
//       That will require adding TypeSpec support to the VPI.

	/* The second argument must be a net, reg or bit variable. */
      arg =  vpi_scan(argv);
      if (arg == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second argument.\n", name);
	    vpi_control(vpiFinish, 1);
      }
      switch(vpi_get(vpiType, arg)) {
	case vpiNet:
	case vpiReg:
	case vpiBitVar:
	case vpiIntegerVar:
	case vpiConstant:
	    break;
	default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s second argument must be a logical net or "
	               "variable.\n", name);
	    vpi_control(vpiFinish, 1);
      }

      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s has too many arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    vpi_free_object(argv);
      }

      return 0;
}

static const size_t BPW = 8 * sizeof(PLI_INT32);
static const size_t BPWM1 = 8 * sizeof(PLI_INT32) - 1;

static PLI_INT32 to_vec_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle darr = vpi_scan(argv);
      vpiHandle vec = vpi_scan(argv);

      vpi_free_object(argv);

	/* Calculate and check the basic array and vector information. */
      int darr_size = vpi_get(vpiSize, darr);
      int darr_word_size = vpi_get(vpiSize, vpi_handle_by_index(darr, 0));
      assert(darr_word_size > 0);
      int darr_bit_size = darr_size * darr_word_size;
      int vec_size = vpi_get(vpiSize, vec);

      if (darr_size <= 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s cannot cast an empty dynamic array.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      if (darr_bit_size != vec_size) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s dynamic array and vector size do not match "
	               "(%d != %d).\n", name, darr_bit_size, vec_size);
            vpi_control(vpiFinish, 1);
            return 0;
      }

	/* Calculate the number of words needed to hold the dynamic array
	 * bits and allocate enough space for them. */
      size_t vec_words = (darr_bit_size + BPWM1) / BPW;
      s_vpi_vecval *vec_val = calloc(vec_words, sizeof(s_vpi_vecval));
      s_vpi_vecval *vec_ptr = vec_val;

	/* The number of words in each array element. */
      unsigned darr_words = (darr_word_size + BPWM1) / BPW;

	/* The offset in bits into the current vector value. */
      unsigned offset = 0;

	/* We want to get each array word as a vector. */
      s_vpi_value darr_val;
      darr_val.format = vpiVectorVal;

	/* We have to reverse the order of the dynamic array words. */
      for (PLI_INT32 i = darr_size - 1; i >= 0; --i) {
	      /* Get the vector value for the current array word. */
            vpiHandle darr_word = vpi_handle_by_index(darr, i);
            vpi_get_value(darr_word, &darr_val);
            assert(darr_val.format == vpiVectorVal);
	      /* The number of bits to copy for this array word. */
            unsigned bits_to_copy = (unsigned)darr_word_size;

	      /* Copy the current array bits to the vector and update the
	       * the vector pointer accordingly. */
            for (unsigned j = 0; j < darr_words; ++j) {
		    /* Get the current array part and copy it into the
		     * correct place. */
		  PLI_UINT32 aval = darr_val.value.vector->aval;
		  PLI_UINT32 bval = darr_val.value.vector->bval;
		  assert(offset < BPW);
		  vec_ptr->aval |= (aval << offset);
		  vec_ptr->bval |= (bval << offset);

		    /* Calculate the new offset into the vector. */
		  offset += (bits_to_copy > BPW) ? BPW : bits_to_copy;

		    /* If the new offset is past the end of the vector part
		     * then the next vector part also needs to be used. */
		  if (offset >= BPW) {
			++vec_ptr;

			  /* Does the current array part also go into the
			   * next vector part? */
			if (offset > BPW) {
				/* This code has not been tested since the
				 * current implementation only supports dynamic
				 * array elements of size 8, 16, 32 or 64 bits
				 * so currently this code is never run. For
				 * now assert since it has not been checked. */
			      assert(0);

				/* Copy the rest of the array part that did not
				 * fit in the previous vector part to the next
				 * vector part. */
			      offset -= BPW;
			      vec_ptr->aval |= (aval >> (darr_word_size -
			                                 offset));
			      vec_ptr->bval |= (bval >> (darr_word_size -
			                                 offset));
			  /* Start at the beginning of the next vector part. */
			} else {
			      offset = 0;
			}
		  }

		    /* Advance to the next part of the array. */
		  bits_to_copy -= BPW;
		  darr_val.value.vector++;
            }
      }

	/* Put the result to the vector and free the allocated space. */
      darr_val.format = vpiVectorVal;
      darr_val.value.vector = vec_val;
      vpi_put_value(vec, &darr_val, 0, vpiNoDelay);

      free(vec_val);

      return 0;
}

static PLI_INT32 from_vec_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle darr = vpi_scan(argv);
      vpiHandle vec = vpi_scan(argv);

      vpi_free_object(argv);

	/* Calculate and check the basic array and vector information. */
      int darr_size = vpi_get(vpiSize, darr);
      int darr_word_size = vpi_get(vpiSize, vpi_handle_by_index(darr, 0));
      assert(darr_word_size > 0);
      int darr_bit_size = darr_size * darr_word_size;

      int vec_size = vpi_get(vpiSize, vec);

      if (vec_size <= 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s cannot cast an empty vector array.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      if (darr_bit_size != vec_size) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s dynamic array and vector size do not match "
	               "(%d != %d).\n", name, darr_bit_size, vec_size);
            vpi_control(vpiFinish, 1);
            return 0;
      }

	/* Calculate the number of words needed to hold the dynamic array
	 * word bits and allocate enough space for them. */
      size_t darr_words = (darr_word_size + BPWM1) / BPW;
      s_vpi_vecval *darr_val = calloc(darr_words, sizeof(s_vpi_vecval));

	/* Get the vector value. */
      s_vpi_value vec_val;
      vec_val.format = vpiVectorVal;
      vpi_get_value(vec, &vec_val);

	/* The offset in bits into the vector value. */
      unsigned offset = 0;

	/* We have to reverse the order of the dynamic array words. */
      for (int i = darr_size - 1; i >= 0; --i) {
            unsigned bits_to_copy = darr_word_size;
	    s_vpi_vecval *darr_ptr = darr_val;

	      /* Copy some of the vector bits to the current array word. */
            while (bits_to_copy > 0) {
		  unsigned copied_bits = (bits_to_copy > BPW) ? BPW :
		                                                bits_to_copy;
		    /* Start with the current vector part. */
		  PLI_UINT32 aval = vec_val.value.vector[offset / BPW].aval;
		  PLI_UINT32 bval = vec_val.value.vector[offset / BPW].bval;

		    /* If this isn't aligned then we may need to get bits
		     * from the next part as well. */
		  unsigned rem_bits = offset % BPW;
		  if (rem_bits) {
			aval >>= rem_bits;
			aval |= vec_val.value.vector[offset / BPW + 1].aval <<
			        (BPW - rem_bits);
			bval >>= rem_bits;
			bval |= vec_val.value.vector[offset / BPW + 1].bval <<
			        (BPW - rem_bits);
		  }

		    /* Advance to the next part of the array and vector. */
		  darr_ptr->aval = aval;
		  darr_ptr->bval = bval;
		  darr_ptr++;
		  offset += copied_bits;
		  bits_to_copy -= copied_bits;
            }

	      /* Put part of the vector to the current dynamic array word. */
	    s_vpi_value result;
            result.format = vpiVectorVal;
            result.value.vector = darr_val;
            vpiHandle darr_word = vpi_handle_by_index(darr, i);
            vpi_put_value(darr_word, &result, 0, vpiNoDelay);
      }

      free(darr_val);

      return 0;
}

void sys_darray_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$size";
      tf_data.calltf    = dobject_size_calltf;
      tf_data.compiletf = dobject_size_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$size";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.sysfunctype = 0;
      tf_data.tfname    = "$ivl_darray_method$to_vec";
      tf_data.calltf    = to_vec_calltf;
      tf_data.compiletf = to_from_vec_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ivl_darray_method$to_vec";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.sysfunctype = 0;
      tf_data.tfname    = "$ivl_darray_method$from_vec";
      tf_data.calltf    = from_vec_calltf;
      tf_data.compiletf = to_from_vec_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ivl_darray_method$from_vec";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

}
