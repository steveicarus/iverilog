// Check that defparam rejects a bit select on the parameter name.

module test;

  parameter [3:0] P = 4'b1100;

  defparam P[0] = 1; // Error: A defparam must override the whole parameter.

  initial $display("FAILED");

endmodule
