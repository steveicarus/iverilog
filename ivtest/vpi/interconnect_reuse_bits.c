/* Reuse indexed-source routes to distinct scalar fanout ports. */
#include <vpi_user.h>
#include <string.h>

static vpiHandle saved[2];
static int errors, calls;

static void require(int condition, const char *message)
{
      if (!condition) {
            vpi_printf("FAIL: %s\n", message);
            errors += 1;
      }
}

static vpiHandle port_bit(const char *scope, const char *name, int index)
{
      vpiHandle module = vpi_handle_by_name((PLI_BYTE8 *)scope, 0);
      vpiHandle ports, port, bits, bit;
      if (!module) return 0;
      ports = vpi_iterate(vpiPort, module);
      if (!ports) return 0;
      while ((port = vpi_scan(ports))) {
            if (strcmp(vpi_get_str(vpiName, port), name)) continue;
            vpi_release_handle(ports);
            if (index < 0) return port;
            bits = vpi_iterate(vpiBit, port);
            if (!bits) return 0;
            while ((bit = vpi_scan(bits))) {
                  if (vpi_get(vpiBit, bit) != index) continue;
                  vpi_release_handle(bits);
                  return bit;
            }
            return 0;
      }
      return 0;
}

static PLI_INT32 check_calltf(PLI_BYTE8 *unused)
{
      int i, count = 0;
      vpiHandle iterator, path;
      (void)unused;
      calls += 1;
      for (i = 0; i < 2; i += 1) {
            s_vpi_time times[2] = {{0}};
            s_vpi_delay delay = {0};
            vpiHandle source = port_bit("top.source", "Q", i);
            vpiHandle sink = port_bit(i ? "top.right" : "top.left", "I", -1);
            require(source && sink, "indexed endpoints exist");
            if (!source || !sink) continue;
            path = vpi_handle_multi(vpiInterModPath, source, sink);
            require(path != 0, "indexed existing path found");
            if (!path) continue;
            if (saved[i])
                  require(vpi_compare_objects(saved[i], path),
                          "indexed path reused");
            saved[i] = path;
            delay.da = times;
            delay.no_of_delays = 2;
            delay.time_type = vpiSimTime;
            vpi_get_delays(path, &delay);
            require(times[0].high == 0 && times[1].high == 0 &&
                    times[0].low == (i ? 7 : 3) &&
                    times[1].low == (i ? 11 : 5),
                    "indexed path retains its own delays");
      }
      if (saved[0] && saved[1])
            require(!vpi_compare_objects(saved[0], saved[1]),
                    "different indexed pairs do not alias");
      iterator = vpi_iterate(vpiInterModPath, 0);
      if (iterator) while (vpi_scan(iterator)) count += 1;
      require(count == 2, "two indexed paths, no duplicate insertion");
      return 0;
}

static PLI_INT32 finish_cb(p_cb_data unused)
{
      (void)unused;
      require(calls == 2, "both indexed lookup phases completed");
      vpi_printf("%s: INTERCONNECT indexed identity separation\n",
                 errors ? "FAIL" : "PASS");
      return 0;
}

static void register_test(void)
{
      s_vpi_systf_data task = {0};
      s_cb_data cb = {0};
      task.type = vpiSysTask;
      task.tfname = "$interconnect_reuse_bits";
      task.calltf = check_calltf;
      vpi_register_systf(&task);
      cb.reason = cbEndOfSimulation;
      cb.cb_rtn = finish_cb;
      vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {register_test, 0};
