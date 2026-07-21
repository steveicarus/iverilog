// Accellera Standard V2.8.1 Open Verification Library (OVL.
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_valid_id (clock, reset, enable, issued, issued_id, returned, returned_id, flush, flush_id, issued_count, fire);

  parameter severity_level   = `OVL_SEVERITY_DEFAULT;
  parameter min_cks          = 1;
  parameter max_cks          = 1;
  parameter width            = 2;
  parameter max_id_instances          = 2;
  parameter max_ids      = 1;
  parameter max_instances_per_id   = 1;
  parameter instance_count_width       = 2;
  parameter property_type    = `OVL_PROPERTY_DEFAULT;
  parameter msg              = `OVL_MSG_DEFAULT;
  parameter coverage_level   = `OVL_COVER_DEFAULT;

  parameter clock_edge       = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity   = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type      = `OVL_GATING_TYPE_DEFAULT;

  input                            clock, reset, enable;
  input                            issued, returned, flush;
  input  [width-1 : 0]             issued_id;
  input  [width-1 : 0]             returned_id;
  input  [width-1 : 0]             flush_id;
  input  [instance_count_width-1 : 0]        issued_count;
  output [`OVL_FIRE_WIDTH-1 : 0]   fire;

  // Parameters that should not be edited
  parameter assert_name = "OVL_VALID_ID";

  `include "std_ovl_reset.h"
  `include "std_ovl_clock.h"
  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_SVA
  `include "./sva05/ovl_valid_id_logic.sv"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`endmodule // ovl_valid_id

