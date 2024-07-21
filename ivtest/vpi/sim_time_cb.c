#ifdef TEST_SCALED_TIME
#define TIME_TYPE   vpiScaledRealTime
#else
#define TIME_TYPE   vpiSimTime
#endif

#include <vpi_user.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static PLI_INT32 monitor_cb(p_cb_data cb_data)
{
    vpiHandle*var_list = (vpiHandle*)cb_data->user_data;

    s_vpi_time  time;
    s_vpi_value value;
    PLI_INT32   index;

    time.type = vpiSimTime;
    vpi_get_time(NULL, &time);
    vpi_printf(" @ %04d :", time.low);

#ifdef TEST_SCALED_TIME
    vpi_printf(" cb_data.time = %1.1f :", cb_data->time->real);
#else
    vpi_printf(" cb_data.time = %04d :", cb_data->time->low);
#endif

    value.format = vpiIntVal;

    index = 0;
    while (var_list[index]) {
        vpi_get_value(var_list[index], &value);
        vpi_printf(" %s = %d", vpi_get_str(vpiName, var_list[index]), value.value.integer);
        index++;
    }
    vpi_printf("\n");

    return 0;
}

static PLI_INT32 monitor_cb_start(p_cb_data cb_data)
{
    vpi_printf("cbStartOfSimTime");
    monitor_cb(cb_data);
    return 0;
}

static PLI_INT32 monitor_cb_delay(p_cb_data cb_data)
{
    vpi_printf("cbAfterDelay    ");
    monitor_cb(cb_data);
    return 0;
}

static PLI_INT32 monitor_cb_synch(p_cb_data cb_data)
{
    vpi_printf("cbReadWriteSynch");
    monitor_cb(cb_data);
    return 0;
}

static PLI_INT32 monitor_cb_end(p_cb_data cb_data)
{
    vpiHandle*var_list = (vpiHandle*)cb_data->user_data;

    vpi_printf("cbEndOfSimTime  ");
    monitor_cb(cb_data);
    free(var_list);
    return 0;
}

static PLI_INT32 monitor_calltf(char*xx)
{
    vpiHandle sys  = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, sys);

    vpiHandle*var_list = 0;

    vpiHandle   handle;
    PLI_INT32   index;
    s_vpi_time  time;
    s_vpi_time  delay;
    s_vpi_value value;
    s_cb_data   cb_data;

    (void)xx;  /* Parameter is not used. */

    delay.type = TIME_TYPE;
    delay.low  = 0;
    delay.high = 0;
    delay.real = 0.0;

    assert(argv);

    handle = vpi_scan(argv);
    assert(handle && (vpi_get(vpiType, handle) == vpiConstant));
#ifdef TEST_SCALED_TIME
    value.format = vpiRealVal;
    vpi_get_value(handle, &value);
    delay.real = value.value.real;
#else
    value.format = vpiIntVal;
    vpi_get_value(handle, &value);
    delay.low = value.value.integer;
#endif

    index = 0;
    while ((handle = vpi_scan(argv))) {
        assert(vpi_get(vpiType, handle) == vpiReg);
        var_list = realloc(var_list, (index + 1) * sizeof(vpiHandle));
        var_list[index++] = handle;
    }
    var_list = realloc(var_list, (index + 1) * sizeof(vpiHandle));
    var_list[index++] = 0;

    assert(index > 0);

    time.type = TIME_TYPE;
    vpi_get_time(var_list[0], &time);
    // cbAtStartOfSimTime and cbAtEndOfSimTime want an absolute time.
    // We know the test is short, so can ignore the high part.
#ifdef TEST_SCALED_TIME
    time.real += delay.real;
#else
    time.low += delay.low;
#endif

    memset(&cb_data, 0, sizeof(cb_data));

    cb_data.reason    = cbAtStartOfSimTime;
    cb_data.cb_rtn    = monitor_cb_start;
    cb_data.user_data = (char*)var_list;
    cb_data.obj       = var_list[0];
    cb_data.time      = &time;
    vpi_register_cb(&cb_data);

    cb_data.reason    = cbAfterDelay;
    cb_data.cb_rtn    = monitor_cb_delay;
    cb_data.user_data = (char*)var_list;
    cb_data.obj       = var_list[0];
    cb_data.time      = &delay;
    vpi_register_cb(&cb_data);

    // Add cbNBASynch when we support it

    cb_data.reason    = cbReadWriteSynch;
    cb_data.cb_rtn    = monitor_cb_synch;
    cb_data.user_data = (char*)var_list;
    cb_data.obj       = var_list[0];
    cb_data.time      = &delay;
    vpi_register_cb(&cb_data);

    cb_data.reason    = cbAtEndOfSimTime;
    cb_data.cb_rtn    = monitor_cb_end;
    cb_data.user_data = (char*)var_list;
    cb_data.obj       = var_list[0];
    cb_data.time      = &time;
    vpi_register_cb(&cb_data);

    memset(&cb_data, 0, sizeof(cb_data));
    memset(&time   , 0, sizeof(time));

    return 0;
}

static void monitor_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$monitor_time_slot";
    tf_data.calltf    = monitor_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
    monitor_register,
    0
};
