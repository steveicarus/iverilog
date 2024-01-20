#include <vpi_user.h>

static void step(void);

static vpiHandle w;

static PLI_INT32 tick_cb(struct t_cb_data *cb)
{
    static struct t_vpi_value val = { .format = vpiIntVal };
    static int                idx;

    (void)cb;

    ++idx;
    val.value.integer = idx & 1;
    vpi_put_value(w, &val, NULL, vpiNoDelay);
    step();
    return 0;
}

/* Request a callback after a delay. */

static void step(void)
{
    static struct t_vpi_time now = { .type = vpiSimTime, .low = 2 };
    static struct t_cb_data  cbd =
        { .reason = cbAfterDelay, .cb_rtn = tick_cb, .time = &now };

    /* Callback after delay. */

    vpi_register_cb(&cbd);
}

/* Callback function - simulation is starting. */

static PLI_INT32 start_cb(struct t_cb_data *cb)
{
    static struct t_vpi_value val = { .format = vpiIntVal };

    (void)cb;

    w = vpi_handle_by_name("test.w", NULL);
    if (!w)
        vpi_printf("No handle!\n");
    vpi_printf("Got handle for %s\n", vpi_get_str(vpiFullName, w));
    val.value.integer = 0;
    vpi_put_value(w, &val, NULL, vpiNoDelay);
    step();
    return 0;
}

/* VPI initialisation. */

static void start(void)
{
    static struct t_vpi_time now = { .type = vpiSimTime };
    static struct t_cb_data  cbd = { .reason = cbStartOfSimulation,
                                     .time = &now, .cb_rtn = start_cb };

    /* At this point VPI objects do not exist,
     * so request a callback once they do.
     */

    vpi_register_cb(&cbd);
}

/* This is a table of registration functions. This table is the external
 * symbol that the VVP simulator looks for when loading this .vpi module.
 */

void (*vlog_startup_routines[])(void) = {
      start,
      0
};
