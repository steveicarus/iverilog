// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_next (clock, reset, enable, start_event, test_expr, fire);

  parameter severity_level      = `OVL_SEVERITY_DEFAULT;
  parameter num_cks             = 1;
  parameter check_overlapping   = 1;
  parameter check_missing_start = 0;
  parameter property_type       = `OVL_PROPERTY_DEFAULT;
  parameter msg                 = `OVL_MSG_DEFAULT;
  parameter coverage_level      = `OVL_COVER_DEFAULT;

  parameter clock_edge          = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity      = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type         = `OVL_GATING_TYPE_DEFAULT;

  input                          clock, reset, enable;
  input                          start_event, test_expr;
  output [`OVL_FIRE_WIDTH-1:0]   fire;

  // Parameters that should not be edited
  parameter assert_name = "OVL_NEXT";

  `include "std_ovl_reset.h"
  `include "std_ovl_clock.h"
  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_SYNTHESIS
`else
  // Sanity Checks
  initial begin
    if (num_cks <= 0) begin
      ovl_error_t(`OVL_FIRE_2STATE,"Illegal value for parameter num_cks which must be set to value greater than 0");
    end
    if ((num_cks == 1) && (check_overlapping == 0)) begin
      ovl_error_t(`OVL_FIRE_2STATE,"check_overlapping=0 is ignored when num_cks=1, recommend changing check_overlapping to 1 (identical behaviour)");
    end
  end
`endif

`ifdef OVL_VERILOG
  `include "./vlog95/ovl_next_logic.v"
`endif

`ifdef OVL_SVA
  `include "./sva05/ovl_next_logic.sv"
`endif

`ifdef OVL_PSL
  `include "./psl05/assert_next_psl_logic.v"

`else
  assign fire = {fire_cover, fire_xcheck, fire_2state};
  `endmodule // ovl_next
`endif
