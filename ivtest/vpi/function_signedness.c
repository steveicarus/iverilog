/* Read the signedness reported by vpiFuncType for a packed function. */

#include <assert.h>
#include <vpi_user.h>

static PLI_INT32 function_is_signed(PLI_BYTE8*data)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      s_vpi_value value;
      (void)data;

      vpiHandle func = vpi_scan(args);
      assert(func);
      assert(vpi_get(vpiType, func) == vpiFunction);
      PLI_INT32 type = vpi_get(vpiFuncType, func);
      assert(type == vpiSizedFunc || type == vpiSizedSignedFunc);

      value.format = vpiIntVal;
      value.value.integer = type == vpiSizedSignedFunc;
      vpi_put_value(call, &value, 0, vpiNoDelay);
      vpi_free_object(args);
      return 0;
}

static void register_functions(void)
{
      s_vpi_systf_data tf = {0};
      tf.type = vpiSysFunc;
      tf.sysfunctype = vpiIntFunc;
      tf.tfname = "$function_is_signed";
      tf.calltf = function_is_signed;
      vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {register_functions, 0};
