#include "string.h"
#include "vpi_user.h"

/*
 * Not all systems support passing a system task/function call handle
 * to vpi_get_systf_info(). All should support vpiUserSystf. You can
 * only get the vpiUserSystf handle when vpiUserDefn is 1 (true).
 */

/*
 * Set the following when compiling if the call handle is not supported:
 *   SYSTF_INFO_CALLH_NOT_SUPPORTED
 */

static vpiHandle registered_task_as;
static vpiHandle registered_func_as;

/* These tasks/functions do not take any arguments. */
static PLI_INT32 sys_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      if (argv) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s does not take any arguments.\n", name);
	    vpi_free_object(argv);
	    vpi_control(vpiFinish, 1);
      }

      return 0;
}

/* Helper function to print the function type as a string value. */
static const char *func_type(PLI_INT32 type)
{
      char *res = 0;
      switch (type) {
	case vpiIntFunc:
	    res = "vpiIntFunc";
	    break;
	case vpiRealFunc:
	    res = "vpiRealFunc";
	    break;
	case vpiTimeFunc:
	    res = "vpiTimeFunc";
	    break;
	case vpiSizedFunc:
	    res = "vpiSizedFunc";
	    break;
	case vpiSizedSignedFunc:
	    res = "vpiSizedSignedFunc";
	    break;
	default:
	    res = "<unknown>";
	    break;
      }
      return res;
}

/* The calltf routine for the task check. */
static PLI_INT32 sys_check_sys_task_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      PLI_INT32 res;
      s_vpi_systf_data tf_data;
      vpiHandle iter;

	/* Check the stashed handle. */
      vpi_get_systf_info(registered_task_as, &tf_data);
      vpi_printf("registered as: %s - %s\n",
                 vpi_get_str(vpiType, registered_task_as),
                 tf_data.tfname);

	/* Check to see if vpiUserDefn is set correctly. */
      res = vpi_get(vpiUserDefn, callh);
      vpi_printf("  vpiUserDefn: is ");
      if (res != 1) {
	    vpi_printf("undefined (%d)!\n", (int)res);
      } else {
	    vpi_printf("defined.\n");

	      /* Check vpi_get_systf_info (just check the name). */
	    vpi_get_systf_info(vpi_handle(vpiUserSystf, callh), &tf_data);
	    vpi_printf("  vpi_get_systf_info: ");
	    if (strcmp(tf_data.tfname, name)) {
		  vpi_printf("failed.\n");
	    } else {
		  vpi_printf("passed.\n");
	    }
#ifdef SYSTF_INFO_CALLH_NOT_SUPPORTED
	    vpi_printf("  vpi_get_systf_info (callh): not supported.\n");
#else
	      /* This is not supported by all simulators. */
	    vpi_get_systf_info(callh, &tf_data);
	    vpi_printf("  vpi_get_systf_info (callh): ");
	    if (strcmp(tf_data.tfname, name)) {
		  vpi_printf("failed.\n");
	    } else {
		  vpi_printf("passed.\n");
	    }
#endif
      }

	/* Look for all the user defined system tasks/functions. */
      vpi_printf("Looking for all user defined system tasks/functions:\n");
      iter = vpi_iterate(vpiUserSystf, 0);
      if (iter) {
            vpiHandle val;
	    while ((val = vpi_scan(iter))) {
		  vpi_get_systf_info(val, &tf_data);
		  vpi_printf("  Found ");
		  if (tf_data.type == vpiSysTask) vpi_printf("task");
		  else vpi_printf("function (%s)",
		                  func_type(tf_data.sysfunctype));
		  vpi_printf(" %s - %s",
		             vpi_get_str(vpiType, val),
		             tf_data.tfname);
		  if (vpi_compare_objects(val, registered_task_as)) {
			vpi_printf(" *****> caller");
		  }
		  vpi_printf(".\n");
	    }
	    vpi_printf("Done.\n");
      } else {
	    vpi_printf("  No user defined system tasks/functions found!\n");
      }

      return 0;
}

