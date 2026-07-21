// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`ifdef OVL_ASSERT_ON

  always @ (`OVL_RESET_SIGNAL or test_expr) begin
    if (`OVL_RESET_SIGNAL != 1'b0) begin
      if ((test_expr ^ test_expr)=={width{1'b0}})
        ;// do nothing; test_expr contains no X or Z
      else begin
`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
   ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
`endif // OVL_XCHECK_OFF
      end
    end
  end // always

`endif // OVL_ASSERT_ON
