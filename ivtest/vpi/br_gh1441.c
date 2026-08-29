#include <string.h>
#include "vpi_user.h"

static int failed;

static void check(const char *name, PLI_INT32 expected_scalar,
                  PLI_INT32 expected_vector)
{
      vpiHandle obj = vpi_handle_by_name((PLI_BYTE8 *)name, 0);
      PLI_INT32 scalar;
      PLI_INT32 vector;

      if (!obj) {
            vpi_printf("FAILED: cannot find %s\n", name);
            failed = 1;
            return;
      }

      scalar = vpi_get(vpiScalar, obj);
      vector = vpi_get(vpiVector, obj);
      if (scalar != expected_scalar || vector != expected_vector) {
            vpi_printf("FAILED: %s has scalar=%d vector=%d; expected %d/%d\n",
                       name, scalar, vector, expected_scalar, expected_vector);
            failed = 1;
      }
}

static PLI_INT32 check_calltf(PLI_BYTE8 *data)
{
      (void)data;

      check("test.scalar_logic", 1, 0);
      check("test.single_element_scalar", 1, 0);
      check("test.vector_logic", 0, 1);
      check("test.unit_vector_zero", 0, 1);
      check("test.unit_vector_nonzero", 0, 1);
      check("test.scalar_wire", 1, 0);
      check("test.unit_wire", 0, 1);
      check("test.unit_net_array[0]", 0, 1);

      if (failed)
            vpi_control(vpiFinish, 1);
      else
            vpi_printf("PASSED\n");
      return 0;
}

static void register_check(void)
{
      s_vpi_systf_data tf = {0};
      tf.type = vpiSysTask;
      tf.tfname = "$check_scalar_vector";
      tf.calltf = check_calltf;
      vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {
      register_check,
      0
};
