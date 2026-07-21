// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_xproduct_bit_coverage (clock, reset, enable, test_expr1, test_expr2, fire);

  parameter severity_level    = `OVL_SEVERITY_DEFAULT;
  parameter width1            = 1;
  parameter width2            = 1;
  parameter test_expr2_enable = 0;
  parameter coverage_check    = 0;
  parameter property_type     = `OVL_PROPERTY_DEFAULT;
  parameter msg               = `OVL_MSG_DEFAULT;
  parameter coverage_level    = `OVL_COVER_DEFAULT;
  
  parameter clock_edge        = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity    = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type       = `OVL_GATING_TYPE_DEFAULT;
  
  // Internal parameters (do not change)
  parameter assert_name       = "OVL_XPRODUCT_BIT_COVERAGE";

  input                        clock, reset, enable;
  input  [width1-1:0]          test_expr1; 
  input  [width2-1:0]          test_expr2; 
  output [`OVL_FIRE_WIDTH-1:0] fire; 

  `include "std_ovl_reset.h"
  `include "std_ovl_clock.h"
  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_SVA
  `include "./sva05/ovl_xproduct_bit_coverage_logic.sv"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`endmodule // ovl_xproduct_bit_coverage

