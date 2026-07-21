// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.


`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_start_event;
  wire valid_test_expr;
  assign valid_start_event = ~(start_event ^ start_event);
  assign valid_test_expr   = ~(test_expr   ^ test_expr);
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF


`ifdef OVL_ASSERT_ON

  // local parameters
  parameter num_cks = (max_cks>min_cks)?max_cks:min_cks;
  parameter FRAME_START = 1'b0;
  parameter FRAME_CHECK = 1'b1;

  reg r_state;
  reg r_start_event;

  integer ii;

  always @(posedge clk)
    if (`OVL_RESET_SIGNAL == 1'b0) // active low reset
      r_start_event <= 1'b1; // reset high to avoid fail straight out of reset
    else
      r_start_event <= start_event;

  always @(posedge clk) begin
      if (`OVL_RESET_SIGNAL != 1'b0) begin // active low reset
        case (r_state)
          FRAME_START: begin

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
   // Do the x/z checking
            if (valid_start_event == 1'b1)
              begin
                // Do nothing
              end
            else
              begin
                ovl_error_t(`OVL_FIRE_XCHECK,"start_event contains X or Z");
              end
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

            // assert_frame() behaves like assert_implication()
            //     when min_cks==0 and max_cks==0
            if ((min_cks==0) && (max_cks==0)) begin

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
   // Do the x/z checking
              if ( (r_start_event == 1'b0) && (start_event == 1'b1) )
                begin
                  if (valid_test_expr == 1'b1)
                    begin
                      // Do nothing
                    end
                  else
                    begin
                      ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
                    end
                end
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

              if ((r_start_event == 1'b0) && (start_event==1'b1) &&
                  (test_expr==1'b0)) begin
                // FAIL, it does not behave like assert_implication()
                ovl_error_t(`OVL_FIRE_2STATE,"Test expression is not TRUE while start event is asserted when both parameters min_cks and max_cks are set to 0");
              end
            end
            // wait for start_event (0->1)
            else if ((r_start_event == 1'b0) && (start_event == 1'b1)) begin

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
   // Do the x/z checking
              if (valid_test_expr == 1'b1)
                begin
                  // Do nothing
                end
              else
                begin
                  ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
                end
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

              if ((min_cks != 0) && (test_expr==1'b1)) begin
                // FAIL, test_expr should not happen before min_cks
                ovl_error_t(`OVL_FIRE_2STATE,"Test expression is TRUE before elapse of specified minimum min_cks cycles from start event");
              end
              else if (!(min_cks == 0 && test_expr == 1'b1)) begin
                r_state <= FRAME_CHECK;
                ii <= 1;
              end
            end
          end
          FRAME_CHECK:
            // start_event (0->1) has occurred
            // start checking
            begin
              // Count clock ticks

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
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
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

              if ((r_start_event == 1'b0) && (start_event == 1'b1)) begin
                if ((min_cks != 0) && (test_expr==1'b1) &&
                    (action_on_new_start == `OVL_RESET_ON_NEW_START)) begin
                  // FAIL, test_expr should not happen before min_cks
                  ovl_error_t(`OVL_FIRE_2STATE,"Test expression is TRUE before elapse of specified minimum min_cks cycles from start event");
                  r_state <= FRAME_START;
                end
                // start_event (0->1) happens again -- re-started!!!
                else if (action_on_new_start == `OVL_IGNORE_NEW_START) begin
                  if (max_cks) ii <= ii + 1;
                  else if (ii < min_cks) ii <= ii + 1;
                end
                else if (action_on_new_start == `OVL_RESET_ON_NEW_START)
                  ii <= 1;
                else if (action_on_new_start == `OVL_ERROR_ON_NEW_START) begin
                  ovl_error_t(`OVL_FIRE_2STATE,"Illegal start event which has reoccured before completion of current window");
                  if (max_cks) ii <= ii + 1;
                  else if (ii < min_cks) ii <= ii + 1;
                end
              end
              else begin
                  if (max_cks) ii <= ii + 1;
                  else if (ii < min_cks) ii <= ii + 1;
              end

              // Check for (0,0), (0,M), (m,0), (m,M) conditions
              if (min_cks == 0) begin
                if (max_cks == 0) begin
                  // (0,0): (min_cks==0, max_cks==0)
                  // This condition is UN-REACHABLE!!!
                  ovl_error_t(`OVL_FIRE_2STATE,"Test expression is not TRUE while start event is asserted when both parameters min_cks and max_cks are set to 0");
                  r_state <= FRAME_START;
                end
                else begin // max_cks > 0

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
   // Do the x/z checking
                  if (valid_test_expr == 1'b1)
                    begin
                      // Do nothing
                    end
                  else
                    begin
                        ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
                    end
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

                  // (0,M): (min_cks==0, max_cks>0)
                  if (test_expr == 1'b1) begin
                    // OK, ckeck is done. Go to FRAME_START state for next check.
                    r_state <= FRAME_START;
                  end
                  else begin
                    if (ii == max_cks &&
                        !(r_start_event == 1'b0 && start_event == 1'b1 &&
                          action_on_new_start == `OVL_RESET_ON_NEW_START)
                       ) begin
                      // FAIL, test_expr does not happen at/before max_cks
                      ovl_error_t(`OVL_FIRE_2STATE,"Test expression is not TRUE within specified maximum max_cks cycles from start event");
                      r_state <= FRAME_START;
                    end
                  end
                end
              end
              else begin // min_cks > 0
                if (max_cks == 0) begin
                  // (m,0): (min_cks>0, max_cks==0)
                  if (ii == min_cks) begin
                      // OK, test_expr does not happen before min_cks
                      r_state <= FRAME_START;
                  end
                  else begin
                    if ((test_expr == 1'b1) &&
                        !(r_start_event == 1'b0 && start_event == 1'b1 &&
                          action_on_new_start == `OVL_RESET_ON_NEW_START)
                       ) begin
                      // FAIL, test_expr should not happen before min_cks
                      ovl_error_t(`OVL_FIRE_2STATE,"Test expression is TRUE before elapse of specified minimum min_cks cycles from start event");
                      r_state <= FRAME_START;
                    end

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
   // Do the x/z checking
                    if (valid_test_expr == 1'b1)
                      begin
                        // Do nothing
                      end
                    else
                      begin
                          ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
                      end
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

                  end
                end
                else begin // max_cks > 0
                  // (m,M): (min_cks>0, max_cks>0)
                  if (test_expr == 1'b1) begin
                    r_state <= FRAME_START;
                    if (ii < min_cks &&
                        !(r_start_event == 1'b0 && start_event == 1'b1 &&
                          action_on_new_start == `OVL_RESET_ON_NEW_START)
                       ) begin
                      // FAIL, test_expr should not happen before min_cks
                      ovl_error_t(`OVL_FIRE_2STATE,"Test expression is TRUE before elapse of specified minimum min_cks cycles from start event");
                    end
                    // else OK, we are done!!!
                  end
                  else begin
                    if (ii == max_cks &&
                        !(r_start_event == 1'b0 && start_event == 1'b1 &&
                          action_on_new_start == `OVL_RESET_ON_NEW_START)
                       ) begin
                      // FAIL, test_expr does not happen at/before max_cks
                      ovl_error_t(`OVL_FIRE_2STATE,"Test expression is not TRUE within specified maximum max_cks cycles from start event");
                      r_state <= FRAME_START;
                    end
                  end
`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
   // Do the x/z checking
                  if (valid_test_expr == 1'b1)
                    begin
                      // Do nothing
                    end
                  else
                    begin
                        ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
                    end
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

                end
              end
            end
        endcase
      end
      else begin
         r_state <= FRAME_START;
         ii      <= 0;
      end
  end // always

`endif // OVL_ASSERT_ON

`ifdef OVL_COVER_ON

  reg prev_start_event;

  always @(posedge clk) begin
   if (`OVL_RESET_SIGNAL != 1'b0) begin
    if (coverage_level != `OVL_COVER_NONE) begin

     if (OVL_COVER_BASIC_ON) begin //basic coverage
      if ((start_event != prev_start_event) && (prev_start_event == 1'b0)) begin
        ovl_cover_t("start_event covered");
      end
      prev_start_event <= start_event;
     end //basic coverage

    end // OVL_COVER_NONE
   end
   else begin
     prev_start_event <= 1'b0;
   end
  end //always

`endif // OVL_COVER_ON
