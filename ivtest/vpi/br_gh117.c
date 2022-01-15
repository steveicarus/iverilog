#include  <vpi_user.h>

PLI_INT32 readWriteSynch(p_cb_data cb_data)
{
    vpi_printf("Read write  - current time %d\n", cb_data->time->low);
    return 0;
}

PLI_INT32 readOnlySynch(p_cb_data cb_data)
{
    vpi_printf("Read only   - current time %d\n", cb_data->time->low);
    return 0;
}

PLI_INT32 afterDelay(p_cb_data cb_data)
{
    s_cb_data cb_data_s;
    s_vpi_time time_s;
    PLI_INT32 time          = cb_data != NULL ? cb_data->time->low : 0;
    PLI_INT32 period        = 10;

    if (time < 50)
    {
        vpi_printf("After delay - current time %d\n", time);

        // set null pointers
        cb_data_s.obj       = NULL;
        cb_data_s.value     = NULL;
        cb_data_s.user_data = NULL;

        // time
        cb_data_s.time      = &time_s;
        time_s.type         = vpiSimTime;
        time_s.high         = 0;

        // register read_write_synch
        time_s.low          = 1;
        cb_data_s.reason    = cbReadWriteSynch;
        cb_data_s.cb_rtn    = readWriteSynch;
        vpi_free_object(vpi_register_cb(&cb_data_s));

        // register read_only_synch
        time_s.low          = period-1;
        cb_data_s.reason    = cbReadOnlySynch;
        cb_data_s.cb_rtn    = readOnlySynch;
        vpi_free_object(vpi_register_cb(&cb_data_s));

        // register next time step
        time_s.low          = period;
        cb_data_s.reason    = cbAfterDelay;
        cb_data_s.cb_rtn    = afterDelay;
        vpi_free_object(vpi_register_cb(&cb_data_s));
    }
    else
    {
        vpi_printf("Finish sim  - current time %d\n", time);

        // finish simulation
        vpi_control(vpiFinish, 0);
    }
    return 0;
}

void registerCallbacks(void)
{
    vpi_printf("Register callbacks\n");
    afterDelay(NULL);
}

void (*vlog_startup_routines[])(void) = {
    registerCallbacks,
    0
};
