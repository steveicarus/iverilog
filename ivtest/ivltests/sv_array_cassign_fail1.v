// Check that it is an error if the element type is not the same in a
// continuous array assignment.

module test;

  wire [3:0] x[1:0];
  reg [1:0] y[1:0];

  assign x = y;

  initial begin
    $display("FAILED");
  end

endmodule
