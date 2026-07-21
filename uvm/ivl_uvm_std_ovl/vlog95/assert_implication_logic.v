// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.


//------------------------------------------------------------------------------
// SHARED CODE
//------------------------------------------------------------------------------
// No shared code for this OVL


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
        ovl_error_t(`OVL_FIRE_2STATE,"Antecedent does not have consequent");
      end
    end
  end

  assign fire_2state_1 = ((antecedent_expr == 1'b1) && (consequent_expr == 1'b0));

  // X-CHECK
  // =======
  `ifdef OVL_XCHECK_OFF
  `else
    `ifdef OVL_IMPLICIT_XCHECK_OFF
    `else
      reg fire_xcheck_1, fire_xcheck_2;
      always @(posedge clk) begin
        if (`OVL_RESET_SIGNAL == 1'b0) begin
          // OVL does not fire during reset
        end
        else begin
          if (fire_xcheck_1) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"antecedent_expr contains X or Z");
          end
          if (fire_xcheck_2) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"consequent_expr contains X or Z");
          end
        end
      end

      wire valid_antecedent_expr = ((antecedent_expr ^ antecedent_expr) == 1'b0);
      wire valid_consequent_expr = ((consequent_expr ^ consequent_expr) == 1'b0);

      always @ (valid_antecedent_expr or consequent_expr) begin
        if (valid_antecedent_expr || consequent_expr) begin
          fire_xcheck_1 = 1'b0;
        end
        else begin
          fire_xcheck_1 = 1'b1; // antecedent X when consequent is 0
        end
      end

      always @ (antecedent_expr or valid_consequent_expr) begin
        if ((antecedent_expr == 1'b0) || valid_consequent_expr) begin
          fire_xcheck_2 = 1'b0;
        end
        else begin
          fire_xcheck_2 = 1'b1; // consequent X when antecedent is 1
        end
      end

    `endif // OVL_IMPLICIT_XCHECK_OFF
  `endif // OVL_XCHECK_OFF

`endif // OVL_ASSERT_ON


//------------------------------------------------------------------------------
// COVERAGE
//------------------------------------------------------------------------------
`ifdef OVL_COVER_ON

  wire fire_cover_1;
  always @ (posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
    end
    else begin
      if (fire_cover_1) begin
        ovl_cover_t("antecedent covered"); // basic
      end
    end
  end

  assign fire_cover_1 = ((OVL_COVER_BASIC_ON > 0) && (antecedent_expr == 1'b1));

`endif // OVL_COVER_ON
