/* Observe the actual UDP output, without deriving it from the public output.
 * Expected events come from the authored UDP tables and isolated stimulus.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vpi_user.h"

/* Let the unchanged baseline reach the missing-relation assertion. */
#ifndef _vpiModPathData
#define _vpiModPathData 0x1000006
#endif

enum { OWNERS = 2, EVENTS = 7, HISTORY = 32 };
static const unsigned sample_times[] = {
    0,80,101,104,112,202,206,212,352,362,401,410,414,501,514,701,714,801,814,901,912
};
struct stream {
    vpiHandle signal;
    vpiHandle callback;
    unsigned count;
    unsigned owner;
    unsigned is_pre;
    unsigned history_count;
    unsigned actual_times[HISTORY];
    char actual_values[HISTORY];
};
struct owner {
    const char *type;
    unsigned rise, fall, unknown;
    char initial;
    unsigned events;
    unsigned source_times[EVENTS];
    char source_values[EVENTS];
    unsigned paths;
    vpiHandle scope, path;
    struct stream pre, post;
};
static struct owner owners[OWNERS] = {
    {"prepath_mux2",3,5,9,'0',7,
     {100,200,400,500,700,800,900},{'1','0','x','1','x','1','0'},
     0,0,0,{0,0,0,0,1,0,{0},{0}},{0,0,0,0,0,0,{0},{0}}},
    {"prepath_mux4",7,11,13,'1',5,
     {100,200,350,700,800,0,0},{'0','1','0','x','0',0,0},
     0,0,0,{0,0,0,1,1,0,{0},{0}},{0,0,0,1,0,0,{0},{0}}}
};
static unsigned enabled, failures, excluded, samples, removed_calls;
static vpiHandle removed_callback;

static unsigned now(void)
{
    s_vpi_time time = {0};
    time.type = vpiSimTime;
    vpi_get_time(NULL, &time);
    if (time.high) { vpi_printf("FAIL time exceeds fixture range\n"); ++failures; }
    return time.low;
}

static char value(vpiHandle handle)
{
    s_vpi_value data = {0};
    data.format = vpiBinStrVal;
    vpi_get_value(handle, &data);
    return data.value.str[0];
}

static void check(int condition, const char *message)
{
    if (!condition) { vpi_printf("FAIL %s\n", message); ++failures; }
}

static unsigned delivery(const struct owner *owner, unsigned event)
{
    unsigned delay;
    char before = event ? owner->source_values[event-1] : owner->initial;
    char after = owner->source_values[event];
    if (before == 'x' || after == 'x') delay = owner->unknown;
    else delay = after == '1' ? owner->rise : owner->fall;
    return owner->source_times[event] + delay;
}

static char expected(const struct owner *owner, unsigned time, int is_pre)
{
    unsigned idx;
    char result = owner->initial;
    if (!is_pre && time < owner->unknown) result = 'x';
    for (idx = 0; idx < owner->events; ++idx) {
        unsigned change = is_pre ? owner->source_times[idx] : delivery(owner, idx);
        if (change <= time) result = owner->source_values[idx];
    }
    return result;
}

static PLI_INT32 changed(p_cb_data data)
{
    struct stream *stream = (struct stream *)data->user_data;
    struct owner *owner = &owners[stream->owner];
    vpiHandle reported_object = data->obj;
    s_vpi_time reported_time = {0};
    int valid_time = data->time != NULL;
    int scalar_value = data->value && data->value->format == vpiBinStrVal &&
                       data->value->value.str && data->value->value.str[0] &&
                       data->value->value.str[1] == '\0';
    char reported_value = scalar_value ? data->value->value.str[0] : '\0';
    unsigned time, idx = stream->count, payload_time;
    char actual, payload_value;
    /* Copy the payload before VPI reads can reuse its scratch storage. */
    if (valid_time) reported_time = *data->time;
    time = now();
    actual = value(stream->signal);
    payload_time = time;
    payload_value = actual;
    /* Initialization retains its snapshot/history checks. Subsequent callback
     * payloads must independently match the authored event schedule. */
    if (time >= 80 && idx < owner->events) {
        payload_time = stream->is_pre ? owner->source_times[idx] : delivery(owner, idx);
        payload_value = owner->source_values[idx];
    }
    check(reported_object == stream->signal, "callback payload object");
    check(valid_time && reported_time.type == vpiSimTime &&
          reported_time.high == 0 && reported_time.low == payload_time,
          "callback payload time");
    check(scalar_value && reported_value == payload_value, "callback payload value");
    if (stream->history_count < HISTORY) {
        stream->actual_times[stream->history_count] = time;
        stream->actual_values[stream->history_count] = actual;
        ++stream->history_count;
    } else {
        check(0, "callback history overflow");
    }
    if (time < 80) return 0; /* Initialization is checked at read-only epochs. */
    if (idx >= owner->events) {
        check(0, "unexpected additional callback");
        return 0;
    }
    check(time == (stream->is_pre ? owner->source_times[idx] : delivery(owner, idx)),
          "callback time");
    check(actual == owner->source_values[idx], "callback value");
    ++stream->count;
    // Do not assert cross-owner callback ordering at coincident source events.
    // Public traces are printed separately at finalization in stable owner order.
    return 0;
}

