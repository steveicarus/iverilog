# include  <vpi_user.h>
# include  <assert.h>

static PLI_INT32 peek_calltf(PLI_BYTE8 *xx)
{
      s_vpi_value val;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      (void)xx;

      arg = vpi_scan(argv);
      vpi_free_object(argv);  /* Free iterator since we did not scan to NULL. */
      assert(arg != 0);

      val.value.real = 0.0;
      val.format = vpiRealVal;
      vpi_get_value(arg, &val);
      vpi_printf("peek    : %f\n", val.value.real);

      return 0;
}

static int count = 1.0;

static PLI_INT32 poke_calltf(PLI_BYTE8 *xx)
{
      s_vpi_value val;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      (void)xx;

      arg = vpi_scan(argv);
      vpi_free_object(argv);  /* Free iterator since we did not scan to NULL. */
      assert(arg != 0);

      val.value.real = count; count += 1.0;
      val.format = vpiRealVal;
      vpi_put_value(arg, &val, 0, vpiNoDelay);
      vpi_printf("poke    : %f\n", val.value.real);

      return 0;
}

static PLI_INT32 force_calltf(PLI_BYTE8 *xx)
{
      s_vpi_value val;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      (void)xx;

      arg = vpi_scan(argv);
      vpi_free_object(argv);  /* Free iterator since we did not scan to NULL. */
      assert(arg != 0);

      val.value.real = 3.0;
      val.format = vpiRealVal;
      vpi_put_value(arg, &val, 0, vpiForceFlag);
      vpi_printf("force   : %f\n", val.value.real);

      return 0;
}

static PLI_INT32 release_calltf(PLI_BYTE8 *xx)
{
      s_vpi_value val;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      (void)xx;

      arg = vpi_scan(argv);
      vpi_free_object(argv);  /* Free iterator since we did not scan to NULL. */
      assert(arg != 0);

      val.format = vpiRealVal;
      vpi_put_value(arg, &val, 0, vpiReleaseFlag);
      vpi_printf("release : %f\n", val.value.real);

      return 0;
}

void peekpoke_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$peek";
      tf_data.calltf    = peek_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$poke";
      tf_data.calltf    = poke_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$force";
      tf_data.calltf    = force_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$release";
      tf_data.calltf    = release_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);
}

/*
 * This is a table of register functions. This table is the external
 * symbol that the simulator looks for when loading this .vpi module.
 */
void (*vlog_startup_routines[])(void) = {
      peekpoke_register,
      0
};
