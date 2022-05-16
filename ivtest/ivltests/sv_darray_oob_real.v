// Check that out-of-bounds access on a real typed dynamic array works and
// returns the correct value.

module test;

  real d[];
  real x;

  initial begin
    x = d[1];
    if (x == 0.0) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected 0.0, got %f", x);
    end
  end

endmodule
