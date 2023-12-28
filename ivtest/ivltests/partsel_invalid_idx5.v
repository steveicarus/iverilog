// Check that a non-existent index into a parameter results in an elaboration error.

module test;

  parameter [31:0] P = 'h0;

  wire [1:0] x;

  assign x = P[does_not_exist+:2]; // Error: Invalid base index

  initial begin
    $display("FAILED");
  end

endmodule
