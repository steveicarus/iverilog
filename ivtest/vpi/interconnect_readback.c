/* Diagnostic test of Icarus' global INTERCONNECT enumeration extension.
 * Expected delivery times are fixed independently of vpi_get_delays results.
 */
#include <vpi_user.h>
#include <stdio.h>
#include <string.h>

#define CHANNELS 8
#define EVENTS 5
#define PATHS 2

struct channel {
      const char *name;
      unsigned rise, fall;
      vpiHandle handle;
      int previous, count;
      unsigned times[EVENTS];
      int values[EVENTS];
};

static struct channel channels[CHANNELS] = {
      {.name="top.drive"},
      {.name="top.source.Q"},
      {.name="top.left.I", .rise=7, .fall=13},
      {.name="top.left.Q", .rise=7, .fall=13},
      {.name="top.right.I", .rise=11, .fall=19},
      {.name="top.right.Q", .rise=11, .fall=19},
      {.name="top.peer.I"},
      {.name="top.peer.Q"}
};

struct path {
      vpiHandle handle;
      char source[64], destination[64];
      unsigned rise, fall;
};

static struct path configured[PATHS];
static int errors, enabled = 1, configured_seen;

static void require(int condition, const char *message)
{
      if (!condition) {
            vpi_printf("FAIL: %s\n", message);
            errors += 1;
      }
}

static unsigned now(void)
{
      s_vpi_time time = {0};
      time.type = vpiSimTime;
      vpi_get_time(0, &time);
      require(time.high == 0, "time exceeds fixture domain");
      return time.low;
}

static int scalar(vpiHandle handle)
{
      s_vpi_value value = {0};
      value.format = vpiScalarVal;
      vpi_get_value(handle, &value);
      return value.value.scalar;
}

static PLI_INT32 changed(p_cb_data cb)
{
      struct channel *channel = (struct channel *)cb->user_data;
      int value = scalar(channel->handle);
      /* Some simulators report initial no-op callbacks. Retain state and every
       * actual transition, including unknowns, rather than count callbacks. */
      if (value == channel->previous) return 0;
      channel->previous = value;
      if (channel->count < EVENTS) {
            channel->times[channel->count] = now();
            channel->values[channel->count] = value;
      }
      channel->count += 1;
      require(channel->count <= EVENTS, "extra passive transition");
      return 0;
}

static void check_histories(int count)
{
      int i, j;
      for (i = 0; i < CHANNELS; i += 1) {
            struct channel *channel = &channels[i];
            require(channel->count == count, "passive transition count");
            require(scalar(channel->handle) == vpi0, "settled final value");
            for (j = 0; j < count && j < channel->count && j < EVENTS; j += 1) {
                  unsigned expected = 100*j +
                        ((j & 1) ? channel->rise : channel->fall);
                  require(channel->times[j] == expected, "passive delivery time");
                  require(channel->values[j] == ((j & 1) ? vpi1 : vpi0),
                          "passive delivered value");
            }
      }
}

static int endpoint(vpiHandle port, int direction, char *name, size_t size)
{
      vpiHandle module;
      const char *text;
      char scope[64];
      size_t scope_len, port_len;
      if (!port) return 0;
      require(vpi_get(vpiType, port) == vpiPort, "endpoint type");
      require(vpi_get(vpiSize, port) == 1, "endpoint scalar width");
      require(vpi_get(vpiDirection, port) == direction, "endpoint direction");
      module = vpi_handle(vpiModule, port);
      if (!module) return 0;
      text = vpi_get_str(vpiFullName, module);
      if (!text) return 0;
      scope_len = strlen(text);
      if (scope_len >= sizeof scope) return 0;
      memcpy(scope, text, scope_len + 1);
      text = vpi_get_str(vpiName, port);
      if (!text) return 0;
      port_len = strlen(text);
      if (scope_len + 1 >= size || port_len >= size - scope_len - 1)
            return 0;
      memcpy(name, scope, scope_len);
      name[scope_len] = '.';
      memcpy(name + scope_len + 1, text, port_len + 1);
      return 1;
}

