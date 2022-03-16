// Check that it is possible to declare the data type for a time type module
// port separately from the direction for non-ANSI style port declarations.
// declarations.

module test(x);
  output x;
  time x;

  initial begin
    if ($bits(x) == 64) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
