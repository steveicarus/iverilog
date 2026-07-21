// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_handshake (clock, reset, enable, req, ack, fire);

  parameter severity_level = `OVL_SEVERITY_DEFAULT;
  parameter min_ack_cycle  = 0;
  parameter max_ack_cycle  = 0;
  parameter req_drop       = 0;
  parameter deassert_count = 0;
  parameter max_ack_length = 0;
  parameter property_type  = `OVL_PROPERTY_DEFAULT;
  parameter msg            = `OVL_MSG_DEFAULT;
  parameter coverage_level = `OVL_COVER_DEFAULT;

  parameter clock_edge     = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type    = `OVL_GATING_TYPE_DEFAULT;

  input                          clock, reset, enable;
  input                          req;
  input                          ack;
  output [`OVL_FIRE_WIDTH-1:0]   fire;

  // Parameters that should not be edited
  parameter assert_name = "OVL_HANDSHAKE";

  `include "std_ovl_reset.h"
  `include "std_ovl_clock.h"
  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_VERILOG
  `include "./vlog95/assert_handshake_logic.v"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`ifdef OVL_SVA
  `include "./sva05/assert_handshake_logic.sv"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`ifdef OVL_PSL
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
  `include "./psl05/assert_handshake_psl_logic.v"
`else
  `endmodule // ovl_handshake
`endif
