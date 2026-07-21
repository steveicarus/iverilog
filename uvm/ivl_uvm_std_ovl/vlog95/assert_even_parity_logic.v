// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.



`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_test_expr;
  assign valid_test_expr = ~((^test_expr) ^ (^test_expr));
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_ASSERT_ON
  always @(posedge clk) begin
      if (`OVL_RESET_SIGNAL != 1'b0) begin
        if ((^(test_expr)) == 1'b1) begin
          ovl_error_t(`OVL_FIRE_2STATE,"Test expression does not exhibit even parity");
        end
      end
  end // always

`endif // OVL_ASSERT_ON

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
 `ifdef OVL_ASSERT_ON
  always @(posedge clk)
    begin
      if (`OVL_RESET_SIGNAL != 1'b0)
        begin
          if (valid_test_expr == 1'b1)
            begin
              // Do nothing
            end
          else
            ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
        end
    end
 `endif // OVL_ASSERT_ON
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_COVER_ON

  reg [width-1:0] prev_test_expr;

  always @(posedge clk) begin

   if (`OVL_RESET_SIGNAL != 1'b0) begin
    if (coverage_level != `OVL_COVER_NONE) begin

     if (OVL_COVER_SANITY_ON) begin //sanity coverage
      if (test_expr != prev_test_expr) begin
        ovl_cover_t("test_expr_change covered");
      end
      prev_test_expr <= test_expr;
     end //sanity coverage

    end // OVL_COVER_NONE
   end
   else begin
`ifdef OVL_INIT_REG
      prev_test_expr <= {width{1'b0}};
`endif
   end
  end //always

`endif // OVL_COVER_ON

