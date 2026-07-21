// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

// local paramaters used as defines
  parameter TIME_START = 1'b0;
  parameter TIME_CHECK = 1'b1;

  reg [31:0] i;

  reg r_state;

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_start_event;
  wire valid_test_expr;
  assign valid_start_event = ~(start_event^start_event);
  assign valid_test_expr = ~((^test_expr)^(^test_expr));
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_SYNTHESIS
`else
  initial begin
    r_state=TIME_START;
  end
`endif


`ifdef OVL_SHARED_CODE

  always @(posedge clk) begin
      if (`OVL_RESET_SIGNAL != 1'b0) begin // active low reset
        case (r_state)
          TIME_START: begin

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  `ifdef OVL_ASSERT_ON
    // Do the x/z checking
            if (valid_start_event == 1'b1)
              begin
                // Do nothing
              end
            else
              begin
                  ovl_error_t(`OVL_FIRE_XCHECK,"start_event contains X or Z");
              end
  `endif // OVL_ASSERT_ON
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

            if (start_event == 1'b1) begin
              r_state <= TIME_CHECK;
              i <= num_cks;

 `ifdef OVL_COVER_ON
              if (coverage_level != `OVL_COVER_NONE) begin
               if (OVL_COVER_BASIC_ON) begin //basic coverage
                ovl_cover_t("window_open covered");
               end
              end
 `endif // OVL_COVER_ON
            end
          end
          TIME_CHECK: begin

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  `ifdef OVL_ASSERT_ON
    // Do the x/z checking
            if (action_on_new_start != `OVL_IGNORE_NEW_START)
              begin
                if (valid_start_event == 1'b1)
                  begin
                    // Do nothing
                  end
                else
                  begin
                    ovl_error_t(`OVL_FIRE_XCHECK,"start_event contains X or Z");
                  end
              end
            if (valid_test_expr == 1'b1)
              begin
                // Do nothing
              end
            else
              begin
                  ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
              end
  `endif // OVL_ASSERT_ON
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

              // Count clock ticks
              if (start_event == 1'b1) begin
                if (action_on_new_start == `OVL_IGNORE_NEW_START)
                  i <= i-1;
                else if (action_on_new_start == `OVL_RESET_ON_NEW_START) begin
                  i <= num_cks;
 `ifdef OVL_COVER_ON
                  if (coverage_level != `OVL_COVER_NONE) begin
                   if (OVL_COVER_CORNER_ON) begin //corner coverage
                    if (action_on_new_start == `OVL_RESET_ON_NEW_START) begin
                      ovl_cover_t("window_resets covered");
                    end
                   end
                  end
 `endif // OVL_COVER_ON
                end
                else if (action_on_new_start == `OVL_ERROR_ON_NEW_START) begin
                  i <= i-1;
 `ifdef OVL_ASSERT_ON
                  ovl_error_t(`OVL_FIRE_2STATE,"Illegal start event which has reoccured before completion of current window");
 `endif // OVL_ASSERT_ON
                end
              end
              else
                i <= i-1;

              // Check that the property is true
 `ifdef OVL_ASSERT_ON
              if (test_expr != 1'b1 &&
                  !(start_event == 1'b1 &&
                    action_on_new_start == `OVL_RESET_ON_NEW_START)) begin
                ovl_error_t(`OVL_FIRE_2STATE,"Test expression is not TRUE within specified num_cks cycles from the start_event");
              end
 `endif // OVL_ASSERT_ON

              // go to start state on last time check
              // NOTE: i == 0 at end of current simulation
              // timeframe due to non-blocking assignment!
              // Hence, check  i == 1.
              if (i == 1 && !(start_event == 1'b1 &&
                              action_on_new_start == `OVL_RESET_ON_NEW_START))
              begin
                r_state <= TIME_START;
 `ifdef OVL_COVER_ON
                if (coverage_level != `OVL_COVER_NONE) begin
                 if (OVL_COVER_BASIC_ON) begin //basic coverage
                  ovl_cover_t("window_close covered");
                 end
                end
 `endif // OVL_COVER_ON
              end
          end
        endcase
      end
      else begin
        r_state <= TIME_START;
        i <= {32{1'b0}};
      end
  end // always

`endif // OVL_SHARED_CODE
