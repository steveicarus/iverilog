// Check that it is possible to declare the data type for a real type module
// port before the direction for non-ANSI style port declarations.

module test(x);
  real x;
  output x;

  initial begin
    if (x == 0.0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
