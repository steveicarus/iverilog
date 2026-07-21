// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

  parameter WIDTH_START = 2'b00;
  parameter WIDTH_CKMIN = 2'b01;
  parameter WIDTH_CKMAX = 2'b10;
  parameter WIDTH_IDLE  = 2'b11;

  reg r_test_expr;
  reg [1:0] r_state;
  integer num_cks;

`ifdef OVL_SYNTHESIS
`else
  initial begin
    r_state=WIDTH_START;
    num_cks = 0;
  end
`endif


`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_test_expr;
  assign valid_test_expr = ~(test_expr ^ test_expr);
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_ASSERT_ON

  always @(posedge clk) begin
    r_test_expr <= test_expr;
      if (`OVL_RESET_SIGNAL != 1'b0) begin
        case (r_state)
          WIDTH_START:
            if ((r_test_expr == 1'b0) && (test_expr == 1'b1)) begin
              num_cks <= 1;
              if      (min_cks > 0) r_state <= WIDTH_CKMIN;
              else if (max_cks > 0) r_state <= WIDTH_CKMAX;
            end
          WIDTH_CKMIN:
            if (test_expr == 1'b1) begin
              num_cks <= num_cks + 1;
              if (num_cks >= min_cks-1) begin
                if (max_cks > 0) r_state <= WIDTH_CKMAX;
                else             r_state <= WIDTH_IDLE;
              end
            end
            else begin
              if (num_cks < min_cks) begin
                ovl_error_t(`OVL_FIRE_2STATE,"Test expression was held TRUE for less than specified minimum min_cks cycles");
              end
              r_state <= WIDTH_START;
            end
          WIDTH_CKMAX:
            if (test_expr == 1'b1) begin
              num_cks <= num_cks + 1;
              if (num_cks >= max_cks) begin
                ovl_error_t(`OVL_FIRE_2STATE,"Test expression was held TRUE for more than specified maximum max_cks cycles");
                r_state <= WIDTH_IDLE;
              end
            end
            else begin
              if (num_cks > max_cks) begin
                ovl_error_t(`OVL_FIRE_2STATE,"Test expression was held TRUE for more than specified maximum max_cks cycles");
              end
              r_state <= WIDTH_START;
            end
          WIDTH_IDLE:
            if (test_expr == 1'b0) begin
              r_state <= WIDTH_START;
            end
        endcase
      end
      else begin
         r_state <= WIDTH_START;
         num_cks <= 0;
      end
  end // always

`endif // OVL_ASSERT_ON

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
 `ifdef OVL_ASSERT_ON
  always @(posedge clk)
    begin
      if (`OVL_RESET_SIGNAL != 1'b0)
        begin
          if (valid_test_expr == 1'b1)
            begin
              // Do Nothing
            end
          else
            ovl_error_t(`OVL_FIRE_XCHECK,"test_expr contains X or Z");
        end
    end

 `endif // OVL_ASSERT_ON
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_COVER_ON

  reg r_test_expr_cover;
  reg timer_started;
  integer num_cks_cover;

`ifdef OVL_SYNTHESIS
`else
  initial begin
    num_cks_cover = 0;
  end
`endif

  always @(posedge clk) begin
    r_test_expr_cover <= test_expr;
    if (`OVL_RESET_SIGNAL != 1'b0 && coverage_level != `OVL_COVER_NONE) begin
      if ((r_test_expr_cover == 1'b0) && (test_expr == 1'b1)) begin
       if (OVL_COVER_BASIC_ON) begin //basic coverage
        num_cks_cover <= 1;
        timer_started <= 1;
        ovl_cover_t("test_expr_asserts covered");
       end //basic coverage
      end
      else if (timer_started && test_expr == 1'b1)
        num_cks_cover <= num_cks_cover + 1;
      else if (timer_started && (r_test_expr_cover == 1'b1) && (test_expr == 1'b0)) begin
        num_cks_cover <= 0;
        timer_started <= 0;

        if (OVL_COVER_CORNER_ON) begin //corner coverage

         if (min_cks > 0 && num_cks_cover == min_cks) begin
          ovl_cover_t("test_expr_asserted_for_min_cks covered");
         end

         if (max_cks > 0 && num_cks_cover == max_cks) begin
          ovl_cover_t("test_expr_asserted_for_max_cks covered");
         end

        end //corner coverage
      end
    end // OVL_COVER_NONE
    else begin // reset condition
      num_cks_cover <= 0;
      timer_started <= 0;
    end // OVL_COVER_NONE
  end // always

`endif // OVL_COVER_ON
