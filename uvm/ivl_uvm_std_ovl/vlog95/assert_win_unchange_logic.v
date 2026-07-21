// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

  // Parameters used for fsm states
  parameter WIN_UNCHANGE_START = 1'b0;
  parameter WIN_UNCHANGE_CHECK = 1'b1;


//------------------------------------------------------------------------------
// SHARED CODE
//------------------------------------------------------------------------------
`ifdef OVL_SHARED_CODE
  reg [width-1:0] r_test_expr;
  reg             r_state;

  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      r_state <= WIN_UNCHANGE_START;
      // r_test_expr deliberately not reset
    end
    else begin
      r_test_expr <= test_expr;
      case (r_state)
        WIN_UNCHANGE_START: if (start_event == 1'b1) begin
          r_state <= WIN_UNCHANGE_CHECK;
        end
        WIN_UNCHANGE_CHECK: if (end_event == 1'b1) begin
          r_state <= WIN_UNCHANGE_START;
        end
      endcase
    end
  end
`endif


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
        ovl_error_t(`OVL_FIRE_2STATE,"Test expression has changed value before the event window closes");
      end
    end
  end

  assign fire_2state_1 = ((r_state == WIN_UNCHANGE_CHECK) && (r_test_expr != test_expr));


  // X-CHECK
  // =======
  `ifdef OVL_XCHECK_OFF
  `else
    `ifdef OVL_IMPLICIT_XCHECK_OFF
    `else
      reg fire_xcheck_1, fire_xcheck_2, fire_xcheck_3;
      always @(posedge clk) begin
        if (`OVL_RESET_SIGNAL == 1'b0) begin
          // OVL does not fire during reset
        end
        else begin
          if (fire_xcheck_1) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"start_event contains X or Z");
          end
          if (fire_xcheck_2) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
          end
          if (fire_xcheck_3) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"end_event contains X or Z");
          end
        end
      end

      wire valid_start_event = ((start_event ^ start_event) == 1'b0);
      wire valid_test_expr   = ((test_expr   ^ test_expr)   == {width{1'b0}});
      wire valid_end_event   = ((end_event   ^ end_event)   == 1'b0);

      always @ (valid_start_event or r_state) begin
        if (valid_start_event || (r_state != WIN_UNCHANGE_START)) begin
          fire_xcheck_1 = 1'b0;
        end
        else begin
          fire_xcheck_1 = 1'b1; // start_event X when r_state is WIN_UNCHANGE_START
        end
      end

      always @ (valid_test_expr or r_state or start_event) begin
        if (valid_test_expr || !((r_state == WIN_UNCHANGE_CHECK) || start_event)) begin
          fire_xcheck_2 = 1'b0;
        end
        else begin
          fire_xcheck_2 = 1'b1; // test_expr X when r_state is CHECK or start_event high
        end
      end

      always @ (valid_end_event or r_state) begin
        if (valid_end_event || (r_state != WIN_UNCHANGE_CHECK)) begin
          fire_xcheck_3 = 1'b0;
        end
        else begin
          fire_xcheck_3 = 1'b1; // end_event X when r_state is WIN_UNCHANGE_CHECK
        end
      end

    `endif // OVL_IMPLICIT_XCHECK_OFF
  `endif // OVL_XCHECK_OFF

`endif // OVL_ASSERT_ON


//------------------------------------------------------------------------------
// COVERAGE
//------------------------------------------------------------------------------
`ifdef OVL_COVER_ON

  wire fire_cover_1, fire_cover_2;
  always @ (posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
    end
    else begin
      if (fire_cover_1) begin
        ovl_cover_t("window_open covered"); // basic
      end
      if (fire_cover_2) begin
        ovl_cover_t("window covered"); // basic
      end
    end
  end

  assign fire_cover_1 = ((OVL_COVER_BASIC_ON > 0) && (start_event == 1'b1) && (r_state == WIN_UNCHANGE_START));
  assign fire_cover_2 = ((OVL_COVER_BASIC_ON > 0) && (end_event   == 1'b1) && (r_state == WIN_UNCHANGE_CHECK));

`endif // OVL_COVER_ON
