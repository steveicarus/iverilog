# include  <vpi_user.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

static PLI_INT32 listparams_compiletf(PLI_BYTE8*name)
{
      (void)name;  /* Parameter is not used. */
      return 0;
}

static void param_by_name(vpiHandle scope, const char*key)
{
      vpiHandle iter = vpi_iterate(vpiParameter, scope);
      vpiHandle item;

      while ( (item = vpi_scan(iter)) ) {

	    s_vpi_value value;

	    if (strcmp(key, vpi_get_str(vpiName,item)) != 0)
		  continue;

	    vpi_printf("%8s: ", vpi_get_str(vpiName, item));

	    switch (vpi_get(vpiConstType, item)) {
		case vpiStringConst:
		  value.format = vpiStringVal;
		  vpi_get_value(item, &value);
		  vpi_printf("%s", value.value.str);
		  break;
		case vpiBinaryConst:
		  value.format = vpiBinStrVal;
		  vpi_get_value(item, &value);
		  vpi_printf("%s", value.value.str);
		  break;
		default:
		  break;
	    }

	    vpi_printf("\n");
      }
}

static PLI_INT32 listparams_calltf(PLI_BYTE8*name)
{
      vpiHandle sys  = vpi_handle(vpiSysTfCall,0);
      vpiHandle scope= vpi_handle(vpiScope, sys);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item;

      (void)name;  /* Parameter is not used. */

      while ( (item = vpi_scan(argv)) ) {
	    s_vpi_value value;
	    value.format = vpiStringVal;
	    vpi_get_value(item, &value);
	    param_by_name(scope, value.value.str);
      }

      return 0;
}

static void listparams_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$listparams";
      tf_data.calltf    = listparams_calltf;
      tf_data.compiletf = listparams_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$listparams";
      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
      listparams_register,
      0
};
