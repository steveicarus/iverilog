// Check that it is an error to declare a non-ANSI module port with implicit
// packed dimensions if it is later redeclared as a vector typed variable and
// the vector type is a scalar.

module test(x);
  output [7:0] x;
  reg x;

  initial begin
    $display("FAILED");
  end

endmodule
