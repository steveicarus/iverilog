/**********************************************************************
 * $my_monitor example -- PLI application using VPI routines
 *
 * C source to place value change callbacks on all nets in a module
 * instance, and print the simulation time and new value whenever
 * any net changes value.
 *
 * Usage:   $my_monitor(<module_instance_name>);
 *
 * For the book, "The Verilog PLI Handbook" by Stuart Sutherland
 *  Copyright 1999 & 2001, Kluwer Academic Publishers, Norwell, MA, USA
 *   Contact: www.wkap.il
 *  Example copyright 1998, Sutherland HDL Inc, Portland, Oregon, USA
 *   Contact: www.sutherland-hdl.com
 *********************************************************************/


#include <stdlib.h>    /* ANSI C standard library */
#include <stdio.h>     /* ANSI C standard input/output library */
#include <stdarg.h>    /* ANSI C standard arguments library */
#include <string.h>
#include <vpi_user.h>  /* IEEE 1364 PLI VPI routine library  */




/* prototypes of routines in this PLI application */
PLI_INT32  PLIbook_MyMonitor_calltf(PLI_BYTE8 *user_data);
PLI_INT32  PLIbook_MyMonitor_compiletf(PLI_BYTE8 *user_data);
PLI_INT32  PLIbook_MyMonitor_callback(p_cb_data cb_data_p);


/* To make this memory clean we need to keep a list of the allocated
 * names so we can free them at EOS. */
static char** name_list = 0;
static unsigned name_count = 0;

static PLI_INT32 sys_end_of_simulation(p_cb_data cb_data)
{
  unsigned idx;

  (void)cb_data;  /* Parameter is not used. */

  for (idx = 0; idx < name_count; idx += 1) {
    free(name_list[idx]);
  }
  free(name_list);
  name_list = 0;
  name_count = 0;

  return 0;
}

/**********************************************************************
 * VPI Registration Data
 *********************************************************************/
void PLIbook_MyMonitor_register(void)
{
  s_vpi_systf_data tf_data;
  s_cb_data    cb_data;

  tf_data.type        = vpiSysTask;
  tf_data.sysfunctype = 0;
  tf_data.tfname      = "$my_monitor";
  tf_data.calltf      = PLIbook_MyMonitor_calltf;
  tf_data.compiletf   = PLIbook_MyMonitor_compiletf;
  tf_data.sizetf      = NULL;
  tf_data.user_data   = NULL;
  vpi_register_systf(&tf_data);

  cb_data.reason = cbEndOfSimulation;
  cb_data.time = 0;
  cb_data.cb_rtn = sys_end_of_simulation;
  cb_data.user_data = "system";
  vpi_register_cb(&cb_data);
}


void (*vlog_startup_routines[])(void) = {
   PLIbook_MyMonitor_register,
   0
};

/* dummy +loadvpi= boostrap routine - mimics old style exec all routines */
/* in standard PLI vlog_startup_routines table */
void vpi_compat_bootstrap(void)
{
 int i;

 for (i = 0;; i++)
  {
   if (vlog_startup_routines[i] == NULL) break;
   vlog_startup_routines[i]();
  }
}


/**********************************************************************
 * compiletf application
 *********************************************************************/
PLI_INT32 PLIbook_MyMonitor_compiletf(PLI_BYTE8 *user_data)
{
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;

  (void)user_data;  /* Parameter is not used. */

  /* obtain a handle to the system task instance */
  systf_handle = vpi_handle(vpiSysTfCall, NULL);

  /* obtain handles to system task arguments */
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator == NULL) {
    vpi_printf("ERROR: $my_monitor requires 1 argument\n");
    vpi_control(vpiFinish, 1);  /* abort simulation */
    return(0);
  }

  /* check the type of object in system task arguments */
  arg_handle = vpi_scan(arg_iterator);
  tfarg_type = vpi_get(vpiType, arg_handle);
  if (tfarg_type != vpiModule) {
    vpi_printf("ERROR: $my_monitor arg1 must be module instance\n");
    vpi_free_object(arg_iterator); /* free iterator memory */
    vpi_control(vpiFinish, 1);  /* abort simulation */
    return(0);
  }

  /* check that there is only 1 system task argument */
  arg_handle = vpi_scan(arg_iterator);
  if (arg_handle != NULL) {
    vpi_printf("ERROR: $my_monitor can only have 1 argument\n");
    vpi_free_object(arg_iterator); /* free iterator memory */
    vpi_control(vpiFinish, 1);  /* abort simulation */
    return(0);
  }
  return(0);  /* no syntax errors detected */
}

/**********************************************************************
 * calltf routine
 *********************************************************************/
PLI_INT32 PLIbook_MyMonitor_calltf(PLI_BYTE8 *user_data)
{
  vpiHandle    systf_h, arg_itr, mod_h, net_itr, net_h, cb_h;
  s_vpi_time   time_s;
  s_vpi_value  value_s;
  s_cb_data    cb_data_s;
  PLI_BYTE8   *net_name_temp, *net_name_keep;

  (void)user_data;  /* Parameter is not used. */

  /* setup value change callback options */
  time_s.type       = vpiScaledRealTime;
  value_s.format    = vpiBinStrVal;

  cb_data_s.reason  = cbValueChange;
  cb_data_s.cb_rtn  = PLIbook_MyMonitor_callback;
  cb_data_s.time    = &time_s;
  cb_data_s.value   = &value_s;

  /* obtain a handle to the system task instance */
  systf_h = vpi_handle(vpiSysTfCall, NULL);

  /* obtain handle to system task argument */
  /* compiletf has already verified only 1 arg with correct type */
  arg_itr = vpi_iterate(vpiArgument, systf_h);
  mod_h = vpi_scan(arg_itr);
  vpi_free_object(arg_itr);  /* free iterator--did not scan to null */

  /* add value change callback for each net in module named in tfarg */
  vpi_printf("\nAdding monitors to all nets in module %s:\n\n",
             vpi_get_str(vpiDefName, mod_h));

  net_itr = vpi_iterate(vpiNet, mod_h);
  while ((net_h = vpi_scan(net_itr)) != NULL) {
    net_name_temp = vpi_get_str(vpiFullName, net_h);
    net_name_keep = malloc(strlen((char *)net_name_temp)+1);
    strcpy((char *)net_name_keep, (char *)net_name_temp);
    cb_data_s.obj = net_h;
    cb_data_s.user_data = net_name_keep;
    name_count += 1;
    name_list = (char **)realloc(name_list, name_count*sizeof(char **));
    name_list[name_count-1] = net_name_keep;
    cb_h = vpi_register_cb(&cb_data_s);
    vpi_free_object(cb_h); /* don't need callback handle */
  }
  return(0);
}

/**********************************************************************
 * Value change callback application
 *********************************************************************/
PLI_INT32 PLIbook_MyMonitor_callback(p_cb_data cb_data_p)
{
  vpi_printf("At time %0.2f", cb_data_p->time->real);
  vpi_printf(":\t %s", cb_data_p->user_data);
  vpi_printf(" = %s\n", cb_data_p->value->value.str);
  return(0);
}

/*********************************************************************/