static PLI_INT32 removed(p_cb_data unused)
{
    (void)unused;
    if (now() >= 80) ++removed_calls;
    return 0;
}

static vpiHandle callback(vpiHandle signal, PLI_INT32 (*fn)(p_cb_data),
                          void *context)
{
    s_cb_data cb = {0};
    s_vpi_time time = {0};
    s_vpi_value val = {0};
    time.type = vpiSimTime;
    val.format = vpiBinStrVal;
    cb.reason = cbValueChange;
    cb.cb_rtn = fn;
    cb.obj = signal;
    cb.time = &time;
    cb.value = &val;
    cb.user_data = (PLI_BYTE8 *)context;
    return vpi_register_cb(&cb);
}

static PLI_INT32 snapshot(p_cb_data unused)
{
    unsigned idx, time = now();
    (void)unused;
    ++samples;
    for (idx = 0; idx < OWNERS; ++idx) {
        struct owner *owner = &owners[idx];
        if (owner->post.signal)
            check(value(owner->post.signal) == expected(owner, time, 0),
                  "postpath snapshot");
        if (owner->pre.signal)
            check(value(owner->pre.signal) == expected(owner, time, 1),
                  "prepath snapshot");
    }
    if (time == 80 && enabled) {
        if (removed_callback)
            check(vpi_remove_cb(removed_callback) != 0, "callback removal");
        for (idx = 0; idx < OWNERS; ++idx) {
            struct owner *owner = &owners[idx];
            if (!owner->pre.signal) continue;
            // Signal handles are borrowed scope objects: release is non-owning.
            vpi_free_object(owner->pre.signal);
            check(vpi_handle(_vpiModPathData, owner->path) == owner->pre.signal,
                  "scope-owned handle survives free");
            check(value(owner->pre.signal) == owner->initial, "released borrowed handle value");
        }
    }
    return 0;
}

static void inspect(vpiHandle scope)
{
    const char *type = vpi_get_str(vpiDefName, scope);
    struct owner *owner = NULL;
    vpiHandle paths, path, children, child;
    unsigned idx, path_count = 0;
    int ineligible = type && !strncmp(type, "prepath_excluded_", 17);
    int no_path = type && !strcmp(type, "prepath_excluded_no_path");
    for (idx = 0; idx < OWNERS; ++idx)
        if (type && !strcmp(type, owners[idx].type)) owner = &owners[idx];
    if (owner) {
        check(!owner->scope, "one instance of each authored owner type");
        owner->scope = scope;
    }
    if (ineligible) ++excluded;
    paths = vpi_iterate(vpiModPath, scope);
    while (paths && (path = vpi_scan(paths))) {
        vpiHandle pre = vpi_handle(_vpiModPathData, path);
        ++path_count;
        if (owner) {
            vpiHandle terms = vpi_iterate(vpiModPathOut, path);
            vpiHandle term = terms ? vpi_scan(terms) : NULL;
            vpiHandle post = term ? vpi_handle(vpiExpr, term) : NULL;
            if (terms) vpi_free_object(terms);
            check(post != NULL, "existing output expression");
            check((pre != NULL) == (enabled != 0), "prepath relationship availability");
            if (!owner->paths) {
                owner->path = path;
                owner->pre.signal = pre;
                owner->post.signal = post;
            } else {
                check(owner->pre.signal == pre, "one prepath handle for all source paths");
                check(owner->post.signal == post, "one output expression per owner");
            }
            ++owner->paths;
            if (pre) {
                check(vpi_get(vpiType, pre) == vpiNet, "prepath object type");
                check(vpi_get(vpiSize, pre) == 1, "prepath scalar width");
                check(vpi_handle(vpiScope, pre) == scope, "prepath owning scope");
                check(pre != post, "prepath is not the public output");
            }
            if (post) check(vpi_handle(_vpiModPathData, post) == NULL,
                            "relationship absent on ordinary net");
        } else if (ineligible) {
            check(pre == NULL, "ineligible object has no prepath relation");
        }
    }
    if (ineligible)
        check(no_path ? path_count == 0 : path_count != 0,
              "ineligible relation checked on actual path population");
    children = vpi_iterate(vpiModule, scope);
    while (children && (child = vpi_scan(children))) inspect(child);
}

