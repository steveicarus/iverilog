// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

  // Guarded parameter for num_cks < 1 (which is bad usage - see warning in top-level file)
  parameter NUM_CKS_1 = (num_cks > 0) ? (num_cks - 1) : 0;


//------------------------------------------------------------------------------
// SHARED CODE
//------------------------------------------------------------------------------
`ifdef OVL_SHARED_CODE
  reg  [NUM_CKS_1:0] monitor;
  wire [NUM_CKS_1:0] monitor_1 = (monitor << 1);

  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      monitor <= {num_cks{1'b0}};
    end
    else begin
      monitor <= (monitor_1 | start_event);
    end
  end
`endif


//------------------------------------------------------------------------------
// ASSERTION
//------------------------------------------------------------------------------
`ifdef OVL_ASSERT_ON

  // 2-STATE
  // =======
  wire fire_2state_1, fire_2state_2, fire_2state_3;
  reg  fire_2state;
  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
      fire_2state <= 1'b0;
    end
    else begin
      if (fire_2state_1) begin
        ovl_error_t(`OVL_FIRE_2STATE,"Illegal overlapping condition of start event is detected");
      end
      if (fire_2state_2) begin
        ovl_error_t(`OVL_FIRE_2STATE,"Test expresson is asserted without a corresponding start_event");
      end
      if (fire_2state_3) begin
        ovl_error_t(`OVL_FIRE_2STATE,"Test expression is not asserted after elapse of num_cks cycles from start event");
      end
      if (fire_2state_1 || fire_2state_2 || fire_2state_3) begin
        fire_2state <= ovl_fire_2state_f(property_type);
      end
      else begin
        fire_2state <= 1'b0;
      end
    end
  end

  assign fire_2state_1 = ((check_overlapping   == 0) && (monitor_1 != {num_cks{1'b0}}) && start_event); // new start_event can occur in cycle test_expr is checked
  assign fire_2state_2 = ((check_missing_start != 0) && ~monitor[NUM_CKS_1] && test_expr);
  assign fire_2state_3 = (monitor[NUM_CKS_1] && ~test_expr);


  // X-CHECK
  // =======
  `ifdef OVL_XCHECK_OFF
    wire fire_xcheck = 1'b0;
  `else
    `ifdef OVL_IMPLICIT_XCHECK_OFF
      wire fire_xcheck = 1'b0;
    `else
      reg fire_xcheck_1, fire_xcheck_2;
      reg fire_xcheck;
      always @(posedge clk) begin
        if (`OVL_RESET_SIGNAL == 1'b0) begin
          // OVL does not fire during reset
          fire_xcheck <= 1'b0;
        end
        else begin
          if (fire_xcheck_1) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"start_event contains X or Z");
          end
          if (fire_xcheck_2) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
          end
          if (fire_xcheck_1 || fire_xcheck_2) begin
            fire_xcheck <= ovl_fire_xcheck_f(property_type);
          end
          else begin
            fire_xcheck <= 1'b0;
          end
        end
      end

      wire valid_start_event = ((start_event ^ start_event) == 1'b0);
      wire valid_test_expr   = ((test_expr   ^ test_expr)   == 1'b0);

      always @ (valid_start_event) begin
        if (valid_start_event) begin
          fire_xcheck_1 = 1'b0;
        end
        else begin
          fire_xcheck_1 = 1'b1;
        end
      end

      always @ (valid_test_expr or monitor) begin
        if (valid_test_expr || ~((check_missing_start==1) || monitor[NUM_CKS_1])) begin
          fire_xcheck_2 = 1'b0;
        end
        else begin
          fire_xcheck_2 = 1'b1; // test_expr X when check_missing_start or monitor[num_cks-1]
        end
      end

    `endif // OVL_IMPLICIT_XCHECK_OFF
  `endif // OVL_XCHECK_OFF

`else
  wire fire_2state = 1'b0;
  wire fire_xcheck = 1'b0;
`endif // OVL_ASSERT_ON


//------------------------------------------------------------------------------
// COVERAGE
//------------------------------------------------------------------------------
`ifdef OVL_COVER_ON

  wire fire_cover_1, fire_cover_2;
  reg  fire_cover;
  always @ (posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
      fire_cover <= 1'b0;
    end
    else begin
      if (fire_cover_1) begin
        ovl_cover_t("start_event covered"); // basic
      end
      if (fire_cover_2) begin
        ovl_cover_t("overlapping_start_events covered"); // corner
      end
      if (fire_cover_1 || fire_cover_2) begin
        fire_cover <= 1'b1;
      end
      else begin
        fire_cover <= 1'b0;
      end
    end
  end

  assign fire_cover_1 = ((OVL_COVER_BASIC_ON  > 0) && (start_event == 1'b1));
  assign fire_cover_2 = ((OVL_COVER_CORNER_ON > 0) && ((check_overlapping==1) && (monitor_1 != {num_cks{1'b0}}) && start_event));

`else
  wire fire_cover = 1'b0;
`endif // OVL_COVER_ON
