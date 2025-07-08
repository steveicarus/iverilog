#include <vpi_user.h>

static PLI_INT32 test_control(PLI_BYTE8 *xx)
{
    int ret_val;
    int failed = 0;

    (void)xx;  /* Parameter is not used. */

    ret_val = vpi_control(9999);
    //vpi_printf("vpi_control(9999) returned %i\n", ret_val);
    if (ret_val != 0) failed = 1;

    ret_val = vpi_sim_control(9999);
    //vpi_printf("vpi_sim_control(9999) returned %i\n", ret_val);
    if (ret_val != 0) failed = 1;

    ret_val = vpi_control(vpiFinish, 0);
    //vpi_printf("vpi_control(vpiFinish, 1) returned %i\n", ret_val);
    if (ret_val != 1) failed = 1;

    ret_val = vpi_sim_control(vpiFinish, 0);
    //vpi_printf("vpi_sim_control(vpiFinish, 1) returned %i\n", ret_val);
    if (ret_val != 1) failed = 1;

    if (failed) {
        vpi_printf("FAILED\n");
    } else {
        vpi_printf("PASSED\n");
    }
    return 0;
}

static void register_test_control(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$test_control";
    tf_data.calltf    = test_control;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
    register_test_control,
    0
};
