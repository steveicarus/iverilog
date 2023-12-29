# include  <vpi_user.h>
# include  <stdlib.h>

static PLI_INT32 memmonitor_compiletf(PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      if (argv == 0) {
	    vpi_printf("ERROR: %s expects at least 1 argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      while ( (arg = vpi_scan(argv)) ) {
	    if (vpi_get(vpiType, arg) != vpiMemory) {
		  vpi_printf("ERROR: %s expects only memory arguments\n", name);
		  vpi_control(vpiFinish, 1);
		  return 0;
	    }
      }

      return 0;
}

static PLI_INT32 callback(struct t_cb_data*cb)
{
      vpi_printf("ValueChange: index=%d, value=%s\n",
		 (int)cb->index, cb->value->value.str);
      return 0;
}

static PLI_INT32 cleanup(struct t_cb_data*cb)
{
      free(cb->value);
      return 0;
}

static PLI_INT32 memmonitor_calltf(PLI_BYTE8*name)
{
      vpiHandle sys  = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      (void)name;  /* Parameter is not used. */

      while ( (arg = vpi_scan(argv)) ) {
	    s_cb_data cb_data;

	    cb_data.reason = cbValueChange;
	    cb_data.cb_rtn = callback;
	    cb_data.obj = arg;
	    cb_data.time = 0;
	    cb_data.value = malloc(sizeof(struct t_vpi_value));
	    cb_data.index = 0;
	    cb_data.user_data = 0;

	    cb_data.value->format = vpiBinStrVal;
	    cb_data.value->value.str = 0;

	    vpi_register_cb(&cb_data);

	    cb_data.reason = cbEndOfSimulation;
	    cb_data.cb_rtn = cleanup;
	    vpi_register_cb(&cb_data);
      }

      return 0;
}

static void memmonitor_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$memmonitor";
      tf_data.calltf    = memmonitor_calltf;
      tf_data.compiletf = memmonitor_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$memmonitor";
      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
      memmonitor_register,
      0
};
