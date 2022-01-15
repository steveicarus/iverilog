
/*
 */
# include  <vpi_user.h>
# include  <assert.h>

static int sn_calltf(char*user_data)
{
      vpiHandle scope = vpi_handle(vpiScope, 0);

      (void)user_data;  /* Parameter is not used. */

      assert(scope);


      vpi_printf("My scope name: %s (s.b. xor_try)\n",
		 vpi_get_str(vpiFullName, scope));
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
