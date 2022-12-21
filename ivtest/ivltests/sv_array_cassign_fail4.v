// Check that it is an error if the element type is not the same in a
// continuous array assignment, even if the difference is just 2-state vs.
// 4-state.

module test;

  wire [1:0] x[1:0];
  bit [1:0] y[1:0];

  assign x = y;

  initial begin
    $display("FAILED");
  end

endmodule
