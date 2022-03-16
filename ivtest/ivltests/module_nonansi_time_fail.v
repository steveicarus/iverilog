// Check that it is an error to declare a non-ANSI module port with implicit
// packed dimensions if it is later redeclared as a time typed variable. Even if
// the size of the packed dimensions matches that of the size of the time type.

module test(x);
  output [63:0] x;
  time x;

  initial begin
    $display("FAILED");
  end

endmodule
