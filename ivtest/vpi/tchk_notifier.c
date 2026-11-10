/* Independent four-state notifier, callback identity and UDP consequence audit.
 * IEEE 1364-2005 15.5 / Table 15-13 permits either binary result from X.
 * Events are counted separately from final values and procedural wakeups. */
#include <vpi_user.h>
#include <stdio.h>
#include <string.h>

static unsigned failed;

static void fail(const char *message)
{
    failed = 1;
    vpi_printf("FAIL notifier audit: %s\n", message);
    vpi_control(vpiFinish, 1);
}

#ifdef vpiTchk
static const char *scope_names[6] = {
    "tb.n0", "tb.n1", "tb.nx", "tb.nz", "tb.shared", "tb.omitted"
};
static const char *q_names[5] = {
    "tb.q0", "tb.q1", "tb.qx", "tb.qz", "tb.qs"
};
static const PLI_INT32 initial_values[5] = {vpi0,vpi1,vpiX,vpiZ,vpi0};
static const unsigned per_slot[5] = {1,1,1,0,2};
static const unsigned long event_times[2] = {15,105};
static const double expected_limits[7] = {10,10,10,10,10,12,10};
static const unsigned object_scope[7] = {0,1,2,3,4,4,5};
static vpiHandle objects[7], notifiers[5];
static unsigned violations[7], changes[5], indices[5];
static PLI_INT32 previous[5];
static const char *kind;
static unsigned checkpoints, armed;
static PLI_INT32 before[5], initial_changes[5][8];
static unsigned initial_count[5];
static unsigned long long initial_times[5][8];
static unsigned initial_violations;
/* These two fixtures declare no timing violation before their #1 audit.
 * Capture initialization separately, rather than adopting the value at #1. */

static unsigned long long now(void)
{
    s_vpi_time time = {0};
    time.type = vpiSimTime;
    vpi_get_time(NULL, &time);
    return ((unsigned long long)time.high << 32) | time.low;
}

/* Snapshot the requested callback fields before independent VPI reads. */
static int payload(p_cb_data cb, PLI_INT32 reason, unsigned long long *time,
                   PLI_INT32 *value)
{
    s_vpi_time t = {0}; s_vpi_value v = {0};
    PLI_INT32 actual_reason = cb->reason; vpiHandle object = cb->obj;
    int ht = cb->time != NULL, hv = cb->value != NULL;
    if (ht) t = *cb->time;
    if (hv) v = *cb->value;
    if (actual_reason != reason || !ht || t.type != vpiSimTime) {
        fail("callback reason/time payload"); return 0;
    }
    *time = ((unsigned long long)t.high << 32) | t.low;
    if (*time != now()) { fail("callback time payload mismatch"); return 0; }
    if (value) {
        s_vpi_value actual = {0}; actual.format = vpiScalarVal;
        if (!hv || v.format != vpiScalarVal) { fail("callback value payload format"); return 0; }
        *value = v.value.scalar; vpi_get_value(object, &actual);
        if (*value != actual.value.scalar) { fail("callback value payload mismatch"); return 0; }
    }
    return 1;
}

static char bit_char(PLI_INT32 value)
{
    switch (value) {
        case vpi0: return '0';
        case vpi1: return '1';
        case vpiZ: return 'z';
        default: return 'x';
    }
}

static PLI_INT32 read_scalar(vpiHandle signal)
{
    s_vpi_value value = {0};
    value.format = vpiScalarVal;
    vpi_get_value(signal, &value);
    return value.value.scalar;
}

static PLI_INT32 argument(void)
{
    vpiHandle call = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle iter = vpi_iterate(vpiArgument, call);
    vpiHandle arg;
    s_vpi_value value = {0};
    if (!iter || !(arg = vpi_scan(iter))) {
        fail("missing task argument"); return -1;
    }
    value.format = vpiIntVal;
    vpi_get_value(arg, &value);
    if (vpi_scan(iter)) { fail("extra task argument"); return -1; }
    return value.value.integer;
}

