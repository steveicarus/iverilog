// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

wire reset_n;

`ifdef OVL_SHARED_CODE
  wire greset;

  `ifdef OVL_GATING_OFF
    assign greset = reset; // Globally disabled gating
  `else
    assign greset = (gating_type == `OVL_GATE_RESET) ? reset & enable
                                                     : reset; // Locally disabled gating
  `endif // OVL_GATING_OFF

  // reset_n (programmable polarity & optional gating)
  assign reset_n = (reset_polarity == `OVL_ACTIVE_LOW) ? greset : ~greset;
`else
  assign reset_n = reset;
`endif // OVL_SHARED_CODE
