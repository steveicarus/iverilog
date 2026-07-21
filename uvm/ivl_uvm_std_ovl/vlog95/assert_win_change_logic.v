// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

  reg r_change;
  reg [width-1:0] r_test_expr;
  reg r_state;

  parameter WIN_CHANGE_START = 1'b0;
  parameter WIN_CHANGE_CHECK = 1'b1;

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
   wire valid_start_event;
   wire valid_test_expr;
   wire valid_end_event;

   assign valid_start_event = ~(start_event^start_event);
   assign valid_test_expr = ~((^test_expr)^(^test_expr));
   assign valid_end_event = ~(end_event^end_event);
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_SYNTHESIS
`else
  initial begin
    r_state=WIN_CHANGE_START;
    r_change=1'b0;
  end
`endif


`ifdef OVL_SHARED_CODE

  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL != 1'b0) begin
      case (r_state)
        WIN_CHANGE_START: begin

          if (start_event == 1'b1) begin
            r_change <= 1'b0;
            r_state <= WIN_CHANGE_CHECK;
            r_test_expr <= test_expr;

 `ifdef OVL_COVER_ON
            if (coverage_level != `OVL_COVER_NONE) begin
             if (OVL_COVER_BASIC_ON) begin //basic coverage
              ovl_cover_t("window_open covered");
             end
            end
 `endif // OVL_COVER_ON
          end

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  `ifdef OVL_ASSERT_ON
          if (valid_start_event == 1'b1)
            begin
              //Do Nothing
            end
          else
            begin
              ovl_error_t(`OVL_FIRE_XCHECK,"start_event contains X or Z");
            end
  `endif // OVL_ASSERT_ON
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

        end
        WIN_CHANGE_CHECK: begin

            if (r_test_expr != test_expr) begin
              r_change <= 1'b1;
            end

            // go to start state on last check
            if (end_event == 1'b1) begin
              r_state <= WIN_CHANGE_START;

 `ifdef OVL_COVER_ON
              if (coverage_level != `OVL_COVER_NONE) begin
               if (OVL_COVER_BASIC_ON) begin //basic coverage
                ovl_cover_t("window covered");
               end
              end
 `endif // OVL_COVER_ON

              // Check that the property is true
 `ifdef OVL_ASSERT_ON
              if ((r_change != 1'b1) && (r_test_expr == test_expr)) begin
                ovl_error_t(`OVL_FIRE_2STATE,"Test expression has not changed value before window is closed");
              end
 `endif // OVL_ASSERT_ON
            end

            r_test_expr <= test_expr;

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  `ifdef OVL_ASSERT_ON
            if (valid_test_expr == 1'b1)
              begin
                //Do Nothing
              end
            else
              begin
                ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
              end

            if (valid_end_event == 1'b1)
              begin
                //Do Nothing
              end
            else
              begin
                ovl_error_t(`OVL_FIRE_XCHECK,"end_event contains X or Z");
              end
  `endif // OVL_ASSERT_ON
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

          end
      endcase
    end
    else begin
      r_state<=WIN_CHANGE_START;
      r_change<=1'b0;
      r_test_expr <= {width{1'b0}};
    end

  end // always

`endif // OVL_SHARED_CODE

