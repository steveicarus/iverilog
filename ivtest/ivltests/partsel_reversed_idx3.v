// Check that an inverted part select in a procedural assign is reported as an
// error.

module test;

  reg [1:0] x;

  initial begin
    x[0:1] = 2'b00; // Error: Part select indices swapped

    $display("FAILED");
  end

endmodule