static PLI_INT32 value_changed(p_cb_data cb)
{
    unsigned index = *(unsigned *)cb->user_data;
    vpiHandle object = cb->obj;
    PLI_INT32 current; unsigned long long time;
    unsigned slot;
    if (!payload(cb, cbValueChange, &time, &current)) return 0;
    if (index >= 5 || object != notifiers[index]) {
        fail("value callback identity"); return 0;
    }
    if (!armed) {
        unsigned n = initial_count[index];
        if (n == 8) { fail("too many initial changes"); return 0; }
        initial_changes[index][n] = current; initial_times[index][n] = time;
        ++initial_count[index]; return 0;
    }
    if (!per_slot[index] || changes[index] >= 2*per_slot[index]) {
        fail("extra notifier value change"); return 0;
    }
    slot = changes[index]/per_slot[index];
    if (time != event_times[slot]) {
        fail("notifier value-change time"); return 0;
    }
    if (current != vpi0 && current != vpi1) {
        fail("notifier did not resolve to a binary value"); return 0;
    }
    if ((previous[index] == vpi0 && current != vpi1) ||
        (previous[index] == vpi1 && current != vpi0) ||
        previous[index] == vpiZ) {
        fail("notifier transition table"); return 0;
    }
    vpi_printf("NOTIFIER_CHANGE %s %s %llu %c->%c\n", kind,
               scope_names[index], time, bit_char(previous[index]),
               bit_char(current));
    previous[index] = current;
    ++changes[index];
    return 0;
}

static PLI_INT32 timing_violation(p_cb_data cb)
{
    unsigned index; vpiHandle object = cb->obj; unsigned long long time;
    if (!payload(cb, cbTchkViolation, &time, NULL)) return 0;
    if (!armed) { ++initial_violations; return 0; }
    for (index=0; index<7; ++index) if (objects[index] == object) break;
    if (index == 7 || violations[index] >= 2) {
        fail("unknown or extra timing callback"); return 0;
    }
    if (time != event_times[violations[index]]) {
        fail("timing callback time"); return 0;
    }
    vpi_printf("NOTIFIER_TCHK %s %s limit=%g time=%llu\n", kind,
               scope_names[object_scope[index]], expected_limits[index], time);
    ++violations[index];
    return 0;
}

static PLI_INT32 observe_start(p_cb_data unused)
{
    PLI_INT32 mode;
    vpiHandle first_scope = vpi_handle_by_name("tb.n0", NULL);
    vpiHandle first_iter = first_scope ? vpi_iterate(vpiTchk, first_scope) : NULL;
    vpiHandle first = first_iter ? vpi_scan(first_iter) : NULL;
    unsigned s;
    if (!first) { fail("missing initial timing object"); return 0; }
    mode = vpi_get(vpiTchkType, first) == vpiPeriod;
    vpi_free_object(first_iter);
    s_cb_data cb = {0};
    s_vpi_time callback_time = {0};
    (void)unused;
    if (mode != 0 && mode != 1) { fail("unknown audit mode"); return 0; }
    kind = mode ? "PERIOD" : "WIDTH";
    for (s=0; s<6; ++s) {
        vpiHandle scope = vpi_handle_by_name((PLI_BYTE8 *)scope_names[s], NULL);
        vpiHandle iter, object;
        unsigned count = 0;
        if (!scope) { fail("missing scope"); return 0; }
        iter = vpi_iterate(vpiTchk, scope);
        if (!iter) { fail("missing typed timing check"); return 0; }
        while ((object = vpi_scan(iter))) {
            vpiHandle notifier = vpi_handle(vpiTchkNotifier, object);
            s_vpi_time limit = {0};
            s_vpi_delay delay = {0};
            unsigned index = s < 4 ? s : (s == 4 ? 4 : 6);
            ++count;
            if (vpi_get(vpiTchkType, object) != (mode ? vpiPeriod : vpiWidth) ||
                vpi_handle(vpiScope, object) != scope) {
                fail("timing check type or owner"); return 0;
            }
            delay.da = &limit;
            delay.no_of_delays = 1;
            delay.time_type = vpiScaledRealTime;
            vpi_get_delays(object, &delay);
            if (s == 4 && limit.real == 12) index = 5;
            if (limit.real != expected_limits[index] || objects[index]) {
                fail("timing check limit or duplicate object"); return 0;
            }
            objects[index] = object;
            if (s == 5) {
                if (notifier) { fail("fabricated omitted notifier"); return 0; }
            } else {
                if (!notifier || strcmp(vpi_get_str(vpiName, notifier), "notifier") ||
                    (notifiers[s] && notifiers[s] != notifier)) {
                    fail("notifier binding"); return 0;
                }
                notifiers[s] = notifier;
            }
        }
        if (count != (s == 4 ? 2U : 1U)) {
            fail("timing check cardinality"); return 0;
        }
    }
    for (s=0; s<5; ++s) {
        s_vpi_value callback_value = {0};
        before[s] = read_scalar(notifiers[s]); previous[s] = initial_values[s];
        indices[s] = s;
        memset(&cb, 0, sizeof(cb));
        cb.reason = cbValueChange;
        cb.obj = notifiers[s];
        cb.cb_rtn = value_changed;
        cb.user_data = (PLI_BYTE8 *)&indices[s];
        callback_time.type = vpiSimTime;
        cb.time = &callback_time;
        callback_value.format = vpiScalarVal; cb.value = &callback_value;
        if (!vpi_register_cb(&cb)) { fail("value callback registration"); return 0; }
    }
    memset(&cb, 0, sizeof(cb));
    cb.reason = cbTchkViolation;
    cb.cb_rtn = timing_violation;
    callback_time.type = vpiSimTime;
    cb.time = &callback_time;
    if (!vpi_register_cb(&cb)) { fail("timing callback registration"); return 0; }
    return 0;
}

