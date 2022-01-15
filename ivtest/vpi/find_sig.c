
/*
 */
# include  <vpi_user.h>
# include  <assert.h>

static int sn_calltf(char*user_data)
{
      vpiHandle obj = vpi_handle_by_name("xor_try.unused", 0);

      (void)user_data;  /* Parameter is not used. */

      if (obj==0) {
	    vpi_printf("FAILED -- no xor_try.unused\n");
	    return 0;
      }

      vpi_printf("PASSED\n");
      return 0;
}

static void vpi_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.calltf    = sn_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.tfname    = "$sn";
      tf_data.user_data = 0;

      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
      vpi_register,
      0
};
