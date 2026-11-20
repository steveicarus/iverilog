/* Copyright (c) 2026 Stephen Williams (steve@icarus.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Exact simulation-tick oracle: no floating-point expectation conversion. */
#include <vpi_user.h>
#include <stdio.h>
#include <string.h>

static unsigned failures, sets, checks;

static void fail(const char *message)
{
      failures += 1;
      vpi_printf("FAILED RECREM exact ticks: %s\n", message);
      vpi_control(vpiFinish, 1);
}

static PLI_UINT32 integer_arg(vpiHandle args)
{
      vpiHandle arg = vpi_scan(args);
      s_vpi_value value = {0};
      if (!arg) { fail("missing integer argument"); return 0; }
      value.format = vpiIntVal;
      vpi_get_value(arg, &value);
      return (PLI_UINT32)value.value.integer;
}

static vpiHandle object_arg(vpiHandle args)
{
      vpiHandle arg = vpi_scan(args), scope, iter, check, found = NULL;
      s_vpi_value value = {0};
      unsigned count = 0;
      if (!arg) { fail("missing scope argument"); return NULL; }
      value.format = vpiStringVal;
      vpi_get_value(arg, &value);
      scope = vpi_handle_by_name(value.value.str, NULL);
      if (!scope) { fail("scope missing"); return NULL; }
      iter = vpi_iterate(vpiTchk, scope);
      while (iter && (check = vpi_scan(iter))) {
            if (vpi_get(vpiTchkType, check) != vpiRecrem) {
                  fail("unexpected timing object type"); return NULL;
            }
            found = check;
            count += 1;
      }
      if (count != 1) { fail("expected one RECREM object"); return NULL; }
      return found;
}

static void exact_readback(vpiHandle check, s_vpi_time *expected)
{
      s_vpi_time actual[2] = {{0}, {0}};
      s_vpi_delay delays = {0};
      unsigned idx;
      delays.da = actual;
      delays.no_of_delays = 2;
      delays.time_type = vpiSimTime;
      vpi_get_delays(check, &delays);
      for (idx = 0; idx < 2; idx += 1) {
            if (actual[idx].high != expected[idx].high ||
                actual[idx].low != expected[idx].low) {
                  vpi_printf("RECREM slot %u: expected %08x:%08x, got %08x:%08x\n",
                             idx, (unsigned)expected[idx].high,
                             (unsigned)expected[idx].low,
                             (unsigned)actual[idx].high, (unsigned)actual[idx].low);
                  fail("annotation changed an omitted limit or misassigned a supplied limit");
                  return;
            }
      }
}

static PLI_INT32 limits(PLI_BYTE8 *mode)
{
      vpiHandle args = vpi_iterate(vpiArgument, vpi_handle(vpiSysTfCall, NULL));
      vpiHandle check;
      s_vpi_time expected[2] = {{0}, {0}};
      s_vpi_delay delays = {0};
      unsigned idx;
      if (!args) { fail("missing arguments"); return 0; }
      check = object_arg(args);
      for (idx = 0; idx < 2; idx += 1) {
            expected[idx].type = vpiSimTime;
            expected[idx].high = integer_arg(args);
            expected[idx].low = integer_arg(args);
      }
      if (vpi_scan(args)) { fail("extra argument"); return 0; }
      if (!check || failures) return 0;
      if (!strcmp((const char *)mode, "set")) {
            delays.da = expected;
            delays.no_of_delays = 2;
            delays.time_type = vpiSimTime;
            vpi_put_delays(check, &delays);
            sets += 1;
      } else {
            checks += 1;
      }
      exact_readback(check, expected);
      return 0;
}

static PLI_INT32 finish(PLI_BYTE8 *unused)
{
      (void)unused;
      if (failures || sets != 4 || checks != 4) fail("missing exact checks");
      else vpi_printf("RECREM_EXACT_COMPLETE sets=4 checks=4\nPASSED\n");
      return 0;
}

static void register_all(void)
{
      s_vpi_systf_data tf = {0};
      tf.type = vpiSysTask;
      tf.tfname = "$rrc_exact_set";
      tf.calltf = limits;
      tf.user_data = (PLI_BYTE8 *)"set";
      vpi_register_systf(&tf);
      tf.tfname = "$rrc_exact_expect";
      tf.user_data = (PLI_BYTE8 *)"expect";
      vpi_register_systf(&tf);
      tf.tfname = "$rrc_exact_finish";
      tf.calltf = finish;
      tf.user_data = NULL;
      vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {register_all, 0};
