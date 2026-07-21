// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_crc (clock, reset, enable, test_expr, initialize, valid, compare, crc, crc_latch, fire);

  parameter severity_level      = `OVL_SEVERITY_DEFAULT;
  parameter width               = 1;
  parameter data_width          = 0;
  parameter crc_width           = 5;
  parameter crc_enable          = 0;
  parameter crc_latch_enable    = 0;
  parameter polynomial          = 5; 
  parameter standard_polynomial = 0; 
  parameter initial_value       = 0;
  parameter lsb_first           = 0;
  parameter big_endian          = 0;
  parameter reverse_endian      = 0;
  parameter invert              = 0;
  parameter combinational       = 0;
  parameter property_type       = `OVL_PROPERTY_DEFAULT;
  parameter msg                 = `OVL_MSG_DEFAULT;
  parameter coverage_level      = `OVL_COVER_DEFAULT;
  
  parameter clock_edge          = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity      = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type         = `OVL_GATING_TYPE_DEFAULT;


  // Internal parameters (do not change)
  parameter assert_name         = "OVL_CRC";

  input                          clock, reset, enable; 
  input  [width-1:0]             test_expr;
  input                          initialize;
  input  [crc_width-1:0]         crc;
  input                          compare;
  input                          valid;
  input                          crc_latch;
  output [`OVL_FIRE_WIDTH-1 : 0] fire;

  `include "std_ovl_reset.h"
  `include "std_ovl_clock.h"
  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_SVA
  `include "./sva05/ovl_crc_logic.sv"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`endmodule // ovl_crc
