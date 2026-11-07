/* Repeated scalar INTERCONNECT lookup must reuse the inserted delay object.
 * Independently fixed histories also check fanout and partial reannotation.
 */
#include <vpi_user.h>
#include <string.h>

#define CHANNELS 8
#define EVENTS 7

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

static vpiHandle left, right;
static int errors, phase;

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

/* Obtain fresh endpoint handles each time; compare design object identities,
 * not opaque handle addresses. */
static vpiHandle port(const char *scope, const char *name)
{
      vpiHandle module = vpi_handle_by_name((PLI_BYTE8 *)scope, 0);
      vpiHandle iterator, item;
      require(module != 0, "endpoint module exists");
      if (!module) return 0;
      iterator = vpi_iterate(vpiPort, module);
      if (!iterator) return 0;
      while ((item = vpi_scan(iterator))) {
            if (!strcmp(vpi_get_str(vpiName, item), name)) {
                  vpi_release_handle(iterator);
                  return item;
            }
      }
      require(0, "endpoint port exists");
      return 0;
}

static vpiHandle lookup(const char *destination)
{
      vpiHandle source_port = port("top.source", "Q");
      vpiHandle destination_port = port(destination, "I");
      vpiHandle path;
      if (!source_port || !destination_port) return 0;
      path = vpi_handle_multi(vpiInterModPath, source_port, destination_port);
      require(path != 0, "existing endpoint pair returns a path");
      return path;
}

static void delays(vpiHandle path, unsigned rise, unsigned fall)
{
      s_vpi_time times[2] = {{0}};
      s_vpi_delay delay = {0};
      if (!path) return;
      delay.da = times;
      delay.no_of_delays = 2;
      delay.time_type = vpiSimTime;
      vpi_get_delays(path, &delay);
      require(times[0].high == 0 && times[1].high == 0 &&
              times[0].low == rise && times[1].low == fall,
              "effective rise/fall delays");
}

static void population(int expected)
{
      vpiHandle iterator = vpi_iterate(vpiInterModPath, 0), path;
      int count = 0, left_count = 0, right_count = 0;
      if (iterator) while ((path = vpi_scan(iterator))) {
            count += 1;
            if (left && vpi_compare_objects(path, left)) left_count += 1;
            if (right && vpi_compare_objects(path, right)) right_count += 1;
      }
      require(count == expected, "lookup does not add delay objects");
      require(left_count == 1, "one object for the left endpoint pair");
      require(right_count == (expected == 2),
              "distinct fanout has its own object");
}

static void check_pair(unsigned rise)
{
      vpiHandle again_left = lookup("top.left");
      vpiHandle again_right = lookup("top.right");
      if (left && again_left)
            require(vpi_compare_objects(left, again_left), "left object reused");
      if (!right) right = again_right;
      if (right && again_right)
            require(vpi_compare_objects(right, again_right), "right object reused");
      if (again_left && again_right)
            require(!vpi_compare_objects(again_left, again_right),
                    "different fanout endpoints do not alias");
      delays(again_left, rise, 13);
      delays(again_right, 11, 19);
      population(2);
}

static PLI_INT32 check_calltf(PLI_BYTE8 *unused)
{
      static const unsigned at[] = {0, 0, 50, 250, 450};
      vpiHandle again;
      (void)unused;
      require(phase < 5, "no extra check phase");
      if (phase >= 5) return 0;
      require(now() == at[phase], "check phase time");
      if (phase == 0) {
            left = lookup("top.left");
            delays(left, 0, 0);
            again = lookup("top.left");
            if (left && again)
                  require(vpi_compare_objects(left, again),
                          "repeated pre-annotation lookup reuses object");
            delays(again, 0, 0);
            population(1);
      } else {
            check_pair(phase == 3 ? 17 : 7);
            if (phase == 2 && left) {
                  /* Release this client's handle, then acquire a fresh handle
                   * before using the path again. The live connection remains. */
                  vpi_release_handle(left);
                  left = lookup("top.left");
                  check_pair(7);
            }
      }
      phase += 1;
      return 0;
}

static PLI_INT32 changed(p_cb_data cb)
{
      struct channel *channel = (struct channel *)cb->user_data;
      int value = scalar(channel->handle);
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

static PLI_INT32 finish_cb(p_cb_data unused)
{
      int i, j;
      (void)unused;
      require(now() == 650 && phase == 5, "all phases completed");
      check_pair(7);
      for (i = 0; i < CHANNELS; i += 1) {
            struct channel *channel = &channels[i];
            require(channel->count == EVENTS, "passive transition count");
            require(scalar(channel->handle) == vpi0, "settled final value");
            vpi_printf("HISTORY %s:", channel->name);
            for (j = 0; j < channel->count && j < EVENTS; j += 1) {
                  unsigned rise = channel->rise;
                  unsigned expected;
                  /* Only the left branch's fourth transition uses the partial
                   * ABSOLUTE update. These values do not come from readback. */
                  if ((i == 2 || i == 3) && j == 3) rise = 17;
                  expected = 100*j + ((j & 1) ? rise : channel->fall);
                  require(channel->times[j] == expected, "passive delivery time");
                  require(channel->values[j] == ((j & 1) ? vpi1 : vpi0),
                          "passive delivered value");
                  vpi_printf(" %u=%d", channel->times[j], channel->values[j]);
            }
            vpi_printf("\n");
      }
      vpi_printf("%s: INTERCONNECT lookup reuse, partial annotation and fanout\n",
                 errors ? "FAIL" : "PASS");
      return 0;
}

static PLI_INT32 start_cb(p_cb_data unused)
{
      int i;
      s_cb_data cb = {0};
      s_vpi_time time = {0};
      s_vpi_value value = {0};
      (void)unused;
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
      task.tfname = "$interconnect_reuse";
      task.calltf = check_calltf;
      vpi_register_systf(&task);
      cb.reason = cbStartOfSimulation;
      cb.cb_rtn = start_cb;
      vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {register_test, 0};
