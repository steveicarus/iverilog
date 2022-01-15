#include <assert.h>
#include "veriuser.h"
#include "acc_user.h"

static int calltf(int ud, int reason)
{
    (void)ud;
    (void)reason;

    tf_asynchon();

    return 0;
}

static int misctf(int ud, int reason, int paramvc)
{
    handle hdl1;
    handle hdl2;
    s_setval_value val;
    s_setval_delay dly;

    (void)ud;
    (void)paramvc;

    io_printf("misctf called for reason %d\n", reason);

    hdl1 = acc_handle_tfarg(1);
    assert(hdl1);
    hdl2 = acc_handle_tfarg(2);
    assert(hdl2);
    if (reason == reason_paramvc && paramvc == 1) {
        val.format = accIntVal;
        (void)acc_fetch_value(hdl1, "%%", &val);
        dly.model = accNoDelay;
        (void)acc_set_value(hdl2, &val, &dly);
    }
    return 0;
}

s_tfcell veriusertfs[2] = {
  {usertask, 0, 0, 0, calltf, misctf, "$background_copy", 1, 0, 0, {0} },
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0} }
};

static void veriusertfs_register(void)
{
      veriusertfs_register_table(veriusertfs);
}

void (*vlog_startup_routines[])(void) = { &veriusertfs_register, 0 };