static void read_path(vpiHandle handle, struct path *path)
{
      vpiHandle iterator, source, destination;
      s_vpi_time times[2] = {{0}};
      s_vpi_delay delays = {0};
      memset(path, 0, sizeof *path);
      path->handle = handle;
      require(vpi_get(vpiType, handle) == vpiInterModPath, "path type");
      iterator = vpi_iterate(vpiPort, handle);
      require(iterator != 0, "path endpoint iterator");
      if (!iterator) return;
      source = vpi_scan(iterator);
      destination = vpi_scan(iterator);
      require(endpoint(source, vpiOutput, path->source, sizeof path->source),
              "source endpoint identity");
      require(endpoint(destination, vpiInput, path->destination,
                       sizeof path->destination), "destination endpoint identity");
      require(vpi_scan(iterator) == 0, "exactly two endpoint ports");
      /* Population checks belong to this diagnostic extension test, not the
       * standard endpoint-lookup regressions. The subsequent snapshots must
       * still contain exactly the original objects after repeated lookup. */
      if (source && destination) {
            vpiHandle again = vpi_handle_multi(vpiInterModPath,
                                              source, destination);
            require(again && vpi_compare_objects(handle, again),
                    "endpoint lookup reuses the enumerated object");
      }
      delays.da = times;
      delays.no_of_delays = 2;
      delays.time_type = vpiSimTime;
      vpi_get_delays(handle, &delays);
      require(times[0].high == 0 && times[1].high == 0, "delay tick range");
      path->rise = times[0].low;
      path->fall = times[1].low;
}

/* Pure snapshot predicate, also challenged below by deliberate mutations. */
static int valid_paths(const struct path *paths, int count)
{
      int i, mask = 0;
      if (count != PATHS) return 0;
      for (i = 0; i < count; i += 1) {
            int bit;
            if (strcmp(paths[i].source, "top.source.Q")) return 0;
            if (!strcmp(paths[i].destination, "top.left.I")) {
                  bit = 1;
                  if (paths[i].rise != 7 || paths[i].fall != 13) return 0;
            } else if (!strcmp(paths[i].destination, "top.right.I")) {
                  bit = 2;
                  if (paths[i].rise != 11 || paths[i].fall != 19) return 0;
            } else return 0;
            if (mask & bit) return 0;
            mask |= bit;
      }
      return mask == 3 && paths[0].handle != paths[1].handle;
}

static int collect(vpiHandle iterator, struct path *paths)
{
      vpiHandle handle;
      int count = 0;
      if (!iterator) return 0;
      while ((handle = vpi_scan(iterator))) {
            if (count < PATHS) read_path(handle, &paths[count]);
            count += 1;
      }
      return count;
}

static void same_paths(const struct path *paths, int count)
{
      int i, j;
      require(valid_paths(paths, count), "exact discovered endpoints and delays");
      if (count != PATHS) return;
      for (i = 0; i < PATHS; i += 1) {
            int found = 0;
            for (j = 0; j < PATHS; j += 1) {
                  if (!strcmp(paths[i].destination, configured[j].destination)) {
                        require(paths[i].handle == configured[j].handle,
                                "stable simulator-owned path identity");
                        found = 1;
                  }
            }
            require(found, "path identity was retained");
      }
}

static void snapshot_controls(void)
{
      struct path bad[PATHS];
      require(!valid_paths(configured, 1), "missing-path control rejected");
      memcpy(bad, configured, sizeof bad);
      bad[1] = bad[0];
      require(!valid_paths(bad, PATHS), "duplicate-path control rejected");
      memcpy(bad, configured, sizeof bad);
      snprintf(bad[0].destination, sizeof bad[0].destination, "top.peer.I");
      require(!valid_paths(bad, PATHS), "wrong-endpoint control rejected");
      memcpy(bad, configured, sizeof bad);
      bad[0].fall += 1;
      require(!valid_paths(bad, PATHS), "wrong-delay control rejected");
}

static void lifecycle_checks(void)
{
      struct path paths[PATHS], retained;
      vpiHandle first, second, borrowed;
      int count;
      first = vpi_iterate(vpiInterModPath, 0);
      second = vpi_iterate(vpiInterModPath, 0);
      require(first != 0 && second != 0, "simultaneous global iterators");
      if (!first || !second) return;
      borrowed = vpi_scan(first);
      require(borrowed != 0, "borrowed handle from early iterator");
      vpi_release_handle(first);
      /* Path handles belong to the simulation. Releasing a client's handle
       * must not destroy a path shared with the second live iterator. */
      if (borrowed) vpi_release_handle(borrowed);
      count = collect(second, paths);
      same_paths(paths, count);
      if (!borrowed) return;
      read_path(borrowed, &retained);
      require(retained.handle == borrowed, "borrowed handle survives release");
      count = collect(vpi_iterate(vpiInterModPath, 0), paths);
      same_paths(paths, count);
}

