// Check that it is an error if the element type is not the same in a
// continuous array assignment, even if one of the types is a packed type and
// the other is a real type.

module test;

  wire [1:0] x[1:0];
  real [1:0] y[1:0];

  assign x = y;

  initial begin
    $display("FAILED");
  end

endmodule
