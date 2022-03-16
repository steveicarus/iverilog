// Check that it is possible to declare the data type for an integer type module
// port separately from the direction for non-ANSI style port declarations.
// declarations.

module test(x);
  output x;
  integer x;

  initial begin
    if ($bits(x) == $bits(integer)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