static PLI_INT32 configured_cb(p_cb_data cb)
{
      int count;
      (void)cb;
      configured_seen += 1;
      require(now() == 50, "configured callback time");
      check_histories(1);
      if (enabled) {
            count = collect(vpi_iterate(vpiInterModPath, 0), configured);
            require(valid_paths(configured, count), "configured discovery/readback");
            if (valid_paths(configured, count)) {
                  snapshot_controls();
                  lifecycle_checks();
                  vpi_printf("CONFIGURED: two paths, delays 7/13 and 11/19 ps\n");
            }
      }
      return 0;
}

static PLI_INT32 finish_cb(p_cb_data cb)
{
      int i, j;
      struct path paths[PATHS];
      (void)cb;
      require(now() == 500, "final callback time");
      require(configured_seen == 1, "configured phase completed exactly once");
      check_histories(EVENTS);
      if (enabled) {
            int count = collect(vpi_iterate(vpiInterModPath, 0), paths);
            same_paths(paths, count);
      }
      for (i = 0; i < CHANNELS; i += 1) {
            vpi_printf("HISTORY %s:", channels[i].name);
            for (j = 0; j < channels[i].count && j < EVENTS; j += 1)
                  vpi_printf(" %u=%d", channels[i].times[j], channels[i].values[j]);
            vpi_printf("\n");
      }
      vpi_printf("%s: INTERCONNECT %s\n", errors ? "FAIL" : "PASS",
                 enabled ? "discovery/readback and passive delivery" : "passive delivery");
      /* vpi_reg.pl compares the complete output with its golden log. A normal
       * simulator exit alone is not acceptance; every FAIL changes that log. */
      return 0;
}

static PLI_INT32 empty_calltf(PLI_BYTE8 *data)
{
      (void)data;
      require(now() == 0, "pre-annotation query time");
      if (enabled) {
            vpiHandle iterator = vpi_iterate(vpiInterModPath, 0);
            require(iterator == 0, "empty discovery before annotation");
            if (iterator) vpi_release_handle(iterator);
      }
      return 0;
}

static PLI_INT32 configured_calltf(PLI_BYTE8 *data)
{
      s_cb_data cb = {0};
      s_vpi_time time = {0};
      (void)data;
      time.type = vpiSimTime;
      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = configured_cb;
      cb.time = &time;
      vpi_register_cb(&cb);
      return 0;
}

static PLI_INT32 start_cb(p_cb_data unused)
{
      int i;
      s_vpi_vlog_info info;
      s_cb_data cb = {0};
      s_vpi_time time = {0};
      s_vpi_value value = {0};
      (void)unused;
      vpi_get_vlog_info(&info);
      for (i = 0; i < info.argc; i += 1)
            if (!strcmp(info.argv[i], "+disable_discovery")) enabled = 0;
      time.type = vpiSimTime;
      value.format = vpiScalarVal;
      cb.reason = cbValueChange;
      cb.cb_rtn = changed;
      cb.time = &time;
      cb.value = &value;
      for (i = 0; i < CHANNELS; i += 1) {
            channels[i].handle = vpi_handle_by_name((PLI_BYTE8 *)channels[i].name, 0);
            require(channels[i].handle != 0, "passive channel exists");
            if (!channels[i].handle) continue;
            channels[i].previous = scalar(channels[i].handle);
            require(channels[i].previous == vpiX, "initial unknown state");
            cb.obj = channels[i].handle;
            cb.user_data = (PLI_BYTE8 *)&channels[i];
            vpi_register_cb(&cb);
      }
      memset(&cb, 0, sizeof cb);
      cb.reason = cbEndOfSimulation;
      cb.cb_rtn = finish_cb;
      vpi_register_cb(&cb);
      return 0;
}

static void register_test(void)
{
      s_vpi_systf_data task = {0};
      s_cb_data cb = {0};
      task.type = vpiSysTask;
      task.tfname = "$interconnect_empty";
      task.calltf = empty_calltf;
      vpi_register_systf(&task);
      task.tfname = "$interconnect_configured";
      task.calltf = configured_calltf;
      vpi_register_systf(&task);
      cb.reason = cbStartOfSimulation;
      cb.cb_rtn = start_cb;
      vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {register_test, 0};
