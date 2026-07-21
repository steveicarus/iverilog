// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_cycle_sequence (clock, reset, enable, event_sequence, fire);

  parameter severity_level      = `OVL_SEVERITY_DEFAULT;
  parameter num_cks             = 2;
  parameter necessary_condition = `OVL_NECESSARY_CONDITION_DEFAULT;
  parameter property_type       = `OVL_PROPERTY_DEFAULT;
  parameter msg                 = `OVL_MSG_DEFAULT;
  parameter coverage_level      = `OVL_COVER_DEFAULT;

  parameter clock_edge     = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type    = `OVL_GATING_TYPE_DEFAULT;

  input                          clock, reset, enable;
  input  [num_cks-1:0]           event_sequence;
  output [`OVL_FIRE_WIDTH-1:0]   fire;

  // Parameters that should not be edited
  parameter assert_name = "OVL_CYCLE_SEQUENCE";

  `include "std_ovl_reset.h"
  `include "std_ovl_clock.h"
  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_SYNTHESIS
`else
  // Sanity Checks
  initial begin
    if (num_cks < 2) begin
      ovl_error_t(`OVL_FIRE_2STATE,"Illegal value for parameter num_cks which must be set to value greater than 1");
    end
  end
`endif

`ifdef OVL_VERILOG
  `include "./vlog95/ovl_cycle_sequence_logic.v"
`endif

`ifdef OVL_SVA
  `include "./sva05/ovl_cycle_sequence_logic.sv"
`endif

`ifdef OVL_PSL
  `include "./psl05/assert_cycle_sequence_psl_logic.v"

`else
  assign fire = {fire_cover, fire_xcheck, fire_2state};
  `endmodule // ovl_cycle_sequence
`endif
