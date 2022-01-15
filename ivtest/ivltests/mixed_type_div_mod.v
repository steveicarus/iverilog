`ifdef __ICARUS__
  `define SUPPORT_REAL_MODULUS_IN_IVTEST
`endif

module top;
  reg pass;
  real result;
  initial begin
    pass = 1'b1;

    // This should turn into a just a load of 0.5.
    result = 1/2.0;
    if (result != 0.5) begin
      $display("Failed: int/real, expected 0.5, got %g", result);
      pass = 1'b0;
    end

    // This should turn into a just a load of 0.5.
    result = 1.0/2;
    if (result != 0.5) begin
      $display("Failed: real/int, expected 0.5, got %g", result);
      pass = 1'b0;
    end

`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
    // This should turn into a just a load of 1.0.
    result = 1%2.0;
    if (result != 1.0) begin
      $display("Failed: int%%real, expected 1.0, got %g", result);
      pass = 1'b0;
    end

    // This should turn into a just a load of 1.0.
    result = 1.0%2;
    if (result != 1.0) begin
      $display("Failed: real%%int, expected 1.0, got %g", result);
      pass = 1'b0;
    end
`endif

    if (pass) $display("PASSED");
  end
endmodule
