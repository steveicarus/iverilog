/*
 * This test verifies the the vpi_get_vlog_info() call returns the
 * version information correctly. This does not currently work in
 * V0.8 so it is marked NI.
 */
#include "vpi_user.h"
#include <string.h>
#include <stdlib.h>

static PLI_INT32 compiletf(PLI_BYTE8 *name)
{
    vpiHandle callh, argv, arg;
    callh = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, callh);

    /* Check that there is an argument. */
    if (argv == 0) {
	vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	           (int)vpi_get(vpiLineNo, callh));
	vpi_printf("%s() requires a single vector argument!", name);
	vpi_control(vpiFinish, 1);
	return 0;
    }
    arg = vpi_scan(argv);
    if (arg == 0) {
	vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	           (int)vpi_get(vpiLineNo, callh));
	vpi_printf("%s() requires a single vector argument!", name);
	vpi_control(vpiFinish, 1);
	return 0;
    }

    /* Check that the argument is a register with at least 640 bits. */
    if (vpi_get(vpiType, arg) != vpiReg) {
	vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	           (int)vpi_get(vpiLineNo, callh));
	vpi_printf("%s() requires a vector argument!", name);
	vpi_free_object(argv);
	vpi_control(vpiFinish, 1);
	return 0;
    }
    if (vpi_get(vpiSize, arg) < 640) {
	vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	           (int)vpi_get(vpiLineNo, callh));
	vpi_printf("%s() vector is too small need 640 bits!", name);
	vpi_free_object(argv);
	vpi_control(vpiFinish, 1);
	return 0;
    }

    /* We only take one argument. */
    arg = vpi_scan(argv);
    if (arg != 0) {
	vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	           (int)vpi_get(vpiLineNo, callh));
	vpi_printf("%s() only takes one argument!", name);
	vpi_free_object(argv);
	vpi_control(vpiFinish, 1);
	return 0;
    }

    return 0;
}

static PLI_INT32 calltf(PLI_BYTE8 *name)
{
    vpiHandle callh, argv, reg;
    s_vpi_vlog_info info;
    s_vpi_value val;
    PLI_INT32 ret;
    char *str, *cp;

    callh = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, callh);
    reg = vpi_scan(argv);
    vpi_free_object(argv);

    /* Get the Verilog version information. */
    ret = vpi_get_vlog_info(&info);
    if (ret == 0) {
	vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	           (int)vpi_get(vpiLineNo, callh));
	vpi_printf("%s() failed to get verilog information!", name);
	vpi_control(vpiFinish, 1);
    }

    /* For Icarus we can just return everything before the space to
     * get the version and subversion. */
    str = strdup(info.version);
    cp = strchr(str, ' ');
    if (cp)
	*cp = '\0';
    val.format = vpiStringVal;
    val.value.str = str;
    vpi_put_value(reg, &val, 0, vpiNoDelay);
    free(str);

    return 0;
}

static void my_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$get_version";
    tf_data.calltf    = calltf;
    tf_data.compiletf = compiletf;
    tf_data.sizetf    = 0;
    tf_data.user_data = "$get_version";
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = { my_register, 0};
