# include  <vpi_user.h>
# include  <assert.h>

static int test_calltf(char*user_data)
{
      s_vpi_value value;

      (void)user_data;  /* Parameter is not used. */

      vpiHandle vec_handle = vpi_handle_by_name("test.vec", 0);

      vpiHandle msb_handle = vpi_handle(vpiLeftRange,  vec_handle);
      vpiHandle lsb_handle = vpi_handle(vpiRightRange, vec_handle);

      assert(msb_handle);
      assert(lsb_handle);

      value.format = vpiBinStrVal;
      vpi_get_value(msb_handle, &value);
      vpi_printf("msb = 'b_%s\n", value.value.str);

      value.format = vpiBinStrVal;
      vpi_get_value(lsb_handle, &value);
      vpi_printf("lsb = 'b_%s\n", value.value.str);

      return 0;
}

static void vpi_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.calltf    = test_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.tfname    = "$test";
      tf_data.user_data = 0;

      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
      vpi_register,
      0
};
