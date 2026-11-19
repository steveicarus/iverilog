/* Passive callback check for conditional WIDTH/PERIOD with omitted notifiers.
 * Expected event times are derived from the HDL stimulus, not runtime verdicts. */
#include <vpi_user.h>
#include <stddef.h>

static unsigned failed, registered, width_count, period_count;
static vpiHandle width_object, period_object, watched_scope;
static unsigned early_count;
static struct { vpiHandle object; unsigned long long time; } early[16];

static void fail(const char *reason)
{
    failed = 1;
    vpi_printf("FAILED conditional no-notifier: %s\n", reason);
    vpi_control(vpiFinish, 1);
}

#ifdef vpiTchk
static unsigned long long now(void)
{
    s_vpi_time t = {0};
    t.type = vpiSimTime;
    vpi_get_time(NULL, &t);
    return ((unsigned long long)t.high << 32) | t.low;
}

static void timing_event(vpiHandle object, unsigned long long time)
{
    static const unsigned long long width_times[] = {25, 45, 95};
    if (vpi_handle(vpiTchkNotifier, object) != NULL ||
        vpi_handle(vpiScope, object) != watched_scope) {
        fail("callback notifier or scope"); return;
    }
    if (object == width_object) {
        if (width_count >= 3 || time != width_times[width_count] ||
            vpi_get(vpiTchkType, object) != vpiWidth) {
            fail("WIDTH callback identity/time/count"); return;
        }
        ++width_count;
    } else if (object == period_object) {
        if (period_count != 0 || time != 40 ||
            vpi_get(vpiTchkType, object) != vpiPeriod) {
            fail("PERIOD callback identity/time/count"); return;
        }
        ++period_count;
    } else {
        fail("unregistered timing callback"); return;
    }
}

static PLI_INT32 violation(p_cb_data cb)
{
    vpiHandle object = cb->obj;
    unsigned long long time = now();
    if (watched_scope) timing_event(object, time);
    else {
        if (early_count == 16) { fail("early callback capacity"); return 0; }
        early[early_count].object = object;
        early[early_count++].time = time;
    }
    return 0;
}

static PLI_INT32 start(p_cb_data unused)
{
    s_cb_data cb = {0}; s_vpi_time time = {0};
    (void)unused;
    cb.reason = cbTchkViolation; cb.cb_rtn = violation;
    time.type = vpiSimTime; cb.time = &time;
    registered = vpi_register_cb(&cb) != NULL;
    if (!registered) fail("callback registration");
    return 0;
}

static PLI_INT32 watch_call(PLI_BYTE8 *unused)
{
    vpiHandle iter, obj, term, reference;
    unsigned count = 0, i;
    (void)unused;
    if (watched_scope || !registered) { fail("duplicate watch or missing startup callback"); return 0; }
    watched_scope = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall, NULL));
    if (!watched_scope || !(iter = vpi_iterate(vpiTchk, watched_scope))) {
        fail("missing timing population"); return 0;
    }
    reference = vpi_handle_by_name("CLK", watched_scope);
    if (!reference) { fail("missing physical reference"); return 0; }
    while ((obj = vpi_scan(iter))) {
        s_vpi_time value = {0};
        s_vpi_delay delay = {0};
        PLI_INT32 kind = vpi_get(vpiTchkType, obj);
        ++count;
        term = vpi_handle(vpiTchkRefTerm, obj);
        if (vpi_handle(vpiTchkNotifier, obj) != NULL ||
            !term || vpi_get(vpiEdge, term) != vpiPosedge ||
            vpi_handle(vpiExpr, term) != reference) {
            fail("object notifier/reference/edge"); return 0;
        }
        delay.da = &value; delay.no_of_delays = 1;
        delay.time_type = vpiScaledRealTime;
        vpi_get_delays(obj, &delay);
        if (kind == vpiWidth && !width_object && value.real == 10) {
            width_object = obj;
        } else if (kind == vpiPeriod && !period_object && value.real == 30) {
            period_object = obj;
        } else { fail("object type/cardinality/limit"); return 0; }
    }
    if (count != 2 || !width_object || !period_object) {
        fail("incomplete timing population"); return 0;
    }
    for (i = 0; i < early_count; ++i) timing_event(early[i].object, early[i].time);
    return 0;
}

static PLI_INT32 complete_call(PLI_BYTE8 *unused)
{
    (void)unused;
    if (!registered || !watched_scope || width_count != 3 || period_count != 1) {
        fail("missing callbacks"); return 0;
    }
    if (!failed) vpi_printf("PASSED\n");
    return 0;
}
#else
static PLI_INT32 start(p_cb_data unused)
{ (void)unused; fail("timing check API unavailable"); return 0; }
static PLI_INT32 watch_call(PLI_BYTE8 *unused)
{ (void)unused; fail("timing check API unavailable"); return 0; }
static PLI_INT32 complete_call(PLI_BYTE8 *unused)
{ (void)unused; fail("timing check API unavailable"); return 0; }
#endif

static void register_all(void)
{
    s_vpi_systf_data tf = {0};
    s_cb_data cb = {0};
    tf.type = vpiSysTask; tf.tfname = "$pulse_no_notifier_watch";
    tf.calltf = watch_call; vpi_register_systf(&tf);
    tf.tfname = "$pulse_no_notifier_complete";
    tf.calltf = complete_call; vpi_register_systf(&tf);
    cb.reason = cbStartOfSimulation; cb.cb_rtn = start;
    if (!vpi_register_cb(&cb)) fail("startup registration");
}
void (*vlog_startup_routines[])(void) = {register_all, 0};
