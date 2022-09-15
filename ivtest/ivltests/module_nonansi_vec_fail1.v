// Check that it is an error to declare a non-ANSI module port with implicit
// packed dimensions if it is later redeclared as a vector typed variable and
// the size of the packed dimensions do not match.

module test(x);
  output [3:0] x;
  reg [7:0] x;

  initial begin
    $display("FAILED");
  end

endmodule
