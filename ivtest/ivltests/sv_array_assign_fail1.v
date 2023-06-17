// Check that trying to do a procedural assign of a scalar to an array results
// in an error.

module test;

  integer x[1:0];

  initial begin
    x = 10; // Error, scalar assigned to array
    $display("FAILED");
  end

endmodule
