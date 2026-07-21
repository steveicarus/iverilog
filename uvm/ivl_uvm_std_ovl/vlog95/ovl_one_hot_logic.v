// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.


//------------------------------------------------------------------------------
// SHARED CODE
//------------------------------------------------------------------------------
`ifdef OVL_SHARED_CODE
  wire [width-1:0]   dec_test_expr =   test_expr -                {{width-1{1'b0}},1'b1};
  wire               zoh_test_expr = ((test_expr & dec_test_expr) == {width{1'b0}});
  wire             valid_test_expr = ((test_expr ^     test_expr) == {width{1'b0}});
`endif


//------------------------------------------------------------------------------
// ASSERTION
//------------------------------------------------------------------------------
`ifdef OVL_ASSERT_ON

  // 2-STATE
  // =======
  wire fire_2state_1;
  reg  fire_2state;
  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
      fire_2state <= 1'b0;
    end
    else begin
      if (fire_2state_1) begin
        ovl_error_t(`OVL_FIRE_2STATE,"Test expression contains more or less than 1 asserted bits");
        fire_2state <= ovl_fire_2state_f(property_type);
      end
      else begin
        fire_2state <= 1'b0;
      end
    end
  end

  assign fire_2state_1 = !zoh_test_expr || (test_expr == {width{1'b0}});

  // X-CHECK
  // =======
  `ifdef OVL_XCHECK_OFF
    wire fire_xcheck = 1'b0;
  `else
    `ifdef OVL_IMPLICIT_XCHECK_OFF
      wire fire_xcheck = 1'b0;
    `else
      reg fire_xcheck_1;
      reg fire_xcheck;
      always @(posedge clk) begin
        if (`OVL_RESET_SIGNAL == 1'b0) begin
          // OVL does not fire during reset
          fire_xcheck <= 1'b0;
        end
        else begin
          if (fire_xcheck_1) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
            fire_xcheck <= ovl_fire_xcheck_f(property_type);
          end
          else begin
            fire_xcheck <= 1'b0;
          end
        end
      end

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

`else
  wire fire_2state = 1'b0;
  wire fire_xcheck = 1'b0;
`endif // OVL_ASSERT_ON


//------------------------------------------------------------------------------
// COVERAGE
//------------------------------------------------------------------------------
`ifdef OVL_COVER_ON

  // Auxiliary logic
  reg [width-1:0]      one_hots_checked;
  reg [width-1:0] prev_one_hots_checked;
  reg [width-1:0] prev_test_expr;
  always @ (posedge clk) begin
    prev_test_expr <= test_expr; // deliberately not reset
    if (`OVL_RESET_SIGNAL == 1'b0) begin
           one_hots_checked <= {width{1'b0}};
      prev_one_hots_checked <= {width{1'b0}};
    end
    else begin
      if (valid_test_expr && zoh_test_expr) begin
        one_hots_checked <= one_hots_checked | test_expr;
      end
      prev_one_hots_checked <= one_hots_checked;
    end
  end
 
  wire fire_cover_1, fire_cover_2;
  reg  fire_cover;
  always @ (posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
      fire_cover <= 1'b0;
    end
    else begin
      if (fire_cover_1) begin
        ovl_cover_t("test_expr_change covered"); // sanity
      end
      if (fire_cover_2) begin
        ovl_cover_t("all_one_hots_checked covered"); // corner
      end
      if (fire_cover_1 || fire_cover_2) begin
        fire_cover <= 1'b1;
      end
      else begin
        fire_cover <= 1'b0;
      end
    end
  end

  assign fire_cover_1 = ((OVL_COVER_SANITY_ON > 0) && (test_expr != prev_test_expr));
  assign fire_cover_2 = ((OVL_COVER_CORNER_ON > 0) && (one_hots_checked == {width{1'b1}}) && (one_hots_checked != prev_one_hots_checked));

`else
  wire fire_cover = 1'b0;
`endif // OVL_COVER_ON
