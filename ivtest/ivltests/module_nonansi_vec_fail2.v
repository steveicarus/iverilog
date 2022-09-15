// Check that it is an error to declare a non-ANSI module port without implicit
// packed dimensions if it is later redeclared as a vector typed variable and
// the vector type is not a scalar.

module test(x);
  output x;
  reg [7:0] x;

  initial begin
    $display("FAILED");
  end

endmodule
