// Check that a non-existent index into a parameter results in an elaboration error.

module test;

  parameter [31:0] P = 'h0;

  wire x;

  assign x = P[does_not_exist]; // Error: Invalid index

  initial begin
    $display("FAILED");
  end

endmodule
