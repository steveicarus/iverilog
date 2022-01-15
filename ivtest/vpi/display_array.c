/*
 * Copyright (c) 2014-2021 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This source code is free software; you can redistribute it
 * and/or modify it in source code form under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include <sv_vpi_user.h>
# include <assert.h>
# include <limits.h>

static PLI_INT32 display_array_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */

      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, array, cell, l_range, r_range;
      s_vpi_value val;

      /* Fetch arguments */
      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      array = vpi_scan(argv);
      assert(array);
      vpi_free_object(argv);

      int array_size = vpi_get(vpiSize, array);
      if(array_size < 0) {
	    vpi_printf("ERROR: Arrays cannot have negative size");
            vpi_control(vpiFinish, 0);
            return 0;
      }

      /* Test range handles */
      l_range = vpi_handle(vpiLeftRange, array);
      r_range = vpi_handle(vpiRightRange, array);
      val.format = vpiIntVal;
      vpi_get_value(l_range, &val);
      int left = val.value.integer;
      vpi_get_value(r_range, &val);
      int right = val.value.integer;
      assert(right - left + 1 == array_size);
      /*vpi_printf("array range: %d to %d\n", left, right);*/

      /* Test accessing cells by index */
      vpi_printf("{ ");
      int i;
      for(i = 0; i < array_size; ++i) {
            cell = vpi_handle_by_index(array, i);
            val.format = vpiObjTypeVal;
            vpi_get_value(cell, &val);

            if(val.format == vpiRealVal)
                vpi_printf("%f", val.value.real);
            else if(val.format == vpiStringVal)
                vpi_printf("%s", val.value.str);
            else { // convenient way to handle all other formats
                val.format = vpiDecStrVal;
                vpi_get_value(cell, &val); // sorry for another vpi call
                vpi_printf("%s", val.value.str);
            }

            if(i != array_size - 1) vpi_printf(", ");
      }
      vpi_printf(" }\n");

      return 0;
}

static PLI_INT32 increase_array_vals_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */

      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv, array, array_iterator, cell;
      PLI_INT32 iter_type;
      s_vpi_value val;

      /* Fetch arguments */
      argv = vpi_iterate(vpiArgument, callh);
      assert(argv);
      array = vpi_scan(argv);
      assert(array);
      vpi_free_object(argv);

      switch(vpi_get(vpiType, array)) {
            case vpiArrayType:
            case vpiRegArray:
                iter_type = vpiReg;
                break;

            case vpiMemory:
                iter_type = vpiMemoryWord;
                break;

            default:
                vpi_printf("sorry: increase_array_vals: missing iterator for "
                            "the given array type\n");
                return 0;
      }

      /* Test accessing cells with iterators */
      array_iterator = vpi_iterate(iter_type, array);
      while((cell = vpi_scan(array_iterator))) {
            /* Test format recognition */
            val.format = vpiObjTypeVal;
            vpi_get_value(cell, &val);

            /* Increase the read value */
            switch(val.format) {
                case vpiIntVal:
                    ++val.value.integer;
                    break;

                case vpiVectorVal:
                    /* Only support a single aval */
                    assert((uint32_t)val.value.vector->aval < UINT_MAX);
                    assert(val.value.vector->bval == 0);
                    ++val.value.vector->aval;
                    break;

                case vpiRealVal:
                    ++val.value.real;
                    break;

                case vpiStringVal:
                    {
                        char*s = val.value.str;
                        while(*s) ++*s++;   // oh yeah, I love C
                    }
                    break;

                default:
                    vpi_printf("sorry: increase_array_vals: format not implemented\n");
                    return 0;
            }

            /* Test data write */
            vpi_put_value(cell, &val, 0, vpiNoDelay);
      }

      return 0;
}


static PLI_INT32 one_arg_array_compiletf(ICARUS_VPI_CONST PLI_BYTE8*user_data)
{
    (void) user_data; /* Parameter is not used. */

    vpiHandle systf_handle, arg_iterator, arg_handle;
    PLI_INT32 arg_type;

    /* obtain a handle to the system task instance */
    systf_handle = vpi_handle(vpiSysTfCall, NULL);
    if (systf_handle == NULL) {
        vpi_printf("ERROR: $display_array failed to obtain systf handle\n");
        vpi_control(vpiFinish,0); /* abort simulation */
        return 0;
    }

    /* obtain handles to system task arguments */
    arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    if (arg_iterator == NULL) {
        vpi_printf("ERROR: $display_array requires exactly 1 argument\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    /* check the type of object in system task arguments */
    arg_handle = vpi_scan(arg_iterator);
    if (vpi_scan(arg_iterator) != NULL) {       /* are there more arguments? */
        vpi_printf("ERROR: $display_array takes only 1 argument\n");
        vpi_free_object(arg_iterator);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    arg_type = vpi_get(vpiType, arg_handle);
    if (arg_type != vpiArrayType && arg_type != vpiRegArray &&
            arg_type != vpiMemory) {
        vpi_printf("%d", arg_type); // TODO remove
        vpi_printf("ERROR: $display_array works only with arrays\n");
        vpi_free_object(arg_iterator);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    return 0;
}

static void test_array_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$display_array";
      tf_data.calltf    = display_array_calltf;
      tf_data.compiletf = one_arg_array_compiletf;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$increase_array_vals";
      tf_data.calltf    = increase_array_vals_calltf;
      tf_data.compiletf = one_arg_array_compiletf;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);
}

/*
 * This is a table of register functions. This table is the external
 * symbol that the simulator looks for when loading this .vpi module.
 */
void (*vlog_startup_routines[])(void) = {
      test_array_register,
      0
};