/* The calltf routine for the function check. */
static PLI_INT32 sys_check_sys_func_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      PLI_INT32 res;
      s_vpi_systf_data tf_data;

	/* Check the stashed handle. */
      vpi_get_systf_info(registered_func_as, &tf_data);
      vpi_printf("registered as: %s - %s\n",
                 vpi_get_str(vpiType, registered_func_as),
                 tf_data.tfname);

	/* Check to see if vpiUserDefn is set correctly. */
      res = vpi_get(vpiUserDefn, callh);
      vpi_printf("  vpiUserDefn: ");
      if (res != 1) {
	    vpi_printf("is undefined (%d)!\n", (int)res);
      } else {
	    vpi_printf("is defined.\n");

	      /* Check vpi_get_systf_info (just check the name). */
	    vpi_get_systf_info(vpi_handle(vpiUserSystf, callh), &tf_data);
	    vpi_printf("  vpi_get_systf_info: ");
	    if (strcmp(tf_data.tfname, name)) {
		  vpi_printf("failed.\n");
	    } else {
		  vpi_printf("passed.\n");
	    }
#ifdef SYSTF_INFO_CALLH_NOT_SUPPORTED
	    vpi_printf("  vpi_get_systf_info (callh): not supported.\n");
#else
	      /* This is not supported by all simulators. */
	    vpi_get_systf_info(callh, &tf_data);
	    vpi_printf("  vpi_get_systf_info (callh): ");
	    if (strcmp(tf_data.tfname, name)) {
		  vpi_printf("failed.\n");
	    } else {
		  vpi_printf("passed.\n");
	    }
#endif
      }

	/* We would normally put a value for a function, but since we're
	 * not testing that with this code we'll just use the default
	 * value (0). */
      return 0;
}

/* A simple task to add a few more definitions to the vpiUserSystf list. */
static PLI_INT32 sys_hello_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpi_printf("Hello from %s at %s:%d.\n", name,
                 vpi_get_str(vpiFile, callh),
                 (int)vpi_get(vpiLineNo, callh));
      return 0;
}

static void register_check_systf(void)
{
      vpiHandle res;
      s_vpi_systf_data tf_data;

      tf_data.type        = vpiSysTask;
      tf_data.calltf      = sys_check_sys_task_calltf;
      tf_data.compiletf   = sys_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$check_sys_task";
      tf_data.user_data   = "$check_sys_task";;
      registered_task_as = vpi_register_systf(&tf_data);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.calltf      = sys_check_sys_func_calltf;
      tf_data.compiletf   = sys_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$check_sys_func";
      tf_data.user_data   = "$check_sys_func";;
      registered_func_as = vpi_register_systf(&tf_data);

      tf_data.type        = vpiSysTask;
      tf_data.calltf      = sys_hello_calltf;
      tf_data.compiletf   = sys_compiletf;
      tf_data.sizetf      = 0;
      tf_data.tfname      = "$hello";
      tf_data.user_data   = "$hello";;
      res = vpi_register_systf(&tf_data);
	/* Icarus does not need this, but it should not be an error either. */
      vpi_free_object(res);


// Does this work on all simulators? I would expect vpi_printf() to be OK,
// but vpi_get_systf_info() could be suspect.
      vpi_get_systf_info(registered_task_as, &tf_data);
      vpi_printf("--> registered task as: %s.\n", tf_data.tfname);
      vpi_get_systf_info(registered_func_as, &tf_data);
      vpi_printf("--> registered func as: %s.\n", tf_data.tfname);
}

void (*vlog_startup_routines[])(void) = {
      register_check_systf,
      0
};

/* This is needed by other simulators. */
void vpi_bootstrap(void)
{
      int i;
      for (i = 0; vlog_startup_routines[i]; i += 1) {
	    vlog_startup_routines[i]();
      }
}
