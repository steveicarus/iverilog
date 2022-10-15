// Check that continuous assignment of two compatible arrays is supported, even
// if the element types have different number of dimensions, but have the same
// packed width.

module test;

  wire [3:0] x[1:0];
  wire [1:0][1:0] y[1:0];

  assign x = y;

  initial begin
    $display("PASSED");
  end

endmodule
