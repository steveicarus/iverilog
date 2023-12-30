// Check that an inverted part select in an expression is reported as an error.

module test;

  reg [1:0] x;
  reg [1:0] y;

  initial begin
    y = x[0:1]; // Error: Part select indices swapped

    $display("FAILED");
  end

endmodule
