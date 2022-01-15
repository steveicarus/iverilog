#include <assert.h>
#include "vpi_user.h"

static PLI_INT32 atEndOfCompile(s_cb_data *data)
{
    vpiHandle handle;
    s_vpi_time time = { vpiSimTime, 0, 0, 0 };
    s_vpi_value value;

    (void)data;  /* Parameter is not used. */

    handle = vpi_handle_by_name("test.flag", 0);
    assert(handle);

    value.format = vpiIntVal;
    value.value.integer = 1;
    vpi_put_value(handle, &value, &time, vpiPureTransportDelay);

    return 0;
}

static void vpiRegister(void)
{
    s_cb_data cb_data;
    s_vpi_time time = { vpiSuppressTime, 0, 0, 0 };

    cb_data.time = &time;
    cb_data.value = 0;
    cb_data.user_data = 0;
    cb_data.obj = 0;
    cb_data.reason = cbEndOfCompile;
    cb_data.cb_rtn = atEndOfCompile;

    vpi_register_cb(&cb_data);
}

void (*vlog_startup_routines[]) (void) = { vpiRegister, 0};
