// Check that it is an error if the array size is not the same in a continuous
// unpacked array assignment.

module test;

  wire [1:0] x[2:0];
  reg [1:0] y[1:0];

  assign x = y;

  initial begin
    $display("FAILED");
  end

endmodule
