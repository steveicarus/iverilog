/* Copyright (c) 2026 Jerry Zhong
 * Independent expected results for vpi_vector4_integer.v.
 */

#include <vpi_user.h>

static const PLI_INT32 expected[] = {
      0, -1, (-2147483647 - 1), 2147483647, 128, -128,
      305419896, -2147483647, 165, -123, 5
};
static unsigned calls;
static unsigned failures;

static PLI_INT32 check_value(PLI_BYTE8*unused)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      vpiHandle arg = args ? vpi_scan(args) : 0;
      s_vpi_value value;
      (void)unused;
      if (!arg || calls >= sizeof(expected) / sizeof(expected[0])) {
            vpi_printf("FAILED unexpected conversion call %u\n", calls);
            failures += 1;
      } else {
            value.format = vpiIntVal;
            vpi_get_value(arg, &value);
            if (value.value.integer != expected[calls]) {
                  vpi_printf("FAILED conversion %u: got %d, expected %d\n",
                             calls, value.value.integer, expected[calls]);
                  failures += 1;
            }
      }
      if (args) vpi_free_object(args);
      calls += 1;
      return 0;
}

static PLI_INT32 finish(PLI_BYTE8*unused)
{
      (void)unused;
      if (calls != sizeof(expected) / sizeof(expected[0])) {
            vpi_printf("FAILED conversion count: %u\n", calls);
            failures += 1;
      }
      if (!failures) vpi_printf("PASSED\n");
      return 0;
}

static void register_test(void)
{
      s_vpi_systf_data tf = {0};
      tf.type = vpiSysTask;
      tf.tfname = "$check_vector4_integer";
      tf.calltf = check_value;
      vpi_register_systf(&tf);
      tf.tfname = "$check_vector4_integer_done";
      tf.calltf = finish;
      vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {register_test, 0};
