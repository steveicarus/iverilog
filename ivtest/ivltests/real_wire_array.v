module top;
  reg passed = 1'b1;
  real rval = 1.0;
  wire real rvar [1:0];

  assign rvar[0] = -1.0;
  assign rvar[1] = 2.0*rval;

  initial begin
    #1;
    if (rvar[0] != -1.0) begin
      $display("Failed: real wire array[0], expected -1.0, got %g", rvar[0]);
      passed = 1'b0;
    end

    if (rvar[1] != 2.0) begin
      $display("Failed: real wire array[1], expected 2.0, got %g", rvar[1]);
      passed = 1'b0;
    end

    rval = 2.0;
    #1;
    if (rvar[1] != 4.0) begin
      $display("Failed: real wire array[1], expected 4.0, got %g", rvar[1]);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
