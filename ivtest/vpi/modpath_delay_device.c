#include <string.h>
#include "vpi_user.h"

static vpiHandle public_net, module, device, input_term, output_term;
static unsigned failures, samples;

struct sample {
    unsigned tick;
    int input, output, net;
};

/* The UDP truth table and explicit path delays determine these snapshots. */
static const struct sample schedule[] = {
    { 80, vpi0, vpi0, vpi0},
    {102, vpi1, vpi0, vpi0},
    {104, vpi1, vpi1, vpi0}, /* Device has delivered, but the net is forced. */
    {121, vpi1, vpi1, vpi1},
    {202, vpi0, vpi1, vpi1},
    {206, vpi0, vpi0, vpi0},
    {301, vpiX, vpi0, vpi0},
    {310, vpiX, vpiX, vpiX},
    {401, vpi1, vpiX, vpiX},
    {410, vpi1, vpi1, vpi1},
    {451, vpi0, vpi1, vpi1},
    {458, vpi1, vpi1, vpi1}
};

struct transition {
    unsigned tick;
    int value;
};

struct stream {
    vpiHandle terminal;
    const struct transition *events;
    unsigned expected_count;
    int time_type;
    unsigned count;
};

static const struct transition input_changes[] = {
    {100, vpi1}, {200, vpi0}, {300, vpiX}, {400, vpi1}, {450, vpi0}, {452, vpi1}
};
static const struct transition output_changes[] = {
    {103, vpi1}, {205, vpi0}, {309, vpiX}, {409, vpi1}
};
static struct stream source_events, delivery_events, removed_events;
static vpiHandle removed_callback;

static int check(int condition, const char *label)
{
    if (!condition) {
        vpi_printf("FAIL %s\n", label);
        ++failures;
    }
    return condition;
}

static int scalar_value(vpiHandle object)
{
    s_vpi_value value = {0};
    value.format = vpiScalarVal;
    vpi_get_value(object, &value);
    if (!check(value.format == vpiScalarVal, "scalar value format")) return -1;
    return value.value.scalar;
}

static PLI_INT32 value_changed(p_cb_data cb)
{
    struct stream *stream = (struct stream *)cb->user_data;
    unsigned index = stream->count++;
    vpiHandle object = cb->obj;
    int reason = cb->reason;
    int scalar = cb->value && cb->value->format == vpiScalarVal;
    int value = scalar ? cb->value->value.scalar : -1;
    s_vpi_time reported = {0}, now = {0};
    if (cb->time) reported = *cb->time;
    if (!check(index < stream->expected_count, "unexpected terminal callback")) return 0;

    now.type = vpiSimTime;
    vpi_get_time(NULL, &now);
    unsigned tick = stream->events[index].tick;
    int expected = stream->events[index].value;
    check(reason == cbValueChange && object == stream->terminal, "callback reason and object");
    check(now.high == 0 && now.low == tick, "callback execution time");
    check(reported.type == stream->time_type, "callback time format");
    if (stream->time_type == vpiSimTime) {
        check(reported.high == 0 && reported.low == tick, "callback simulation ticks");
    } else {
        /* The owning module uses ns, while the global tick is ps. */
        double difference = reported.real - tick / 1000.0;
        check(difference > -1e-12 && difference < 1e-12, "callback module-scaled time");
    }
    check(scalar && value == expected, "callback scalar payload");
    check(scalar_value(stream->terminal) == expected, "callback terminal readback");
    return 0;
}

static vpiHandle observe(struct stream *stream, vpiHandle terminal,
                         const struct transition *events, unsigned expected_count, int time_type)
{
    s_vpi_time time = {0};
    s_vpi_value value = {0};
    s_cb_data cb = {0};
    stream->terminal = terminal;
    stream->events = events;
    stream->expected_count = expected_count;
    stream->time_type = time_type;
    time.type = time_type;
    value.format = vpiScalarVal;
    cb.reason = cbValueChange;
    cb.cb_rtn = value_changed;
    cb.obj = terminal;
    cb.time = &time;
    cb.value = &value;
    cb.user_data = (PLI_BYTE8 *)stream;
    vpiHandle handle = vpi_register_cb(&cb);
    check(handle != NULL, "terminal callback registration");
    return handle;
}

static vpiHandle sole_driver(vpiHandle net)
{
    vpiHandle iterator = vpi_iterate(vpiDriver, net);
    if (!check(iterator != NULL, "driver iterator on scalar path output"))
        return NULL;
    vpiHandle first = vpi_scan(iterator);
    if (!check(first != NULL, "driver terminal")) return NULL;
    if (!check(vpi_scan(iterator) == NULL, "one driver terminal across three arcs"))
        vpi_free_object(iterator);
    return first;
}

static void check_type(vpiHandle object, int type, const char *name)
{
    const char *type_name = vpi_get_str(vpiType, object);
    check(type_name && !strcmp(type_name, name), "VPI type name");
    check(vpi_get(vpiType, object) == type, "VPI object type");
    check(vpi_get(vpiDelayType, object) == vpiModPathDelay, "module-path delay type");
}

