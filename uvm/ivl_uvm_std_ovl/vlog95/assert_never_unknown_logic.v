// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.


//------------------------------------------------------------------------------
// SHARED CODE
//------------------------------------------------------------------------------
// No shared code for this OVL


//------------------------------------------------------------------------------
// ASSERTION
//------------------------------------------------------------------------------
`ifdef OVL_ASSERT_ON

  // 2-STATE
  // =======

  // X-CHECK
  // =======
  `ifdef OVL_XCHECK_OFF
  `else
    reg fire_xcheck_1;
    always @(posedge clk) begin
      if (`OVL_RESET_SIGNAL == 1'b0) begin
        // OVL does not fire during reset
      end
      else begin
        if (qualifier & fire_xcheck_1) begin
          ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
        end
      end
    end

    wire valid_test_expr = ((test_expr ^ test_expr) == 1'b0);

    always @ (valid_test_expr) begin
      if (valid_test_expr) begin
        fire_xcheck_1 = 1'b0;
      end
      else begin
        fire_xcheck_1 = 1'b1;
      end
    end

  `endif // OVL_XCHECK_OFF

`endif // OVL_ASSERT_ON


//------------------------------------------------------------------------------
// COVERAGE
//------------------------------------------------------------------------------
`ifdef OVL_COVER_ON

  // Auxiliary logic
  reg [width-1:0] prev_test_expr;
  always @ (posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      prev_test_expr <= {width{1'b0}};
    end
    else begin
      if (qualifier) begin
        prev_test_expr <= test_expr;
      end
    end
  end

  wire fire_cover_1, fire_cover_2;
  always @ (posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
    end
    else begin
      if (fire_cover_1) begin
        ovl_cover_t("qualifier covered"); // basic
      end
      if (fire_cover_2) begin
        ovl_cover_t("test_expr_change covered"); // sanity
      end
    end
  end

  assign fire_cover_1 = ((OVL_COVER_BASIC_ON  > 0) && (qualifier == 1'b1));
  assign fire_cover_2 = ((OVL_COVER_SANITY_ON > 0) && (qualifier == 1'b1) && (test_expr != prev_test_expr));

`endif // OVL_COVER_ON
