// Check that an inverted part select on an inner dimension in a continuous
// assign is reported as an error.

module test;

  reg [1:0][1:0] x;

  assign x[0:1] = 2'b00; // Error: Part select indices swapped

  initial begin
    $display("FAILED");
  end

endmodule
