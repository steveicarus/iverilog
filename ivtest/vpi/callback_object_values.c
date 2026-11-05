/* Compare callback payloads with explicit reads of their objects. */

#include <vpi_user.h>

static unsigned calls;
static int failed;

static PLI_INT32 changed(p_cb_data cb)
{
      s_vpi_value expected = {0};
      s_vpi_value actual = *cb->value;
      expected.format = actual.format;
      vpi_get_value(cb->obj, &expected);
      calls += 1;
      if ((actual.format == vpiIntVal &&
           actual.value.integer != expected.value.integer) ||
          (actual.format == vpiRealVal &&
           actual.value.real != expected.value.real)) {
            vpi_printf("FAILED: callback payload differs from its object\n");
            failed = 1;
      }
      return 0;
}

static PLI_INT32 watch(PLI_BYTE8*unused)
{
      (void)unused;
      vpiHandle args = vpi_iterate(vpiArgument, vpi_handle(vpiSysTfCall, 0));
      s_vpi_time time = {vpiSuppressTime, 0, 0, 0.0};
      s_vpi_value value = {0};
      s_cb_data cb = {0};
      cb.obj = vpi_scan(args);
      vpi_free_object(args);
      value.format = vpi_get(vpiType, cb.obj) == vpiRealVar ? vpiRealVal : vpiIntVal;
      cb.reason = cbValueChange;
      cb.cb_rtn = changed;
      cb.time = &time;
      cb.value = &value;
      vpi_register_cb(&cb);
      return 0;
}

static PLI_INT32 check(PLI_BYTE8*unused)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      s_vpi_value value;
      (void)unused;

      value.format = vpiIntVal;
      vpi_get_value(vpi_scan(args), &value);
      vpi_free_object(args);
      value.value.integer = !failed && calls == (unsigned)value.value.integer;
      vpi_put_value(call, &value, 0, vpiNoDelay);
      return 0;
}

static void register_test(void)
{
      s_vpi_systf_data task = {0};
      task.type = vpiSysTask;
      task.tfname = "$watch_value";
      task.calltf = watch;
      vpi_register_systf(&task);
      task.type = vpiSysFunc;
      task.sysfunctype = vpiIntFunc;
      task.tfname = "$check_values";
      task.calltf = check;
      vpi_register_systf(&task);
}

void (*vlog_startup_routines[])(void) = {register_test, 0};
