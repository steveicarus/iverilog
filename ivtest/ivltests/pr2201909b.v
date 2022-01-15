module top;
  reg passed = 1'b1;
  realtime rvar [1:0];

  initial begin
    #1;
    rvar[0] = -1.0;
    if (rvar[0] != -1.0) begin
      $display("Failed: real time array[0], expected -1.0, got %g", rvar[0]);
      passed = 1'b0;
    end

    rvar[1] = 2.0;
    if (rvar[1] != 2.0) begin
      $display("Failed: real time array[1], expected 2.0, got %g", rvar[1]);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
