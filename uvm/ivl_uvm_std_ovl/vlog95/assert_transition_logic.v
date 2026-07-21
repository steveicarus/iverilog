// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_test_expr;
  wire valid_start_state;
  wire valid_next_state;

  assign valid_test_expr = ~((^test_expr)^(^test_expr));
  assign valid_start_state = ~((^start_state)^(^start_state));
  assign valid_next_state = ~((^next_state)^(^next_state));
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_SHARED_CODE

  reg [width-1:0] r_start_state, r_next_state;

  reg assert_state;

`ifdef OVL_SYNTHESIS
`else
  initial begin
    assert_state = 1'b0;
  end
`endif

  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL != 1'b0) begin

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  `ifdef OVL_ASSERT_ON
    // Do the x/z checking
            if (valid_test_expr == 1'b1)
              begin
                // Do nothing
              end
            else
              begin
                  ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
              end

            if (valid_start_state == 1'b1)
              begin
                // Do nothing
              end
            else
              begin
                  ovl_error_t(`OVL_FIRE_XCHECK,"start_state contains X or Z");
              end

            if (valid_next_state == 1'b1)
              begin
                // Do nothing
              end
            else
              begin
                if (start_state != test_expr)
                 begin
                   // Do Nothing
                 end
                else
                 begin
                  ovl_error_t(`OVL_FIRE_XCHECK,"next_state contains X or Z");
                 end
              end
  `endif // OVL_ASSERT_ON
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

      if (assert_state == 1'b0) begin // INIT_STATE
        if (test_expr == start_state) begin
 `ifdef OVL_COVER_ON
            if (coverage_level != `OVL_COVER_NONE) begin
             if (OVL_COVER_BASIC_ON) begin //basic coverage
              ovl_cover_t("start_state covered");
             end
            end
 `endif // OVL_COVER_ON
          assert_state  <= 1'b1; // CHECK_STATE
          r_start_state <= start_state;
          r_next_state  <= next_state;
        end
      end
      else begin                      // CHECK_STATE
        if (test_expr == r_next_state) begin
          if (test_expr == start_state) begin
 `ifdef OVL_COVER_ON
              if (coverage_level != `OVL_COVER_NONE) begin
               if (OVL_COVER_BASIC_ON) begin //basic coverage
                ovl_cover_t("start_state covered");
               end
              end
 `endif // OVL_COVER_ON
            assert_state  <= 1'b1; // CHECK_STATE
            r_start_state <= start_state;
            r_next_state  <= next_state;
          end
          else
            assert_state <= 1'b0; // done ok.
        end
        else if (test_expr != r_start_state) begin
 `ifdef OVL_ASSERT_ON
            ovl_error_t(`OVL_FIRE_2STATE,"Test expression transitioned from value start_state to a value other than next_state");    // test_expr moves to unexpected state
 `endif // OVL_ASSERT_ON
          if (test_expr == start_state) begin
 `ifdef OVL_COVER_ON
              if (coverage_level != `OVL_COVER_NONE) begin
               if (OVL_COVER_BASIC_ON) begin //basic coverage
                ovl_cover_t("start_state covered");
               end
              end
 `endif // OVL_COVER_ON
            assert_state  <= 1'b1; // CHECK_STATE
            r_start_state <= start_state;
            r_next_state  <= next_state;
          end
          else
            assert_state <= 1'b0; // done error.
        end
      end
    end
    else begin
      assert_state <= 1'b0;
      r_start_state <= {width{1'b0}};
      r_next_state <= {width{1'b0}};
    end

  end // always

`endif // OVL_SHARED_CODE
