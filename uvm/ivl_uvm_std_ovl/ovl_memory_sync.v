// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_memory_sync (reset, enable, start_addr, end_addr, r_clock, ren, raddr, rdata,
                         w_clock, wen, waddr, wdata, fire);

  parameter severity_level   = `OVL_SEVERITY_DEFAULT;
  parameter data_width       = 1;
  parameter addr_width       = 1;
  parameter mem_size         = 2;
  parameter addr_check         = 1;
  parameter init_check         = 1;
  parameter conflict_check     = 0;
  parameter pass_thru        = 0;
  parameter one_read_check        = 0;
  parameter one_write_check       = 0;
  parameter value_check        = 0;
  parameter property_type    = `OVL_PROPERTY_DEFAULT;
  parameter msg              = `OVL_MSG_DEFAULT;
  parameter coverage_level   = `OVL_COVER_DEFAULT;

  parameter wen_edge     = `OVL_CLOCK_EDGE_DEFAULT;
  parameter ren_edge     = `OVL_CLOCK_EDGE_DEFAULT;
  parameter reset_polarity   = `OVL_RESET_POLARITY_DEFAULT;
  parameter gating_type      = `OVL_GATING_TYPE_DEFAULT;

  input                            reset, enable;
  input                            ren, wen, r_clock, w_clock;
  input  [addr_width-1 : 0]        start_addr;
  input  [addr_width-1 : 0]        end_addr;
  input  [addr_width-1 : 0]        raddr;
  input  [data_width-1 : 0]        rdata;
  input  [addr_width-1 : 0]        waddr;
  input  [data_width-1 : 0]        wdata;
  output [`OVL_FIRE_WIDTH-1 : 0]   fire;

  // Parameters that should not be edited
  parameter assert_name = "OVL_MEMORY_SYNC";

`ifdef OVL_EVERYTHING_OFF
  // No logic if ASSERT & COVER both OFF
`else
  // latch based gated clock

  wire rd_clk, wr_clk;

`ifdef OVL_SHARED_CODE
  wire rd_gclk, wr_gclk;


 `ifdef OVL_GATING_OFF
  assign rd_gclk = r_clock; // Globally disabled gating
  assign wr_gclk = w_clock;
 `else
   // LATCH based gated clock
  reg  rd_clken, wr_clken;

  always @ (r_clock or enable) begin
    if (r_clock == 1'b0)
      rd_clken <= enable;
  end

  always @ (w_clock or enable) begin
    if (w_clock == 1'b0)
      wr_clken <= enable;
  end

  assign rd_gclk = (gating_type == `OVL_GATE_CLOCK) ? r_clock & rd_clken : r_clock;
  assign wr_gclk = (gating_type == `OVL_GATE_CLOCK) ? w_clock & wr_clken : w_clock;

`endif // OVL_GATING_OFF

  // clk (programmable edge)

  assign rd_clk = (ren_edge == `OVL_POSEDGE) ? rd_gclk : ~rd_gclk;
  assign wr_clk = (wen_edge == `OVL_POSEDGE) ? wr_gclk : ~wr_gclk;

`else
  assign rd_clk = r_clock;
  assign wr_clk = w_clock;

`endif // OVL_SHARED_CODE

  // reset_n (programmable polarity & optional gating)

  wire   reset_n;
  assign reset_n = (gating_type == `OVL_GATE_RESET) ? ((reset_polarity == `OVL_ACTIVE_LOW) ? reset & enable : ~reset & enable)
                                                    : ((reset_polarity == `OVL_ACTIVE_LOW) ? reset          : ~reset);
`endif // OVL_EVERYTHING_OFF

  `include "std_ovl_cover.h"
  `include "std_ovl_task.h"
  `include "std_ovl_init.h"

`ifdef OVL_SVA
  `include "./sva05/ovl_memory_sync_logic.sv"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`endmodule // ovl_memory_sync

