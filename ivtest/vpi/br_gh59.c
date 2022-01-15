#include <assert.h>
#include "vpi_user.h"

static PLI_INT32
EndOfCompile(s_cb_data *data)
{
    s_vpi_time timerec = { vpiSimTime, 0, 0, 0 };
    s_vpi_value val;

    vpiHandle b0_handle;
    vpiHandle wr_handle;
    vpiHandle in_handle;

    (void)data;  /* Parameter is not used. */

    b0_handle = vpi_handle_by_name("test.B0", 0);
    assert(b0_handle);

    wr_handle = vpi_handle_by_name("test.WR", 0);
    assert(wr_handle);

    in_handle = vpi_handle_by_name("test.IN", 0);
    assert(in_handle);

    timerec.low = 500;
    val.value.str = "01";
    val.format = vpiBinStrVal;
    vpi_put_value(in_handle, &val,  &timerec, vpiInertialDelay);

    val.value.str = "zz";
    val.format = vpiBinStrVal;
    vpi_put_value(b0_handle, &val,  &timerec, vpiInertialDelay);

    val.format = vpiIntVal;

    val.value.integer = 0;
    vpi_put_value(wr_handle, &val, &timerec, vpiInertialDelay);

    timerec.low = 500;
    val.value.integer = 0;
    vpi_put_value(wr_handle, &val,  &timerec, vpiInertialDelay);

    val.value.integer = 1;
    timerec.low = 1000;
    vpi_put_value(wr_handle, &val,  &timerec, vpiInertialDelay);

    timerec.low = 1500;
    vpi_put_value(wr_handle, &val,  &timerec, vpiInertialDelay);

    timerec.low = 2000;
    val.value.integer = 0;
    vpi_put_value(wr_handle, &val,  &timerec, vpiInertialDelay);

    timerec.low = 3000;
    val.value.integer = 0;
    vpi_put_value(wr_handle, &val,  &timerec, vpiInertialDelay);

    return 0;
}


static void
VPIRegister(void)
{
    s_cb_data cb_data;
    s_vpi_time timerec = { vpiSuppressTime, 0, 0, 0 };

    cb_data.time = &timerec;
    cb_data.value = 0;
    cb_data.user_data = 0;
    cb_data.obj = 0;
    cb_data.reason = cbEndOfCompile;
    cb_data.cb_rtn = EndOfCompile;

    vpi_register_cb(&cb_data);
}

void (*vlog_startup_routines[])(void) = {
  VPIRegister,
  0
};