static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    PLI_INT32 mode = argument(); unsigned s; (void)unused;
    if (armed || (mode != 0 && mode != 1) || strcmp(kind, mode ? "PERIOD" : "WIDTH")) {
        fail("audit mode or duplicate start"); return 0;
    }
    if (initial_violations) { fail("unexpected startup timing callback"); return 0; }
    for (s=0; s<5; ++s) {
        vpiHandle q = vpi_handle_by_name((PLI_BYTE8 *)q_names[s], NULL);
        unsigned expected = before[s] == initial_values[s] ? 0 : 1;
        if (initial_count[s] != expected ||
            (expected && (initial_times[s][0] != 0 || initial_changes[s][0] != initial_values[s])) ||
            read_scalar(notifiers[s]) != initial_values[s] || !q || read_scalar(q) != vpi0) {
            fail("initial notifier history or UDP value"); return 0;
        }
    }
    armed = 1;
    vpi_printf("NOTIFIER_OBJECTS %s 7\n", kind);
    return 0;
}

static PLI_INT32 checkpoint(PLI_BYTE8 *unused)
{
    PLI_INT32 phase = argument();
    unsigned s;
    (void)unused;
    if (phase != (PLI_INT32)++checkpoints || phase < 1 || phase > 2 ||
        now() != event_times[phase-1]+1) {
        fail("checkpoint sequence"); return 0;
    }
    for (s=0; s<7; ++s) if (violations[s] != (unsigned)phase) {
        fail("missing timing callback"); return 0;
    }
    for (s=0; s<5; ++s) {
        vpiHandle q = vpi_handle_by_name((PLI_BYTE8 *)q_names[s], NULL);
        if (changes[s] != (unsigned)phase*per_slot[s] ||
            read_scalar(notifiers[s]) != previous[s]) {
            fail("notifier history count or current value"); return 0;
        }
        if (!q || read_scalar(q) != (s == 3 ? vpi0 : vpiX)) {
            fail("UDP consequence"); return 0;
        }
    }
    vpi_printf("NOTIFIER_CHECKPOINT %s %d callbacks=%u changes=%u UDP=x,x,x,0,x\n",
               kind, phase, 7*phase, 5*phase);
    if (phase == 2 && !failed) vpi_printf("PASSED\n");
    return 0;
}
#else
static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    (void)unused; fail("typed timing check API unavailable"); return 0;
}
static PLI_INT32 checkpoint(PLI_BYTE8 *unused)
{
    (void)unused; fail("typed timing check API unavailable"); return 0;
}
#endif

static void register_all(void)
{
    s_vpi_systf_data tf = {0};
#ifdef vpiTchk
    s_cb_data cb = {0}; cb.reason = cbStartOfSimulation; cb.cb_rtn = observe_start;
    if (!vpi_register_cb(&cb)) fail("startup registration");
#endif
    tf.type = vpiSysTask;
    tf.tfname = "$notifier_audit";
    tf.calltf = audit;
    vpi_register_systf(&tf);
    tf.tfname = "$notifier_checkpoint";
    tf.calltf = checkpoint;
    vpi_register_systf(&tf);
}
void (*vlog_startup_routines[])(void) = {register_all,0};
