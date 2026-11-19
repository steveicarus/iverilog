/* Passive audit of the proposed qualified-history WIDTH/PERIOD behavior.
 * Each fixture supplies independently derived times. Actual notifier changes
 * and cbTchkViolation callbacks are checked separately; final parity is not
 * an acceptance criterion. No PDK or generated VVP records are used. */
#include <vpi_user.h>
#include <stdio.h>
#include <string.h>

static unsigned failed;

static void fail(const char *reason)
{
    failed = 1;
    vpi_printf("FAILED conditional pulse: %s\n", reason);
    vpi_control(vpiFinish, 1);
}

#ifdef vpiTchk
#define MAX_OWNERS 24
#define MAX_EVENTS 16
struct watch {
    vpiHandle notifier, object;
    char name[256];
    unsigned count, changes, violations;
    unsigned long long times[MAX_EVENTS];
    PLI_INT32 kind;
};
static struct watch owners[MAX_OWNERS];
static unsigned owner_count, callback_registered;
struct startup_notifier {
    vpiHandle signal;
    PLI_INT32 before, values[MAX_EVENTS + 1];
    unsigned count;
    unsigned long long times[MAX_EVENTS + 1];
    struct watch *watch;
};
static struct startup_notifier startup[MAX_OWNERS];
static unsigned startup_count, early_count;
static struct {
    vpiHandle object;
    unsigned long long time;
    unsigned consumed;
} early[MAX_OWNERS * MAX_EVENTS];

static unsigned long long now(void)
{
    s_vpi_time t = {0};
    t.type = vpiSimTime;
    vpi_get_time(NULL, &t);
    return ((unsigned long long)t.high << 32) | t.low;
}

static PLI_INT32 integer(vpiHandle h)
{
    s_vpi_value v = {0};
    v.format = vpiIntVal;
    vpi_get_value(h, &v);
    return v.value.integer;
}

static PLI_INT32 scalar(vpiHandle h)
{
    s_vpi_value v = {0};
    v.format = vpiScalarVal;
    vpi_get_value(h, &v);
    return v.value.scalar;
}

static void notifier_event(struct watch *w, vpiHandle object,
                           unsigned long long time, PLI_INT32 value)
{
    if (object != w->notifier || w->changes >= w->count ||
        time != w->times[w->changes] ||
        value != ((w->changes & 1) ? vpi0 : vpi1)) {
        fail("notifier identity/time/value"); return;
    }
    ++w->changes;
    vpi_printf("PULSE_CHANGE %s %llu %d\n", w->name, time, value == vpi1);
}

static void timing_event(struct watch *w, vpiHandle object,
                         unsigned long long time)
{
    if (w->violations >= w->count || time != w->times[w->violations] ||
        vpi_get(vpiTchkType, object) != w->kind ||
        vpi_handle(vpiTchkNotifier, object) != w->notifier) {
        fail("timing callback identity/time/type"); return;
    }
    ++w->violations;
    vpi_printf("PULSE_TCHK %s %llu\n", w->name, time);
}

static PLI_INT32 changed(p_cb_data cb)
{
    vpiHandle object = cb->obj;
    struct startup_notifier *n = (struct startup_notifier *)cb->user_data;
    unsigned long long time = now();
    PLI_INT32 value = scalar(object);
    if (object != n->signal) { fail("startup notifier identity"); return 0; }
    if (n->watch) notifier_event(n->watch, object, time, value);
    else {
        if (n->count == MAX_EVENTS + 1) { fail("startup notifier capacity"); return 0; }
        n->times[n->count] = time; n->values[n->count++] = value;
    }
    return 0;
}

static PLI_INT32 violation(p_cb_data cb)
{
    vpiHandle object = cb->obj;
    unsigned i;
    unsigned long long time = now();
    for (i = 0; i < owner_count; ++i)
        if (owners[i].object == object) {
            timing_event(&owners[i], object, time); return 0;
        }
    if (early_count == MAX_OWNERS * MAX_EVENTS) {
        fail("early callback capacity"); return 0;
    }
    early[early_count].object = object;
    early[early_count++].time = time;
    return 0;
}

static void observe_scope(vpiHandle scope)
{
    vpiHandle iter = vpi_iterate(vpiTchk, scope), object, child;
    while (iter && (object = vpi_scan(iter))) {
        vpiHandle signal = vpi_handle(vpiTchkNotifier, object);
        struct startup_notifier *n;
        s_cb_data cb = {0}; s_vpi_time time = {0};
        unsigned i;
        if (!signal) continue;
        for (i = 0; i < startup_count; ++i) if (startup[i].signal == signal) break;
        if (i != startup_count) continue;
        if (startup_count == MAX_OWNERS) { fail("startup notifier population"); return; }
        n = &startup[startup_count++]; n->signal = signal; n->before = scalar(signal);
        time.type = vpiSimTime;
        cb.reason = cbValueChange; cb.cb_rtn = changed; cb.obj = signal;
        cb.time = &time; cb.user_data = (PLI_BYTE8 *)n;
        if (!vpi_register_cb(&cb)) { fail("startup notifier registration"); return; }
    }
    iter = vpi_iterate(vpiModule, scope);
    while (iter && (child = vpi_scan(iter))) observe_scope(child);
}

static PLI_INT32 start(p_cb_data unused)
{
    s_cb_data cb = {0};
    vpiHandle iter, scope;
    (void)unused;
    cb.reason = cbTchkViolation;
    cb.cb_rtn = violation;
    callback_registered = vpi_register_cb(&cb) != NULL;
    if (!callback_registered) fail("timing callback registration");
    iter = vpi_iterate(vpiModule, NULL);
    while (iter && (scope = vpi_scan(iter))) observe_scope(scope);
    return 0;
}

