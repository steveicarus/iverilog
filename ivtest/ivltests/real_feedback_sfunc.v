// Check that a real system function does not repeatedly schedule a stable loop.

module test;

  real value;

  assign value = $pow(value, 1.0);

  initial begin
    #1;
    if (value == 0.0) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected zero, got %f", value);
    end
  end

endmodule
