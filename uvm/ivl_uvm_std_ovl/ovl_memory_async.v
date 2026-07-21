// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`include "std_ovl_defines.h"

`module ovl_memory_async (reset, enable, start_addr, end_addr, ren, raddr, rdata, wen, waddr,
                          wdata, fire);

  parameter severity_level   = `OVL_SEVERITY_DEFAULT;
  parameter data_width       = 1;
  parameter addr_width       = 1;
  parameter mem_size         = 2;
  parameter addr_check       = 1;
  parameter init_check         = 1;
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
  input                            ren, wen;
  input  [addr_width-1 : 0]        start_addr;
  input  [addr_width-1 : 0]        end_addr;
  input  [addr_width-1 : 0]        raddr;
  input  [data_width-1 : 0]        rdata;
  input  [addr_width-1 : 0]        waddr;
  input  [data_width-1 : 0]        wdata;

  output [`OVL_FIRE_WIDTH-1 : 0]   fire;

  // Parameters that should not be edited
  parameter assert_name = "OVL_MEMORY_ASYNC";

`ifdef OVL_EVERYTHING_OFF
  // No logic if ASSERT & COVER both OFF
`else
  // latch based gated clock

 wire   ren_clk, wen_clk;

`ifdef OVL_SHARED_CODE

  wire ren_gclk, wen_gclk;

 `ifdef OVL_GATING_OFF
    assign ren_gclk = ren; // Globally disabled gating
    assign wen_gclk = wen; 
  `else
    // LATCH based gated clock

  reg  ren_clken, wen_clken;

  always @ (ren or enable) begin
    if (ren == 1'b0)
      ren_clken <= enable;
  end

  always @ (wen or enable) begin
    if (wen == 1'b0)
      wen_clken <= enable;
  end

  assign ren_gclk = (gating_type == `OVL_GATE_CLOCK) ? ren & ren_clken : ren;
  assign wen_gclk = (gating_type == `OVL_GATE_CLOCK) ? wen & wen_clken : wen;

  `endif // OVL_GATING_OFF

  // clk (programmable edge)

  assign ren_clk = (ren_edge == `OVL_POSEDGE) ? ren_gclk : ~ren_gclk;
  assign wen_clk = (wen_edge == `OVL_POSEDGE) ? wen_gclk : ~wen_gclk;

`else
  assign ren_clk = ren;
  assign wen_clk = wen;

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
  `include "./sva05/ovl_memory_async_logic.sv"
  assign fire = {`OVL_FIRE_WIDTH{1'b0}}; // Tied low in V2.3
`endif

`endmodule // ovl_memory_async

