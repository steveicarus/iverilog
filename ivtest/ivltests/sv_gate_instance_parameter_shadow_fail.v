// Check that a gate instance hides an outer parameter.

parameter i_g = 23;

module test;
  wire out;
  and i_g(out, 1'b1, 1'b1);

  initial $display("%0d", i_g); // Error: i_g is a gate instance, not a value.
endmodule
