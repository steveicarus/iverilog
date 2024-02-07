#if     defined(TEST_SCALED_TIME)
#define TIME_TYPE vpiScaledRealTime
#elif   defined(TEST_SIM_TIME)
#define TIME_TYPE vpiSimTime
#else
#define TIME_TYPE vpiSuppressTime
#endif

#include <sv_vpi_user.h>
#include <assert.h>
#include <string.h>

static PLI_INT32 report_change(p_cb_data cb)
{
    vpiHandle handle = (vpiHandle)(cb->user_data);

    s_vpi_time time;

#ifndef TEST_NULL_TIME
    assert(cb->time && (cb->time->type == TIME_TYPE));
#endif
    assert(cb->value);

#if defined(TEST_SCALED_TIME) || defined(TEST_SIM_TIME)
    time = *(cb->time);
#else
    time.type = vpiSimTime;
    vpi_get_time(NULL, &time);
#endif

    switch (cb->value->format) {
      case vpiIntVal:
#if     defined(TEST_SCALED_TIME)
        vpi_printf("At time %f %s = %d\n", time.real, vpi_get_str(vpiName, handle), cb->value->value.integer);
#else
        vpi_printf("At time %d %s = %d\n", time.low,  vpi_get_str(vpiName, handle), cb->value->value.integer);
#endif
        break;
      case vpiRealVal:
#ifdef TEST_SCALED_TIME
        vpi_printf("At time %f %s = %f\n", time.real, vpi_get_str(vpiName, handle), cb->value->value.real);
#else
        vpi_printf("At time %d %s = %f\n", time.low,  vpi_get_str(vpiName, handle), cb->value->value.real);
#endif
        break;
      case vpiSuppressVal:
#ifdef TEST_SCALED_TIME
        vpi_printf("At time %f %s changed\n", time.real, vpi_get_str(vpiName, handle));
#else
        vpi_printf("At time %d %s changed\n", time.low,  vpi_get_str(vpiName, handle));
#endif
        break;
      default:
        vpi_printf("ERROR: unexpected value format %d\n", cb->value->format);
        break;
    }
    return 0;
}

static s_cb_data   cb;
static s_vpi_time  time;
static s_vpi_value value;

static PLI_INT32 my_monitor_calltf(PLI_BYTE8 *xx)
{
    (void)xx;  /* Parameter is not used. */

    vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, sys);

    vpiHandle handle;

    memset(&cb,    0, sizeof(cb));
    memset(&time,  0, sizeof(time));
    memset(&value, 0, sizeof(value));

    time.type = TIME_TYPE;

    cb.reason = cbValueChange;
    cb.cb_rtn = report_change;
    cb.time   = &time;
    cb.value  = &value;

    while ((handle = vpi_scan(argv))) {
        PLI_INT32 type = vpi_get(vpiType, handle);
        cb.obj = handle;
        cb.user_data = (char *)handle;
        switch (type) {
          case vpiNet:
          case vpiReg:
          case vpiIntegerVar:
          case vpiBitVar:
          case vpiByteVar:
          case vpiShortIntVar:
          case vpiIntVar:
          case vpiLongIntVar:
          case vpiPartSelect:
          case vpiMemoryWord:
          case vpiMemory:
            value.format = vpiIntVal;
            break;
          case vpiRealVar:
            value.format = vpiRealVal;
            break;
          case vpiNamedEvent:
            value.format = vpiSuppressVal;
            break;
          default:
            vpi_printf("ERROR: unexpected object type %d\n", type);
            break;
        }
        vpi_register_cb(&cb);
    }

    memset(&cb,    0, sizeof(cb));
    memset(&time,  0, sizeof(time));
    memset(&value, 0, sizeof(value));

    return 0;
}

static void my_monitor_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$my_monitor";
    tf_data.calltf    = my_monitor_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
    my_monitor_register,
    0
};
