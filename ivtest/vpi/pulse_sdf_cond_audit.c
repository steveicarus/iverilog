/* Copyright (c) 2026 Stephen Williams (steve@icarus.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Reuse the passive pulse oracle; inspect standard condition metadata. */
#include <vpi_user.h>
#define vlog_startup_routines inherited_startup_routines
#include "pulse_conditional.c"
#undef vlog_startup_routines

static vpiHandle owned_object(vpiHandle notifier)
{
    vpiHandle scope = vpi_handle(vpiScope, notifier);
    vpiHandle iter = vpi_iterate(vpiTchk, scope), object, match = NULL;
    unsigned count = 0;
    while (iter && (object = vpi_scan(iter))) {
        if (vpi_handle(vpiTchkNotifier, object) == notifier) {
            match = object; ++count;
        }
    }
    if (count != 1) { fail("condition object ownership"); return NULL; }
    return match;
}

static int operands(vpiHandle iterator, vpiHandle *left, vpiHandle *right)
{
    vpiHandle operand; unsigned count = 0;
    *left = *right = NULL;
    if (!iterator) return 0;
    while (count < 3 && (operand = vpi_scan(iterator))) {
        if (count == 0) *left = operand;
        if (count == 1) *right = operand;
        ++count;
    }
    if (count == 3) vpi_free_object(iterator);
    return count == 2;
}

static PLI_INT32 condition_call(PLI_BYTE8 *unused)
{
    vpiHandle args = vpi_iterate(vpiArgument, vpi_handle(vpiSysTfCall, NULL));
    vpiHandle notifier = vpi_scan(args), form_arg = vpi_scan(args);
    vpiHandle signal = vpi_scan(args), wanted_arg = vpi_scan(args);
    vpiHandle object, term, expr, lhs, rhs, lhs2, rhs2, it, it2;
    s_vpi_value value = {0};
    int form, wanted;
    (void)unused;
    if (!notifier || !form_arg || !signal || !wanted_arg || vpi_scan(args)) {
        fail("condition arguments"); return 0;
    }
    form = integer(form_arg); wanted = integer(wanted_arg);
    object = owned_object(notifier);
    if (!object) return 0;
    term = vpi_handle(vpiTchkRefTerm, object);
    expr = vpi_handle(vpiCondition, term);
    if (!term || term != vpi_handle(vpiTchkRefTerm, object) ||
        expr != vpi_handle(vpiCondition, term)) {
        fail("condition persistent terminal/handle"); return 0;
    }
    if (form == 0) {
        if (expr) fail("unexpected condition");
    } else if (form == 1) {
        if (expr != signal || vpi_get(vpiSize, expr) != 1)
            fail("bare condition identity");
    } else if (form == 2) {
        if (!expr || vpi_get(vpiType, expr) != vpiOperation ||
            vpi_get(vpiOpType, expr) != vpiCaseEqOp || vpi_get(vpiSize, expr) != 1 ||
            !operands(vpi_iterate(vpiOperand, expr), &lhs, &rhs) ||
            lhs != signal || !rhs || vpi_get(vpiType, rhs) != vpiConstant ||
            vpi_get(vpiSize, rhs) != 1 || vpi_get(vpiSigned, rhs)) {
            fail("case condition shape/operands"); return 0;
        }
        it = vpi_iterate(vpiOperand, expr);
        it2 = vpi_iterate(vpiOperand, expr);
        if (!it || !it2) { fail("condition repeated iterators"); return 0; }
        vpi_free_object(it);
        if (!operands(it2, &lhs2, &rhs2) || lhs != lhs2 || rhs != rhs2 ||
            vpi_handle(vpiCondition, term) != expr) {
            fail("condition operand lifetime"); return 0;
        }
        value.format = vpiScalarVal; vpi_get_value(rhs, &value);
        if (value.value.scalar != (wanted ? vpi1 : vpi0)) {
            fail("condition constant value"); return 0;
        }
        vpi_get_value(expr, &value);
        if (value.value.scalar != (scalar(signal) == (wanted ? vpi1 : vpi0) ? vpi1 : vpi0))
            fail("condition expression value");
    } else fail("unknown expected condition form");
    return 0;
}

static PLI_INT32 limit_call(PLI_BYTE8 *unused)
{
    vpiHandle args = vpi_iterate(vpiArgument, vpi_handle(vpiSysTfCall, NULL));
    vpiHandle notifier = vpi_scan(args), kind = vpi_scan(args);
    vpiHandle expected = vpi_scan(args), object;
    s_vpi_time time = {0}; s_vpi_delay delay = {0};
    s_vpi_value value = {0};
    (void)unused;
    if (!notifier || !kind || !expected || vpi_scan(args)) {
        fail("intermediate limit arguments"); return 0;
    }
    object = owned_object(notifier);
    if (!object || vpi_get(vpiTchkType, object) != integer(kind)) {
        fail("intermediate object kind"); return 0;
    }
    value.format = vpiRealVal; vpi_get_value(expected, &value);
    delay.da = &time; delay.no_of_delays = 1; delay.time_type = vpiScaledRealTime;
    vpi_get_delays(object, &delay);
    if (time.real != value.value.real) fail("intermediate limit readback");
    return 0;
}

static void register_sdf_audit(void)
{
    s_vpi_systf_data tf = {0};
    tf.type = vpiSysTask; tf.tfname = "$pulse_sdf_condition"; tf.calltf = condition_call;
    vpi_register_systf(&tf);
    tf.tfname = "$pulse_sdf_limit"; tf.calltf = limit_call;
    vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {register_all, register_sdf_audit, 0};
