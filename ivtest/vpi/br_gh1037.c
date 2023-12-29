# include  <stdlib.h>
# include  <string.h>
# include  <sv_vpi_user.h>

static PLI_INT32 check_items_calltf(PLI_BYTE8 *xx)
{
      (void)xx;  /* Parameter is not used. */
      vpiHandle th, ih;

	// Get the top level instances.
      th = vpi_iterate(vpiInstance, NULL);
      while((ih = vpi_scan(th))) {
	    vpiHandle vh;
	    vpi_printf("Found an item named %s ", vpi_get_str(vpiFullName, ih));
	    vpi_printf("of type %s\n", vpi_get_str(vpiType, ih));
	    vh = vpi_iterate(vpiVariables, ih);
	    if (vh != 0) {
		   vpiHandle vi;
		   while((vi = vpi_scan(vh))) {
			s_vpi_value value;
			char*str_val;
			value.format = vpiDecStrVal;
			vpi_get_value(vi, &value);
			str_val = strdup(value.value.str);
			vpi_printf(" - Has a variable named %s ", vpi_get_str(vpiFullName, vi));
			vpi_printf("of type %s and a value of %s\n", vpi_get_str(vpiType, vi), str_val);
			free(str_val);
		   }
	    }
      }
      vpi_printf("All done with items\n");
      return 0;
}

static void check_items_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$check_items";
      tf_data.calltf    = check_items_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);
}

/*
 * This is a table of register functions. This table is the external
 * symbol that the simulator looks for when loading this .vpi module.
 */
void (*vlog_startup_routines[])(void) = {
      check_items_register,
      0
};
