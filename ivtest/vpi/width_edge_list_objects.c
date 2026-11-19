/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (c) 2026 Jerry Zhong */
#include <string.h>
#include "vpi_user.h"

static PLI_INT32 check_objects(ICARUS_VPI_CONST PLI_BYTE8 *unused)
{
      vpiHandle scope = vpi_handle_by_name("tb.dut", 0);
      vpiHandle iterator = vpi_iterate(vpiTchk, scope), check;
      unsigned positive = 0, negative = 0;
      (void)unused;
      while (iterator && (check = vpi_scan(iterator))) {
            vpiHandle term = vpi_handle(vpiTchkRefTerm, check);
            int edge = vpi_get(vpiEdge, term);
            s_vpi_time value;
            s_vpi_delay delay;
            memset(&value, 0, sizeof value);
            memset(&delay, 0, sizeof delay);
            delay.da = &value;
            delay.no_of_delays = 1;
            delay.time_type = vpiScaledRealTime;
            vpi_get_delays(check, &delay);
            if (vpi_get(vpiTchkType, check) != vpiWidth ||
                (edge != vpiPosedge && edge != vpiNegedge) ||
                value.real != (edge == vpiPosedge ? 12.0 : 14.0)) {
                  vpi_printf("FAILED WIDTH edge-list object or SDF binding\n");
                  vpi_control(vpiFinish, 0);
                  return 0;
            }
            if (edge == vpiPosedge) positive += 1;
            else negative += 1;
      }
      if (positive != 4 || negative != 4) {
            vpi_printf("FAILED WIDTH edge-list object count %u/%u\n",
                       positive, negative);
            vpi_control(vpiFinish, 0);
      }
      return 0;
}
static void register_objects(void)
{
      s_vpi_systf_data data;
      memset(&data, 0, sizeof data);
      data.type = vpiSysTask;
      data.tfname = "$width_edge_list_objects";
      data.calltf = check_objects;
      vpi_register_systf(&data);
}
void (*vlog_startup_routines[])(void) = {register_objects, 0};
