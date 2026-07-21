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
  wire fire_2state_1;
  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
    end
    else begin
      if (fire_2state_1) begin
        ovl_error_t(`OVL_FIRE_2STATE,"Test expression evaluates to a value outside the range specified by parameters min and max");
      end
    end
  end

  assign fire_2state_1 = ((test_expr < min) || (test_expr > max));

  // X-CHECK
  // =======
  `ifdef OVL_XCHECK_OFF
  `else
    `ifdef OVL_IMPLICIT_XCHECK_OFF
    `else
      reg fire_xcheck_1;
      always @(posedge clk) begin
        if (`OVL_RESET_SIGNAL == 1'b0) begin
          // OVL does not fire during reset
        end
        else begin
          if (fire_xcheck_1) begin
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

    `endif // OVL_IMPLICIT_XCHECK_OFF
  `endif // OVL_XCHECK_OFF

`endif // OVL_ASSERT_ON


//------------------------------------------------------------------------------
// COVERAGE
//------------------------------------------------------------------------------
`ifdef OVL_COVER_ON

  // Auxiliary logic
  reg [width-1:0] prev_test_expr;
  always @ (posedge clk) begin
    // REVISIT: update only if SANITY on?
    prev_test_expr <= test_expr; // deliberately not reset
  end
 
  wire fire_cover_1, fire_cover_2, fire_cover_3;
  always @ (posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
    end
    else begin
      if (fire_cover_1) begin
        ovl_cover_t("test_expr_change covered"); // sanity
      end
      if (fire_cover_2) begin
        ovl_cover_t("test_expr_at_min covered"); // corner
      end
      if (fire_cover_3) begin
        ovl_cover_t("test_expr_at_max covered"); // corner
      end
    end
  end

  assign fire_cover_1 = ((OVL_COVER_SANITY_ON > 0) && (test_expr != prev_test_expr));
  assign fire_cover_2 = ((OVL_COVER_CORNER_ON > 0) && (test_expr == min));
  assign fire_cover_3 = ((OVL_COVER_CORNER_ON > 0) && (test_expr == max));

`endif // OVL_COVER_ON
