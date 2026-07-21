// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.



`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_sample_event;
  wire valid_state_expr;
  wire valid_check_value;

  assign valid_sample_event = ~(sample_event^sample_event);
  assign valid_state_expr = ~((^state_expr)^(^state_expr));
  assign valid_check_value = ~((^check_value)^(^check_value));

 `ifdef OVL_END_OF_SIMULATION
  wire valid_EOS;
  assign valid_EOS = ~(`OVL_END_OF_SIMULATION ^ `OVL_END_OF_SIMULATION);
 `endif // OVL_END_OF_SIMULATION
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  reg valid_r_sample_event;
 `ifdef OVL_END_OF_SIMULATION
  reg valid_r_EOS;
 `endif // OVL_END_OF_SIMULATION
  always @(posedge clk or negedge `OVL_RESET_SIGNAL)
    begin
      if (`OVL_RESET_SIGNAL != 1'b1)
        begin
          valid_r_sample_event <= 1'b1;
 `ifdef OVL_END_OF_SIMULATION
          valid_r_EOS <= 1'b1;
 `endif // OVL_END_OF_SIMULATION
        end
      else
        begin
          valid_r_sample_event <= valid_sample_event;
 `ifdef OVL_END_OF_SIMULATION
          valid_r_EOS <= valid_EOS;
 `endif // OVL_END_OF_SIMULATION
        end
    end
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_ASSERT_ON

  reg r_sample_event;

  always @(posedge clk) begin
     if (`OVL_RESET_SIGNAL != 1'b0) begin
        r_sample_event <= sample_event;
     end
     else begin
`ifdef OVL_INIT_REG
        r_sample_event <= 0;
`endif
     end
  end

 `ifdef OVL_END_OF_SIMULATION
    reg r_EOS;
    always @(posedge clk) r_EOS <= `OVL_END_OF_SIMULATION;
 `endif // OVL_END_OF_SIMULATION

  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL != 1'b0) begin
 `ifdef OVL_END_OF_SIMULATION
        if (((r_EOS == 1'b0 && `OVL_END_OF_SIMULATION ==1'b1) ||
             (r_sample_event == 1'b0 && sample_event == 1'b1)) &&
		(state_expr  != check_value)) begin
 `else
        if ((r_sample_event == 1'b0 && sample_event == 1'b1) &&
		(state_expr  != check_value)) begin
 `endif // OVL_END_OF_SIMULATION
          ovl_error_t(`OVL_FIRE_2STATE,"State expression is not equal to check_value while sample event is asserted");
        end


`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  `ifdef OVL_END_OF_SIMULATION
        if (((r_EOS == 1'b0 && `OVL_END_OF_SIMULATION ==1'b1) ||
             (r_sample_event == 1'b0 && sample_event == 1'b1)))
  `else
        if ((r_sample_event == 1'b0 && sample_event == 1'b1))
  `endif // OVL_END_OF_SIMULATION
          begin
            if (valid_state_expr == 1'b1)
              begin
                //do nothing
              end
            else
              begin
                ovl_error_t(`OVL_FIRE_XCHECK,"state_expr contains X or Z");
              end
          end

  `ifdef OVL_END_OF_SIMULATION
        if (((r_EOS == 1'b0 && `OVL_END_OF_SIMULATION ==1'b1) ||
             (r_sample_event == 1'b0 && sample_event == 1'b1)))
  `else
        if ((r_sample_event == 1'b0 && sample_event == 1'b1))
  `endif // OVL_END_OF_SIMULATION
          begin
            if (valid_check_value == 1'b1)
              begin
                //do nothing
              end
            else
              begin
                ovl_error_t(`OVL_FIRE_XCHECK,"check_value contains X or Z");
              end
          end

        if (valid_r_sample_event == 1'b1)
          begin
            if (r_sample_event == 1'b0)
              begin
                if (valid_sample_event == 1'b1)
                  begin
                    // Do Nothing
                  end
                else
                  begin
                    ovl_error_t(`OVL_FIRE_XCHECK,"sample_event contains X or Z");
                  end
              end
          end
        else if (valid_sample_event == 1'b1)
          begin
            if (sample_event == 1'b1)
              begin
                ovl_error_t(`OVL_FIRE_XCHECK,"sample_event contains X or Z");
              end
          end
        else
          begin
            ovl_error_t(`OVL_FIRE_XCHECK,"sample_event contains X or Z");
          end

  `ifdef OVL_END_OF_SIMULATION
        if (valid_r_EOS == 1'b1)
          begin
            if (r_EOS == 1'b0)
              begin
                if (valid_EOS == 1'b1)
                  begin
                    // Do Nothing
                  end
                else
                  begin
                    ovl_error_t(`OVL_FIRE_XCHECK,"`OVL_END_OF_SIMULATION contains X or Z");
                  end
              end
          end
        else if (valid_EOS == 1'b1)
          begin
            if (`OVL_END_OF_SIMULATION == 1'b1)
              begin
                ovl_error_t(`OVL_FIRE_XCHECK,"`OVL_END_OF_SIMULATION contains X or Z");
              end
          end
        else
          begin
            ovl_error_t(`OVL_FIRE_XCHECK,"`OVL_END_OF_SIMULATION contains X or Z");
          end
  `endif // OVL_END_OF_SIMULATION
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF


    end
  end

`endif // OVL_ASSERT_ON

