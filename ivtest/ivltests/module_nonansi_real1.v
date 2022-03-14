// Check that it is possible to declare the data type for a real type module
// port separately from the direction for non-ANSI style port declarations.
// declarations.

module test(x);
  output x;
  real x;

  initial begin
    if (x == 0.0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
