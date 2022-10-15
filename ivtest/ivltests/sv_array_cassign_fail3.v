// Check that it is an error if the number of unpacked dimensions do not match
// in an continuous array assignment, even if the canonical size of the array is
// the same.

module test;

  wire [1:0] x[1:0][1:0];
  reg [1:0] y[3:0];

  assign x = y;

  initial begin
    $display("FAILED");
  end

endmodule