static PLI_INT32 watch_call(PLI_BYTE8 *unused)
{
    vpiHandle call = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle args = vpi_iterate(vpiArgument, call), arg, iter, obj, scope;
    struct watch *w;
    unsigned i, matches = 0;
    PLI_INT32 count;
    double expected_limit;
    s_vpi_value value = {0};
    struct startup_notifier *n = NULL;
    const char *name;
    (void)unused;
    if (!args || owner_count >= MAX_OWNERS) { fail("watch arguments/capacity"); return 0; }
    w = &owners[owner_count];
    if (!(w->notifier = vpi_scan(args)) || !(arg = vpi_scan(args))) {
        fail("watch notifier/kind"); return 0;
    }
    w->kind = integer(arg);
    if (w->kind != vpiWidth && w->kind != vpiPeriod) { fail("watch kind"); return 0; }
    if (!(arg = vpi_scan(args))) { fail("watch limit"); return 0; }
    value.format = vpiRealVal; vpi_get_value(arg, &value); expected_limit = value.value.real;
    if (!(arg = vpi_scan(args))) { fail("watch count"); return 0; }
    count = integer(arg);
    if (count < 0 || count > MAX_EVENTS) { fail("watch event count"); return 0; }
    w->count = (unsigned)count;
    for (i = 0; i < w->count; ++i) {
        if (!(arg = vpi_scan(args)) || integer(arg) < 0) { fail("watch event time"); return 0; }
        w->times[i] = (unsigned long long)integer(arg);
    }
    if (vpi_scan(args)) { fail("extra watch argument"); return 0; }
    for (i = 0; i < owner_count; ++i)
        if (owners[i].notifier == w->notifier) { fail("duplicate notifier watch"); return 0; }
    name = vpi_get_str(vpiFullName, w->notifier);
    if (!name || strlen(name) >= sizeof(w->name)) { fail("notifier name"); return 0; }
    memcpy(w->name, name, strlen(name) + 1);
    scope = vpi_handle(vpiScope, w->notifier);
    if (!scope || !(iter = vpi_iterate(vpiTchk, scope))) { fail("missing timing objects"); return 0; }
    while ((obj = vpi_scan(iter))) {
        s_vpi_time delay_value = {0};
        s_vpi_delay delay = {0};
        if (vpi_handle(vpiTchkNotifier, obj) != w->notifier) continue;
        ++matches;
        if (vpi_get(vpiTchkType, obj) != w->kind || vpi_handle(vpiScope, obj) != scope) {
            fail("object type/scope"); return 0;
        }
        delay.da = &delay_value; delay.no_of_delays = 1; delay.time_type = vpiScaledRealTime;
        vpi_get_delays(obj, &delay);
        if (delay_value.real != expected_limit) { fail("effective limit readback"); return 0; }
        w->object = obj;
    }
    if (matches != 1) { fail("timing object cardinality"); return 0; }
    for (i = 0; i < startup_count; ++i)
        if (startup[i].signal == w->notifier) { n = &startup[i]; break; }
    if (!n) { fail("notifier not observed from startup"); return 0; }
    i = 0;
    /* Existing fixtures declare zero independently. Ignore only its actual
     * initialization delivery, never an arbitrary value sampled at #1. */
    if (n->before != vpi0) {
        if (!n->count || n->times[0] != 0 || n->values[0] != vpi0) {
            fail("declared zero initialization missing"); return 0;
        }
        i = 1;
    }
    for (; i < n->count; ++i)
        notifier_event(w, n->signal, n->times[i], n->values[i]);
    n->watch = w;
    for (i = 0; i < early_count; ++i) if (early[i].object == w->object) {
        timing_event(w, early[i].object, early[i].time); early[i].consumed = 1;
    }
    if (scalar(w->notifier) != ((w->changes & 1) ? vpi1 : vpi0)) {
        fail("startup notifier final value"); return 0;
    }
    ++owner_count;
    return 0;
}

static PLI_INT32 complete_call(PLI_BYTE8 *unused)
{
    vpiHandle args = vpi_iterate(vpiArgument, vpi_handle(vpiSysTfCall, NULL));
    vpiHandle arg;
    unsigned i;
    PLI_INT32 expected;
    (void)unused;
    if (!args || !(arg = vpi_scan(args))) { fail("completion owner count"); return 0; }
    expected = integer(arg);
    if (vpi_scan(args) || expected <= 0 || owner_count != (unsigned)expected || !callback_registered) {
        fail("completion population"); return 0;
    }
    for (i = 0; i < early_count; ++i) if (!early[i].consumed) {
        fail("callback from unregistered timing object"); return 0;
    }
    for (i = 0; i < owner_count; ++i) {
        struct watch *w = &owners[i];
        vpiHandle scope = vpi_handle(vpiScope, w->notifier);
        vpiHandle iter = vpi_iterate(vpiTchk, scope), object;
        if (w->changes != w->count || w->violations != w->count ||
            scalar(w->notifier) != ((w->count & 1) ? vpi1 : vpi0)) {
            fail("missing callback or final notifier mismatch"); return 0;
        }
        while (iter && (object = vpi_scan(iter))) {
            unsigned j;
            for (j = 0; j < owner_count; ++j)
                if (owners[j].object == object) break;
            if (j == owner_count) { fail("unobserved timing object in watched scope"); return 0; }
        }
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
    tf.type = vpiSysTask; tf.tfname = "$pulse_watch"; tf.calltf = watch_call;
    vpi_register_systf(&tf);
    tf.tfname = "$pulse_complete"; tf.calltf = complete_call;
    vpi_register_systf(&tf);
    cb.reason = cbStartOfSimulation; cb.cb_rtn = start;
    vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {register_all, 0};
