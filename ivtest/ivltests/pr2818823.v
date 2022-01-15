module top;
  parameter C1 = 1.0e-6;

  reg pass;
  real rval;
  real exp_result;

  initial begin
    pass = 1'b1;
    exp_result = -1000000.0;

    // Check with a constant and a parameter.
    rval = -1 / C1;
    if (rval != exp_result) begin
      $display ("FAILED: -1/%f gave %f, expected %f", C1, rval, exp_result);
      pass = 1'b0;
    end

    // Check with both constants.
    rval = -1 / 1.0e-6;
    if (rval != exp_result) begin
      $display ("FAILED: -1/1.0e-6 gave %f, expected %f", rval, exp_result);
      pass = 1'b0;
    end

    // Check with a positive value.
    exp_result = 1000000.0;
    rval = 1 / C1;
    if (rval != exp_result) begin
      $display ("FAILED: 1/%f gave %f, not expected %f", C1, rval, exp_result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
