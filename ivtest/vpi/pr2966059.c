#include "vpi_user.h"

static PLI_INT32 number_compiletf(PLI_BYTE8 *x)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      char *name;

      s_vpi_value var;

      (void)x;  /* Parameter is not used. */

      if (argv == 0) {
	    vpi_printf("ERROR: missing required numeric argument.\n"),
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      arg = vpi_scan(argv);

	/* Check to see what vpi_get_value does during compiletf. */
      name = vpi_get_str(vpiName, arg);
      vpi_printf("vpi_get_value (%s):\n", name ? name : "<N/A>");
      var.format = vpiObjTypeVal;
      vpi_get_value(arg, &var);
      vpi_printf("  format = %d\n", (int) var.format);
      var.format = vpiDecStrVal;
      vpi_get_value(arg, &var);
      vpi_printf("   value = %s\n", var.value.str);

      vpi_free_object(argv);
      return 0;
}

static void local_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type = vpiSysTask;
      tf_data.tfname = "$check_number";
      tf_data.calltf = 0;
      tf_data.compiletf = number_compiletf;
      tf_data.sizetf = 0;
      tf_data.user_data = 0;
      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = {
local_register, 0};
