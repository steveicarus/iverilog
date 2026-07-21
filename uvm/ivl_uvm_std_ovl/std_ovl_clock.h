// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

wire clk;

`ifdef OVL_SHARED_CODE
  wire gclk;

  `ifdef OVL_GATING_OFF
    assign gclk = clock; // Globally disabled gating
  `else
    // LATCH based gated clock
    reg  clken;
    always @ (clock or enable) begin
      if (clock == 1'b0)
        clken <= enable;
    end
    assign gclk = (gating_type == `OVL_GATE_CLOCK) ? clock & clken
                                                   : clock; // Locally disabled gating
  `endif // OVL_GATING_OFF

  // clk (programmable edge & optional gating)
  assign clk = (clock_edge == `OVL_POSEDGE) ? gclk : ~gclk;
`else
  assign clk = clock;
`endif // OVL_SHARED_CODE
