/* Copyright (c) 2026 Stephen Williams (steve@icarus.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Reuse the callback/limit oracle; additionally inspect the standard
 * terminal-condition relationship without evaluating it to choose a check. */
/* Preserve the Windows export before renaming the included startup table. */
#include <vpi_user.h>
#define vlog_startup_routines inherited_startup_routines
#include "setup_hold_audit.c"
#undef vlog_startup_routines

static int condition_signal(vpiHandle signal, vpiHandle scope, const char *name)
{
    return signal && vpi_get(vpiSize, signal) == 1 &&
        vpi_handle(vpiScope, signal) == scope &&
        !strcmp(vpi_get_str(vpiName, signal), name);
}

static int condition_operands(vpiHandle iterator, vpiHandle *left, vpiHandle *right)
{
    vpiHandle operand; unsigned count = 0;
    *left = *right = NULL;
    if (!iterator) return 0;
    while (count < 3 && (operand = vpi_scan(iterator))) {
        if (count == 0) *left = operand;
        if (count == 1) *right = operand;
        ++count;
    }
    /* A scan returning NULL frees the iterator. A third operand means that
       iteration has not reached that point; release only this iterator. */
    if (count == 3) vpi_free_object(iterator);
    return count == 2;
}

static PLI_INT32 condition(PLI_BYTE8 *unused)
{
    vpiHandle call = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle args = vpi_iterate(vpiArgument, call), scope, it, obj, match = NULL;
    vpiHandle term, expr, lhs, rhs, lhs2, rhs2, it2;
    char *owner, *notifier, *role, *form, *signal;
    int expected, term_kind; unsigned count = 0;
    s_vpi_value value = {0};
    (void)unused;
    owner = string_arg(args); notifier = string_arg(args); role = string_arg(args);
    form = string_arg(args); signal = string_arg(args); expected = (int)number_arg(args);
    if (failed || vpi_scan(args)) { fail("condition expectation arguments"); return 0; }
    scope = vpi_handle_by_name(owner, NULL);
    term_kind = !strcmp(role, "reference") ? vpiTchkRefTerm :
                !strcmp(role, "data") ? vpiTchkDataTerm : 0;
    if (!scope || !term_kind) { fail("condition scope/role"); return 0; }
    it = vpi_iterate(vpiTchk, scope);
    while (it && (obj = vpi_scan(it))) {
        vpiHandle n = vpi_handle(vpiTchkNotifier, obj);
        if (n && !strcmp(vpi_get_str(vpiName, n), notifier)) { match = obj; ++count; }
    }
    if (count != 1) { fail("condition exact notifier identity"); return 0; }
    term = vpi_handle(term_kind, match);
    expr = vpi_handle(vpiCondition, term);
    if (expr != vpi_handle(vpiCondition, term)) {
        fail("condition persistent handle"); return 0;
    }
    if (!strcmp(form, "none")) {
        if (expr) { fail("unexpected condition"); return 0; }
    } else if (!strcmp(form, "bare")) {
        if (!condition_signal(expr, scope, signal)) {
            fail("bare condition signal identity"); return 0;
        }
    } else if (!strcmp(form, "case")) {
        if (!expr || vpi_get(vpiType, expr) != vpiOperation ||
            vpi_get(vpiOpType, expr) != vpiCaseEqOp || vpi_get(vpiSize, expr) != 1) {
            fail("condition case-equality shape"); return 0;
        }
        it = vpi_iterate(vpiOperand, expr);
        if (!condition_operands(it, &lhs, &rhs) ||
            !condition_signal(lhs, scope, signal) || !rhs ||
            vpi_get(vpiType, rhs) != vpiConstant || vpi_get(vpiSize, rhs) != 1 ||
            vpi_get(vpiSigned, rhs)) {
            fail("condition operand identity/width"); return 0;
        }
        it = vpi_iterate(vpiOperand, expr);
        it2 = vpi_iterate(vpiOperand, expr);
        if (!it || !it2) { fail("condition repeated iterators"); return 0; }
        vpi_free_object(it);
        if (!condition_operands(it2, &lhs2, &rhs2) || lhs2 != lhs || rhs2 != rhs ||
            vpi_handle(vpiCondition, term) != expr) {
            fail("condition persistent operands"); return 0;
        }
        value.format = vpiScalarVal; vpi_get_value(rhs, &value);
        if (value.value.scalar != (expected ? vpi1 : vpi0)) {
            fail("condition constant value"); return 0;
        }
        value.format = vpiScalarVal; vpi_get_value(expr, &value);
        if (value.value.scalar != (scalar(lhs) == (expected ? vpi1 : vpi0) ? vpi1 : vpi0)) {
            fail("condition case-equality value"); return 0;
        }
    } else { fail("condition unknown expected form"); return 0; }
    vpi_printf("COND_OBJECT %s %s %s %s %s %d\n",
               owner, notifier, role, form, signal, expected);
    free(owner); free(notifier); free(role); free(form); free(signal);
    return 0;
}

static void register_condition(void)
{
    s_vpi_systf_data tf = {0};
    tf.type = vpiSysTask; tf.tfname = "$tchk_condition"; tf.calltf = condition;
    vpi_register_systf(&tf);
}
void (*vlog_startup_routines[])(void) = {register_all, register_condition, 0};
