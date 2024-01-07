#include <vpi_user.h>

static PLI_INT32 start_cb(struct t_cb_data *cb)
{
    static struct t_vpi_value val;
    vpiHandle wire;

    (void)cb;  // suppress unused parameter warning

    wire = vpi_handle_by_name("test.w4", NULL);
    if (wire) {
        val.format = vpiIntVal;
        val.value.integer = 1;
        vpi_put_value(wire, &val, NULL, vpiNoDelay);
    } else {
        vpi_printf("Failed to get handle for w4\n");
    }

    wire = vpi_handle_by_name("test.w8", NULL);
    if (wire) {
        val.format = vpiIntVal;
        val.value.integer = 1;
        vpi_put_value(wire, &val, NULL, vpiNoDelay);
    } else {
        vpi_printf("Failed to get handle for w8\n");
    }

    wire = vpi_handle_by_name("test.wr", NULL);
    if (wire) {
        val.format = vpiRealVal;
        val.value.real = 1.0;
        vpi_put_value(wire, &val, NULL, vpiNoDelay);
    } else {
        vpi_printf("Failed to get handle for wr\n");
    }

    return 0;
}

static void register_cb(void)
{
    struct t_cb_data         cbd = {};

    cbd.reason = cbStartOfSimulation;
    cbd.cb_rtn = start_cb;
    vpi_register_cb(&cbd);
}

void (*vlog_startup_routines[])(void) = {
    register_cb,
    0
};
