/*
 * Check that vpiName and vpiDefName preserve their respective names when both
 * contain double quotes that require escaping in the generated VVP code.
 */

#include <string.h>
#include <vpi_user.h>

static PLI_INT32 check_scope_names(PLI_BYTE8*data)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      vpiHandle scope = vpi_scan(args);
      s_vpi_value value;
      (void)data;

      value.format = vpiIntVal;
      value.value.integer =
	    strcmp(vpi_get_str(vpiName, scope), "i\"m") == 0 &&
	    strcmp(vpi_get_str(vpiDefName, scope), "M\"name") == 0;
      vpi_put_value(call, &value, 0, vpiNoDelay);
      vpi_free_object(args);
      return 0;
}

static void register_functions(void)
{
      s_vpi_systf_data tf = {0};
      tf.type = vpiSysFunc;
      tf.sysfunctype = vpiIntFunc;
      tf.tfname = "$check_scope_names";
      tf.calltf = check_scope_names;
      vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {register_functions, 0};
