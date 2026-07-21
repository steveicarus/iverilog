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

  reg [width-1:0] last_test_expr;
  reg [width:0] temp_expr;
  reg r_reset_n;

`ifdef OVL_SYNTHESIS
`else
  initial begin
    r_reset_n = 1'b0;
  end
`endif

  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL != 1'b0) begin
      r_reset_n <= `OVL_RESET_SIGNAL;
      last_test_expr <= test_expr;

      // check second clock after reset
      if (r_reset_n && (last_test_expr != test_expr)) begin
        temp_expr = {1'b0,last_test_expr} - {1'b0,test_expr};
      // 2's complement result
        if (temp_expr[width-1:0] != value) begin
          ovl_error_t(`OVL_FIRE_2STATE,"Test expression is decreased by a value other than specified");
        end
      end
    end
    else begin
      r_reset_n <= 0;
`ifdef OVL_INIT_REG
      last_test_expr <= {width{1'b0}};
      temp_expr = {(width+1){1'b0}};
`endif
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

     if (OVL_COVER_BASIC_ON) begin //basic coverage
      if (test_expr != prev_test_expr) begin
        ovl_cover_t("test_expr_change covered");
      end
      prev_test_expr <= test_expr;
     end //basic coverage

    end // OVL_COVER_NONE
   end
   else begin
`ifdef OVL_INIT_REG
    prev_test_expr <= {width{1'b0}};
`endif
   end
  end //always

`endif // OVL_COVER_ON

