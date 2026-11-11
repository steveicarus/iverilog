/* Copyright (c) 2026 Jerry Zhong
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Resolve declared scope and non-scope objects using standard VPI. */
#include <vpi_user.h>
#include <stddef.h>

struct expected_object {
    const char *name;
    PLI_INT32 type;
    const char *owner;
};

static PLI_INT32 inventory(PLI_BYTE8 *unused)
{
    static const struct expected_object expected[] = {
        {"tb.u", vpiModule, "tb"},
#ifdef CLEANUP_GENERATED
        {"tb.u.generated", vpiGenScope, "tb.u"},
#endif
        {"tb.u.plus_two", vpiFunction, "tb.u"},
        {"tb.u.record", vpiTask, "tb.u"},
        {"tb.u.active", vpiNamedBegin, "tb.u"},
        {"tb.u.fork_owner", vpiNamedBegin, "tb.u"},
        {"tb.u.fork_owner.sleepers", vpiNamedFork, "tb.u.fork_owner"},
        {"tb.u.a", vpiNet, "tb.u"},
        {"tb.u.y", vpiNet, "tb.u"},
        {"tb.u.total", vpiIntegerVar, "tb.u"},
        {"tb.u.memory", vpiMemory, "tb.u"},
        {"tb.u.ratio", vpiRealVar, "tb.u"},
        {"tb.u.tick", vpiNamedEvent, "tb.u"},
        {"tb.u.BIAS", vpiParameter, "tb.u"},
#ifdef CLEANUP_GENERATED
        {"tb.u.generated.marker", vpiReg, "tb.u.generated"}
#else
        {"tb.u.marker", vpiReg, "tb.u"}
#endif
    };
    size_t idx;
    vpiHandle call = vpi_handle(vpiSysTfCall, NULL);
    s_vpi_value result = {0};
    (void)unused;
    result.format = vpiIntVal;
    result.value.integer = 0;
    vpi_put_value(call, &result, NULL, vpiNoDelay);
    for (idx = 0; idx < sizeof expected / sizeof expected[0]; idx += 1) {
        vpiHandle object = vpi_handle_by_name(expected[idx].name, NULL);
        vpiHandle owner = vpi_handle_by_name(expected[idx].owner, NULL);
        if (!object || !owner || vpi_get(vpiType, object) != expected[idx].type ||
            vpi_handle(vpiScope, object) != owner) {
            vpi_printf("FAILED cleanup object %s: expected type %d, got %d\n",
                       expected[idx].name, (int)expected[idx].type,
                       object ? (int)vpi_get(vpiType, object) : -1);
            return 0;
        }
    }
#ifdef CLEANUP_GENERATED
    vpi_printf("CLEANUP_INVENTORY scopes=7 objects=8\n");
#else
    vpi_printf("CLEANUP_INVENTORY scopes=6 objects=8\n");
#endif
    result.value.integer = 1;
    vpi_put_value(call, &result, NULL, vpiNoDelay);
    return 0;
}

static void register_inventory(void)
{
    s_vpi_systf_data data = {0};
    data.type = vpiSysFunc;
    data.sysfunctype = vpiIntFunc;
    data.tfname = "$cleanup_inventory";
    data.calltf = inventory;
    vpi_register_systf(&data);
}

void (*vlog_startup_routines[])(void) = {register_inventory, 0};
