/* Read every replicated module path using standard VPI relationships. */
#include <string.h>
#include <vpi_user.h>

static int has_name(vpiHandle terminal, const char *expected)
{
      vpiHandle expression = vpi_handle(vpiExpr, terminal);
      const char *name = expression ? vpi_get_str(vpiName, expression) : 0;
      if (!name) {
            vpi_printf("FAIL missing module-path expression/name for %s\n", expected);
            return 0;
      }
      return strcmp(name, expected) == 0;
}

static PLI_INT32 check_paths(PLI_BYTE8 *user_data)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      vpiHandle arg, scope, iter, path, previous = 0;
      s_vpi_value value;
      unsigned count = 0;
      PLI_UINT32 expected[2];
      int ok = 1;
      (void)user_data;

      arg = vpi_scan(args);
      value.format = vpiStringVal;
      vpi_get_value(arg, &value);
      scope = vpi_handle_by_name(value.value.str, 0);
      arg = vpi_scan(args);
      value.format = vpiIntVal;
      vpi_get_value(arg, &value);
      expected[0] = value.value.integer;
      arg = vpi_scan(args);
      vpi_get_value(arg, &value);
      expected[1] = value.value.integer;
      vpi_free_object(args);

      iter = scope ? vpi_iterate(vpiModPath, scope) : 0;
      if (!iter) ok = 0;
      while (iter && (path = vpi_scan(iter))) {
            vpiHandle input = vpi_handle(vpiModPathIn, path);
            vpiHandle output = vpi_handle(vpiModPathOut, path);
            s_vpi_time times[2];
            s_vpi_delay delays;
            unsigned i;
            count += 1;
            if (path == previous || !input || !output) ok = 0;
            previous = path;
            if (!input || !output) continue;
            if (!has_name(input, "A") || !has_name(output, "Y"))
                  ok = 0;
            memset(times, 0, sizeof times);
            memset(&delays, 0, sizeof delays);
            delays.da = times;
            delays.no_of_delays = 2;
            delays.time_type = vpiSimTime;
            vpi_get_delays(path, &delays);
            for (i = 0; i < 2; ++i)
                  if (times[i].high != 0 || times[i].low != expected[i]) ok = 0;
      }
      if (count != 2) ok = 0;
      if (!ok) vpi_printf("FAIL replicated path count, identity, or delay\n");
      value.format = vpiIntVal;
      value.value.integer = ok;
      vpi_put_value(call, &value, 0, vpiNoDelay);
      return 0;
}

static void register_check(void)
{
      s_vpi_systf_data data;
      memset(&data, 0, sizeof data);
      data.type = vpiSysFunc;
      data.sysfunctype = vpiIntFunc;
      data.tfname = "$check_multidriver_paths";
      data.calltf = check_paths;
      vpi_register_systf(&data);
}

void (*vlog_startup_routines[])(void) = {register_check, 0};
