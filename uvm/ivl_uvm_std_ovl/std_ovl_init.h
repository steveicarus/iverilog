// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`ifdef OVL_SHARED_CODE
  `ifdef OVL_SYNTHESIS
  `else
    `ifdef OVL_INIT_MSG
      initial
        ovl_init_msg_t; // Call the User Defined Init Message Routine
    `endif // OVL_INIT_MSG
  `endif // OVL_SYNTHESIS
`endif // OVL_SHARED_CODE