static unsigned count_paths(vpiHandle owner)
{
    vpiHandle iterator = vpi_iterate(vpiModPath, owner);
    unsigned count = 0;
    while (iterator && vpi_scan(iterator)) ++count;
    return count;
}

static void check_excluded(const char *path, int has_path, int is_vector)
{
    vpiHandle owner = vpi_handle_by_name(path, NULL);
    if (!check(owner != NULL, "excluded module exists")) return;
    vpiHandle net = vpi_handle_by_name("q", owner);
    if (!check(net != NULL, "excluded output exists")) return;
    check(vpi_get(vpiVector, net) == is_vector, "excluded output shape");
    check((count_paths(owner) != 0) == has_path, "excluded module path population");
    vpiHandle iterator = vpi_iterate(vpiDriver, net);
    vpiHandle driver;
    while (iterator && (driver = vpi_scan(iterator)))
        check(vpi_get(vpiType, driver) != vpiDelayTerm,
              "excluded output exposes no delay terminal");
}

static PLI_INT32 sample_values(p_cb_data cb)
{
    const struct sample *expected = (const struct sample *)cb->user_data;
    s_vpi_time time = {0};
    time.type = vpiSimTime;
    vpi_get_time(NULL, &time);
    ++samples;
    check(time.high == 0 && time.low == expected->tick, "snapshot time");
    check(sole_driver(public_net) == output_term, "stable output terminal");
    check(vpi_handle(vpiDelayDevice, input_term) == device, "stable delay device");
    check(vpi_handle(vpiInTerm, device) == input_term, "stable input terminal");
    check(scalar_value(input_term) == expected->input, "delay input value");
    check(scalar_value(output_term) == expected->output, "device output value");
    check(scalar_value(public_net) == expected->net, "public output value");
    if (expected->tick == 80) {
        observe(&source_events, input_term, input_changes,
                sizeof(input_changes)/sizeof(input_changes[0]), vpiSimTime);
        observe(&delivery_events, output_term, output_changes,
                sizeof(output_changes)/sizeof(output_changes[0]), vpiScaledRealTime);
        removed_callback = observe(&removed_events, output_term, output_changes, 1, vpiSimTime);
    } else if (expected->tick == 104) {
        check(removed_events.count == 1, "callback ran before removal");
        if (removed_callback) check(vpi_remove_cb(removed_callback) == 1, "callback removal");
    }
    return 0;
}

static PLI_INT32 discover(p_cb_data ignored)
{
    (void)ignored;
    module = vpi_handle_by_name("test.dut", NULL);
    if (!check(module != NULL, "module owning the scalar path")) return 0;
    public_net = vpi_handle_by_name("q", module);
    if (!check(public_net != NULL, "public scalar path output")) return 0;
    check(vpi_get(vpiType, public_net) == vpiNet && vpi_get(vpiScalar, public_net),
          "scalar net type");
    check(count_paths(module) == 3, "three specify paths");
    output_term = sole_driver(public_net);
    if (!output_term) return 0;
    check_type(output_term, vpiDelayTerm, "vpiDelayTerm");
    device = vpi_handle(vpiDelayDevice, output_term);
    if (!check(device != NULL, "delay device from output terminal")) return 0;
    check_type(device, vpiDelayDevice, "vpiDelayDevice");
    check(vpi_handle(vpiModule, device) == module, "module relation from device");
    check(vpi_handle(vpiOutTerm, device) == output_term, "output terminal from device");
    input_term = vpi_handle(vpiInTerm, device);
    if (!check(input_term && input_term != output_term, "distinct input terminal"))
        return 0;
    check_type(input_term, vpiDelayTerm, "vpiDelayTerm");
    check_excluded("test.ordinary", 0, 0);
    check_excluded("test.packed_owner", 1, 1);
    check_excluded("test.resolved_owner", 1, 0);

    unsigned idx;
    for (idx = 0; idx < sizeof(schedule)/sizeof(schedule[0]); ++idx) {
        s_vpi_time time = {0};
        s_cb_data cb = {0};
        time.type = vpiSimTime;
        time.low = schedule[idx].tick;
        cb.reason = cbReadOnlySynch;
        cb.cb_rtn = sample_values;
        cb.time = &time;
        cb.user_data = (PLI_BYTE8 *)&schedule[idx];
        check(vpi_register_cb(&cb) != NULL, "snapshot registration");
    }
    return 0;
}

static PLI_INT32 finish(p_cb_data ignored)
{
    (void)ignored;
    if (input_term) {
        check(samples == sizeof(schedule)/sizeof(schedule[0]), "complete snapshots");
        check(source_events.count == source_events.expected_count &&
              delivery_events.count == delivery_events.expected_count,
              "complete input and delivery callbacks");
        check(removed_events.count == 1, "removed callback stayed inactive");
    }
    if (!failures) vpi_printf("PASSED\n");
    return 0;
}

static void register_test(void)
{
    s_cb_data cb = {0};
    cb.reason = cbEndOfCompile;
    cb.cb_rtn = discover;
    vpi_register_cb(&cb);
    cb.reason = cbEndOfSimulation;
    cb.cb_rtn = finish;
    vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {register_test, 0};
