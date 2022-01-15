
/*
 */
# include  <vpi_user.h>
# include  <assert.h>

static int sn_calltf(char*user_data)
{
      s_vpi_value value;

      int left_value;
      int right_value;

      (void)user_data;  /* Parameter is not used. */

	/* Get the handle of an object that we know to be present. */
      vpiHandle xor_hand   = vpi_handle_by_name("xor_try.inp_xor",0);

	/* The object is a vector, get the expressions for the left
	   and right range values. */
      vpiHandle left_hand  = vpi_handle(vpiLeftRange, xor_hand);
      vpiHandle right_hand = vpi_handle(vpiRightRange, xor_hand);

      assert(left_hand);
      assert(right_hand);

	/* Extract the values from the expressions. */
      value.format = vpiIntVal;
      vpi_get_value(left_hand, &value);
      left_value = value.value.integer;

      value.format = vpiIntVal;
      vpi_get_value(right_hand, &value);
      right_value = value.value.integer;

      vpi_printf("Dimensions of xor_try.inp_xor: [%d:%d]\n", left_value, right_value);
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
