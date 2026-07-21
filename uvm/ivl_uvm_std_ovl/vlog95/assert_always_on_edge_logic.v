// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

  reg sampling_event_prev;
  reg r_reset_n;

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_sampling_event;
  wire valid_test_expr;
  wire valid_sampling_event_prev;
  assign valid_sampling_event = ~(sampling_event^sampling_event);
  assign valid_test_expr = ~(test_expr^test_expr);
  assign valid_sampling_event_prev = ~(sampling_event_prev ^
                                       sampling_event_prev);
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_ASSERT_ON

`ifdef OVL_SYNTHESIS
`else
  initial begin
    r_reset_n = 1'b0;
  end
`endif

  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL != 1'b0) begin
      r_reset_n <= `OVL_RESET_SIGNAL;
// Capture Sampling Event @Clock for rising edge detections
      sampling_event_prev <= sampling_event;
      if ((edge_type == `OVL_NOEDGE) && (!test_expr))
        ovl_error_t(`OVL_FIRE_2STATE,"Test expression is FALSE irrespective of sampling event");
      else if ((edge_type == `OVL_POSEDGE) && (!sampling_event_prev) &&
	       (sampling_event) && (!test_expr) && r_reset_n)
        ovl_error_t(`OVL_FIRE_2STATE,"Test expression is FALSE on posedge of sampling event");
      else if ((edge_type == `OVL_NEGEDGE) && (sampling_event_prev) &&
	       (!sampling_event) && (!test_expr) && r_reset_n)
        ovl_error_t(`OVL_FIRE_2STATE,"Test expression is FALSE on negedge of sampling event");
      else if ((edge_type == `OVL_ANYEDGE) &&
	       (sampling_event_prev != sampling_event) && (!test_expr) &&
               r_reset_n)
        ovl_error_t(`OVL_FIRE_2STATE,"Test expression is FALSE on any edge of sampling event");

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else

//x/z checking for test_expr

      if ( (edge_type == `OVL_NOEDGE) ||
           ((edge_type == `OVL_POSEDGE) && (!sampling_event_prev) &&
              (sampling_event) && r_reset_n) ||
           ((edge_type == `OVL_NEGEDGE) && (sampling_event_prev) &&
              (!sampling_event) && r_reset_n) ||
           ((edge_type == `OVL_ANYEDGE) &&
            (sampling_event_prev != sampling_event) && r_reset_n) )
        begin
          if ( valid_test_expr == 1'b1 )
            begin
              //Do nothing
            end
          else
            begin
              ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
            end
        end

//x/z checking for sampling_event

      if ( r_reset_n && (edge_type != `OVL_NOEDGE) )
        begin
          if ( valid_sampling_event == 1'b1 )
            begin
              if ( valid_sampling_event_prev == 1'b1 )
                begin
                  //Do nothing
                end
              else if ( ( (edge_type == `OVL_POSEDGE) &&
                          (sampling_event == 1'b1) ) ||
                        ( (edge_type == `OVL_NEGEDGE) &&
                          (sampling_event == 1'b0) ) ||
                        ( (edge_type == `OVL_ANYEDGE) ) )
                begin
                  ovl_error_t(`OVL_FIRE_XCHECK,"sampling_event contains X or Z");
                end
            end
          else if ( ( (edge_type == `OVL_POSEDGE) && (!sampling_event_prev) )||
                   ( (edge_type == `OVL_NEGEDGE) && (sampling_event_prev) ) ||
                   ( (edge_type == `OVL_ANYEDGE) ) )
            begin
              ovl_error_t(`OVL_FIRE_XCHECK,"sampling_event contains X or Z");
            end
          else if ( valid_sampling_event_prev == 1'b1 )
            begin
              //Do nothing
            end
          else
            begin
              ovl_error_t(`OVL_FIRE_XCHECK,"sampling_event contains X or Z");
            end
        end
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

    end
    else begin
      r_reset_n <= 1'b0;
`ifdef OVL_INIT_REG
      sampling_event_prev <= 1'b0;
`endif
    end
  end

`endif // OVL_ASSERT_ON

