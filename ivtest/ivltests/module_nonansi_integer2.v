// Check that it is possible to declare the data type for an integer type module
// port before the direction for non-ANSI style port declarations.

module test(x);
  integer x;
  output x;

  initial begin
    if ($bits(x) == $bits(integer)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
