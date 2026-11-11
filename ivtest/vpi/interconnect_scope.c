/*
 * Exercise Icarus's existing, mutating vpi_handle_multi creation API.
 * The chosen Icarus contract is destination-module scaling and precision;
 * this test does not claim that contract for every simulator.
 */
#include <string.h>
#include <vpi_user.h>

static unsigned checks, failures, events;
static int setup_done;
static s_vpi_time callback_time;
static s_vpi_value callback_value;
static s_cb_data callback_data;

static int check(int condition, const char *message)
{
      checks += 1;
      if (!condition) {
            failures += 1;
            vpi_printf("FAIL VPI: %s\n", message);
      }
      return condition;
}

static void return_int(vpiHandle call, int value)
{
      s_vpi_value result;
      result.format = vpiIntVal;
      result.value.integer = value;
      vpi_put_value(call, &result, 0, vpiNoDelay);
}

static vpiHandle find_port(vpiHandle module, const char *name)
{
      vpiHandle iterator = vpi_iterate(vpiPort, module);
      vpiHandle port;
      if (!iterator) return 0;
      while ((port = vpi_scan(iterator))) {
            if (strcmp(vpi_get_str(vpiName, port), name) == 0) {
                  vpi_free_object(iterator);
                  return port;
            }
      }
      return 0;
}

static PLI_INT32 output_change(p_cb_data data)
{
      static const PLI_UINT32 expected_ticks[4] = {
            100007, 200013, 300007, 400013
      };
      static const int expected_values[4] = {vpi1, vpi0, vpi1, vpi0};
      s_vpi_time now;
      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      if (now.high == 0 && now.low < 50000) return 0;
      events += 1;
      if (!check(events <= 4, "extra output callback")) return 0;
      check(now.high == 0 && now.low == expected_ticks[events-1],
            "output callback physical tick");
      check(data->value->value.scalar == expected_values[events-1],
            "output callback value");
      return 0;
}

static PLI_INT32 setup(PLI_BYTE8 *user_data)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      vpiHandle source = args ? vpi_scan(args) : 0;
      vpiHandle destination = args ? vpi_scan(args) : 0;
      vpiHandle output = args ? vpi_scan(args) : 0;
      vpiHandle port1, port2, path, scope;
      s_vpi_time times[2];
      s_vpi_delay delays;
      (void)user_data;
      if (args) vpi_free_object(args);
      return_int(call, 0);
      if (!check(source && destination && output, "three setup arguments"))
            return 0;
      port1 = find_port(source, "y");
      port2 = find_port(destination, "a");
      if (!check(port1 != 0, "source output port")) return 0;
      if (!check(port2 != 0, "destination input port")) return 0;

      /* This call inserts one delay node into the driven connection. */
      path = vpi_handle_multi(vpiInterModPath, port1, port2);
      if (!check(path != 0, "interconnect creation")) return 0;
      check(vpi_get(vpiType, path) == vpiInterModPath, "interconnect type");
      scope = vpi_handle(vpiScope, path);
      if (!check(scope && vpi_compare_objects(scope, destination),
                 "scope is destination module")) return 0;
      check(vpi_compare_objects(vpi_handle(vpiModule, path), destination),
            "module is destination module");
      check(vpi_get(vpiTimeUnit, scope) == -9, "destination unit is ns");
      check(vpi_get(vpiTimePrecision, scope) == -12,
            "destination precision is ps");

      memset(times, 0, sizeof times);
      memset(&delays, 0, sizeof delays);
      delays.da = times;
      delays.no_of_delays = 2;
      delays.time_type = vpiScaledRealTime;
      times[0].type = times[1].type = vpiScaledRealTime;
      times[0].real = 0.007;
      times[1].real = 0.013;
      vpi_put_delays(path, &delays);
      delays.time_type = vpiSimTime;
      vpi_get_delays(path, &delays);
      check(times[0].high == 0 && times[0].low == 7,
            "scaled-real rise becomes 7 global ps ticks");
      check(times[1].high == 0 && times[1].low == 13,
            "scaled-real fall becomes 13 global ps ticks");
      delays.time_type = vpiScaledRealTime;
      vpi_get_delays(path, &delays);
      check(times[0].real > 0.006999999 && times[0].real < 0.007000001,
            "scaled-real rise readback is in destination ns");
      check(times[1].real > 0.012999999 && times[1].real < 0.013000001,
            "scaled-real fall readback is in destination ns");

      /* Integer delay writes are already global ticks, independent of unit. */
      memset(times, 0, sizeof times);
      delays.time_type = vpiSimTime;
      times[0].type = times[1].type = vpiSimTime;
      times[0].low = 17;
      times[1].low = 19;
      vpi_put_delays(path, &delays);
      vpi_get_delays(path, &delays);
      check(times[0].high == 0 && times[0].low == 17,
            "integer rise remains 17 global ticks");
      check(times[1].high == 0 && times[1].low == 19,
            "integer fall remains 19 global ticks");
      delays.time_type = vpiScaledRealTime;
      vpi_get_delays(path, &delays);
      check(times[0].real > 0.016999999 && times[0].real < 0.017000001,
            "integer rise readback is 0.017 destination ns");
      check(times[1].real > 0.018999999 && times[1].real < 0.019000001,
            "integer fall readback is 0.019 destination ns");

      times[0].type = times[1].type = vpiScaledRealTime;
      times[0].real = 0.007;
      times[1].real = 0.013;
      vpi_put_delays(path, &delays);
      callback_time.type = vpiSimTime;
      callback_value.format = vpiScalarVal;
      callback_data.reason = cbValueChange;
      callback_data.cb_rtn = output_change;
      callback_data.obj = output;
      callback_data.time = &callback_time;
      callback_data.value = &callback_value;
      check(vpi_register_cb(&callback_data) != 0, "register output callback");
      setup_done = 1;
      return_int(call, failures ? 0 : checks);
      return 0;
}

static PLI_INT32 finish(PLI_BYTE8 *user_data)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      (void)user_data;
      check(setup_done && events == 4, "four output callbacks after setup");
      vpi_printf("VPI CHECKS %u EVENTS %u\n", checks, events);
      return_int(call, failures ? 0 : checks);
      return 0;
}

static void register_functions(void)
{
      s_vpi_systf_data tf;
      memset(&tf, 0, sizeof tf);
      tf.type = vpiSysFunc;
      tf.sysfunctype = vpiIntFunc;
      tf.tfname = "$interconnect_scope_setup";
      tf.calltf = setup;
      vpi_register_systf(&tf);
      tf.tfname = "$interconnect_scope_finish";
      tf.calltf = finish;
      vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {register_functions, 0};
