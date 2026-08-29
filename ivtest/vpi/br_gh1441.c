#include <string.h>
#include "vpi_user.h"

static int failed = 0;

static void check(const char *name, int exp_scalar, int exp_vector)
{
      vpiHandle h = vpi_handle_by_name((PLI_BYTE8*)name, 0);
      if (!h) {
            vpi_printf("FAILED: cannot find %s\n", name);
            failed = 1;
            return;
      }
      int is_scalar = vpi_get(vpiScalar, h);
      int is_vector = vpi_get(vpiVector, h);
      if (is_scalar != exp_scalar || is_vector != exp_vector) {
            vpi_printf("FAILED: %s scalar=%d (exp %d) vector=%d (exp %d)\n",
                       name, is_scalar, exp_scalar, is_vector, exp_vector);
            failed = 1;
      } else {
            vpi_printf("PASSED: %s scalar=%d vector=%d\n", name, is_scalar, is_vector);
      }
}

static PLI_INT32 check_cb(PLI_BYTE8 *data)
{
      (void)data;
      check("top.scalar_logic", 1, 0);
      check("top.vector_logic", 0, 1);
      check("top.single_element_vector_logic", 0, 1);
      if (failed)
            vpi_control(vpiFinish, 1);
      else
            vpi_printf("PASSED: all VPI scalar/vector checks\n");
      return 0;
}

static void register_cb(void)
{
      s_cb_data cb = {0};
      cb.reason = cbStartOfSimulation;
      cb.cb_rtn = check_cb;
      vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {
      register_cb,
      0
};
