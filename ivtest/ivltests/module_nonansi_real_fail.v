// Check that it is an error to declare a non-ANSI module port with implicit
// packed dimensions if it is later redeclared as a real typed variable.

module test(x);
  output [3:0] x;
  real x;

  initial begin
    $display("FAILED");
  end

endmodule
