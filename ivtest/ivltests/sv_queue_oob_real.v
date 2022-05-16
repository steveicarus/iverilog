// Check that out-of-bounds access on a real typed queue works and returns the
// correct value.

module test;

  real q[$];
  real x;

  initial begin
    x = q[1];
    if (x == 0.0) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected 0.0, got %f", x);
    end
  end

endmodule
