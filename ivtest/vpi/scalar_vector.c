/*
 * $check_sv(signal, is_vector, size): returns 0 if the checks pass, else 1.
 * Check that vpiVector, vpiScalar and
 * vpiSize of the signal argument match the expectation.
 * is_vector is 1 for an object declared with a packed range (even if the
 * range has a single element), 0 for a plain scalar and 2 for an object
 * that is neither, such as a real.
 */

#include <stdio.h>
#include <vpi_user.h>

static void set_result(vpiHandle call, int failed)
{
      s_vpi_value value;
      value.format = vpiIntVal;
      value.value.integer = failed;
      vpi_put_value(call, &value, 0, vpiNoDelay);
}

static int check_obj(vpiHandle obj, const char*name, int is_vector, int size)
{
      int is_scalar = is_vector == 0;
      is_vector = is_vector == 1;
      int failed = vpi_get(vpiVector, obj) != is_vector ||
	  vpi_get(vpiScalar, obj) != is_scalar ||
	  vpi_get(vpiSize, obj) != size;
      if (failed)
	    vpi_printf("FAILED: %s: vpiVector=%d (exp %d) vpiScalar=%d (exp %d)"
		       " vpiSize=%d (exp %d)\n", name,
		       (int)vpi_get(vpiVector, obj), is_vector,
		       (int)vpi_get(vpiScalar, obj), is_scalar,
		       (int)vpi_get(vpiSize, obj), size);
      return failed;
}

static PLI_INT32 check_sv(PLI_BYTE8*data)
{
      (void)data;
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      s_vpi_value value;

      vpiHandle obj = vpi_scan(args);
      const char*name = vpi_get_str(vpiFullName, obj);

      value.format = vpiIntVal;
      vpi_get_value(vpi_scan(args), &value);
      int is_vector = value.value.integer;

      vpi_get_value(vpi_scan(args), &value);
      int size = value.value.integer;
      vpi_free_object(args);

      int failed = check_obj(obj, name, is_vector, size);
      set_result(call, failed);
      return 0;
}

/*
 * $check_sv_words(array, is_vector, size): like $check_sv, but applied
 * to every word returned by iterating vpiMemoryWord on the array.
 */
static PLI_INT32 check_sv_words(PLI_BYTE8*data)
{
      (void)data;
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      s_vpi_value value;

      vpiHandle arr = vpi_scan(args);
      const char*name = vpi_get_str(vpiFullName, arr);

      value.format = vpiIntVal;
      vpi_get_value(vpi_scan(args), &value);
      int is_vector = value.value.integer;

      vpi_get_value(vpi_scan(args), &value);
      int size = value.value.integer;
      vpi_free_object(args);

      int failed = 0;
      vpiHandle words = vpi_iterate(vpiMemoryWord, arr);
      vpiHandle word;
      if (!words) {
	    vpi_printf("FAILED: %s: no words to iterate\n", name);
	    failed = 1;
      }
      while (words && (word = vpi_scan(words)))
	    failed |= check_obj(word, name, is_vector, size);
      set_result(call, failed);
      return 0;
}

static void register_tasks(void)
{
      s_vpi_systf_data tf = {0};
      tf.type = vpiSysFunc;
      tf.sysfunctype = vpiIntFunc;
      tf.tfname = "$check_sv";
      tf.calltf = check_sv;
      vpi_register_systf(&tf);

      tf.tfname = "$check_sv_words";
      tf.calltf = check_sv_words;
      vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {register_tasks, 0};
