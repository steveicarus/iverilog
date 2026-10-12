/* Write array elements using explicit VPI value formats. */

#include <assert.h>
#include <vpi_user.h>

static PLI_INT32 put_word(PLI_BYTE8*data)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      s_vpi_value value;

      value.format = vpiStringVal;
      vpi_get_value(vpi_scan(args), &value);
      vpiHandle array = vpi_handle_by_name(value.value.str, 0);
      assert(array);

      value.format = vpiIntVal;
      vpi_get_value(vpi_scan(args), &value);
      vpiHandle word = vpi_handle_by_index(array, value.value.integer);
      assert(word);

      value.format = *(PLI_INT32*)data;
      vpi_get_value(vpi_scan(args), &value);
      vpi_put_value(word, &value, 0, vpiNoDelay);
      vpi_free_object(args);
      return 0;
}

static void register_tasks(void)
{
      static struct {
	    const char*name;
	    PLI_INT32 format;
      } tasks[] = {
	    {"$put_array_scalar", vpiScalarVal},
	    {"$put_array_int", vpiIntVal},
	    {"$put_array_vector", vpiVectorVal}
      };

      for (unsigned idx = 0 ; idx < sizeof(tasks)/sizeof(tasks[0]) ; idx += 1) {
	    s_vpi_systf_data tf = {0};
	    tf.type = vpiSysTask;
	    tf.tfname = tasks[idx].name;
	    tf.calltf = put_word;
	    tf.user_data = (PLI_BYTE8*)&tasks[idx].format;
	    vpi_register_systf(&tf);
      }
}

void (*vlog_startup_routines[])(void) = {register_tasks, 0};
