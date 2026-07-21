// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_fifo (clock, reset, enable, enq, enq_data, deq, deq_data, full, empty,
                  preload, fire);

  parameter severity_level  = `OVL_SEVERITY_DEFAULT;
  parameter depth           = 2;
  parameter width           = 1;
  parameter high_water_mark = 0;
  parameter enq_latency     = 0;
  parameter deq_latency     = 0;
  parameter value_check     = 1;
  parameter pass_thru       = 0;
  parameter registered      = 0;
  parameter preload_count   = 0;
  parameter property_type   = `OVL_PROPERTY_DEFAULT;
  parameter msg             = `OVL_MSG_DEFAULT;
  parameter coverage_level  = `OVL_COVER_DEFAULT;

  parameter clock_edge      = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity  = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type     = `OVL_GATING_TYPE_DEFAULT;

  // Local parameter (do not modify)
  parameter preload_data_width = (preload_count == 0) ? width
                                                      : (preload_count*width);

  input                             clock, reset, enable;
  input                             enq, deq, full, empty;
  input  [width-1 : 0]              enq_data, deq_data;
  input  [preload_data_width-1 : 0] preload;
  output [`OVL_FIRE_WIDTH-1 : 0]    fire;

  // Parameters that should not be edited
  parameter assert_name = "OVL_FIFO";

  `include "std_ovl_reset.h"
  `include "std_ovl_clock.h"
  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_SVA
  `include "./sva05/ovl_fifo_logic.sv"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`endmodule // ovl_fifo

