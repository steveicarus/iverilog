// Check that it is possible to declare the data type for a vector type module
// port before the direction for non-ANSI style port declarations.

module test(x);
  reg [7:0] x;
  output [7:0] x;

  initial begin
    if ($bits(x) == 8) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
