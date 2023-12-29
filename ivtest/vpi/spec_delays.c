# include  <vpi_user.h>
# include  <string.h>
# include  <stdlib.h>
# include  <assert.h>


static PLI_INT32 dump_specify_compiletf(PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item;

      item = vpi_scan(argv);
      if (argv == 0) {
	    vpi_printf("%s: scope name argument missing.\n", name);
	    vpi_control(vpiFinish, 1);
	    return -1;
      }

      if (vpi_get(vpiType, item) != vpiModule) {
	    vpi_printf("%s: Argument is not a vpiModule\n", name);
	    vpi_control(vpiFinish, 1);
	    return -1;
      }

      item = vpi_scan(argv);
      if (item != 0) {
	    vpi_printf("%s: Too many arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return -1;
      }

      return 0;
}

static PLI_INT32 dump_specify_calltf(PLI_BYTE8*name)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      (void)name;  /* Parameter is not used. */

      assert(item);
      vpi_free_object(argv);

      vpi_printf("** Look for vpiModPath objects in %s.\n",
		 vpi_get_str(vpiName, item));

      argv = vpi_iterate(vpiModPath, item);
      if (argv == 0) {
	    vpi_printf("**  NO modpath items?\n");
      } else {
	    struct t_vpi_time delay_times[12];
	    struct t_vpi_delay delays;
	    for (item = vpi_scan(argv); item; item = vpi_scan(argv)) {
		  vpiHandle in_argv = vpi_iterate(vpiModPathIn, item);
		  vpiHandle in_term = in_argv ? vpi_scan(in_argv) : 0;
		  vpiHandle in_expr = in_term ? vpi_handle(vpiExpr, in_term) : 0;
		  if (in_argv) vpi_free_object(in_argv);
		  vpiHandle out_argv = vpi_iterate(vpiModPathOut, item);
		  vpiHandle out_term = out_argv ? vpi_scan(out_argv) : 0;
		  vpiHandle out_expr = out_term ? vpi_handle(vpiExpr, out_term) : 0;
		  if (out_argv) vpi_free_object(out_argv);
		  vpi_printf("**    got path: %s ",
			     in_expr  ? vpi_get_str(vpiName, in_expr)  : "?");
		  vpi_printf("--> %s\n",
			     out_expr ? vpi_get_str(vpiName, out_expr) : "?");

		  delays.da = delay_times;
		  delays.no_of_delays = 12;
		  delays.time_type = vpiSimTime;
		  delays.mtm_flag = 0;
		  delays.append_flag = 0;
		  delays.pulsere_flag = 0;
		  vpi_get_delays(item, &delays);
		  vpi_printf("**        (%d,%d,%d, %d,%d,%d, %d,%d,%d, %d,%d,%d)\n",
			     (int)delay_times[0].low,
			     (int)delay_times[1].low,
			     (int)delay_times[2].low,
			     (int)delay_times[3].low,
			     (int)delay_times[4].low,
			     (int)delay_times[5].low,
			     (int)delay_times[6].low,
			     (int)delay_times[7].low,
			     (int)delay_times[8].low,
			     (int)delay_times[9].low,
			     (int)delay_times[10].low,
			     (int)delay_times[11].low);

		  delays.da = delay_times;
		  delays.no_of_delays = 12;
		  delays.time_type = vpiScaledRealTime;
		  delays.mtm_flag = 0;
		  delays.append_flag = 0;
		  delays.pulsere_flag = 0;
		  vpi_get_delays(item, &delays);
		  vpi_printf("**        (%f,%f,%f, %f,%f,%f, %f,%f,%f, %f,%f,%f)\n",
			     delay_times[0].real,
			     delay_times[1].real,
			     delay_times[2].real,
			     delay_times[3].real,
			     delay_times[4].real,
			     delay_times[5].real,
			     delay_times[6].real,
			     delay_times[7].real,
			     delay_times[8].real,
			     delay_times[9].real,
			     delay_times[10].real,
			     delay_times[11].real);

		  delays.time_type = vpiScaledRealTime;
		  delay_times[0].real = 3.0;
		  delay_times[1].real = 3.0;
		  delay_times[2].real = 3.0;
		  delay_times[3].real = 3.0;
		  delay_times[4].real = 3.0;
		  delay_times[5].real = 3.0;
		  delay_times[6].real = 3.0;
		  delay_times[7].real = 3.0;
		  delay_times[8].real = 3.0;
		  delay_times[9].real = 3.0;
		  delay_times[10].real = 3.0;
		  delay_times[11].real = 3.0;
		  vpi_put_delays(item, &delays);

		  delays.da = delay_times;
		  delays.no_of_delays = 12;
		  delays.time_type = vpiScaledRealTime;
		  delays.mtm_flag = 0;
		  delays.append_flag = 0;
		  delays.pulsere_flag = 0;
		  vpi_get_delays(item, &delays);
		  vpi_printf("**        (%f,%f,%f, %f,%f,%f, %f,%f,%f, %f,%f,%f)\n",
			     delay_times[0].real,
			     delay_times[1].real,
			     delay_times[2].real,
			     delay_times[3].real,
			     delay_times[4].real,
			     delay_times[5].real,
			     delay_times[6].real,
			     delay_times[7].real,
			     delay_times[8].real,
			     delay_times[9].real,
			     delay_times[10].real,
			     delay_times[11].real);
	    }
      }

      vpi_printf("** done\n");
      return 0;
}

static void sys_register(void)
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dump_specify";
      tf_data.calltf    = dump_specify_calltf;
      tf_data.compiletf = dump_specify_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dump_specify";
      vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
      sys_register,
      0
};
