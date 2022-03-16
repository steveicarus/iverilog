// Check that it is possible to declare the data type for a time type module
// port before the direction for non-ANSI style port declarations.

module test(x);
  time x;
  output x;

  initial begin
    if ($bits(x) == 64) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
