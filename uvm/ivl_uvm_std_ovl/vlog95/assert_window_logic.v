// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

// local paramaters used as defines
  parameter WINDOW_START = 1'b0;
  parameter WINDOW_CHECK = 1'b1;

  reg r_state;

`ifdef OVL_SYNTHESIS
`else
  initial begin
    r_state=WINDOW_START;
  end
`endif

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


`ifdef OVL_SHARED_CODE

  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL != 1'b0) begin
      case (r_state)
        WINDOW_START: begin
          if (start_event == 1'b1) begin
            r_state <= WINDOW_CHECK;
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

        end // WINDOW_START
        WINDOW_CHECK: begin
          if (end_event == 1'b1) begin
            r_state <= WINDOW_START;
 `ifdef OVL_COVER_ON
            if (coverage_level != `OVL_COVER_NONE) begin
             if (OVL_COVER_BASIC_ON) begin //basic coverage
              ovl_cover_t("window covered");
             end
            end
 `endif // OVL_COVER_ON
          end
 `ifdef OVL_ASSERT_ON
          if (test_expr != 1'b1) begin
            ovl_error_t(`OVL_FIRE_2STATE,"Test expression changed value during an open event window");
          end
 `endif // OVL_ASSERT_ON

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

        end // WINDOW_CHECK
      endcase
    end
    else begin
      r_state <= WINDOW_START;
    end

  end // always

`endif // OVL_SHARED_CODE

