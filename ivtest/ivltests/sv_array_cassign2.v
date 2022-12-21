// Check that continuous assignment of two compatible arrays is supported, even
// if the upper and lower bounds of the arrays are not identical, as long as the
// size is the same.

module test;

  wire [1:0] x[1:0];
  wire [1:0] y[2:1];

  assign x = y;

  initial begin
    $display("PASSED");
  end

endmodule