static PLI_INT32 start(p_cb_data unused)
{
    vpiHandle setting = vpi_handle_by_name("test.expect_enable", NULL);
    vpiHandle modules, module;
    s_vpi_value val = {0};
    unsigned idx;
    (void)unused;
    check(setting != NULL, "fixture setting available");
    if (!setting) return 0;
    val.format = vpiIntVal;
    vpi_get_value(setting, &val);
    enabled = val.value.integer;
    modules = vpi_iterate(vpiModule, NULL);
    while (modules && (module = vpi_scan(modules))) inspect(module);
    check(excluded == 5, "all ineligible owner types visited");
    check(owners[0].paths == 3 && owners[1].paths == 6, "nonempty complete path population");
    if (owners[0].pre.signal && owners[1].pre.signal)
        check(owners[0].pre.signal != owners[1].pre.signal, "distinct owners have distinct handles");
    for (idx = 0; idx < OWNERS; ++idx) {
        struct owner *owner = &owners[idx];
        if (owner->pre.signal) {
            check(value(owner->pre.signal) == 'z', "prepath filter initial value");
            owner->pre.callback = callback(owner->pre.signal, changed, &owner->pre);
            check(owner->pre.callback != NULL, "prepath callback registration");
        }
        if (owner->post.signal) {
            check(value(owner->post.signal) == 'z', "postpath filter initial value");
            owner->post.callback = callback(owner->post.signal, changed, &owner->post);
            check(owner->post.callback != NULL, "postpath callback registration");
        }
    }
    if (owners[0].pre.signal)
        removed_callback = callback(owners[0].pre.signal, removed, NULL);
    if (enabled && owners[0].pre.signal)
        check(removed_callback != NULL, "removable callback registration");
    for (idx = 0; idx < sizeof(sample_times)/sizeof(sample_times[0]); ++idx) {
        s_cb_data cb = {0};
        s_vpi_time time = {0};
        time.type = vpiSimTime;
        time.low = sample_times[idx];
        cb.reason = cbReadOnlySynch;
        cb.cb_rtn = snapshot;
        cb.time = &time;
        check(vpi_register_cb(&cb) != NULL, "snapshot registration");
    }
    return 0;
}

static PLI_INT32 finish(p_cb_data unused)
{
    unsigned idx, event;
    (void)unused;
    check(now() == 1000, "terminal time");
    check(samples == sizeof(sample_times)/sizeof(sample_times[0]), "complete snapshots");
    check(removed_calls == 0, "removed callback stays removed");
    for (idx = 0; idx < OWNERS; ++idx) {
        struct owner *owner = &owners[idx];
        check(owner->post.count == owner->events, "complete postpath callbacks");
        check(owner->pre.count == (enabled ? owner->events : 0), "complete prepath callbacks");
        if (enabled && owner->pre.signal) {
            check(owner->pre.history_count == owner->events + 2,
                  "complete initial prepath callback history");
            check(owner->pre.actual_times[0] == 0 && owner->pre.actual_values[0] == 'x',
                  "initial UDP X propagation");
            check(owner->pre.actual_times[1] == 0 && owner->pre.actual_values[1] == owner->initial,
                  "initial UDP functional settlement");
        }
        if (owner->post.signal) check(value(owner->post.signal) == '0', "postpath terminal state");
        if (owner->pre.signal) check(value(owner->pre.signal) == '0', "prepath terminal state");
        for (event = 0; event < owner->post.history_count; ++event)
            vpi_printf("PUBLIC %s %u %u %c\n", owner->type, event,
                       owner->post.actual_times[event], owner->post.actual_values[event]);
        for (event = 0; event < owner->pre.history_count; ++event)
            vpi_printf("PREDATA %s %u %u %c\n", owner->type, event,
                       owner->pre.actual_times[event], owner->pre.actual_values[event]);
    }
    if (!failures) vpi_printf("PASS udp_prepath enabled=%u\n", enabled);
    else vpi_printf("FAIL udp_prepath errors=%u\n", failures);
    return 0;
}

static void register_test(void)
{
    s_cb_data cb = {0};
    cb.reason = cbEndOfCompile;
    cb.cb_rtn = start;
    vpi_register_cb(&cb);
    cb.reason = cbEndOfSimulation;
    cb.cb_rtn = finish;
    vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {register_test, 0};
