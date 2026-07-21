// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_next_state (clock, reset, enable, test_expr, curr_state, next_state, fire);

  parameter severity_level   = `OVL_SEVERITY_DEFAULT;
  parameter next_count       = 1;
  parameter width            = 1;
  parameter min_hold         = 1;
  parameter max_hold         = 1;
  parameter disallow         = 0;
  parameter property_type    = `OVL_PROPERTY_DEFAULT;
  parameter msg              = `OVL_MSG_DEFAULT;
  parameter coverage_level   = `OVL_COVER_DEFAULT;

  parameter clock_edge       = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity   = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type      = `OVL_GATING_TYPE_DEFAULT;

  input                            clock, reset, enable;
  input  [width-1 : 0]             test_expr;
  input  [width-1 : 0]             curr_state;
  input  [next_count*width-1:0]    next_state;
  output [`OVL_FIRE_WIDTH-1 : 0]   fire;

  // Parameters that should not be edited
  parameter assert_name = "ASSERT_NEXT_STATE";

  `include "std_ovl_reset.h"
  `include "std_ovl_clock.h"
  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_SVA
  `include "./sva05/ovl_next_state_logic.sv"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`endmodule // ovl_next_state

