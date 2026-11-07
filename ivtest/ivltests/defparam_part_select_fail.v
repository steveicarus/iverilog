// Check that defparam rejects a part select on the parameter name.

module M;
  parameter [3:0] P = 4'b1100;
endmodule

module test;

  M i_m();

  defparam i_m.P[1:0] = 1; // Error: A defparam must override the whole parameter.

  initial $display("FAILED");

endmodule
