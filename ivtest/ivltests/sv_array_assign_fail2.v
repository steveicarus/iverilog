// Check that trying to do a continuous assign of a scalar to an array results
// in an error.

module test;

  integer x[1:0];

  assign x = 10; // Error, scalar assigned to array

  initial begin
    $display("FAILED");
  end

endmodule
